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

#include "trustflow/attestation/generation/interface/generator.h"

#include "trustflow/attestation/utils/json2pb.h"

namespace trustflow {
namespace attestation {
namespace generation {

std::string AttestationGenerator::GenerateReportJson(
    const std::string& gen_params_json) {
  secretflowapis::v2::sdc::UnifiedAttestationGenerationParams gen_params;
  JSON2PB(gen_params_json, &gen_params);

  auto report = GenerateReport(gen_params);

  std::string report_json;
  PB2JSON(report, &report_json);
  return report_json;
}
}  // namespace generation
}  // namespace attestation
}  // namespace trustflow