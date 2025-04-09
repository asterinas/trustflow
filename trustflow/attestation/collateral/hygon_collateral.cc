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

#include "trustflow/attestation/collateral/hygon_collateral.h"

#include "brpc/channel.h"
#include "cppcodec/base64_rfc4648.hpp"
#include "csv/attestation/attestation.h"
#include "spdlog/spdlog.h"
#include "yacl/base/exception.h"

#include "trustflow/attestation/utils/json2pb.h"

namespace trustflow {
namespace attestation {
namespace collateral {

namespace {
constexpr int kHttpTimeoutMs = 500;

constexpr int kHttpRetryTimes = 3;

constexpr char kHygonCertUrl[] = "https://cert.hygon.cn";

constexpr char kHygonHskCekSubUrl[] = "/hsk_cek?snumber=";

}  // namespace

// Get Hygon Hsk Cek
void GetHygonCsvCollateral(
    const std::string &chip_id,
    secretflowapis::v2::sdc::HygonCsvCertChain &hygon_csv_cert_chain) {
  brpc::Channel channel;
  brpc::ChannelOptions options;
  options.protocol = brpc::ProtocolType::PROTOCOL_HTTP;
  options.timeout_ms = kHttpTimeoutMs;
  options.max_retry = kHttpRetryTimes;
  YACL_ENFORCE_EQ(channel.Init(kHygonCertUrl, &options), 0,
                  "hygon cert server channel init failed");

  brpc::Controller cntl;
  cntl.http_request().uri() = kHygonHskCekSubUrl + chip_id;
  cntl.http_request().set_method(brpc::HTTP_METHOD_GET);
  channel.CallMethod(nullptr, &cntl, nullptr, nullptr, nullptr);
  YACL_ENFORCE(!cntl.Failed(),
               "Get hygon csv hsk and cek failed, error code: {}, message: {}",
               cntl.ErrorCode(), cntl.ErrorText());
  const std::string response_body = cntl.response_attachment().to_string();

  // response_body is a binary string
  YACL_ENFORCE_EQ(
      response_body.length(), sizeof(CHIP_ROOT_CERT_t) + sizeof(CSV_CERT_t),
      "hsk and cek length should be {}, but got {}",
      sizeof(CHIP_ROOT_CERT_t) + sizeof(CSV_CERT_t), response_body.length());
  hygon_csv_cert_chain.set_b64_hsk_cert(cppcodec::base64_rfc4648::encode(
      response_body.data(), sizeof(CHIP_ROOT_CERT_t)));
  hygon_csv_cert_chain.set_b64_cek_cert(cppcodec::base64_rfc4648::encode(
      response_body.data() + sizeof(CHIP_ROOT_CERT_t), sizeof(CSV_CERT_t)));

  SPDLOG_INFO("Get hygon csv hsk and cek from hygon website succeed");
}

}  // namespace collateral
}  // namespace attestation
}  // namespace trustflow