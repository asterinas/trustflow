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

#include "trustflow/attestation/generation/sgx2/sgx2_generator.h"

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/ioctl.h>

#include <string>

#include "absl/strings/ascii.h"
#include "absl/strings/escaping.h"
#include "cppcodec/base64_rfc4648.hpp"
#include "sgx_dcap_quoteverify.h"
#include "sgx_ql_lib_common.h"
#include "sgx_quote_3.h"
#include "sgx_report.h"
#include "spdlog/spdlog.h"
#include "yacl/base/exception.h"
#include "yacl/crypto/hash/hash_utils.h"
#include "yacl/utils/scope_guard.h"

#include "trustflow/attestation/collateral/intel_collateral.h"
#include "trustflow/attestation/common/constants.h"
#include "trustflow/attestation/utils/json2pb.h"

#define SGXIOC_GET_DCAP_QUOTE_SIZE _IOR('s', 7, uint32_t)
#define SGXIOC_GEN_DCAP_QUOTE _IOWR('s', 8, sgxioc_gen_dcap_quote_arg_t)

namespace trustflow {
namespace attestation {
namespace generation {

namespace {

// occlum sgx device
constexpr char kSgxDevice[] = "/dev/sgx";

typedef struct {
  sgx_report_data_t *report_data;  // input
  uint32_t *quote_len;             // input/output
  uint8_t *quote_buf;              // output
} sgxioc_gen_dcap_quote_arg_t;

void PrepareSgxReportData(
    const secretflowapis::v2::sdc::UnifiedAttestationGenerationParams
        &gen_params,
    sgx_report_data_t &report_data) {
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

  memset(report_data.d, 0, SGX_REPORT_DATA_SIZE);
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
Sgx2AttestationGenerator::GenerateReport(
    const secretflowapis::v2::sdc::UnifiedAttestationGenerationParams
        &gen_params) {
  SPDLOG_INFO("Start generating sgx2 report");
  YACL_ENFORCE(gen_params.report_type() ==
                       trustflow::attestation::ReportType::kReportTypeBgcheck ||
                   gen_params.report_type() ==
                       trustflow::attestation::ReportType::kReportTypePassport,
               "Unsupport report_type: {}", gen_params.report_type());

  int sgx_fd = open(kSgxDevice, O_RDONLY);
  YACL_ENFORCE_GE(sgx_fd, 0, "Fail to open {}", kSgxDevice);
  ON_SCOPE_EXIT([&] { close(sgx_fd); });

  uint32_t quote_size = 0;
  YACL_ENFORCE_EQ(ioctl(sgx_fd, SGXIOC_GET_DCAP_QUOTE_SIZE, &quote_size), 0,
                  "Fail to get quote size, errno = {}", errno);

  sgx_report_data_t report_data = {0};
  PrepareSgxReportData(gen_params, report_data);

  // prepare parameters for getting quote
  std::string quote;
  quote.resize(quote_size, 0);
  sgxioc_gen_dcap_quote_arg_t gen_quote_arg = {
      .report_data = &report_data,
      .quote_len = &quote_size,
      .quote_buf =
          reinterpret_cast<uint8_t *>(const_cast<char *>(quote.data()))};

  // get quote
  YACL_ENFORCE_EQ(ioctl(sgx_fd, SGXIOC_GEN_DCAP_QUOTE, &gen_quote_arg), 0,
                  "Fail to get quote, errno = {}", errno);

  SPDLOG_INFO("Get sgx2 quote succeed");

  secretflowapis::v2::sdc::DcapReport dcap_report;
  dcap_report.set_b64_quote(
      cppcodec::base64_rfc4648::encode(quote.data(), quote.size()));

  if (gen_params.report_type() ==
      trustflow::attestation::ReportType::kReportTypePassport) {
    SPDLOG_INFO("Start getting sgx2 collateral");

    secretflowapis::v2::sdc::SgxQlQveCollateral collateral;
    trustflow::attestation::collateral::GetIntelCollateral(quote, quote_size,
                                                           collateral);
    PB2JSON(collateral, dcap_report.mutable_json_collateral());

    SPDLOG_INFO("Get sgx2 collateral succeed");
  }
  secretflowapis::v2::sdc::UnifiedAttestationReport report;
  PB2JSON(dcap_report, report.mutable_json_report());

  report.set_str_report_version(trustflow::attestation::kReportVersion);
  report.set_str_report_type(gen_params.report_type());
  report.set_str_tee_platform(
      trustflow::attestation::Platform::kPlatformSgxDcap);
  SPDLOG_INFO("Generate sgx2 report succeed");

  return report;
}

}  // namespace generation
}  // namespace attestation
}  // namespace trustflow