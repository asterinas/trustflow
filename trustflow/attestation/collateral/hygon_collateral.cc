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

#include "cppcodec/base64_rfc4648.hpp"
#include "csv/attestation/attestation.h"
#include "spdlog/spdlog.h"
#include "yacl/base/exception.h"

#include "trustflow/attestation/utils/json2pb.h"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"

namespace trustflow {
namespace attestation {
namespace collateral {

namespace {

constexpr int kHttpRetryTimes = 3;

constexpr int kHttpResponseOk = 200;

constexpr char kHygonCertUrl[] = "https://cert.hygon.cn";

constexpr char kHygonHskCekSubUrl[] = "/hsk_cek?snumber=";

}  // namespace

// Get Hygon Hsk Cek
void GetHygonCsvCollateral(
    const std::string &chip_id,
    secretflowapis::v2::sdc::HygonCsvCertChain &hygon_csv_cert_chain) {
  httplib::Client client(kHygonCertUrl);
  // TODO support server certificate verification
  client.enable_server_certificate_verification(false);

  std::string hsk_cek_url = std::string(kHygonHskCekSubUrl) + chip_id;

  httplib::Result ret{nullptr, httplib::Error::Unknown};
  for (int i = 0; i < kHttpRetryTimes; i++) {
    ret = client.Get(hsk_cek_url);
    if (ret && ret->status == kHttpResponseOk) {
      break;
    }
  }
  YACL_ENFORCE(ret, "Failed to connect hygon cert server, http ret is null");
  YACL_ENFORCE(ret->status == kHttpResponseOk,
               "Get hygon csv hsk and cek failed, http status: {}, reason: {}",
               ret->status, ret->reason);

  // ret->body is a binary string
  YACL_ENFORCE_EQ(
      ret->body.length(), sizeof(CHIP_ROOT_CERT_t) + sizeof(CSV_CERT_t),
      "hsk and cek length should be {}, but got {}",
      sizeof(CHIP_ROOT_CERT_t) + sizeof(CSV_CERT_t), ret->body.length());
  hygon_csv_cert_chain.set_b64_hsk_cert(cppcodec::base64_rfc4648::encode(
      ret->body.data(), sizeof(CHIP_ROOT_CERT_t)));
  hygon_csv_cert_chain.set_b64_cek_cert(cppcodec::base64_rfc4648::encode(
      ret->body.data() + sizeof(CHIP_ROOT_CERT_t), sizeof(CSV_CERT_t)));

  SPDLOG_INFO("Get hygon csv hsk and cek from hygon website succeed");
}

}  // namespace collateral
}  // namespace attestation
}  // namespace trustflow