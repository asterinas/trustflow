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

#include "trustflow/attestation/verification/wrapper/verifier_wrapper.h"

#include "trustflow/attestation/common/constants.h"
#include "trustflow/attestation/utils/json2pb.h"
#include "trustflow/attestation/verification/csv/csv_verifier.h"
#include "trustflow/attestation/verification/sgx2/sgx2_verifier.h"
#include "trustflow/attestation/verification/tdx/tdx_verifier.h"
#include "trustflow/attestation/verification/verifier_factory.h"

#include "secretflowapis/v2/sdc/ual.pb.h"

namespace trustflow {
namespace attestation {
namespace verification {

trustflow::attestation::Status AttestationReportVerify(
    const std::string& report_json_str, const std::string& policy_json_str) {
  secretflowapis::v2::sdc::UnifiedAttestationPolicy policy;

  trustflow::attestation::Status status = {0, "success", ""};
  try {
    VerifierFactory factory;
    factory.Register(Platform::kPlatformTdx, &TdxAttestationVerifier::Create);
    factory.Register(Platform::kPlatformSgxDcap,
                     &Sgx2AttestationVerifier::Create);
    factory.Register(Platform::kPlatformCsv, &CsvAttestationVerifier::Create);

    JSON2PB(policy_json_str, &policy);
    factory.Create(report_json_str)->VerifyReport(policy);
  } catch (const yacl::ArgumentError& ex) {
    status.code =
        static_cast<int>(trustflow::attestation::ErrorCode::kArgumentError);
    status.message = ex.what();
    status.details = ex.stack_trace();
  } catch (const yacl::InvalidFormat& ex) {
    status.code =
        static_cast<int>(trustflow::attestation::ErrorCode::kInvalidFormat);
    status.message = ex.what();
    status.details = ex.stack_trace();
  } catch (const yacl::Exception& ex) {
    status.code =
        static_cast<int>(trustflow::attestation::ErrorCode::kInternalError);
    status.message = ex.what();
    status.details = ex.stack_trace();
  } catch (const std::exception& ex) {
    status.code =
        static_cast<int>(trustflow::attestation::ErrorCode::kInternalError);
    status.message = ex.what();
  }
  return status;
}

}  // namespace verification
}  // namespace attestation
}  // namespace trustflow