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

#pragma once

#include "trustflow/attestation/utils/json2pb.h"

#include "secretflowapis/v2/sdc/ual.pb.h"

namespace trustflow {
namespace attestation {
namespace verification {

class AttestationVerifier {
 public:
  virtual ~AttestationVerifier() = default;

  AttestationVerifier(const std::string& report_json_str) {
    JSON2PB(report_json_str, &report_);
  }

  AttestationVerifier(
      const secretflowapis::v2::sdc::UnifiedAttestationReport& report)
      : report_(report) {}

  /**
   * @brief Parse report to UnifiedAttestationAttributes
   *
   * @param attrs
   */
  virtual void ParseUnifiedReport(
      secretflowapis::v2::sdc::UnifiedAttestationAttributes& attrs) = 0;

  /**
   * @brief Verify platform information
   *
   */
  virtual void VerifyPlatform() = 0;

  static void VerifyAttributes(
      const secretflowapis::v2::sdc::UnifiedAttestationAttributes& actual_attrs,
      const secretflowapis::v2::sdc::UnifiedAttestationPolicy& policy);

  virtual void VerifyReport(
      const secretflowapis::v2::sdc::UnifiedAttestationPolicy& policy);

 protected:
  secretflowapis::v2::sdc::UnifiedAttestationReport report_;
};

}  // namespace verification
}  // namespace attestation
}  // namespace trustflow
