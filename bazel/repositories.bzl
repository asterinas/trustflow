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

def trustedflow_dependencies():
    """
    trustedflow deps
    """
    _com_github_gperftools_gperftools()

    _com_github_grpc_grpc()

    _com_github_rules_proto_grpc()

    _com_github_sf_apis()

    _com_github_cppcodec()

    _com_github_yacl()

    _local_sgxsdk()

    _com_github_dcap()

    _com_gitee_hygon_csv_header()

    _com_github_httplib()

def _com_github_gperftools_gperftools():
    maybe(
        http_archive,
        name = "com_github_gperftools_gperftools",
        type = "tar.gz",
        strip_prefix = "gperftools-2.9.1",
        sha256 = "ea566e528605befb830671e359118c2da718f721c27225cbbc93858c7520fee3",
        urls = [
            "https://github.com/gperftools/gperftools/releases/download/gperftools-2.9.1/gperftools-2.9.1.tar.gz",
        ],
        build_file = "@trustedflow//bazel:gperftools.BUILD",
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
        patches = ["@trustedflow//bazel:patches/grpc.patch"],
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
            "https://github.com/secretflow/secure-data-capsule-apis/archive/986ce7e6fed128a8ebedc0c02cc6abae01124716.tar.gz",
        ],
        strip_prefix = "secure-data-capsule-apis-986ce7e6fed128a8ebedc0c02cc6abae01124716",
        build_file = "@trustedflow//bazel:sf_apis.BUILD",
        sha256 = "d664a2381bfc64f5c0a352ed15388dbba568d473b46cdd07a3a02de6ced4bb39",
    )

def _com_github_cppcodec():
    maybe(
        http_archive,
        name = "cppcodec",
        build_file = "@trustedflow//bazel:cppcodec.BUILD",
        urls = [
            "https://github.com/tplgy/cppcodec/archive/refs/tags/v0.2.tar.gz",
        ],
        strip_prefix = "cppcodec-0.2",
        sha256 = "0edaea2a9d9709d456aa99a1c3e17812ed130f9ef2b5c2d152c230a5cbc5c482",
    )

# include openssl deps
def _com_github_yacl():
    maybe(
        http_archive,
        name = "yacl",
        urls = [
            "https://github.com/secretflow/yacl/archive/ea6e1ea567903804f17525fe04ede706815ece38.tar.gz",
        ],
        strip_prefix = "yacl-ea6e1ea567903804f17525fe04ede706815ece38",
        sha256 = "51c58b3c9704e439101311f74a46d97b0bc3afe2d1c53866e6a80629745cabe1",
    )

def _local_sgxsdk():
    maybe(
        native.new_local_repository,
        name = "sgxsdk",
        build_file = "@trustedflow//bazel:sgxsdk.BUILD",
        path = "/opt/intel/sgxsdk",
    )

def _com_github_dcap():
    maybe(
        http_archive,
        name = "dcap",
        urls = [
            "https://github.com/intel/SGXDataCenterAttestationPrimitives/archive/refs/tags/DCAP_1.20.tar.gz",
        ],
        build_file = "@trustedflow//bazel:dcap.BUILD",
        strip_prefix = "SGXDataCenterAttestationPrimitives-DCAP_1.20",
        sha256 = "a71bba80f8da53ce2877f4c2bd2d1713ab97acc4a3e7987d82d7430cda2d8fb1",
    )

def _com_gitee_hygon_csv_header():
    maybe(
        http_file,
        name = "hygon_csv_header",
        url = "https://gitee.com/anolis/hygon-devkit/raw/master/csv/attestation/attestation.h",
        downloaded_file_path = "csv/attestation/attestation.h",
    )

def _com_github_httplib():
    maybe(
        http_archive,
        name = "com_github_httplib",
        sha256 = "e620d030215733c4831fdc7813d5eb37a6fd599f8192a730662662e1748a741b",
        strip_prefix = "cpp-httplib-0.11.2",
        build_file = "@trustedflow//bazel:httplib.BUILD",
        type = "tar.gz",
        urls = [
            "https://github.com/yhirose/cpp-httplib/archive/refs/tags/v0.11.2.tar.gz",
        ],
    )
