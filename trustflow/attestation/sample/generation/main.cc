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

#include <string>

#include "absl/strings/ascii.h"
#include "absl/strings/escaping.h"
#include "spdlog/spdlog.h"

#include "trustflow/attestation/common/status.h"
#include "trustflow/attestation/generation/wrapper/generator_wrapper.h"
#include "trustflow/attestation/generation/wrapper/generator_wrapper_c_api.h"
#include "trustflow/attestation/utils/json2pb.h"

#include "secretflowapis/v2/sdc/ual.pb.h"

int main() {
  try {
    // test class api
    auto generator =
        trustflow::attestation::generation::CreateAttestationGenerator();
    secretflowapis::v2::sdc::UnifiedAttestationGenerationParams gen_params;
    gen_params.set_report_type("Passport");
    gen_params.mutable_report_params()->set_hex_user_data(
        absl::BytesToHexString("user_data"));
    auto report = generator->GenerateReport(gen_params);
    std::string report_json;
    PB2JSON(report, &report_json);
    SPDLOG_INFO("report from c++ api: {}, len: {}", report_json,
                report_json.size());

    // test c api
    std::string params_buf;
    PB2JSON(gen_params, &params_buf);
    unsigned int report_len;
    auto code = GetAttestationReportSize(params_buf.c_str(), params_buf.size(),
                                         &report_len);
    YACL_ENFORCE_EQ(code,
                    static_cast<int>(trustflow::attestation::ErrorCode::kOk),
                    "GetAttestationReportSize err: {}", code);

    unsigned int msg_len = 512;
    unsigned int details_len = 4096;
    std::vector<char> report_buf(report_len);
    std::vector<char> msg(msg_len);
    std::vector<char> details(details_len);
    code = GenerateAttestationReport(params_buf.c_str(), params_buf.size(),
                                     report_buf.data(), &report_len, msg.data(),
                                     &msg_len, details.data(), &details_len);
    YACL_ENFORCE_EQ(code,
                    static_cast<int>(trustflow::attestation::ErrorCode::kOk),
                    "GenerateAttestationReport err: {}, msg: {}, details: {}",
                    code, msg.data(), details.data());
    SPDLOG_INFO("report from c api: {}, len: {}", report_buf.data(),
                report_len);
  } catch (const std::exception& e) {
    SPDLOG_ERROR("{}", e.what());
    return -1;
  }

  return 0;
}