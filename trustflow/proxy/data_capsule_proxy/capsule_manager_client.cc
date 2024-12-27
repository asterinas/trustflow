// Copyright 2023 Ant Group Co., Ltd.
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

#include "trustflow/proxy/data_capsule_proxy/capsule_manager_client.h"

#include "cppcodec/base64_url_unpadded.hpp"
#include "include/grpcpp/grpcpp.h"
#include "spdlog/spdlog.h"
#include "yacl/crypto/hash/hash_utils.h"
#include "yacl/crypto/rand/rand.h"

#include "trustflow/proxy/utils/crypto_util.h"
#include "trustflow/proxy/utils/ra_util.h"

namespace trustflow {
namespace proxy {
namespace data_capsule_proxy {

namespace {
namespace capsule_manager = ::secretflowapis::v2::sdc::capsule_manager;

constexpr uint8_t kNonceBytes = 16;

constexpr uint32_t kGrpcMaxMsgSizeMb = 1024;
constexpr uint32_t kGrpcTimeoutMs = 5 * 1000;

constexpr char kTeePlatSim[] = "sim";

inline void VerifySfResponseStatus(const secretflowapis::v2::Status& status) {
  YACL_ENFORCE(status.code() == secretflowapis::v2::Code::OK,
               "Call service failed, error code: {}, message: {}",
               status.code(), status.message());
}

}  // namespace

CapsuleManagerClient::CapsuleManagerClient(
    const std::string& capsule_manager_endpoint) {
  ::grpc::ChannelArguments chan_args;
  chan_args.SetMaxReceiveMessageSize(kGrpcMaxMsgSizeMb * 1024 * 1024);
  chan_args.SetGrpclbFallbackTimeout(kGrpcTimeoutMs);

  capsule_manager_stub_ =
      capsule_manager::CapsuleManager::NewStub(::grpc::CreateCustomChannel(
          capsule_manager_endpoint, ::grpc::InsecureChannelCredentials(),
          chan_args));
  SPDLOG_INFO("Try to get Ra Cert from Capsule Manager");
  GetRaCert();
  SPDLOG_INFO("Got Ra Cert");
}

std::string CapsuleManagerClient::GetRaCert() {
  capsule_manager::GetRaCertRequest request;
  capsule_manager::GetRaCertResponse response;

  const auto nonce = yacl::crypto::RandBytes(kNonceBytes);
  request.set_nonce(cppcodec::base64_url_unpadded::encode(nonce));

  ::grpc::ClientContext context;
  const auto deadline = std::chrono::system_clock::now() +
                        std::chrono::milliseconds(kGrpcTimeoutMs);
  context.set_deadline(deadline);

  const auto status =
      capsule_manager_stub_->GetRaCert(&context, request, &response);
  YACL_ENFORCE(status.ok(),
               "Calling GetRaCert failed, error code: {}, message: {}",
               static_cast<int>(status.error_code()), status.error_message());

  VerifySfResponseStatus(response.status());

  capsule_manager_cert_ = response.cert();

  return response.cert();
}

std::vector<capsule_manager::DataKey> CapsuleManagerClient::GetDataKeys(
    const std::string& plat, const std::string& cert,
    const std::string& private_key,
    capsule_manager::ResourceRequest& resource_req) const {
  YACL_ENFORCE(capsule_manager_cert_.size() > 0,
               "capsule_manager_cert not found. you should call GetRaCert() "
               "before other capsule_manager's interface");

  // get encrypted data key from capsule manager
  capsule_manager::GetDataKeysRequest request;

  if (plat != kTeePlatSim) {
    SPDLOG_INFO("Serializing resource_req...");
    std::string serialized_resource_req;
    YACL_ENFORCE(resource_req.SerializeToString(&serialized_resource_req),
                 "Serializing resource_req failed");

    SPDLOG_INFO("Generating attestation report...");
    const std::string user_data = cert + "." + serialized_resource_req;
    auto report = trustflow::proxy::utils::GenRaReport(user_data);

    *request.mutable_attestation_report() = std::move(report);
  }

  request.set_cert(cert);
  *request.mutable_resource_request() = std::move(resource_req);

  const auto enc_req = trustflow::proxy::utils::GenEncryptedRequest<
      capsule_manager::GetDataKeysRequest>(request, private_key, cert,
                                           capsule_manager_cert_, true);
  capsule_manager::EncryptedResponse enc_res;

  ::grpc::ClientContext context;
  const auto deadline = std::chrono::system_clock::now() +
                        std::chrono::milliseconds(kGrpcTimeoutMs);
  context.set_deadline(deadline);

  const auto status =
      capsule_manager_stub_->GetDataKeys(&context, enc_req, &enc_res);
  YACL_ENFORCE(status.ok(),
               "Calling GetDataKeys failed, error code: {}, message: {}",
               static_cast<int>(status.error_code()), status.error_message());
  auto [res_status, response] = trustflow::proxy::utils::ParseEncryptedResponse<
      capsule_manager::GetDataKeysResponse>(enc_res, private_key);
  VerifySfResponseStatus(res_status);

  return std::vector<capsule_manager::DataKey>(response.data_keys().begin(),
                                               response.data_keys().end());
}

secretflowapis::v2::sdc::capsule_manager::TlsAsset
CapsuleManagerClient::GetTlsAsset(
    const std::string& plat, const std::string& cert,
    const std::string& private_key,
    secretflowapis::v2::sdc::capsule_manager::ResourceRequest& resource_req)
    const {
  YACL_ENFORCE(capsule_manager_cert_.size() > 0,
               "capsule_manager_cert not found. you should call GetRaCert() "
               "before other capsule_manager's interface");

  capsule_manager::GetTlsAssetRequest request;

  if (plat != kTeePlatSim) {
    SPDLOG_INFO("Serializing resource_req...");
    std::string serialized_resource_req;
    YACL_ENFORCE(resource_req.SerializeToString(&serialized_resource_req),
                 "Serializing resource_req failed");

    SPDLOG_INFO("Generating attestation report...");
    const std::string user_data = cert + "." + serialized_resource_req;
    auto report = trustflow::proxy::utils::GenRaReport(user_data);

    *request.mutable_attestation_report() = std::move(report);
  }

  request.set_cert(cert);
  *request.mutable_resource_request() = std::move(resource_req);

  const auto enc_req = trustflow::proxy::utils::GenEncryptedRequest<
      capsule_manager::GetTlsAssetRequest>(request, private_key, cert,
                                           capsule_manager_cert_, true);
  capsule_manager::EncryptedResponse enc_res;

  ::grpc::ClientContext context;
  const auto deadline = std::chrono::system_clock::now() +
                        std::chrono::milliseconds(kGrpcTimeoutMs);
  context.set_deadline(deadline);

  const auto status =
      capsule_manager_stub_->GetTlsAsset(&context, enc_req, &enc_res);
  YACL_ENFORCE(status.ok(),
               "Calling GetDataKeys failed, error code: {}, message: {}",
               static_cast<int>(status.error_code()), status.error_message());
  auto [res_status, response] = trustflow::proxy::utils::ParseEncryptedResponse<
      capsule_manager::GetTlsAssetResponse>(enc_res, private_key);
  VerifySfResponseStatus(res_status);

  return response.tls_asset();
}

void CapsuleManagerClient::CreateResultDataKey(
    const std::string& plat, const std::string& cert,
    const std::string& private_key,
    secretflowapis::v2::sdc::capsule_manager::CreateResultDataKeyRequest::Body&
        body) const {
  YACL_ENFORCE(capsule_manager_cert_.size() > 0,
               "capsule_manager_cert not found. you should call GetRaCert() "
               "before other capsule_manager's interface");

  capsule_manager::CreateResultDataKeyRequest request;
  if (plat != kTeePlatSim) {
    SPDLOG_INFO("Serializing CreateResultDataKeyRequest body...");
    std::string serialized_body;
    YACL_ENFORCE(body.SerializeToString(&serialized_body),
                 "Serializing CreateResultDataKeyRequest body failed");

    SPDLOG_INFO("Generating attestation report...");
    const auto report = trustflow::proxy::utils::GenRaReport(serialized_body);

    *request.mutable_attestation_report() = std::move(report);
  }

  *request.mutable_body() = std::move(body);

  const auto enc_req = trustflow::proxy::utils::GenEncryptedRequest<
      capsule_manager::CreateResultDataKeyRequest>(request, private_key, cert,
                                                   capsule_manager_cert_, true);
  capsule_manager::EncryptedResponse enc_res;

  ::grpc::ClientContext context;
  const auto deadline = std::chrono::system_clock::now() +
                        std::chrono::milliseconds(kGrpcTimeoutMs);
  context.set_deadline(deadline);
  const auto status =
      capsule_manager_stub_->CreateResultDataKey(&context, enc_req, &enc_res);
  YACL_ENFORCE(
      status.ok(),
      "Calling CreateResultDataKey failed, error code: {}, message: {}",
      static_cast<int>(status.error_code()), status.error_message());
  VerifySfResponseStatus(enc_res.status());
}

}  // namespace data_capsule_proxy
}  // namespace proxy
}  // namespace trustflow
