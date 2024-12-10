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

#include "trustflow/attestation/generation/hyperenclave/hyperenclave_generator.h"

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/ioctl.h>

#include <string>
#include <vector>

#include "absl/strings/ascii.h"
#include "absl/strings/escaping.h"
#include "cppcodec/base64_rfc4648.hpp"
#include "sgx_quote.h"
#include "sgx_uae_epid.h"
#include "sgx_uae_quote_ex.h"
#include "sgx_urts.h"
#include "sgx_utils.h"
#include "spdlog/spdlog.h"
#include "yacl/base/exception.h"
#include "yacl/crypto/hash/hash_utils.h"
#include "yacl/crypto/rand/rand.h"
#include "yacl/utils/scope_guard.h"

#include "trustflow/attestation/common/constants.h"
#include "trustflow/attestation/utils/json2pb.h"

#define SGXIOC_GET_EPID_GROUP_ID _IOR('s', 1, sgx_epid_group_id_t)
#define SGXIOC_GEN_QUOTE _IOWR('s', 2, sgxioc_gen_epid_quote_arg_t)

namespace trustflow {
namespace attestation {
namespace generation {

namespace {
// occlum sgx device
constexpr char kSgxDevice[] = "/dev/sgx";
constexpr int kMaxQuoteLen = 4096;

/**
 * report_data    Input report data which will be included in quote data.
 *                The first 32 bytes should be the SHA256 hash value of
 *                the public key which is used in the RA work flow.
 * nonce          Nonce value to avoid replay attack. All zero to ignore it.
 * spid           The service provider ID, please use you real SPID,
 *                otherwise, IAS will return bad request.
 * quote_type     Maybe SGX_UNLINKABLE_SIGNATURE or SGX_LINKABLE_SIGNATURE
 *                quote type.
 * sigrl_ptr      The SigRL data buffer
 * sigrl_len      The total length of SigRL data
 * quote          Output quote structure data in binary format.
 */
typedef struct {
  sgx_report_data_t report_data;     // input
  sgx_quote_sign_type_t quote_type;  // input
  sgx_spid_t spid;                   // input
  sgx_quote_nonce_t nonce;           // input
  const uint8_t* sigrl_ptr;          // input (optional)
  uint32_t sigrl_len;                // input (optional)
  uint32_t quote_buf_len;            // input
  union {
    uint8_t* as_buf;
    sgx_quote_t* as_quote;
  } quote;  // output
} sgxioc_gen_epid_quote_arg_t;

void PrepareHyperEnclaveReportData(
    const secretflowapis::v2::sdc::UnifiedAttestationGenerationParams&
        gen_params,
    sgx_report_data_t& report_data) {
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

void SgxDeviceGetQuote(sgxioc_gen_epid_quote_arg_t& quote_args) {
  YACL_ENFORCE(
      quote_args.quote.as_buf != nullptr && quote_args.quote_buf_len > 0,
      "quote buffer is null or length is 0");
  int sgx_fd = open(kSgxDevice, O_RDONLY);
  YACL_ENFORCE_GE(sgx_fd, 0, "Fail to open {}", kSgxDevice);
  ON_SCOPE_EXIT([&] { close(sgx_fd); });

  YACL_ENFORCE_EQ(ioctl(sgx_fd, SGXIOC_GEN_QUOTE, &quote_args), 0,
                  "Fail to generate quote from {}", kSgxDevice);
  YACL_ENFORCE(quote_args.quote.as_quote->signature_len > 0,
               "Invalid quote signature length from {}", kSgxDevice);
}

}  // namespace

secretflowapis::v2::sdc::UnifiedAttestationReport
HyperenclaveAttestationGenerator::GenerateReport(
    const secretflowapis::v2::sdc::UnifiedAttestationGenerationParams&
        gen_params) {
  SPDLOG_INFO("Start generating hyperenclave report");
  YACL_ENFORCE(gen_params.report_type() ==
                       trustflow::attestation::ReportType::kReportTypeBgcheck ||
                   gen_params.report_type() ==
                       trustflow::attestation::ReportType::kReportTypePassport,
               "Unsupport report_type: {}", gen_params.report_type());

  sgxioc_gen_epid_quote_arg_t quote_args;
  memset(&quote_args, 0, sizeof(sgxioc_gen_epid_quote_arg_t));

  sgx_report_data_t report_data = {0};
  PrepareHyperEnclaveReportData(gen_params, report_data);
  memcpy(quote_args.report_data.d, report_data.d, sizeof(sgx_report_data_t));

  quote_args.quote_type = SGX_LINKABLE_SIGNATURE;
  // HyperEnclave don't need SPID, just use empty value
  memset(quote_args.spid.id, 0, sizeof(sgx_spid_t));
  std::vector<uint8_t> nonce =
      yacl::crypto::RandBytes(sizeof(sgx_quote_nonce_t));
  std::copy(nonce.begin(), nonce.end(), quote_args.nonce.rand);

  // HyperEnclave don't support SigRL yet
  quote_args.sigrl_ptr = NULL;
  quote_args.sigrl_len = 0;

  std::vector<uint8_t> quote_buf;
  quote_buf.resize(kMaxQuoteLen, 0);
  quote_args.quote.as_buf = quote_buf.data();
  quote_args.quote_buf_len = quote_buf.size();

  SgxDeviceGetQuote(quote_args);
  SPDLOG_INFO("Sgx Device get hyperenclave report success");

  std::vector<uint8_t> quote;
  size_t quote_signature_len =
      static_cast<size_t>(quote_args.quote.as_quote->signature_len);
  size_t quote_len = sizeof(sgx_quote_t) + quote_signature_len;
  quote.resize(quote_len, 0);
  std::copy(quote_args.quote.as_buf, quote_args.quote.as_buf + quote_len,
            quote.begin());

  secretflowapis::v2::sdc::HyperEnclaveReport hyper_report;
  hyper_report.set_b64_quote(
      cppcodec::base64_rfc4648::encode(quote.data(), quote.size()));

  // For hyperenclave, there is no external reference data
  // BackgroundCheck and Passport type report has the same json_report
  secretflowapis::v2::sdc::UnifiedAttestationReport report;
  PB2JSON(hyper_report, report.mutable_json_report());

  report.set_str_report_version(trustflow::attestation::kReportVersion);
  report.set_str_report_type(gen_params.report_type());
  report.set_str_tee_platform(
      trustflow::attestation::Platform::kPlatformHyperEnclave);

  SPDLOG_INFO("Generate hyperenclave report succeed");

  return report;
}

}  // namespace generation
}  // namespace attestation
}  // namespace trustflow