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

#include "trustflow/attestation/generation/csv/csv_generator.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <string>

#include "absl/strings/ascii.h"
#include "absl/strings/escaping.h"
#include "cppcodec/base64_rfc4648.hpp"
#include "csv/attestation/attestation.h"
#include "spdlog/spdlog.h"
#include "yacl/base/exception.h"
#include "yacl/crypto/hash/hash_utils.h"
#include "yacl/crypto/hmac/hmac_sm3.h"
#include "yacl/crypto/rand/rand.h"
#include "yacl/utils/scope_guard.h"

#include "trustflow/attestation/collateral/hygon_collateral.h"
#include "trustflow/attestation/common/constants.h"
#include "trustflow/attestation/utils/json2pb.h"

// copy from hygon csv_sdk (defined in ioctl_get_attestation_report.c)
#define CSV_GUEST_IOC_TYPE 'D'
#define GET_ATTESTATION_REPORT _IOWR(CSV_GUEST_IOC_TYPE, 1, csv_guest_mem)

namespace trustflow {
namespace attestation {
namespace generation {

namespace {

constexpr char kCsvDevice[] = "/dev/csv-guest";

constexpr uint32_t kPageSize = 4096;

// copy from hygon csv_sdk (defined in ioctl_get_attestation_report.c)
struct csv_guest_mem {
  uint64_t va;
  int32_t size;
};

// copy from hygon csv_sdk (defined in ioctl_get_attestation_report.c)
struct csv_attestation_user_data {
  uint8_t data[GUEST_ATTESTATION_DATA_SIZE];
  uint8_t mnonce[GUEST_ATTESTATION_NONCE_SIZE];
  hash_block_u hash;
};

void PrepareCsvUserData(
    const secretflowapis::v2::sdc::UnifiedAttestationGenerationParams
        &gen_params,
    csv_attestation_user_data &user_data) {
  // note: 2 hex chars represent 1 byte
  YACL_ENFORCE_LE(
      gen_params.report_hex_nonce().length(),
      static_cast<size_t>(GUEST_ATTESTATION_NONCE_SIZE * 2),
      "report_hex_nonce length should not be greater than {}, got {}",
      GUEST_ATTESTATION_NONCE_SIZE * 2, gen_params.report_hex_nonce().length());
  // HASH_LEN defined in hygon attestation.h
  YACL_ENFORCE_LE(gen_params.report_params().hex_user_data().length(),
                  static_cast<size_t>(HASH_LEN * 2),
                  "hex_user_data length should not be greater than {}, got {}",
                  HASH_LEN * 2,
                  gen_params.report_params().hex_user_data().length());

  YACL_ENFORCE(gen_params.report_hex_nonce().empty() ||
                   gen_params.report_params().hex_user_data().empty(),
               "Not support both nonce and user data");

  memset(&user_data, 0, sizeof(csv_attestation_user_data));
  if (!gen_params.report_hex_nonce().empty()) {
    auto tmp_nonce = absl::HexStringToBytes(gen_params.report_hex_nonce());
    memcpy(user_data.data, tmp_nonce.data(), tmp_nonce.size());
  }
  if (!gen_params.report_params().hex_user_data().empty()) {
    auto tmp_user_data =
        absl::HexStringToBytes(gen_params.report_params().hex_user_data());
    memcpy(user_data.data, tmp_user_data.data(), tmp_user_data.size());
  }
  if (!gen_params.report_params().pem_public_key().empty()) {
    auto public_key_hash = yacl::crypto::Sha256(
        absl::HexStringToBytes(gen_params.report_params().pem_public_key()));
    memcpy(user_data.data + HASH_LEN, public_key_hash.data(),
           public_key_hash.size());
  }
  yacl::crypto::FillRand(reinterpret_cast<char *>(user_data.mnonce),
                         GUEST_ATTESTATION_NONCE_SIZE, true);

  auto user_data_hash = yacl::crypto::Sm3(yacl::ByteContainerView(
      &user_data, GUEST_ATTESTATION_DATA_SIZE + GUEST_ATTESTATION_NONCE_SIZE));
  std::copy(user_data_hash.begin(), user_data_hash.end(), user_data.hash.block);
}

}  // namespace

secretflowapis::v2::sdc::UnifiedAttestationReport
CsvAttestationGenerator::GenerateReport(
    const secretflowapis::v2::sdc::UnifiedAttestationGenerationParams
        &gen_params) {
  SPDLOG_INFO("Start generating csv report");
  YACL_ENFORCE(gen_params.report_type() ==
                       trustflow::attestation::ReportType::kReportTypeBgcheck ||
                   gen_params.report_type() ==
                       trustflow::attestation::ReportType::kReportTypePassport,
               "Unsupport report_type: {}", gen_params.report_type());

  YACL_ENFORCE_LE(
      sizeof(csv_attestation_report), static_cast<size_t>(kPageSize),
      "csv report size should not be greater than PAGESIZE({})", kPageSize);

  csv_attestation_user_data user_data;
  PrepareCsvUserData(gen_params, user_data);

  int csv_fd = open(kCsvDevice, O_RDWR);
  YACL_ENFORCE_GE(csv_fd, 0, "Fail to open {}", kCsvDevice);
  ON_SCOPE_EXIT([&] { close(csv_fd); });

  // csv GET_ATTESTATION_REPORT ioctl need extra space, given kPageSize here
  // same as csv sdk
  std::vector<uint8_t> quote(kPageSize);
  auto p_quote = reinterpret_cast<csv_attestation_report *>(quote.data());

  YACL_ENFORCE_GT(
      sizeof(csv_attestation_report), sizeof(csv_attestation_user_data),
      "csv_attestation_report's size(got {}) should be greater "
      "than csv_attestation_user_data's size(got {})",
      sizeof(csv_attestation_report), sizeof(csv_attestation_user_data));
  memcpy(p_quote, &user_data, sizeof(csv_attestation_user_data));

  csv_guest_mem guest_mem;
  guest_mem.va = reinterpret_cast<uint64_t>(p_quote);
  guest_mem.size = kPageSize;

  YACL_ENFORCE_EQ(ioctl(csv_fd, GET_ATTESTATION_REPORT, &guest_mem), 0,
                  "Csv ioctl GET_ATTESTATION_REPORT failed, errno = {}", errno);

  SPDLOG_INFO("csv ioctl GET_ATTESTATION_REPORT succeed");

  // compute session_mac and verify
  auto hmac_sm3 = yacl::crypto::HmacSm3(
      yacl::ByteContainerView(user_data.mnonce, GUEST_ATTESTATION_NONCE_SIZE));

  auto hmac_res =
      hmac_sm3
          .Update(yacl::ByteContainerView(&(p_quote->pek_cert),
                                          sizeof(p_quote->pek_cert)))
          .Update(yacl::ByteContainerView(p_quote->sn, sizeof(p_quote->sn)))
          .Update(yacl::ByteContainerView(p_quote->reserved2,
                                          sizeof(p_quote->reserved2)))
          .CumulativeMac();
  YACL_ENFORCE_EQ(memcmp(hmac_res.data(), p_quote->mac.block, HASH_LEN), 0,
                  "PEK cert and chip id hmac verify failed");
  SPDLOG_INFO("PEK cert and chip id hmac verify succeed");

  memset(p_quote->reserved2, 0, sizeof(p_quote->reserved2));

  secretflowapis::v2::sdc::HygonCsvReport hygon_csv_report;
  hygon_csv_report.set_b64_quote(cppcodec::base64_rfc4648::encode(
      quote.data(), sizeof(csv_attestation_report)));

  // parse chip id from report
  uint8_t chip_id[SN_LEN];
  for (size_t i = 0; i < sizeof(chip_id) / sizeof(p_quote->anonce); i++) {
    reinterpret_cast<uint32_t *>(chip_id)[i] =
        reinterpret_cast<uint32_t *>(p_quote->sn)[i] ^ p_quote->anonce;
  }
  std::string chip_id_str(reinterpret_cast<char *>(chip_id), SN_LEN);
  SPDLOG_INFO("chip id = {}", chip_id_str.c_str());

  // get collateral
  if (gen_params.report_type() ==
      trustflow::attestation::ReportType::kReportTypePassport) {
    SPDLOG_INFO("Start getting csv collateral");
    secretflowapis::v2::sdc::HygonCsvCertChain hygon_csv_cert_chain;
    trustflow::attestation::collateral::GetHygonCsvCollateral(
        chip_id_str, hygon_csv_cert_chain);

    PB2JSON(hygon_csv_cert_chain, hygon_csv_report.mutable_json_cert_chain());
    SPDLOG_INFO("Get csv collateral succeed");
  }

  hygon_csv_report.set_str_chip_id(std::move(chip_id_str));

  secretflowapis::v2::sdc::UnifiedAttestationReport report;
  PB2JSON(hygon_csv_report, report.mutable_json_report());

  report.set_str_report_version(trustflow::attestation::kReportVersion);
  report.set_str_report_type(gen_params.report_type());
  report.set_str_tee_platform(trustflow::attestation::Platform::kPlatformCsv);
  SPDLOG_INFO("Generate csv report succeed");

  return report;
}

}  // namespace generation
}  // namespace attestation
}  // namespace trustflow