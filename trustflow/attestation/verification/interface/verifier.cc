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

#include "trustflow/attestation/verification/interface/verifier.h"

#include "absl/strings/escaping.h"
#include "absl/strings/match.h"
#include "yacl/crypto/hash/ssl_hash.h"

namespace trustflow {
namespace attestation {
namespace verification {

namespace {

namespace ual = secretflowapis::v2::sdc;

const char kUaAttrPlatform[] = "PLATFORM";
const char kUaAttrPlatformHwVer[] = "PLATFORMHWVERSION";
const char kUaAttrPlatformSwVer[] = "PLATFORMSWVERSION";
const char kUaAttrSecureFlags[] = "SECUREFLAGS";
const char kUaAttrMrplatform[] = "MRPLATFORM";
const char kUaAttrMrboot[] = "MRBOOT";
const char kUaAttrMrTa[] = "MRTRUSTAPP";
const char kUaAttrMrTaDyn[] = "MRTRUSTAPPDYN";
const char kUaAttrSigner[] = "SIGNER";
const char kUaAttrProdID[] = "PRODID";
const char kUaAttrIsvSvn[] = "ISVSVN";
const char kUaAttrDebugDisabled[] = "DEBUGDISABLED";
const char kUaAttrUserData[] = "USERDATA";
const char kUaAttrPublickey[] = "PUBLICKEY";
const char kUaAttrNonce[] = "NONCE";
const char kUaAttrSpid[] = "SPID";

inline bool IsStrEqual(const std::string& name, const std::string& actual_value,
                       const std::string& expected_value, bool required = false,
                       std::string* error_msg = nullptr) {
  // If the expected value is equal to the actual value, or if the expected
  // value is empty and the field does not require strict validation, then it is
  // considered to be passed.
  bool result = (expected_value.empty() && !required) ||
                absl::EqualsIgnoreCase(expected_value, actual_value);

  // If a mismatch occurs and there is a need to return the error message,
  // then，
  if (!result && error_msg != nullptr) {
    *error_msg = fmt::format("{} is not match: actual {} vs expected {}.", name,
                             actual_value, expected_value);
  }
  YACL_ENFORCE(result, "{} is not match: actual {} vs expected {}.", name,
               actual_value, expected_value);
  return result;
}

inline bool StrToBoolean(const std::string& bool_str) {
  return (absl::EqualsIgnoreCase(bool_str, "true") || (bool_str == "1"))
             ? true
             : false;
}

inline bool IsBoolEqual(const std::string& name,
                        const std::string& actual_value,
                        const std::string& expected_value,
                        bool required = false,
                        std::string* error_msg = nullptr) {
  // If the expected value is equal to the actual value, or if the expected
  // value is empty and the field does not require strict validation, then it is
  // considered to be passed.
  bool result = (expected_value.empty() && !required) ||
                StrToBoolean(expected_value) == StrToBoolean(actual_value);

  // If a mismatch occurs and there is a need to return the error message,
  // then，
  if (!result && error_msg != nullptr) {
    *error_msg = fmt::format("{} is not match: actual {} vs expected {}.", name,
                             actual_value, expected_value);
  }
  return result;
}

inline bool IsGreaterEqualThan(const std::string& name,
                               const std::string& actual_value,
                               const std::string& expected_value,
                               bool required = false,
                               std::string* error_msg = nullptr) {
  bool result = false;
  // If the actual value is greater than or equal to the expected value, or if
  // the expected value is empty and the field does not require strict
  // validation, it is considered to pass. However, considering that the
  // parameters might be invalid, `std::stoi` could throw an exception, so it is
  // necessary to catch the exception.
  try {
    result = (expected_value.empty() && !required) ||
             std::stoi(expected_value) <= std::stoi(actual_value);
    // If a mismatch occurs and there is a need to return the error message,
    // then，
    if (!result && error_msg != nullptr) {
      *error_msg = fmt::format(
          "{} is not match: actual {} is not large than expected {}.", name,
          actual_value, expected_value);
    }
  } catch (std::invalid_argument const& ex) {
    *error_msg = fmt::format("std::invalid_argument, actual {} or expected {}",
                             actual_value, expected_value);
    return false;
  } catch (std::out_of_range const& ex) {
    *error_msg = fmt::format("std::out_of_range, actual {} or expected {}",
                             actual_value, expected_value);
    return false;
  }
  return result;
}

bool AttrsIsMatch(const ual::UnifiedAttestationAttributes& actual_attrs,
                  const ual::UnifiedAttestationAttributes& expected_attrs,
                  std::string* error_msg = nullptr) {
  if (!IsStrEqual(kUaAttrPlatform, actual_attrs.str_tee_platform(),
                  expected_attrs.str_tee_platform(), false, error_msg)) {
    return false;
  }

  if (!IsStrEqual(kUaAttrPlatformHwVer, actual_attrs.hex_platform_hw_version(),
                  expected_attrs.hex_platform_hw_version(), false, error_msg)) {
    return false;
  }

  if (!IsStrEqual(kUaAttrPlatformSwVer, actual_attrs.hex_platform_sw_version(),
                  expected_attrs.hex_platform_sw_version(), false, error_msg)) {
    return false;
  }

  if (!IsStrEqual(kUaAttrSecureFlags, actual_attrs.hex_secure_flags(),
                  expected_attrs.hex_secure_flags(), false, error_msg)) {
    return false;
  }

  if (!IsStrEqual(kUaAttrMrplatform, actual_attrs.hex_platform_measurement(),
                  expected_attrs.hex_platform_measurement(), false,
                  error_msg)) {
    return false;
  }

  if (!IsStrEqual(kUaAttrMrboot, actual_attrs.hex_boot_measurement(),
                  expected_attrs.hex_boot_measurement(), false, error_msg)) {
    return false;
  }

  if (!IsStrEqual(kUaAttrMrTa, actual_attrs.hex_ta_measurement(),
                  expected_attrs.hex_ta_measurement(), false, error_msg)) {
    return false;
  }

  if (!IsStrEqual(kUaAttrMrTaDyn, actual_attrs.hex_ta_dyn_measurement(),
                  expected_attrs.hex_ta_dyn_measurement(), false, error_msg)) {
    return false;
  }

  if (!IsStrEqual(kUaAttrSigner, actual_attrs.hex_signer(),
                  expected_attrs.hex_signer(), false, error_msg)) {
    return false;
  }

  if (!IsStrEqual(kUaAttrProdID, actual_attrs.hex_prod_id(),
                  expected_attrs.hex_prod_id(), false, error_msg)) {
    return false;
  }

  if (!IsGreaterEqualThan(kUaAttrIsvSvn, actual_attrs.str_min_isvsvn(),
                          expected_attrs.str_min_isvsvn(), false, error_msg)) {
    return false;
  }

  if (!IsBoolEqual(kUaAttrDebugDisabled, actual_attrs.bool_debug_disabled(),
                   expected_attrs.bool_debug_disabled(), false, error_msg)) {
    return false;
  }

  if (!IsStrEqual(kUaAttrUserData, actual_attrs.hex_user_data(),
                  expected_attrs.hex_user_data(), false, error_msg)) {
    return false;
  }

  if (!expected_attrs.hex_hash_or_pem_pubkey().empty()) {
    auto expected_hash =
        yacl::crypto::SslHash(yacl::crypto::HashAlgorithm::SHA256)
            .Update(expected_attrs.hex_hash_or_pem_pubkey())
            .CumulativeHash();

    std::string expected_hash_hex = absl::BytesToHexString(
        absl::string_view((char*)expected_hash.data(), expected_hash.size()));

    if (!IsStrEqual(kUaAttrPublickey, actual_attrs.hex_hash_or_pem_pubkey(),
                    expected_hash_hex, false, error_msg)) {
      return false;
    }
  }

  if (!IsStrEqual(kUaAttrNonce, actual_attrs.hex_nonce(),
                  expected_attrs.hex_nonce(), false, error_msg)) {
    return false;
  }

  return true;
}

}  // namespace

void AttestationVerifier::VerifyAttributes(
    const ual::UnifiedAttestationAttributes& actual_attrs,
    const ual::UnifiedAttestationPolicy& policy) {
  std::vector<std::string> error_msgs;
  for (int i = 0; i < policy.main_attributes_size(); i++) {
    auto expected_attrs = policy.main_attributes(i);
    std::string error_msg;
    if (AttrsIsMatch(actual_attrs, expected_attrs, &error_msg)) {
      return;
    }
    error_msgs.push_back(fmt::format("#{}: {}", i, error_msg));
  }
  YACL_THROW("Verify attributes failed:\n {}.", fmt::join(error_msgs, "\n"));
}

void AttestationVerifier::VerifyReport(
    const ual::UnifiedAttestationPolicy& policy) {
  VerifyPlatform();
  // Parse report
  ual::UnifiedAttestationAttributes attrs;
  ParseUnifiedReport(attrs);
  VerifyAttributes(attrs, policy);
}

}  // namespace verification
}  // namespace attestation
}  // namespace trustflow