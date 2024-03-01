# Copyright 2024 Ant Group Co., Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "sgx_dcap_quoteverify",
    linkopts = ["-lsgx_dcap_quoteverify"],
    deps = [
        ":sgx_dcap_quoteverify_header",
        "@sgxsdk//:sgxsdk_header",
    ],
)

cc_library(
    name = "sgx_dcap_quoteverify_header",
    hdrs = [
        "QuoteVerification/dcap_quoteverify/inc/sgx_dcap_quoteverify.h",
    ],
    strip_include_prefix = "QuoteVerification/dcap_quoteverify/inc",
)

cc_library(
    name = "qgs_msg_lib",
    srcs = ["QuoteGeneration/quote_wrapper/qgs_msg_lib/qgs_msg_lib.cpp"],
    hdrs = [
        "QuoteGeneration/quote_wrapper/qgs_msg_lib/inc/qgs_msg_lib.h",
    ],
    strip_include_prefix = "QuoteGeneration/quote_wrapper/qgs_msg_lib/inc",
)

cc_library(
    name = "tdx_attest",
    srcs = ["QuoteGeneration/quote_wrapper/tdx_attest/tdx_attest.c"],
    hdrs = [
        "QuoteGeneration/quote_wrapper/tdx_attest/tdx_attest.h",
    ],
    strip_include_prefix = "QuoteGeneration/quote_wrapper/tdx_attest",
    deps = [":qgs_msg_lib"],
)
