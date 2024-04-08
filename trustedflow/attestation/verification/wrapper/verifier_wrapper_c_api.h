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

extern unsigned int GetAttributesMaxSize();

/// @brief C API for parse attestation report
///
/// @param report_json_c_str:   [Input] Attestation report in JSON format
/// string.
/// @param report_json_str_len: [Input] The length of report_json_c_str.
/// @param attrs_buf:   [Output] Report attributes in JSON format
/// string.
/// @param attrs_buf_len: [Input and Ouput] The maximal size of attrs_buf as
/// Input.
///                        The real attrs buf size as Output.
///
/// @param msg_buf:     [Output] The error message if any error occurs.
/// @param msg_buf_len: [Input and Output] The maximal error message buffer size
/// as input. The real error message size as output.
/// @param details_buf: [Output] The detailed error message if any error occurs.
/// @param details_buf_len: [Input and Output] The maximal detailed error
/// message buffer size as input. The real detailed error message size as
/// output.
///
/// @note All the lengths are memory sizes in bytes. For example, char array[] =
/// "hello" has a length of 6, not 5.
///
/// @return Error code.
///
extern int ParseAttributesFromReport(const char* report_json_c_str,
                                     unsigned int report_json_str_len,
                                     char* attrs_buf,
                                     unsigned int* attrs_buf_len, char* msg_buf,
                                     unsigned int* msg_buf_len,
                                     char* details_buf,
                                     unsigned int* details_buf_len);

/// @brief C API for verification
///
/// @param report_json_c_str:   [Input] Attestation report in JSON format
/// string.
/// @param report_json_str_len: [Input] The length of report_json_c_str.
/// @param policy_json_c_str:   [Input] Attestation policy in JSON format
/// string.
/// @param policy_json_str_len: [Input] The length of policy_json_c_str.
///
/// @param msg_buf:     [Output] The error message if any error occurs.
/// @param msg_buf_len: [Input and Output] The maximal error message buffer size
/// as input. The real error message size as output.
/// @param details_buf: [Output] The detailed error message if any error occurs.
/// @param details_buf_len: [Input and Output] The maximal detailed error
/// message buffer size as input. The real detailed error message size as
/// output.
///
/// @note All the lengths are memory sizes in bytes. For example, char array[] =
/// "hello" has a length of 6, not 5.
///
/// @return Error code.
///
extern int AttestationReportVerify(const char* report_json_c_str,
                                   unsigned int report_json_str_len,
                                   const char* policy_json_c_str,
                                   unsigned int policy_json_str_len,
                                   char* msg_buf, unsigned int* msg_buf_len,
                                   char* details_buf,
                                   unsigned int* details_buf_len);

#ifdef __cplusplus
}
#endif
