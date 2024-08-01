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

#include "trustflow/attestation/generation/wrapper/generator_wrapper_c_api.h"

#include "spdlog/spdlog.h"
#include "yacl/base/exception.h"

#include "trustflow/attestation/common/constants.h"
#include "trustflow/attestation/common/status.h"
#include "trustflow/attestation/generation/interface/generator.h"
#include "trustflow/attestation/generation/wrapper/generator_wrapper.h"
#include "trustflow/attestation/utils/json2pb.h"

#include "secretflowapis/v2/sdc/ual.pb.h"

#ifdef __cplusplus
extern "C" {
#endif

int GetAttestationReportSize(const char* params_buf,
                             const unsigned int params_len,
                             unsigned int* report_len) {
  // TODO get exact report size
  // note: report size is hard to determine without generating the report and
  // format pb to json. To avoid generating report twice, return Max Size
  // instead.
  *report_len = trustflow::attestation::kReportMaxSize;

  return 0;
}

// The C API for  report generation
int GenerateAttestationReport(const char* params_buf,
                              const unsigned int params_len, char* report_buf,
                              unsigned int* report_len, char* msg,
                              unsigned int* msg_len, char* details,
                              unsigned int* details_len) {
  int code = 0;
  try {
    YACL_ENFORCE(nullptr != params_buf && nullptr != report_buf,
                 "params_buf or report_buf is null");
    // convert string to pb
    secretflowapis::v2::sdc::UnifiedAttestationGenerationParams gen_params;
    std::string report_params(params_buf, params_len);
    JSON2PB(report_params, &gen_params);

    // generate report
    auto generator =
        trustflow::attestation::generation::CreateAttestationGenerator();
    auto report = generator->GenerateReport(gen_params);

    // report to string
    std::string report_json;
    PB2JSON(report, &report_json);
    SPDLOG_INFO("report len: {}", report_json.size());
    SPDLOG_DEBUG("report: {}", report_json);

    // output
    YACL_ENFORCE_GE(
        *report_len, report_json.size(),
        "report_len(got {}) should not be less than report json size({})",
        *report_len, report_json.size());
    *report_len = report_json.size();
    memcpy(report_buf, report_json.data(), *report_len);
    SPDLOG_INFO("GenerateAttestationReport success");
  } catch (const yacl::ArgumentError& ex) {
    code = static_cast<int>(trustflow::attestation::ErrorCode::kArgumentError);
    strncpy(msg, ex.what(), *msg_len);
    strncpy(details, ex.stack_trace().c_str(), *details_len);
  } catch (const yacl::InvalidFormat& ex) {
    code = static_cast<int>(trustflow::attestation::ErrorCode::kInvalidFormat);
    strncpy(msg, ex.what(), *msg_len);
    strncpy(details, ex.stack_trace().c_str(), *details_len);
  } catch (const yacl::Exception& ex) {
    code = static_cast<int>(trustflow::attestation::ErrorCode::kInternalError);
    strncpy(msg, ex.what(), *msg_len);
    strncpy(details, ex.stack_trace().c_str(), *details_len);
  } catch (const std::exception& ex) {
    code = static_cast<int>(trustflow::attestation::ErrorCode::kInternalError);
    strncpy(msg, ex.what(), *msg_len);
  }

  // ensure msg end with '\0'
  if (*msg_len > 0) {
    msg[*msg_len - 1] = '\0';
    *msg_len = strlen(msg) + 1;
  }

  // ensure details end with '\0'
  if (*details_len > 0) {
    details[*details_len - 1] = '\0';
    *details_len = strlen(details) + 1;
  }

  return code;
}

#ifdef __cplusplus
}
#endif