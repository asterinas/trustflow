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

"""
This module contains build rules for project dependencies.
"""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive", "http_file")
load("@bazel_tools//tools/build_defs/repo:utils.bzl", "maybe")

def trustflow_dependencies():
    """
    trustflow deps
    """
    _local_openssl_openssl()

    _local_curl()

    _com_github_grpc_grpc()

    _com_github_rules_proto_grpc()

    _com_github_sf_apis()

    _com_github_cppcodec()

    _com_github_yacl()

    _local_sgxsdk()

    _com_github_dcap()

    _com_gitee_hygon_csv_header()

    _com_github_httplib()

    _com_github_pybind11()

    _aliyun_oss_cpp_sdk()

    _com_github_yaml_cpp()

def _local_openssl_openssl():
    maybe(
        native.new_local_repository,
        name = "com_github_openssl_openssl",
        build_file = "@trustflow//bazel:openssl.BUILD",
        path = "bazel",
    )

def _local_curl():
    maybe(
        native.new_local_repository,
        name = "com_github_curl",
        build_file = "@trustflow//bazel:curl.BUILD",
        path = "bazel",
    )

def _com_github_grpc_grpc():
    maybe(
        http_archive,
        name = "com_github_grpc_grpc",
        sha256 = "fb1ed98eb3555877d55eb2b948caca44bc8601c6704896594de81558639709ef",
        strip_prefix = "grpc-1.50.1",
        type = "tar.gz",
        patch_args = ["-p1"],
        # Set grpc to use local go toolchain
        patches = ["@trustflow//bazel:patches/grpc.patch"],
        urls = [
            "https://github.com/grpc/grpc/archive/refs/tags/v1.50.1.tar.gz",
        ],
    )

def _com_github_rules_proto_grpc():
    maybe(
        http_archive,
        name = "rules_proto_grpc",
        type = "tar.gz",
        sha256 = "7954abbb6898830cd10ac9714fbcacf092299fda00ed2baf781172f545120419",
        strip_prefix = "rules_proto_grpc-3.1.1",
        urls = [
            "https://github.com/rules-proto-grpc/rules_proto_grpc/archive/refs/tags/3.1.1.tar.gz",
        ],
    )

def _com_github_sf_apis():
    maybe(
        http_archive,
        name = "sf_apis",
        urls = [
            "https://github.com/secretflow/secure-data-capsule-apis/archive/bf3a19c4eddb0e2cf4c9c21c134413dffdf321c9.tar.gz",
        ],
        strip_prefix = "secure-data-capsule-apis-bf3a19c4eddb0e2cf4c9c21c134413dffdf321c9",
        build_file = "@trustflow//bazel:sf_apis.BUILD",
        sha256 = "99d7de35d410a7870d92e63d03c7c2d202bd5b3749e94cb6e0955dd6d62c06fa",
    )

def _com_github_cppcodec():
    maybe(
        http_archive,
        name = "cppcodec",
        build_file = "@trustflow//bazel:cppcodec.BUILD",
        urls = [
            "https://github.com/tplgy/cppcodec/archive/refs/tags/v0.2.tar.gz",
        ],
        strip_prefix = "cppcodec-0.2",
        sha256 = "0edaea2a9d9709d456aa99a1c3e17812ed130f9ef2b5c2d152c230a5cbc5c482",
        patches = ["@trustflow//bazel:patches/cppcodec.patch"],
        patch_args = ["-p1"],
    )

def _com_github_yacl():
    maybe(
        http_archive,
        name = "yacl",
        urls = [
            "https://github.com/secretflow/yacl/archive/refs/tags/0.4.5b0.tar.gz",
        ],
        strip_prefix = "yacl-0.4.5b0",
        sha256 = "68d1dbeb255d404606d3ba9380b915fbbe3886cde575bbe89795657286742bd2",
    )

def _local_sgxsdk():
    maybe(
        native.new_local_repository,
        name = "sgxsdk",
        build_file = "@trustflow//bazel:sgxsdk.BUILD",
        path = "/opt/intel/sgxsdk",
    )

def _com_github_dcap():
    maybe(
        http_archive,
        name = "dcap",
        urls = [
            "https://github.com/intel/SGXDataCenterAttestationPrimitives/archive/refs/tags/DCAP_1.20.tar.gz",
        ],
        build_file = "@trustflow//bazel:dcap.BUILD",
        strip_prefix = "SGXDataCenterAttestationPrimitives-DCAP_1.20",
        sha256 = "a71bba80f8da53ce2877f4c2bd2d1713ab97acc4a3e7987d82d7430cda2d8fb1",
    )

def _com_gitee_hygon_csv_header():
    maybe(
        http_file,
        name = "hygon_csv_header",
        url = "https://gitee.com/anolis/hygon-devkit/raw/2683fd4577a1d2e0fc19247b47a6ca68f4879c33/csv/attestation/attestation.h",
        downloaded_file_path = "csv/attestation/attestation.h",
    )

def _com_github_httplib():
    maybe(
        http_archive,
        name = "com_github_httplib",
        sha256 = "e620d030215733c4831fdc7813d5eb37a6fd599f8192a730662662e1748a741b",
        strip_prefix = "cpp-httplib-0.11.2",
        build_file = "@trustflow//bazel:httplib.BUILD",
        type = "tar.gz",
        urls = [
            "https://github.com/yhirose/cpp-httplib/archive/refs/tags/v0.11.2.tar.gz",
        ],
    )

def _com_github_pybind11():
    # Python binding.
    maybe(
        http_archive,
        name = "pybind11_bazel",
        sha256 = "e8355ee56c2ff772334b4bfa22be17c709e5573f6d1d561c7176312156c27bd4",
        strip_prefix = "pybind11_bazel-2.11.1",
        urls = [
            "https://github.com/pybind/pybind11_bazel/releases/download/v2.11.1/pybind11_bazel-2.11.1.tar.gz",
        ],
    )

    # We still require the pybind library.
    maybe(
        http_archive,
        name = "pybind11",
        build_file = "@pybind11_bazel//:pybind11.BUILD",
        sha256 = "d475978da0cdc2d43b73f30910786759d593a9d8ee05b1b6846d1eb16c6d2e0c",
        strip_prefix = "pybind11-2.11.1",
        urls = [
            "https://github.com/pybind/pybind11/archive/refs/tags/v2.11.1.tar.gz",
        ],
    )

def _aliyun_oss_cpp_sdk():
    maybe(
        http_archive,
        name = "com_github_aliyun_oss_cpp_sdk",
        sha256 = "adee3beb0b7b88bfd947eb9dae5e0d22c8b3f315563aab076a7b60c140125f31",
        strip_prefix = "aliyun-oss-cpp-sdk-1.9.0",
        build_file = "//:bazel/oss_sdk.BUILD",
        type = "tar.gz",
        urls = [
            "https://codeload.github.com/aliyun/aliyun-oss-cpp-sdk/tar.gz/1.9.0",
        ],
    )

def _com_github_yaml_cpp():
    maybe(
        http_archive,
        name = "com_github_yaml_cpp",
        sha256 = "43e6a9fcb146ad871515f0d0873947e5d497a1c9c60c58cb102a97b47208b7c3",
        strip_prefix = "yaml-cpp-yaml-cpp-0.7.0",
        type = "tar.gz",
        urls = [
            "https://github.com/jbeder/yaml-cpp/archive/refs/tags/yaml-cpp-0.7.0.tar.gz",
        ],
    )
