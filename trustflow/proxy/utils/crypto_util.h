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

#pragma once

#include <string>
#include <vector>

#include "cppcodec/base64_rfc4648.hpp"
#include "cppcodec/base64_url_unpadded.hpp"
#include "spdlog/spdlog.h"
#include "src/google/protobuf/util/json_util.h"
#include "yacl/crypto/aead/gcm_crypto.h"
#include "yacl/crypto/aead/sm4_mac.h"
#include "yacl/crypto/block_cipher/symmetric_crypto.h"
#include "yacl/crypto/hmac/hmac_sha256.h"
#include "yacl/crypto/pke/asymmetric_crypto.h"
#include "yacl/crypto/pke/asymmetric_rsa_crypto.h"
#include "yacl/crypto/pke/asymmetric_sm2_crypto.h"
#include "yacl/crypto/rand/rand.h"
#include "yacl/crypto/sign/rsa_signing.h"

#include "secretflowapis/v2/sdc/capsule_manager/capsule_manager.pb.h"

namespace trustflow {
namespace proxy {
namespace utils {

constexpr char kRs256[] = "RS256";
constexpr char kRsaOaep[] = "RSA-OAEP";
constexpr char kAes128Gcm[] = "A128GCM";

constexpr uint8_t kIvBytes = 12;
constexpr uint8_t kMacBytes = 16;
constexpr uint8_t kContentKeyBytes = 16;
const std::string kJwsConcatDelimiter = ".";

// Convert byte array to int
template <typename T>
T Bytes2Int(yacl::ByteContainerView bytes) {
  size_t len = bytes.size();
  YACL_ENFORCE_LE(len, sizeof(T), "Converting bytes to integer overflow");
  T ret = 0;
  // Little-endian
  for (size_t i = 0; i < len; i++) {
    ret = (ret << 8 | bytes[len - i - 1]);
  }
  return ret;
}

// Decrypt a ciphertext file at src_path to a plaintext file at dest_path
// with data_key
void DecryptFile(const std::string& src_path, const std::string& dest_path,
                 yacl::ByteContainerView data_key);

void DecryptToDir(const std::string& src_path, const std::string& dest_path,
                  yacl::ByteContainerView data_key);

// Encrypt a plaintext file at src_path to a ciphertext file at dest_path
// with data_key
void EncryptFile(const std::string& src_path, const std::string& dest_path,
                 yacl::ByteContainerView data_key);

void EncryptToDir(const std::string& src_path, const std::string& dest_path,
                  yacl::ByteContainerView data_key);

std::vector<uint8_t> X509CertPemToDer(const std::string& pem_cert);

std::string GeneratePartyId(const std::string& pem_cert);

// Generate EncryptedRequest with Jwe inside
// Only support RSA-SHA256 with AES128GCM yet(yacl not support sm2 cert's
// operation)
//  sig_alg = "RS256", key_enc_alg = "RSA-OAEP", content_enc_alg = "A128GCM"
template <typename T>
secretflowapis::v2::sdc::capsule_manager::EncryptedRequest GenEncryptedRequest(
    const T& request, const std::string& private_key, const std::string& cert,
    const std::string& peer_cert, const bool has_signature) {
  ::google::protobuf::util::JsonPrintOptions options;
  options.preserve_proto_field_names = false;
  options.always_print_primitive_fields = true;

  std::string request_str;
  auto pb_status = ::google::protobuf::util::MessageToJsonString(
      request, &request_str, options);
  YACL_ENFORCE(pb_status.ok(),
               "Parsing pb message to json failed, "
               "error: {}",
               pb_status.ToString());

  secretflowapis::v2::sdc::capsule_manager::EncryptedRequest enc_req;
  secretflowapis::v2::RequestHeader header;
  secretflowapis::v2::sdc::Jwe jwe;

  secretflowapis::v2::sdc::Jwe::JoseHeader jwe_header;
  jwe_header.set_alg(kRsaOaep);
  jwe_header.set_enc(kAes128Gcm);
  std::string jwe_header_str;
  pb_status = ::google::protobuf::util::MessageToJsonString(
      jwe_header, &jwe_header_str, options);
  YACL_ENFORCE(pb_status.ok(),
               "Parsing pb message to json failed, "
               "error: {}",
               pb_status.ToString());
  jwe.set_protected_header(
      cppcodec::base64_url_unpadded::encode(jwe_header_str));

  // gen content encryption key
  const auto cek = yacl::crypto::RandBytes(kContentKeyBytes);
  jwe.set_encrypted_key(cppcodec::base64_url_unpadded::encode(
      yacl::crypto::RsaEncryptor(
          yacl::crypto::LoadX509CertPublicKeyFromBuf(peer_cert))
          .Encrypt(cek)));

  const auto iv = yacl::crypto::RandBytes(kIvBytes);
  jwe.set_iv(cppcodec::base64_url_unpadded::encode(iv));

  std::vector<uint8_t> aad;
  const auto aad_b64 = cppcodec::base64_url_unpadded::encode(aad);
  std::vector<uint8_t> tag(kMacBytes);

  if (has_signature) {
    // gen jws
    secretflowapis::v2::sdc::Jws jws;

    secretflowapis::v2::sdc::Jws::JoseHeader jws_header;
    jws_header.set_alg(kRs256);
    std::string cert_der =
        cppcodec::base64_rfc4648::encode(X509CertPemToDer(cert));
    jws_header.add_x5c(cert_der);
    std::string jws_header_str;
    pb_status = ::google::protobuf::util::MessageToJsonString(
        jws_header, &jws_header_str, options);
    YACL_ENFORCE(pb_status.ok(),
                 "Parsing pb message to json failed, "
                 "error: {}",
                 pb_status.ToString());
    jws.set_protected_header(
        cppcodec::base64_url_unpadded::encode(jws_header_str));

    jws.set_payload(cppcodec::base64_url_unpadded::encode(request_str));

    const std::string sign_input =
        jws.protected_header() + kJwsConcatDelimiter + jws.payload();
    const std::vector<uint8_t> sig =
        yacl::crypto::RsaSigner(yacl::crypto::LoadPemKey(private_key))
            .Sign(sign_input);
    jws.set_signature(cppcodec::base64_url_unpadded::encode(sig));

    // Jwe(jws)
    std::string jws_str;
    pb_status =
        ::google::protobuf::util::MessageToJsonString(jws, &jws_str, options);
    YACL_ENFORCE(pb_status.ok(),
                 "Parsing pb message to json failed, "
                 "error: {}",
                 pb_status.ToString());
    std::vector<uint8_t> cipher(jws_str.size());
    yacl::crypto::Aes128GcmCrypto(cek, iv).Encrypt(jws_str, aad_b64,
                                                   absl::Span<uint8_t>(cipher),
                                                   absl::Span<uint8_t>(tag));

    jwe.set_ciphertext(cppcodec::base64_url_unpadded::encode(cipher));
    jwe.set_tag(cppcodec::base64_url_unpadded::encode(tag));
  } else {
    std::vector<uint8_t> cipher(request_str.size());
    yacl::crypto::Aes128GcmCrypto(cek, iv).Encrypt(request_str, aad_b64,
                                                   absl::Span<uint8_t>(cipher),
                                                   absl::Span<uint8_t>(tag));

    jwe.set_ciphertext(cppcodec::base64_url_unpadded::encode(cipher));
    jwe.set_tag(cppcodec::base64_url_unpadded::encode(tag));
  }
  jwe.set_aad(aad_b64);

  *enc_req.mutable_header() = std::move(header);
  enc_req.set_has_signature(has_signature);
  *enc_req.mutable_message() = std::move(jwe);

  return enc_req;
}

template <typename T>
std::tuple<secretflowapis::v2::Status, T> ParseEncryptedResponse(
    const secretflowapis::v2::sdc::capsule_manager::EncryptedResponse& enc_res,
    const std::string& private_key) {
  T response;
  const auto status = enc_res.status();
  if (status.code() != secretflowapis::v2::Code::OK) {
    return std::make_tuple(status, response);
  }
  const auto jwe = enc_res.message();
  secretflowapis::v2::sdc::Jwe::JoseHeader jwe_header;
  auto pb_status = ::google::protobuf::util::JsonStringToMessage(
      cppcodec::base64_url_unpadded::decode<std::string>(
          jwe.protected_header()),
      &jwe_header);
  YACL_ENFORCE(pb_status.ok(),
               "Parsing json to pb message failed, json: {}, "
               "error: {}",
               cppcodec::base64_url_unpadded::decode<std::string>(
                   jwe.protected_header()),
               pb_status.ToString());

  const auto encrypted_key =
      cppcodec::base64_url_unpadded::decode(jwe.encrypted_key());
  const auto iv = cppcodec::base64_url_unpadded::decode(jwe.iv());
  const auto cipher = cppcodec::base64_url_unpadded::decode(jwe.ciphertext());
  const auto tag = cppcodec::base64_url_unpadded::decode(jwe.tag());
  const auto aad = cppcodec::base64_url_unpadded::decode(jwe.aad());

  const auto cek =
      yacl::crypto::RsaDecryptor(yacl::crypto::LoadPemKey(private_key))
          .Decrypt(encrypted_key);

  std::vector<uint8_t> plain(cipher.size());
  yacl::crypto::Aes128GcmCrypto(cek, iv).Decrypt(cipher, aad, tag,
                                                 absl::Span<uint8_t>(plain));

  pb_status = ::google::protobuf::util::JsonStringToMessage(
      std::string(plain.begin(), plain.end()), &response);
  YACL_ENFORCE(pb_status.ok(),
               "Parsing json to pb message failed, json: {}, "
               "error: {}",
               std::string(plain.begin(), plain.end()), pb_status.ToString());
  return std::make_tuple(status, response);
}

}  // namespace utils
}  // namespace proxy
}  // namespace trustflow
