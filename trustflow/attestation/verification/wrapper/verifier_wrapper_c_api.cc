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

#include "trustflow/attestation/verification/wrapper/verifier_wrapper_c_api.h"

#include "trustflow/attestation/common/constants.h"
#include "trustflow/attestation/common/status.h"
#include "trustflow/attestation/utils/json2pb.h"
#include "trustflow/attestation/verification/csv/csv_verifier.h"
#include "trustflow/attestation/verification/sgx2/sgx2_verifier.h"
#include "trustflow/attestation/verification/tdx/tdx_verifier.h"
#include "trustflow/attestation/verification/verifier_factory.h"

#include "secretflowapis/v2/sdc/ual.pb.h"

#ifdef __cplusplus
extern "C" {
#endif

unsigned int GetAttributesMaxSize() {
  return trustflow::attestation::kAttributeMaxSize;
}

std::unique_ptr<trustflow::attestation::verification::VerifierFactory>
CreateFactory() {
  std::unique_ptr<trustflow::attestation::verification::VerifierFactory>
      factory(new trustflow::attestation::verification::VerifierFactory());
  factory->Register(
      trustflow::attestation::Platform::kPlatformTdx,
      &trustflow::attestation::verification::TdxAttestationVerifier::Create);
  factory->Register(
      trustflow::attestation::Platform::kPlatformSgxDcap,
      &trustflow::attestation::verification::Sgx2AttestationVerifier::Create);
  factory->Register(
      trustflow::attestation::Platform::kPlatformCsv,
      &trustflow::attestation::verification::CsvAttestationVerifier::Create);
  return factory;
}

int ParseAttributesFromReport(const char* report_json_c_str,
                              unsigned int report_json_str_len, char* attrs_buf,
                              unsigned int* attrs_buf_len, char* msg_buf,
                              unsigned int* msg_buf_len, char* details_buf,
                              unsigned int* details_buf_len) {
  int code = 0;
  try {
    std::unique_ptr<trustflow::attestation::verification::VerifierFactory>
        factory = CreateFactory();
    std::string report_json_str(report_json_c_str, report_json_str_len);
    secretflowapis::v2::sdc::UnifiedAttestationAttributes attrs;
    factory->Create(report_json_str)->ParseUnifiedReport(attrs);
    // attrs to string
    std::string attrs_json;
    PB2JSON(attrs, &attrs_json);

    YACL_ENFORCE_GE(
        *attrs_buf_len, attrs_json.size(),
        "attrs_buf_len(got {}) should not be less than attrs json size({})",
        *attrs_buf_len, attrs_json.size());
    *attrs_buf_len = attrs_json.size();
    memcpy(attrs_buf, attrs_json.data(), *attrs_buf_len);

  } catch (const yacl::ArgumentError& ex) {
    code = static_cast<int>(trustflow::attestation::ErrorCode::kArgumentError);
    strncpy(msg_buf, ex.what(), *msg_buf_len);
    strncpy(details_buf, ex.stack_trace().c_str(), *details_buf_len);
  } catch (const yacl::InvalidFormat& ex) {
    code = static_cast<int>(trustflow::attestation::ErrorCode::kInvalidFormat);
    strncpy(msg_buf, ex.what(), *msg_buf_len);
    strncpy(details_buf, ex.stack_trace().c_str(), *details_buf_len);
  } catch (const yacl::Exception& ex) {
    code = static_cast<int>(trustflow::attestation::ErrorCode::kInternalError);
    strncpy(msg_buf, ex.what(), *msg_buf_len);
    strncpy(details_buf, ex.stack_trace().c_str(), *details_buf_len);
  } catch (const std::exception& ex) {
    code = static_cast<int>(trustflow::attestation::ErrorCode::kInternalError);
    strncpy(msg_buf, ex.what(), *msg_buf_len);
  }

  // ensure msg end with '\0'
  if (*msg_buf_len > 0) {
    msg_buf[*msg_buf_len - 1] = '\0';
    *msg_buf_len = strlen(msg_buf) + 1;
  }

  // ensure details end with '\0'
  if (*details_buf_len > 0) {
    details_buf[*details_buf_len - 1] = '\0';
    *details_buf_len = strlen(details_buf) + 1;
  }

  return code;
}

int AttestationReportVerify(const char* report_json_c_str,
                            unsigned int report_json_str_len,
                            const char* policy_json_c_str,
                            unsigned int policy_json_str_len, char* msg_buf,
                            unsigned int* msg_buf_len, char* details_buf,
                            unsigned int* details_buf_len) {
  int code = 0;
  try {
    std::unique_ptr<trustflow::attestation::verification::VerifierFactory>
        factory = CreateFactory();
    std::string report_json_str(report_json_c_str, report_json_str_len);
    std::string policy_json_str(policy_json_c_str, policy_json_str_len);
    secretflowapis::v2::sdc::UnifiedAttestationPolicy policy;
    JSON2PB(policy_json_str, &policy);
    factory->Create(report_json_str)->VerifyReport(policy);
  } catch (const yacl::ArgumentError& ex) {
    code = static_cast<int>(trustflow::attestation::ErrorCode::kArgumentError);
    strncpy(msg_buf, ex.what(), *msg_buf_len);
    strncpy(details_buf, ex.stack_trace().c_str(), *details_buf_len);
  } catch (const yacl::InvalidFormat& ex) {
    code = static_cast<int>(trustflow::attestation::ErrorCode::kInvalidFormat);
    strncpy(msg_buf, ex.what(), *msg_buf_len);
    strncpy(details_buf, ex.stack_trace().c_str(), *details_buf_len);
  } catch (const yacl::Exception& ex) {
    code = static_cast<int>(trustflow::attestation::ErrorCode::kInternalError);
    strncpy(msg_buf, ex.what(), *msg_buf_len);
    strncpy(details_buf, ex.stack_trace().c_str(), *details_buf_len);
  } catch (const std::exception& ex) {
    code = static_cast<int>(trustflow::attestation::ErrorCode::kInternalError);
    strncpy(msg_buf, ex.what(), *msg_buf_len);
  }

  // ensure msg end with '\0'
  if (*msg_buf_len > 0) {
    msg_buf[*msg_buf_len - 1] = '\0';
    *msg_buf_len = strlen(msg_buf) + 1;
  }

  // ensure details end with '\0'
  if (*details_buf_len > 0) {
    details_buf[*details_buf_len - 1] = '\0';
    *details_buf_len = strlen(details_buf) + 1;
  }

  return code;
}

#ifdef __cplusplus
}
#endif