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

#include <algorithm>

#include "absl/strings/escaping.h"
#include "yacl/base/exception.h"

#include "trustflow/attestation/verification/interface/verifier.h"

namespace trustflow {
namespace attestation {
namespace verification {

class VerifierFactory {
 public:
  VerifierFactory() {}
  VerifierFactory(const VerifierFactory &) = delete;
  VerifierFactory &operator=(const VerifierFactory &) = delete;

  std::unique_ptr<AttestationVerifier> Create(
      const std::string &report_json_str) {
    secretflowapis::v2::sdc::UnifiedAttestationReport report;
    JSON2PB(report_json_str, &report);
    const std::string &name = report.str_tee_platform();

    std::vector<std::string> supported_platforms(creators_.size());
    std::transform(creators_.begin(), creators_.end(),
                   supported_platforms.begin(),
                   [](const auto &pair) { return pair.first; });

    YACL_ENFORCE(creators_.count(name) > 0,
                 "Supported platform list: {}, but not include {}",
                 fmt::join(supported_platforms, ", "), name);
    return creators_[name](report);
  }

  void Register(
      const std::string &name,
      std::function<std::unique_ptr<AttestationVerifier>(
          const secretflowapis::v2::sdc::UnifiedAttestationReport &report)>
          &&creator) {
    creators_[name] = std::move(creator);
  }

 protected:
  std::unordered_map<
      std::string,
      std::function<std::unique_ptr<AttestationVerifier>(
          const secretflowapis::v2::sdc::UnifiedAttestationReport &report)>>
      creators_;
};

}  // namespace verification
}  // namespace attestation
}  // namespace trustflow