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

#include "yacl/base/buffer.h"

#include "trustflow/attestation/verification/interface/verifier.h"

namespace trustflow {
namespace attestation {
namespace verification {

class TdxAttestationVerifier : public AttestationVerifier {
 public:
  explicit TdxAttestationVerifier(const std::string& report_json_str)
      : AttestationVerifier(report_json_str) {
    Init();
  }

  explicit TdxAttestationVerifier(
      const secretflowapis::v2::sdc::UnifiedAttestationReport& report)
      : AttestationVerifier(report) {
    Init();
  }

  void ParseUnifiedReport(
      secretflowapis::v2::sdc::UnifiedAttestationAttributes& attrs) override;

  void VerifyPlatform() override;

  static std::unique_ptr<AttestationVerifier> Create(
      const secretflowapis::v2::sdc::UnifiedAttestationReport& report) {
    return std::make_unique<TdxAttestationVerifier>(report);
  }

 protected:
  void Init();

 private:
  std::vector<uint8_t> quote_;
  secretflowapis::v2::sdc::SgxQlQveCollateral collateral_;
};

}  // namespace verification
}  // namespace attestation
}  // namespace trustflow