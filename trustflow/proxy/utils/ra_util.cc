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

#include "trustflow/proxy/utils/ra_util.h"

#include "absl/strings/ascii.h"
#include "absl/strings/escaping.h"
#include "cppcodec/base64_rfc4648.hpp"
#include "spdlog/spdlog.h"
#include "yacl/crypto/hash/hash_utils.h"

#include "trustflow/attestation/common/constants.h"
#include "trustflow/attestation/generation/wrapper/generator_wrapper.h"

namespace trustflow {
namespace proxy {
namespace utils {

secretflowapis::v2::sdc::UnifiedAttestationReport GenRaReport(
    yacl::ByteContainerView user_data) {
  auto generator =
      trustflow::attestation::generation::CreateAttestationGenerator();
  secretflowapis::v2::sdc::UnifiedAttestationGenerationParams gen_params;
  gen_params.set_tee_identity("1");
  gen_params.set_report_type("Passport");
  auto digest = yacl::crypto::Sha256(user_data);
  gen_params.mutable_report_params()->set_hex_user_data(
      absl::BytesToHexString(absl::string_view(
          reinterpret_cast<const char*>(digest.data()), digest.size())));

  return generator->GenerateReport(gen_params);
}

}  // namespace utils
}  // namespace proxy
}  // namespace trustflow
