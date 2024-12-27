// Copyright 2024 Ant Group Co., Ltd.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "trustflow/proxy/data_capsule_proxy/data_capsule_proxy.h"

#include <filesystem>

#include "cppcodec/base64_rfc4648.hpp"
#include "src/butil/logging.h"
#include "src/google/protobuf/util/json_util.h"
#include "yacl/base/exception.h"
#include "yacl/crypto/hash/hash_utils.h"
#include "yacl/crypto/rand/rand.h"

#include "trustflow/proxy/data_capsule_proxy/capsule_manager_client.h"
#include "trustflow/proxy/data_capsule_proxy/oss_client.h"
#include "trustflow/proxy/utils/crypto_util.h"

#include "secretflowapis/v2/sdc/ual.pb.h"

namespace trustflow {
namespace proxy {
namespace data_capsule_proxy {

namespace {

namespace capsule_manager = ::secretflowapis::v2::sdc::capsule_manager;

constexpr char kTempDir[] = "/tmp";
constexpr char kResponseContentType[] = "application/json";
constexpr int kKeyBytes = 16;

std::filesystem::path GenTempDir(const std::string& path) {
  const auto path_hash = yacl::crypto::Sha256(path);
  return std::filesystem::path(kTempDir) /
         cppcodec::base64_rfc4648::encode(path_hash);
}

capsule_manager::ResourceRequest GenResourceRequest(
    const secretflowapis::v2::sdc::data_capsule_proxy::CmResourceConfig&
        cm_resource_config,
    const std::string& cert) {
  capsule_manager::ResourceRequest resource_request;
  resource_request.set_initiator_party_id(
      trustflow::proxy::utils::GeneratePartyId(cert));
  resource_request.set_scope(cm_resource_config.scope());
  resource_request.set_op_name(cm_resource_config.op_name());

  capsule_manager::ResourceRequest::Resource resource;
  resource.set_resource_uri(cm_resource_config.resource_uri());
  resource.mutable_columns()->CopyFrom(cm_resource_config.columns());

  *(resource_request.add_resources()) = std::move(resource);
  resource_request.set_global_attrs(cm_resource_config.global_attrs());

  return resource_request;
}
}  // namespace

void DataCapsuleProxyImpl::GetInputData(
    ::google::protobuf::RpcController* cntl_base,
    const ::secretflowapis::v2::sdc::data_capsule_proxy::GetInputDataRequest*
        request,
    ::secretflowapis::v2::sdc::data_capsule_proxy::GetInputDataResponse*
        response,
    ::google::protobuf::Closure* done) {
  brpc::ClosureGuard done_guard(done);
  brpc::Controller* cntl = static_cast<brpc::Controller*>(cntl_base);
  try {
    cntl->http_response().set_content_type(kResponseContentType);

    const std::string& dest_path = request->dest_config().path();

    std::vector<uint8_t> data_key;
    if (request->has_data_key_b64()) {
      data_key = cppcodec::base64_rfc4648::decode(request->data_key_b64());
      SPDLOG_INFO("Got data key from request");
    } else if (request->has_cm_resource_config()) {
      auto resource_request =
          GenResourceRequest(request->cm_resource_config(), cert_);

      SPDLOG_INFO("Try to get data key from Capsule Manager");
      const std::string& cm_endpoint =
          request->cm_resource_config().endpoint().empty()
              ? cm_endpoint_
              : request->cm_resource_config().endpoint();
      CapsuleManagerClient capsule_manager_client(cm_endpoint);
      const auto data_keys = capsule_manager_client.GetDataKeys(
          plat_, cert_, private_key_, resource_request);
      YACL_ENFORCE(!data_keys.empty(), "Data keys empty");
      data_key =
          cppcodec::base64_rfc4648::decode(data_keys.at(0).data_key_b64());
      SPDLOG_INFO("Got data key from capsule manager");
    } else {
      YACL_THROW("Data key config not found in request");
    }

    if (request->has_s3_config()) {
      std::filesystem::path download_temp_path = GenTempDir(dest_path);
      std::filesystem::remove_all(download_temp_path);

      const auto& s3_config = request->s3_config();
      DownloadFromOss(s3_config.endpoint(), s3_config.bucket(),
                      s3_config.path(), download_temp_path,
                      s3_config.access_key_id(), s3_config.access_key_secret(),
                      s3_config.sts_token());

      trustflow::proxy::utils::DecryptToDir(download_temp_path, dest_path,
                                            data_key);
      std::filesystem::remove_all(download_temp_path);
    } else if (request->has_local_fs_config()) {
      trustflow::proxy::utils::DecryptToDir(request->local_fs_config().path(),
                                            dest_path, data_key);
    } else {
      YACL_THROW("Source config not found");
    }
    secretflowapis::v2::Status status;
    status.set_code(secretflowapis::v2::Code::OK);
    status.set_message("success");
    *(response->mutable_status()) = std::move(status);
  } catch (const std::exception& e) {
    SPDLOG_ERROR(e.what());
    cntl->SetFailed(e.what());
  }
}

void DataCapsuleProxyImpl::PutResultData(
    ::google::protobuf::RpcController* cntl_base,
    const ::secretflowapis::v2::sdc::data_capsule_proxy::PutResultDataRequest*
        request,
    ::secretflowapis::v2::sdc::data_capsule_proxy::PutResultDataResponse*
        response,
    ::google::protobuf::Closure* done) {
  brpc::ClosureGuard done_guard(done);
  brpc::Controller* cntl = static_cast<brpc::Controller*>(cntl_base);

  try {
    cntl->http_response().set_content_type(kResponseContentType);

    const std::string& src_path = request->source_config().path();
    std::vector<uint8_t> data_key;
    if (request->data_key_b64().empty()) {
      data_key = yacl::crypto::RandBytes(kKeyBytes);
    } else {
      data_key = cppcodec::base64_rfc4648::decode(request->data_key_b64());
    }

    if (request->has_s3_config()) {
      const auto& s3_config = request->s3_config();
      const std::filesystem::path enc_temp_path = GenTempDir(src_path);

      trustflow::proxy::utils::EncryptToDir(src_path, enc_temp_path, data_key);
      UploadToOss(s3_config.endpoint(), s3_config.bucket(), enc_temp_path,
                  s3_config.path(), s3_config.access_key_id(),
                  s3_config.access_key_secret(), s3_config.sts_token());

      std::filesystem::remove_all(enc_temp_path);
    } else if (request->has_local_fs_config()) {
      trustflow::proxy::utils::EncryptToDir(
          src_path, request->local_fs_config().path(), data_key);
    } else {
      YACL_THROW("Dest config not found");
    }

    const auto& cm_result_config = request->cm_result_config();
    if (!cm_result_config.resource_uri().empty()) {
      const std::string& cm_endpoint = cm_result_config.endpoint().empty()
                                           ? cm_endpoint_
                                           : cm_result_config.endpoint();
      CapsuleManagerClient capsule_manager_client(cm_endpoint);

      YACL_ENFORCE(!cm_result_config.scope().empty(), "scope can not be empty");
      YACL_ENFORCE(!cm_result_config.resource_uri().empty(),
                   "resource_uri can not be empty");
      YACL_ENFORCE(cm_result_config.ancestor_uuids().size() > 0,
                   "ancestor_uuids can not be empty");
      capsule_manager::CreateResultDataKeyRequest::Body body;
      body.set_scope(cm_result_config.scope());
      body.set_resource_uri(cm_result_config.resource_uri());
      body.set_data_key_b64(cppcodec::base64_rfc4648::encode(data_key));
      body.mutable_ancestor_uuids()->CopyFrom(
          cm_result_config.ancestor_uuids());

      capsule_manager_client.CreateResultDataKey(plat_, cert_, private_key_,
                                                 body);
    }

    secretflowapis::v2::Status status;
    status.set_code(secretflowapis::v2::Code::OK);
    status.set_message("success");
    *(response->mutable_status()) = std::move(status);
  } catch (const std::exception& e) {
    SPDLOG_ERROR(e.what());
    cntl->SetFailed(e.what());
  }
}

}  // namespace data_capsule_proxy
}  // namespace proxy
}  // namespace trustflow