# Copyright 2023 Ant Group Co., Ltd.
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
import stat
from pathlib import Path

import setuptools
from setuptools.command import build_ext


def read(fname):
    with open(os.path.join(os.path.dirname(__file__), fname)) as f:
        return f.read()


# [ref](https://github.com/google/trimmed_match/blob/master/setup.py)
class CargoExtension(setuptools.Extension):
    """A rust extension that is defined as a Cargo target."""

    def __init__(self, cargo_workspace, cargo_target, dest_path):
        self._cargo_target = cargo_target
        self._cargo_workspace = cargo_workspace
        self._dest_path = dest_path
        setuptools.Extension.__init__(self, "", sources=[])


class BuildCargoExtension(build_ext.build_ext):
    """A command that runs Cargo to build a rust extension."""

    def run(self):
        for ext in self.extensions:
            self.cargo_build(ext)

    def cargo_build(self, ext):
        Path(self.build_temp).mkdir(parents=True, exist_ok=True)
        cargo_argv = [
            "cargo",
            "build",
            "--manifest-path",
            f"{ext._cargo_workspace}/Cargo.toml",
            "--release",
            "--target-dir",
            f"{os.path.join(os.path.abspath(self.build_temp))}/target",
        ]

        self.spawn(cargo_argv)

        ext_cargo_bin_path = os.path.join(
            os.path.abspath(self.build_temp),
            "target",
            "release",
            ext._cargo_target,
        )
        if not self.inplace:
            ext_dest_path = os.path.join(self.build_lib, ext._dest_path)
        else:
            ext_dest_path = ext._dest_path
        Path(ext_dest_path).parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(ext_cargo_bin_path, ext_dest_path)
        os.chmod(ext_dest_path, stat.S_IRWXU | stat.S_IRWXG | stat.S_IRWXO)


setuptools.setup(
    name="tee-dm",
    version="0.1.0.dev20231101",
    author="secretflow",
    author_email="secretflow-contact@service.alipay.com",
    description="TEE data management component",
    long_description_content_type="text/markdown",
    long_description="TEE data management component",
    license="Apache 2.0",
    url="https://github.com/asterinas/trustflow",
    packages=setuptools.find_namespace_packages(exclude=("tests", "tests.*")),
    install_requires=read("requirements.txt"),
    ext_modules=[
        CargoExtension("componentize/server", "server", "componentize/dmserver"),
    ],
    cmdclass=dict(build_ext=BuildCargoExtension),
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: Apache Software License",
        "Operating System :: POSIX :: Linux",
    ],
    include_package_data=True,
)
