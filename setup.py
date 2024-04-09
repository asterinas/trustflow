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


import os
import shutil
from datetime import date
from pathlib import Path

import setuptools

# Available at setup time due to pyproject.toml
from setuptools.command import build_ext

__version__ = "0.2.0.dev$$DATE$$"

ROOT_DIR = Path(__file__).resolve().parent

PACKAGE_TYPE = os.getenv("PACKAGE_TYPE", default="verification")
TEE_TYPE = os.getenv("TEE_TYPE")


def get_version():
    date_str = date.today().strftime("%Y%m%d")
    return __version__.replace("$$DATE$$", date_str)


def read(fname):
    with open(ROOT_DIR / Path(fname)) as f:
        return f.read()


# [ref](https://github.com/google/trimmed_match/blob/master/setup.py)
class BazelExtension(setuptools.Extension):
    """A C/C++ extension that is defined as a Bazel BUILD target."""

    def __init__(self, bazel_workspace, bazel_target, ext_name):
        self._bazel_workspace = bazel_workspace
        self._bazel_target = bazel_target
        setuptools.Extension.__init__(self, ext_name, sources=[])


class BuildBazelExtension(build_ext.build_ext):
    """A command that runs Bazel to build a C/C++ extension."""

    def run(self):
        for ext in self.extensions:
            self.bazel_build(ext)

    def bazel_build(self, ext):
        # bazel exec
        command = (
            f"bazel --output_base={ROOT_DIR}/target build --define tee_type={TEE_TYPE}"
            + f" //{ext._bazel_workspace}:{ext._bazel_target}"
            + " --compilation_mode="
            + ("dbg" if self.debug else "opt")
            + " --repository_cache=/tmp/bazel_repo_cache"
        )
        os.system(command)

        # copy lib to python package
        ext_bazel_bin_path = (
            Path(ROOT_DIR)
            / Path("bazel-bin")
            / Path(ext._bazel_workspace)
            / Path(ext.name)
        )
        ext_dest_path = Path(ROOT_DIR) / Path(ext._bazel_workspace) / Path(ext.name)
        Path(ext_dest_path).parent.mkdir(parents=True, exist_ok=True)
        shutil.copy(ext_bazel_bin_path, ext_dest_path)


class BinaryDistribution(setuptools.Distribution):
    def has_ext_modules(self):
        return True


def build_generator():
    setuptools.setup(
        name=f"trustedflow-{TEE_TYPE}-generation",
        version=get_version(),
        author="secretflow",
        author_email="secretflow-contact@service.alipay.com",
        url="https://github.com/secretflow/trustedflow",
        description=f"An attestation report generation library for {TEE_TYPE}",
        long_description_content_type="text/markdown",
        long_description=f"An attestation report generation library for {TEE_TYPE}",
        license="Apache 2.0",
        package_dir={"": "pylib"},
        packages=["trustedflow/attestation/generation"],
        package_data={
            "": ["*.so"],
        },
        install_requires=read("requirements.txt"),
        ext_modules=[
            BazelExtension(
                "pylib/trustedflow/attestation/generation",  # bazel_workspace
                "generator_module",  # bazel_target
                "generator.so",  # ext_name
            ),
        ],
        cmdclass=dict(build_ext=BuildBazelExtension),
        # The BinaryDistribution argument triggers build_ext.
        distclass=BinaryDistribution,
        classifiers=[
            "Programming Language :: Python :: 3",
            "License :: OSI Approved :: Apache Software License",
            "Operating System :: POSIX :: Linux",
        ],
        options={
            "bdist_wheel": {"plat_name": "manylinux2014_x86_64"},
        },
        include_package_data=True,
    )


def build_verifier():
    setuptools.setup(
        name=f"trustedflow-verification",
        version=get_version(),
        author="secretflow",
        author_email="secretflow-contact@service.alipay.com",
        url="https://github.com/secretflow/trustedflow",
        description=f"An attestation report verification library",
        long_description_content_type="text/markdown",
        long_description="An attestation report verification library",
        license="Apache 2.0",
        package_dir={"": "pylib"},
        packages=["trustedflow/attestation/verification"],
        package_data={
            "": ["*.so"],
        },
        install_requires=read("requirements.txt"),
        ext_modules=[
            BazelExtension(
                "pylib/trustedflow/attestation/verification",  # bazel_workspace
                "verifier_module",  # bazel_target
                "verifier.so",  # ext_name
            ),
        ],
        cmdclass=dict(build_ext=BuildBazelExtension),
        # The BinaryDistribution argument triggers build_ext.
        distclass=BinaryDistribution,
        classifiers=[
            "Programming Language :: Python :: 3",
            "License :: OSI Approved :: Apache Software License",
            "Operating System :: POSIX :: Linux",
        ],
        options={
            "bdist_wheel": {"plat_name": "manylinux2014_x86_64"},
        },
        include_package_data=True,
    )


if __name__ == "__main__":
    if PACKAGE_TYPE == "generation":
        build_generator()
    elif PACKAGE_TYPE == "verification":
        build_verifier()
    else:
        raise Exception(f"Unknown package type: {PACKAGE_TYPE}")
