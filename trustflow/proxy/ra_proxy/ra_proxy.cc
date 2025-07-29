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

#include "trustflow/proxy/ra_proxy/ra_proxy.h"

#include "absl/strings/ascii.h"
#include "absl/strings/escaping.h"
#include "spdlog/spdlog.h"
#include "src/google/protobuf/util/json_util.h"
#include "yacl/base/exception.h"
#include "yacl/crypto/hash/hash_utils.h"

#include "trustflow/proxy/utils/ra_util.h"

#include "secretflowapis/v2/sdc/ual.pb.h"

namespace trustflow {
namespace proxy {
namespace ra_proxy {

namespace {
constexpr char kResponseContentType[] = "application/json";
constexpr char kTeePlatSim[] = "sim";
}  // namespace

void RaProxyImpl::GetRaCert(
    ::google::protobuf::RpcController* cntl_base,
    const ::secretflowapis::v2::sdc::ra_proxy::GetRaCertRequest* request,
    ::secretflowapis::v2::sdc::ra_proxy::GetRaCertResponse* response,
    ::google::protobuf::Closure* done) {
  brpc::ClosureGuard done_guard(done);
  brpc::Controller* cntl = static_cast<brpc::Controller*>(cntl_base);

  try {
    cntl->http_response().set_content_type(kResponseContentType);

    if (plat_ != kTeePlatSim) {
      const std::string user_data = cert_ + "." + request->nonce();
      const auto report = trustflow::proxy::utils::GenRaReport(user_data);
      *(response->mutable_attestation_report()) = std::move(report);
    }

    secretflowapis::v2::Status status;
    status.set_code(secretflowapis::v2::Code::OK);
    status.set_message("success");
    *(response->mutable_status()) = std::move(status);
    response->set_cert(cert_);
  } catch (const std::exception& e) {
    SPDLOG_ERROR(e.what());
    cntl->SetFailed(e.what());
  }
}

}  // namespace ra_proxy
}  // namespace proxy
}  // namespace trustflow