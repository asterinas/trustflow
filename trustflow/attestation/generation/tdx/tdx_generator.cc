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

#include "trustflow/attestation/generation/tdx/tdx_generator.h"

#include <fcntl.h>
#include <stdint.h>
#include <sys/ioctl.h>

#include <string>

#include "absl/strings/ascii.h"
#include "absl/strings/escaping.h"
#include "cppcodec/base64_rfc4648.hpp"
#include "sgx_report.h"
#include "spdlog/spdlog.h"
#include "tdx_attest.h"
#include "yacl/base/exception.h"
#include "yacl/crypto/hash/hash_utils.h"

#include "trustflow/attestation/collateral/intel_collateral.h"
#include "trustflow/attestation/common/constants.h"
#include "trustflow/attestation/utils/json2pb.h"

namespace trustflow {
namespace attestation {
namespace generation {

namespace {
void PrepareTdxReportData(
    const secretflowapis::v2::sdc::UnifiedAttestationGenerationParams&
        gen_params,
    tdx_report_data_t& report_data) {
  // note: 2 hex chars represent 1 byte
  YACL_ENFORCE_LE(
      gen_params.report_hex_nonce().length(),
      static_cast<size_t>(SGX_HASH_SIZE * 2),
      "report_hex_nonce length should not be greater than {}, got {}",
      SGX_HASH_SIZE * 2, gen_params.report_hex_nonce().length());
  YACL_ENFORCE_LE(gen_params.report_params().hex_user_data().length(),
                  static_cast<size_t>(SGX_HASH_SIZE * 2),
                  "hex_user_data length should not be greater than {}, got {}",
                  SGX_HASH_SIZE * 2,
                  gen_params.report_params().hex_user_data().length());

  YACL_ENFORCE(gen_params.report_hex_nonce().empty() ||
                   gen_params.report_params().hex_user_data().empty(),
               "Not support both nonce and user data");
  memset(report_data.d, 0, TDX_REPORT_DATA_SIZE);
  if (!gen_params.report_hex_nonce().empty()) {
    auto tmp_nonce = absl::HexStringToBytes(gen_params.report_hex_nonce());
    memcpy(report_data.d, tmp_nonce.data(), tmp_nonce.size());
  }
  if (!gen_params.report_params().hex_user_data().empty()) {
    auto tmp_user_data =
        absl::HexStringToBytes(gen_params.report_params().hex_user_data());
    memcpy(report_data.d, tmp_user_data.data(), tmp_user_data.size());
  }
  if (!gen_params.report_params().pem_public_key().empty()) {
    auto public_key_hash = yacl::crypto::Sha256(
        absl::HexStringToBytes(gen_params.report_params().pem_public_key()));
    memcpy(report_data.d + SGX_HASH_SIZE, public_key_hash.data(),
           public_key_hash.size());
  }
}
}  // namespace

secretflowapis::v2::sdc::UnifiedAttestationReport
TdxAttestationGenerator::GenerateReport(
    const secretflowapis::v2::sdc::UnifiedAttestationGenerationParams&
        gen_params) {
  SPDLOG_INFO("Start generating tdx report");
  YACL_ENFORCE(gen_params.report_type() ==
                       trustflow::attestation::ReportType::kReportTypeBgcheck ||
                   gen_params.report_type() ==
                       trustflow::attestation::ReportType::kReportTypePassport,
               "Unsupport report_type: {}", gen_params.report_type());
  tdx_report_data_t report_data = {0};
  PrepareTdxReportData(gen_params, report_data);

  std::string quote;
  // generate quote
  tdx_uuid_t selected_att_key_id = {0};
  uint8_t* p_quote_buf = nullptr;
  uint32_t quote_size = 0;

  int ret = tdx_att_get_quote(&report_data, nullptr, 0, &selected_att_key_id,
                              &p_quote_buf, &quote_size, 0);
  YACL_ENFORCE(ret == tdx_attest_error_t::TDX_ATTEST_SUCCESS && p_quote_buf,
               "tdx_att_get_quote err: {:#x}", ret);
  SPDLOG_INFO("tdx_att_get_quote succeed");

  quote.assign(reinterpret_cast<char*>(p_quote_buf), quote_size);

  if (p_quote_buf) {
    tdx_att_free_quote(p_quote_buf);
  }

  secretflowapis::v2::sdc::IntelTdxReport tdx_report;
  tdx_report.set_b64_quote(
      cppcodec::base64_rfc4648::encode(quote.data(), quote.size()));
  if (gen_params.report_type() ==
      trustflow::attestation::ReportType::kReportTypePassport) {
    SPDLOG_INFO("Start getting tdx collateral");

    secretflowapis::v2::sdc::SgxQlQveCollateral collateral;
    trustflow::attestation::collateral::GetIntelCollateral(quote, quote_size,
                                                           collateral);
    PB2JSON(collateral, tdx_report.mutable_json_collateral());

    SPDLOG_INFO("Get tdx collateral succeed");
  }
  secretflowapis::v2::sdc::UnifiedAttestationReport report;
  PB2JSON(tdx_report, report.mutable_json_report());

  report.set_str_report_version(trustflow::attestation::kReportVersion);
  report.set_str_report_type(gen_params.report_type());
  report.set_str_tee_platform(trustflow::attestation::Platform::kPlatformTdx);
  SPDLOG_INFO("Generate tdx report succeed");

  return report;
}

}  // namespace generation
}  // namespace attestation
}  // namespace trustflow