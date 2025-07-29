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

#include "trustflow/proxy/utils/crypto_util.h"

#include <filesystem>
#include <future>

#include "absl/strings/ascii.h"
#include "cppcodec/base32_rfc4648_unpadded.hpp"
#include "openssl/pem.h"
#include "yacl/crypto/key_utils.h"
#include "yacl/io/stream/file_io.h"

namespace trustflow {
namespace proxy {
namespace utils {

namespace {

// Header:
//  Version: 4 bytes
//  Schema: 4 bytes
//  Packet count: 8 bytes
//  Block length: 4 bytes
constexpr uint32_t kVersion = 1;
constexpr uint32_t kSchema = 1;
constexpr size_t kVersionBytes = sizeof(kVersion);
constexpr size_t kSchemaBytes = sizeof(kSchema);

constexpr uint32_t kBlockBytes = 0x2000;
constexpr size_t kPacketCntBytes = sizeof(uint64_t);
constexpr size_t kBlockLenBytes = sizeof(kBlockBytes);

// Reserve 32 bytes for IV and MAC, the actual used bytes should be inferred
// from IV length and MAC length fields.
// Data block:
//  IV length: 1 byte
//  MAC length: 1 byte
//  IV: 32 bytes
//  MAC: 32 bytes

// define next two const in crypto_util.h
//  constexpr uint8_t kIvBytes = 12;
//  constexpr uint8_t kMacBytes = 16;
constexpr uint8_t kIvFieldBytes = 32;
constexpr uint8_t kMacFieldBytes = 32;
constexpr uint8_t kAes128KeyLen = 16;
constexpr uint8_t kAes256KeyLen = 32;
constexpr size_t kIvLenBytes = sizeof(kIvBytes);
constexpr size_t kMacLenBytes = sizeof(kMacBytes);

constexpr size_t kBufSize = 4096;

constexpr char kEncSuffix[] = ".enc";

// Step 1: parse data block header
// Step 2: decrypt data and return
std::vector<uint8_t> DecryptDataBlock(yacl::ByteContainerView data_block,
                                      yacl::ByteContainerView data_key) {
  YACL_ENFORCE_GE(data_block.size(), kIvLenBytes,
                  "Data block format is not correct");
  // parse iv length
  uint64_t offset = 0;
  uint64_t iv_len =
      Bytes2Int<uint64_t>(data_block.subspan(offset, kIvLenBytes));
  offset += kIvLenBytes;

  // get iv
  YACL_ENFORCE_GE(data_block.size(), offset + kIvFieldBytes,
                  "Data block format is not correct");
  yacl::ByteContainerView iv = data_block.subspan(offset, iv_len);
  offset += kIvFieldBytes;

  // parse mac length
  YACL_ENFORCE_GE(data_block.size(), offset + kMacLenBytes,
                  "Data block format is not correct");
  uint64_t mac_len =
      Bytes2Int<uint64_t>(data_block.subspan(offset, kMacLenBytes));
  offset += kMacLenBytes;

  // get mac
  YACL_ENFORCE_GE(data_block.size(), offset + kMacFieldBytes,
                  "Data block format is not correct");
  yacl::ByteContainerView mac = data_block.subspan(offset, mac_len);
  offset += kMacFieldBytes;

  // get data
  yacl::ByteContainerView encrypted_data = data_block.subspan(offset);

  // decrypt data
  std::vector<uint8_t> raw_data(encrypted_data.size());

  if (data_key.size() == kAes128KeyLen) {
    yacl::crypto::Aes128GcmCrypto(data_key, iv)
        .Decrypt(encrypted_data, "", mac, absl::Span<uint8_t>(raw_data));
  } else if (data_key.size() == kAes256KeyLen) {
    yacl::crypto::Aes256GcmCrypto(data_key, iv)
        .Decrypt(encrypted_data, "", mac, absl::Span<uint8_t>(raw_data));
  } else {
    YACL_THROW("data_key size error got {}", data_key.size());
  }

  return raw_data;
}

void EncryptDataBlock(yacl::io::FileInputStream& in,
                      yacl::io::FileOutputStream& out, uint32_t block_len,
                      yacl::ByteContainerView data_key) {
  // get iv mac data
  std::vector<uint8_t> raw_data(block_len);
  in.Read(&raw_data[0], raw_data.size());

  auto iv = yacl::crypto::RandBytes(kIvBytes, true);
  std::vector<uint8_t> mac(kMacBytes);
  std::vector<uint8_t> encrypted_data(block_len);

  if (data_key.size() == kAes128KeyLen) {
    yacl::crypto::Aes128GcmCrypto(data_key, iv)
        .Encrypt(raw_data, "", absl::Span<uint8_t>(encrypted_data),
                 absl::Span<uint8_t>(mac));
  } else if (data_key.size() == kAes256KeyLen) {
    yacl::crypto::Aes256GcmCrypto(data_key, iv)
        .Encrypt(raw_data, "", absl::Span<uint8_t>(encrypted_data),
                 absl::Span<uint8_t>(mac));
  } else {
    YACL_THROW("data_key size error got {}", data_key.size());
  }

  // write iv
  out.Write(&kIvBytes, kIvLenBytes);
  out.Write(iv.data(), iv.size());
  // padding iv
  std::vector<uint8_t> iv_padding(kIvFieldBytes - iv.size());
  out.Write(&iv_padding[0], iv_padding.size());
  // write mac
  out.Write(&kMacBytes, kMacLenBytes);
  out.Write(mac.data(), mac.size());
  // padding mac
  std::vector<uint8_t> mac_padding(kMacFieldBytes - mac.size());
  out.Write(&mac_padding[0], mac_padding.size());
  // write encrypted data
  out.Write(encrypted_data.data(), encrypted_data.size());
}

}  // namespace

using UniqueBio = std::unique_ptr<BIO, decltype(&BIO_free)>;
using UniqueX509 = std::unique_ptr<X509, decltype(&X509_free)>;
using UniqueEVP = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>;

std::vector<uint8_t> X509CertPemToDer(const std::string& pem_cert) {
  UniqueBio cert_bio(BIO_new_mem_buf(pem_cert.data(), pem_cert.size()),
                     BIO_free);
  YACL_ENFORCE(cert_bio != nullptr, "cert_bio is null");
  UniqueX509 cert(PEM_read_bio_X509(cert_bio.get(), nullptr, nullptr, nullptr),
                  X509_free);
  YACL_ENFORCE(cert != nullptr, "cert is null");

  UniqueBio der_bio(BIO_new(BIO_s_mem()), BIO_free);
  YACL_ENFORCE(der_bio != nullptr, "der_bio is null");
  YACL_ENFORCE(i2d_X509_bio(der_bio.get(), cert.get()),
               "Failed to write DER certificate.");

  std::vector<uint8_t> der_data;
  char buffer[kBufSize];
  int bytesRead;
  while ((bytesRead = BIO_read(der_bio.get(), buffer, sizeof(buffer))) > 0) {
    der_data.insert(der_data.end(), buffer, buffer + bytesRead);
  }
  return der_data;
}

std::string GeneratePartyId(const std::string& pem_cert) {
  auto public_key = yacl::crypto::LoadX509CertPublicKeyFromBuf(pem_cert);
  auto public_key_der = yacl::crypto::ExportPublicKeyToDerBuf(public_key);

  return cppcodec::base32_rfc4648_unpadded::encode(
      yacl::crypto::Sha256(public_key_der));
}

// Decrypt a file from src_path to dest_path
// Step 1: parse file header from src_path
// Step 2: read data block from src_path
// Step 3: decrypt data block
// Step 4: write data block to src_path
void DecryptFile(const std::string& src_path, const std::string& dest_path,
                 yacl::ByteContainerView data_key) {
  SPDLOG_INFO("Decrypting {} to {}", src_path, dest_path);
  YACL_ENFORCE_NE(src_path, dest_path, "Inplace decryption is not allowed");

  yacl::io::FileInputStream in(src_path);
  yacl::io::FileOutputStream out(dest_path);

  // parse file header
  auto file_len = in.GetLength();
  auto header_len =
      kVersionBytes + kSchemaBytes + kPacketCntBytes + kBlockLenBytes;
  YACL_ENFORCE_GT(file_len, header_len,
                  "File length {} is less than required header length {}",
                  file_len, header_len);
  // skip version and schema
  in.Seekg(kVersionBytes + kSchemaBytes);

  // read packet count
  std::vector<uint8_t> buf(kPacketCntBytes);
  in.Read(&buf[0], buf.size());
  uint64_t packet_cnt = Bytes2Int<uint64_t>(buf);
  YACL_ENFORCE_GE(packet_cnt, 1u, "Packet cnt is less than 1");

  // read block len
  buf.resize(kBlockLenBytes);
  in.Read(&buf[0], buf.size());
  uint32_t block_len = Bytes2Int<uint32_t>(buf);

  // avoid mul overflow
  YACL_ENFORCE(block_len != 0, "block len should not be 0");
  YACL_ENFORCE_EQ((packet_cnt - 1) * block_len / block_len, (packet_cnt - 1),
                  "uint64 overflow in DecryptFile");
  YACL_ENFORCE_EQ(packet_cnt * block_len / block_len, packet_cnt,
                  "uint64 overflow in DecryptFile");

  // check length
  YACL_ENFORCE_GE(file_len - header_len, (packet_cnt - 1) * block_len,
                  "N - 1 Data block len is more than required file length");
  YACL_ENFORCE_GE(block_len * packet_cnt, file_len - header_len,
                  "N Data block len is less than required file length");

  // read 1 ~ (n - 1) data block
  buf.resize(block_len);
  for (uint64_t i = 0; i < packet_cnt - 1; ++i) {
    in.Read(&buf[0], buf.size());
    auto decrypted_data = DecryptDataBlock(buf, data_key);
    out.Write(decrypted_data.data(), decrypted_data.size());
  }

  // read last data block
  buf.resize(file_len - header_len - (packet_cnt - 1) * block_len);
  in.Read(&buf[0], buf.size());
  auto decrypted_data = DecryptDataBlock(buf, data_key);
  out.Write(decrypted_data.data(), decrypted_data.size());

  // close file
  out.Close();
  in.Close();

  SPDLOG_INFO("Decrypt {} to {} success", src_path, dest_path);
}

void DecryptToDir(const std::string& src_path, const std::string& dest_path,
                  yacl::ByteContainerView data_key) {
  YACL_ENFORCE(std::filesystem::exists(src_path), "src_path {} not exists",
               src_path);

  std::filesystem::path dest_object_path;
  if (std::filesystem::is_regular_file(src_path)) {
    dest_object_path = std::filesystem::path(dest_path) /
                       std::filesystem::path(src_path).filename();
    if (!std::filesystem::exists(dest_object_path.parent_path())) {
      std::filesystem::create_directories(dest_object_path.parent_path());
    }
    if (std::filesystem::path(src_path).extension() == kEncSuffix) {
      dest_object_path.replace_extension("");
      SPDLOG_INFO("Decrypting {} to {}", src_path, dest_object_path.string());
      DecryptFile(src_path, dest_object_path, data_key);
      SPDLOG_INFO("Decrypt {} to {} success", src_path,
                  dest_object_path.string());
    } else {
      SPDLOG_INFO("Coping {} without .enc to {}", src_path,
                  dest_object_path.string());
      std::filesystem::copy(src_path, dest_object_path,
                            std::filesystem::copy_options::overwrite_existing);
      SPDLOG_INFO("Copy {} to {} success", src_path, dest_object_path.string());
    }
  } else if (std::filesystem::is_directory(src_path)) {
    std::vector<std::future<void>> futures;
    for (const auto& src_item :
         std::filesystem::recursive_directory_iterator(src_path)) {
      if (std::filesystem::is_regular_file(src_item.path())) {
        std::filesystem::path relative_path =
            std::filesystem::relative(src_item.path(), src_path);

        dest_object_path = std::filesystem::path(dest_path / relative_path);
        if (!std::filesystem::exists(dest_object_path.parent_path())) {
          std::filesystem::create_directories(dest_object_path.parent_path());
        }
        if (src_item.path().extension() == kEncSuffix) {
          dest_object_path.replace_extension("");

          futures.emplace_back(std::async(std::launch::async, DecryptFile,
                                          src_item.path().string(),
                                          dest_object_path, data_key));
        } else {
          // copy files without .enc (not need to decrypt)
          SPDLOG_INFO("Coping {} without .enc to {}", src_item.path().string(),
                      dest_object_path.string());
          std::filesystem::copy(
              src_item.path().string(), dest_object_path,
              std::filesystem::copy_options::overwrite_existing);
          SPDLOG_INFO("Copy {} to {} success", src_item.path().string(),
                      dest_object_path.string());
        }
      }
    }
    for (auto& future : futures) {
      future.get();
    }
  } else {
    YACL_THROW("src_path {} is not a file or directory", src_path);
  }
}

// Encrypt a file from src_path to dest_path
// Step 1: read raw data from from src_path
// Step 2: encrypt raw data
// Step 3: write header to dest_path file
// Step 4: write data block to dest_path file
void EncryptFile(const std::string& src_path, const std::string& dest_path,
                 yacl::ByteContainerView data_key) {
  SPDLOG_INFO("Encrypting {} to {}", src_path, dest_path);
  YACL_ENFORCE_NE(src_path, dest_path, "Inplace encryption is not allowed");

  // read raw data
  yacl::io::FileInputStream in(src_path);
  auto file_len = in.GetLength();
  uint32_t block_header_len =
      kIvLenBytes + kIvFieldBytes + kMacLenBytes + kMacFieldBytes;
  uint32_t block_data_len = kBlockBytes - block_header_len;
  uint64_t packet_cnt =
      file_len / block_data_len + (file_len % block_data_len != 0);
  YACL_ENFORCE_GE(packet_cnt, 1u, "Pack cnt less than 1");

  // write file header
  yacl::io::FileOutputStream out(dest_path);
  out.Write(reinterpret_cast<const char*>(&kVersion), kVersionBytes);
  out.Write(reinterpret_cast<const char*>(&kSchema), kSchemaBytes);
  out.Write(reinterpret_cast<const char*>(&packet_cnt), kPacketCntBytes);
  out.Write(reinterpret_cast<const char*>(&kBlockBytes), kBlockLenBytes);

  // block from 1 to pack_cnt - 1
  for (uint64_t i = 0; i < packet_cnt - 1; ++i) {
    EncryptDataBlock(in, out, block_data_len, data_key);
  }
  // the last block
  EncryptDataBlock(in, out, file_len - (packet_cnt - 1) * block_data_len,
                   data_key);

  out.Close();
  in.Close();
  SPDLOG_INFO("Encrypt {} to {} success", src_path, dest_path);
}

void EncryptToDir(const std::string& src_path, const std::string& dest_path,
                  yacl::ByteContainerView data_key) {
  YACL_ENFORCE(std::filesystem::exists(src_path), "src_path {} not exists",
               src_path);
  std::filesystem::path dest_object_path;

  if (std::filesystem::is_regular_file(src_path)) {
    dest_object_path = (std::filesystem::path(dest_path) /
                        std::filesystem::path(src_path).filename())
                           .concat(kEncSuffix);
    if (!std::filesystem::exists(dest_object_path.parent_path())) {
      std::filesystem::create_directories(dest_object_path.parent_path());
    }
    EncryptFile(src_path, dest_object_path, data_key);
  } else if (std::filesystem::is_directory(src_path)) {
    std::vector<std::future<void>> futures;
    for (const auto& src_item :
         std::filesystem::recursive_directory_iterator(src_path)) {
      if (std::filesystem::is_regular_file(src_item.path())) {
        std::filesystem::path relative_path =
            std::filesystem::relative(src_item.path(), src_path);

        // for files in directory
        dest_object_path =
            std::filesystem::path(dest_path / relative_path).concat(kEncSuffix);
        if (!std::filesystem::exists(dest_object_path.parent_path())) {
          std::filesystem::create_directories(dest_object_path.parent_path());
        }
        futures.emplace_back(std::async(std::launch::async, EncryptFile,
                                        src_item.path(), dest_object_path,
                                        data_key));
      }
    }
    for (auto& future : futures) {
      future.get();
    }
  } else {
    YACL_THROW("src_path {} is not a file or directory", src_path);
  }
}

}  // namespace utils
}  // namespace proxy
}  // namespace trustflow
