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

#ifdef __cplusplus
extern "C" {
#endif

/// @brief C API for getting the size of the generated report
/// @param params_buf: [Input] The other report generation parameters buffer.
/// @param params_len: [Input] The length of other report generation parameters.
/// @param report_len: [Output] The max report size
///
/// @return Error code.
///
extern int GetAttestationReportSize(const char* params_buf,
                                    const unsigned int params_len,
                                    unsigned int* report_len);

/// @brief C API for  report generation
/// @param params_buf: [Input] The other report generation parameters buffer.
/// @param params_len: [Input] The length of other report generation parameters.
/// @param report_buf: [Output] The output serialized JSON string of the report
/// @param report_len: [Input and Output] The maximal JSON report buffer size as
/// input,
///                         and the real JSON report string size as output.
/// @param msg: [Output] The error message if any error occurs.
/// @param msg_len: [Input and Output] The maximal error message buffer size as
/// input.
///                     The real error message size as output.
/// @param details: [Output] The detailed error message if any error occurs.
/// @param details_len: [Input and Output] The maximal detailed error message
/// buffer size as input.
///                      The real detailed error message size as output.
///
///
/// @note All the lengths are memory sizes in bytes. For example, char array[] =
/// "hello" has a length of 6, not 5.
///
/// @return Error code.
///
extern int GenerateAttestationReport(const char* params_buf,
                                     const unsigned int params_len,
                                     char* report_buf, unsigned int* report_len,
                                     char* msg, unsigned int* msg_len,
                                     char* details, unsigned int* details_len);

#ifdef __cplusplus
}
#endif