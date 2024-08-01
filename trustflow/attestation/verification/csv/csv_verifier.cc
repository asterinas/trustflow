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

#include "trustflow/attestation/verification/csv/csv_verifier.h"

#include "absl/strings/escaping.h"
#include "cppcodec/base64_rfc4648.hpp"
#include "openssl/core_names.h"
#include "openssl/param_build.h"
#include "openssl/params.h"
#include "spdlog/spdlog.h"
#include "yacl/base/exception.h"
#include "yacl/crypto/openssl_wrappers.h"
#include "yacl/crypto/sign/sm2_signing.h"

#include "trustflow/attestation/common/constants.h"
#include "trustflow/attestation/utils/json2pb.h"

namespace trustflow {
namespace attestation {
namespace verification {

namespace {

// struct defined by Hygon in attestation.h
// hrk pubkey parsed from https://cert.hygon.cn/hrk
const ecc_pubkey_t kHrkPubkey = {
    .curve_id = CURVE_ID_TYPE_SM2_256,

    .Qx = {0x92c2f62d, 0x2af5f21d, 0x85cd1f50, 0xc80935e7, 0x09563a75,
           0xf3d702db, 0x62a4f14c, 0xbe62e14d},
    .Qy = {0xb41eb946, 0x38744d68, 0xb9be8847, 0x4a640c10, 0x164e9538,
           0x584f8b97, 0x57bb7015, 0x3bab123a},

    .user_id = {0x5948000d, 0x2d4e4f47, 0x2d445353, 0x004b5248},
};

using UniqueParamBld =
    std::unique_ptr<OSSL_PARAM_BLD, decltype(&OSSL_PARAM_BLD_free)>;
using UniqueParam = std::unique_ptr<OSSL_PARAM, decltype(&OSSL_PARAM_free)>;
using UniqueECDSASig = std::unique_ptr<ECDSA_SIG, decltype(&ECDSA_SIG_free)>;
using UniquePkeyCtx =
    std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)>;
using UniquePkey = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>;
using UniqueMdCtx = std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)>;
using UniqueBn = std::unique_ptr<BIGNUM, decltype(&BN_free)>;

UniquePkey ImportHygonSm2Pubkey(const ecc_pubkey_t& hygon_pubkey) {
  // generate pubkey buf
  uint8_t pubkey[1 + ECC_LEN + ECC_LEN];
  pubkey[0] = POINT_CONVERSION_UNCOMPRESSED;
  memcpy(pubkey + 1, hygon_pubkey.Qx, ECC_LEN);
  memcpy(pubkey + 1 + ECC_LEN, hygon_pubkey.Qy, ECC_LEN);
  std::reverse(pubkey + 1, pubkey + 1 + ECC_LEN);
  std::reverse(pubkey + 1 + ECC_LEN, pubkey + 1 + ECC_LEN + ECC_LEN);

  auto pctx = UniquePkeyCtx(EVP_PKEY_CTX_new_id(EVP_PKEY_SM2, nullptr),
                            &EVP_PKEY_CTX_free);
  YACL_ENFORCE(pctx != nullptr && pctx.get() != nullptr,
               "UniquePkeyCtx init err");

  auto param_bld = UniqueParamBld(OSSL_PARAM_BLD_new(), &OSSL_PARAM_BLD_free);
  YACL_ENFORCE(param_bld != nullptr && param_bld.get() != nullptr,
               "UniqueParamBld init err");

  YACL_ENFORCE_EQ(OSSL_PARAM_BLD_push_utf8_string(
                      param_bld.get(), OSSL_PKEY_PARAM_GROUP_NAME, SN_sm2, 0),
                  1, "OSSL_PARAM_BLD: SM2 group name push err");
  YACL_ENFORCE_EQ(
      OSSL_PARAM_BLD_push_octet_string(param_bld.get(), OSSL_PKEY_PARAM_PUB_KEY,
                                       pubkey, sizeof(pubkey)),
      1, "OSSL_PARAM_BLD: public key push err");

  auto params =
      UniqueParam(OSSL_PARAM_BLD_to_param(param_bld.get()), &OSSL_PARAM_free);
  YACL_ENFORCE(params != nullptr && params.get() != nullptr,
               "OSSL_PARAM_BLD_to_param err");

  EVP_PKEY* raw_pkey = nullptr;
  YACL_ENFORCE_EQ(EVP_PKEY_fromdata_init(pctx.get()), 1);
  YACL_ENFORCE_EQ(EVP_PKEY_fromdata(pctx.get(), &raw_pkey, EVP_PKEY_PUBLIC_KEY,
                                    params.get()),
                  1);

  auto pkey = UniquePkey(raw_pkey, &EVP_PKEY_free);
  YACL_ENFORCE(pkey != nullptr && pkey.get() != nullptr,
               "EVP_PKEY_fromdata err");

  return pkey;
}

std::vector<uint8_t> ImportHygonSm2Sig(const ecc_signature_t& hygon_sig) {
  ecdsa_sign ecdsa_sig;
  memcpy(ecdsa_sig.r, hygon_sig.sig_r, ECC_LEN);
  memcpy(ecdsa_sig.s, hygon_sig.sig_s, ECC_LEN);
  std::reverse(ecdsa_sig.r, ecdsa_sig.r + ECC_LEN);
  std::reverse(ecdsa_sig.s, ecdsa_sig.s + ECC_LEN);

  auto bn_r = UniqueBn(BN_bin2bn(ecdsa_sig.r, ECC_LEN, nullptr), &BN_free);
  auto bn_s = UniqueBn(BN_bin2bn(ecdsa_sig.s, ECC_LEN, nullptr), &BN_free);
  YACL_ENFORCE(bn_r != nullptr && bn_r.get() != nullptr && bn_s != nullptr &&
                   bn_s.get() != nullptr,
               "r, s to BIGNUM err");

  auto sig = UniqueECDSASig(ECDSA_SIG_new(), &ECDSA_SIG_free);

  YACL_ENFORCE(sig != nullptr && sig.get() != nullptr, "ECDSA_SIG init err");
  // ECDSA_SIG_free will free bn_sig_r and bn_sig_s, so we need to release them
  YACL_ENFORCE_EQ(ECDSA_SIG_set0(sig.get(), bn_r.release(), bn_s.release()), 1,
                  "BIGNUM r, s to ECDSA err");

  // convert ECDSA_SIG into asn1 der format
  const int sig_der_len = i2d_ECDSA_SIG(sig.get(), nullptr);
  YACL_ENFORCE(sig_der_len > 0, "get i2d_ECDSA_SIG length err");
  std::vector<uint8_t> sig_der(sig_der_len);
  uint8_t* psig_der = sig_der.data();
  YACL_ENFORCE_EQ(i2d_ECDSA_SIG(sig.get(), &psig_der), sig_der_len,
                  "i2d_ECDSA_SIG err");

  return sig_der;
}

// hygon uses specific user_id in sm2 sig/verify,
// while yacl uses default user_id (not as a parameter)
// so we need to verify sm2 signature using openssl
void Sm2Verify(yacl::ByteContainerView message,
               yacl::ByteContainerView signature, const UniquePkey& pk,
               yacl::ByteContainerView user_id) {
  auto pctx =
      UniquePkeyCtx(EVP_PKEY_CTX_new(pk.get(), nullptr), &EVP_PKEY_CTX_free);
  YACL_ENFORCE(pctx != nullptr && pctx.get() != nullptr,
               "UniquePkeyCtx init err");

  YACL_ENFORCE_EQ(
      EVP_PKEY_CTX_set1_id(pctx.get(), user_id.data(), user_id.size()), 1,
      "EVP_PKEY_CTX_set1_id err");

  // create message digest context
  auto mctx = UniqueMdCtx(EVP_MD_CTX_new(), &EVP_MD_CTX_free);
  YACL_ENFORCE(mctx != nullptr && mctx.get() != nullptr,
               "UniqueMdCtx init err");
  EVP_MD_CTX_set_pkey_ctx(mctx.get(), pctx.get());
  YACL_ENFORCE_EQ(
      EVP_DigestVerifyInit(
          mctx.get(), /* pkey ctx has already been inited */ nullptr, EVP_sm3(),
          /* engine */ nullptr, pk.get()),
      1, "EVP_DigestVerifyInit err");

  YACL_ENFORCE_EQ(
      EVP_DigestVerifyUpdate(mctx.get(), message.data(), message.size()), 1,
      "EVP_DigestVerifyUpdate err");

  YACL_ENFORCE_EQ(
      EVP_DigestVerifyFinal(mctx.get(), signature.data(), signature.size()), 1,
      "Sm2 verify failed");
}

void VerifyHskCertWithHrkPubkey(const CHIP_ROOT_CERT_t& hsk_cert) {
  YACL_ENFORCE_EQ(hsk_cert.key_usage, static_cast<uint32_t>(KEY_USAGE_TYPE_HSK),
                  "HSK cert usage type err, expect {}, got {}",
                  static_cast<uint32_t>(KEY_USAGE_TYPE_HSK),
                  hsk_cert.key_usage);

  auto pkey = ImportHygonSm2Pubkey(kHrkPubkey);
  auto sig_der = ImportHygonSm2Sig(hsk_cert.ecc_sig);
  // hsd_cert's content before signature is msg
  size_t msg_len = sizeof(hsk_cert.version) + sizeof(hsk_cert.key_id) +
                   sizeof(hsk_cert.certifying_id) + sizeof(hsk_cert.key_usage) +
                   sizeof(hsk_cert.reserved1) + sizeof(hsk_cert.ecc_pubkey) +
                   sizeof(hsk_cert.reserved2);

  Sm2Verify(yacl::ByteContainerView(&hsk_cert, msg_len), sig_der, pkey,
            yacl::ByteContainerView(
                (reinterpret_cast<const userid_u*>(kHrkPubkey.user_id))->uid,
                (reinterpret_cast<const userid_u*>(kHrkPubkey.user_id))->len));

  SPDLOG_INFO("VerifyHskCertWithHrkPubkey succeed");
}

void VerifyCekCertWithHskCert(const CSV_CERT_t& cek_cert,
                              const CHIP_ROOT_CERT_t& hsk_cert) {
  YACL_ENFORCE_EQ(
      cek_cert.pubkey_usage, static_cast<uint32_t>(KEY_USAGE_TYPE_CEK),
      "CEK cert pubkey_usage type err, expect {}, got {}",
      static_cast<uint32_t>(KEY_USAGE_TYPE_CEK), cek_cert.pubkey_usage);

  YACL_ENFORCE_EQ(
      cek_cert.sig1_usage, static_cast<uint32_t>(KEY_USAGE_TYPE_HSK),
      "CEK cert sig1 usage type err, expect {}, got {}",
      static_cast<uint32_t>(KEY_USAGE_TYPE_HSK), cek_cert.sig1_usage);

  auto pkey = ImportHygonSm2Pubkey(hsk_cert.ecc_pubkey);
  auto sig_der = ImportHygonSm2Sig(cek_cert.ecc_sig1);

  size_t msg_len = sizeof(cek_cert.version) + sizeof(cek_cert.api_major) +
                   sizeof(cek_cert.api_minor) + sizeof(cek_cert.reserved1) +
                   sizeof(cek_cert.reserved2) + sizeof(cek_cert.pubkey_usage) +
                   sizeof(cek_cert.pubkey_algo) + sizeof(cek_cert.ecc_pubkey) +
                   sizeof(cek_cert.reserved3);

  Sm2Verify(
      yacl::ByteContainerView(&cek_cert, msg_len), sig_der, pkey,
      yacl::ByteContainerView(
          (reinterpret_cast<const userid_u*>(hsk_cert.ecc_pubkey.user_id))->uid,
          (reinterpret_cast<const userid_u*>(hsk_cert.ecc_pubkey.user_id))
              ->len));

  SPDLOG_INFO("VerifyCekCertWithHskCert succeed");
}

void VerifyPekCertWithCekCert(const CSV_CERT_t& pek_cert,
                              const CSV_CERT_t& cek_cert) {
  YACL_ENFORCE_EQ(
      pek_cert.pubkey_usage, static_cast<uint32_t>(KEY_USAGE_TYPE_PEK),
      "PEK cert pubkey_usage type err, expect {}, got {}",
      static_cast<uint32_t>(KEY_USAGE_TYPE_PEK), pek_cert.pubkey_usage);

  YACL_ENFORCE_EQ(
      pek_cert.sig1_usage, static_cast<uint32_t>(KEY_USAGE_TYPE_CEK),
      "PEK cert sig1 usage type err, expect {}, got {}",
      static_cast<uint32_t>(KEY_USAGE_TYPE_CEK), pek_cert.sig1_usage);

  auto pkey = ImportHygonSm2Pubkey(cek_cert.ecc_pubkey);
  auto sig_der = ImportHygonSm2Sig(pek_cert.ecc_sig1);

  size_t msg_len = sizeof(pek_cert.version) + sizeof(pek_cert.api_major) +
                   sizeof(pek_cert.api_minor) + sizeof(pek_cert.reserved1) +
                   sizeof(pek_cert.reserved2) + sizeof(pek_cert.pubkey_usage) +
                   sizeof(pek_cert.pubkey_algo) + sizeof(pek_cert.ecc_pubkey) +
                   sizeof(pek_cert.reserved3);

  Sm2Verify(
      yacl::ByteContainerView(&pek_cert, msg_len), sig_der, pkey,
      yacl::ByteContainerView(
          (reinterpret_cast<const userid_u*>(cek_cert.ecc_pubkey.user_id))->uid,
          (reinterpret_cast<const userid_u*>(cek_cert.ecc_pubkey.user_id))
              ->len));

  SPDLOG_INFO("VerifyPekCertWithCekCert succeed");
}

void VerifyQuoteSignature(const csv_attestation_report& quote,
                          const CSV_CERT_t& pek_cert) {
  auto pkey = ImportHygonSm2Pubkey(pek_cert.ecc_pubkey);
  auto sig_der = ImportHygonSm2Sig(quote.ecc_sig1);

  size_t msg_len = sizeof(quote.user_pubkey_digest) + sizeof(quote.vm_id) +
                   sizeof(quote.vm_version) + sizeof(quote.user_data) +
                   sizeof(quote.mnonce) + sizeof(quote.measure) +
                   sizeof(quote.policy);

  Sm2Verify(
      yacl::ByteContainerView(&quote, msg_len), sig_der, pkey,
      yacl::ByteContainerView(
          (reinterpret_cast<const userid_u*>(pek_cert.ecc_pubkey.user_id))->uid,
          (reinterpret_cast<const userid_u*>(pek_cert.ecc_pubkey.user_id))
              ->len));

  SPDLOG_INFO("VerifyQuoteSignature succeed");
}

// Note: compilers will perform RVO, refer to https://abseil.io/tips/11
template <typename T>
std::vector<T> RetrieveCsvQuotePlainData(const T* src_data,
                                         const uint32_t element_num,
                                         const uint32_t anonce) {
  size_t data_size = sizeof(T) * element_num;
  YACL_ENFORCE_EQ(
      data_size % sizeof(uint32_t), size_t(0),
      "Data size in bytes must be times of sizeof(uint32_t), but got {}",
      data_size);

  std::vector<T> dst_data(element_num);
  for (size_t i = 0; i < data_size / sizeof(uint32_t); i++) {
    reinterpret_cast<uint32_t*>(dst_data.data())[i] =
        reinterpret_cast<const uint32_t*>(src_data)[i] ^ anonce;
  }

  return dst_data;
}

}  // namespace

void CsvAttestationVerifier::Init() {
  YACL_ENFORCE_EQ(
      report_.str_report_version(), trustflow::attestation::kReportVersion,
      "Ua report version not match, expect {}, got {}",
      trustflow::attestation::kReportVersion, report_.str_report_version());

  YACL_ENFORCE_EQ(report_.str_report_type(),
                  trustflow::attestation::ReportType::kReportTypePassport,
                  "Unsupport Ua report type: {}, only {} is supported",
                  report_.str_report_type(),
                  trustflow::attestation::ReportType::kReportTypePassport);

  YACL_ENFORCE_EQ(report_.str_tee_platform(),
                  trustflow::attestation::Platform::kPlatformCsv,
                  "Ua report platform not match, expect {}, got {}",
                  trustflow::attestation::Platform::kPlatformCsv,
                  report_.str_tee_platform());

  secretflowapis::v2::sdc::HygonCsvReport hygon_csv_report;
  JSON2PB(report_.json_report(), &hygon_csv_report);

  // Parse HygonCsvReport
  auto quote_bytes =
      cppcodec::base64_rfc4648::decode(hygon_csv_report.b64_quote());
  YACL_ENFORCE_EQ(quote_bytes.size(), sizeof(csv_attestation_report),
                  "Csv quote size err, expect {}, got {}",
                  sizeof(csv_attestation_report), quote_bytes.size());
  memcpy(&quote_, quote_bytes.data(), sizeof(csv_attestation_report));

  secretflowapis::v2::sdc::HygonCsvCertChain hygon_csv_cert_chain;
  JSON2PB(hygon_csv_report.json_cert_chain(), &hygon_csv_cert_chain);
  auto hsk_cert_bytes =
      cppcodec::base64_rfc4648::decode(hygon_csv_cert_chain.b64_hsk_cert());
  YACL_ENFORCE_EQ(hsk_cert_bytes.size(), sizeof(CHIP_ROOT_CERT_t),
                  "Csv hsk cert size err, expect {}, got {}",
                  sizeof(CHIP_ROOT_CERT_t), hsk_cert_bytes.size());
  memcpy(&hsk_cert_, hsk_cert_bytes.data(), sizeof(CHIP_ROOT_CERT_t));

  auto cek_cert_bytes =
      cppcodec::base64_rfc4648::decode(hygon_csv_cert_chain.b64_cek_cert());
  YACL_ENFORCE_EQ(cek_cert_bytes.size(), sizeof(CSV_CERT_t),
                  "Csv cek cert size err, expect {}, got {}",
                  sizeof(CSV_CERT_t), cek_cert_bytes.size());
  memcpy(&cek_cert_, cek_cert_bytes.data(), sizeof(CSV_CERT_t));
}

void CsvAttestationVerifier::VerifyPlatform() {
  VerifyHskCertWithHrkPubkey(hsk_cert_);

  VerifyCekCertWithHskCert(cek_cert_, hsk_cert_);

  auto pek_cert_vec =
      RetrieveCsvQuotePlainData<CSV_CERT_t>(&quote_.pek_cert, 1, quote_.anonce);

  VerifyPekCertWithCekCert(pek_cert_vec.at(0), cek_cert_);

  VerifyQuoteSignature(quote_, pek_cert_vec.at(0));

  SPDLOG_INFO("Csv report's platform verification passed!");
}

void CsvAttestationVerifier::ParseUnifiedReport(
    secretflowapis::v2::sdc::UnifiedAttestationAttributes& attrs) {
  attrs.set_str_tee_platform(report_.str_tee_platform());

  auto vm_id = RetrieveCsvQuotePlainData<uint8_t>(quote_.vm_id, VM_ID_SIZE,
                                                  quote_.anonce);
  attrs.set_hex_prod_id(absl::BytesToHexString(
      absl::string_view(reinterpret_cast<char*>(vm_id.data()), vm_id.size())));

  auto vm_version = RetrieveCsvQuotePlainData<uint8_t>(
      quote_.vm_version, VM_VERSION_SIZE, quote_.anonce);
  attrs.set_hex_platform_sw_version(absl::BytesToHexString(absl::string_view(
      reinterpret_cast<char*>(vm_version.data()), vm_version.size())));

  // we get the first 32 bytes as user_data and the last 32 bytes as
  // public_key_hash
  auto user_data = RetrieveCsvQuotePlainData<uint8_t>(quote_.user_data,
                                                      HASH_LEN, quote_.anonce);
  attrs.set_hex_user_data(absl::BytesToHexString(absl::string_view(
      reinterpret_cast<char*>(user_data.data()), user_data.size())));

  auto public_key_hash = RetrieveCsvQuotePlainData<uint8_t>(
      quote_.user_data + HASH_LEN, HASH_LEN, quote_.anonce);
  attrs.set_hex_hash_or_pem_pubkey(absl::BytesToHexString(
      absl::string_view(reinterpret_cast<char*>(public_key_hash.data()),
                        public_key_hash.size())));

  auto mnonce = RetrieveCsvQuotePlainData<uint8_t>(
      quote_.mnonce, GUEST_ATTESTATION_NONCE_SIZE, quote_.anonce);
  attrs.set_hex_nonce(absl::BytesToHexString(absl::string_view(
      reinterpret_cast<char*>(mnonce.data()), mnonce.size())));

  auto measure_vec = RetrieveCsvQuotePlainData<hash_block_t>(&quote_.measure, 1,
                                                             quote_.anonce);
  attrs.set_hex_boot_measurement(absl::BytesToHexString(
      absl::string_view(reinterpret_cast<char*>(&(measure_vec.at(0))),
                        sizeof(measure_vec.at(0)))));

  uint32_t policy = quote_.policy ^ quote_.anonce;
  attrs.set_hex_secure_flags(absl::BytesToHexString(
      absl::string_view(reinterpret_cast<char*>(&policy), sizeof(uint32_t))));

  // Export the higher 32 bytes as public key hash
  // HASH_LEN defined in hygon attestation.h
  attrs.set_hex_hash_or_pem_pubkey(absl::BytesToHexString(absl::string_view(
      (reinterpret_cast<char*>(user_data.data())) + HASH_LEN, HASH_LEN)));
}

}  // namespace verification
}  // namespace attestation
}  // namespace trustflow