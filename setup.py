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
import re
import subprocess
import sys
import setuptools

# Available at setup time due to pyproject.toml
from setuptools.command import build_ext

__version__ = "0.3.0.dev$$DATE$$"

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


# Convert distutils Windows platform specifiers to CMake -A arguments
PLAT_TO_CMAKE = {
    "win32": "Win32",
    "win-amd64": "x64",
    "win-arm32": "ARM",
    "win-arm64": "ARM64",
}


# A CMakeExtension needs a sourcedir instead of a file list.
# The name must be the _single_ output extension from the CMake build.
# If you need multiple extensions, see scikit-build.
class CMakeExtension(setuptools.Extension):
    def __init__(self, name: str, sourcedir: str = "") -> None:
        super().__init__(name, sources=[])
        self.sourcedir = os.fspath(Path(sourcedir).resolve())


class CMakeBuild(build_ext.build_ext):
    def build_extension(self, ext: CMakeExtension) -> None:
        # Must be in this form due to bug in .resolve() only fixed in Python 3.10+
        ext_fullpath = Path.cwd() / self.get_ext_fullpath(ext.name)
        print(f"extenstion: name {ext.name}")
        extdir = ext_fullpath.parent.resolve()

        # Using this requires trailing slash for auto-detection & inclusion of
        # auxiliary "native" libs

        debug = int(os.environ.get("DEBUG", 0)) if self.debug is None else self.debug
        cfg = "Debug" if debug else "Release"

        # CMake lets you override the generator - we need to check this.
        # Can be set with Conda-Build, for example.
        cmake_generator = os.environ.get("CMAKE_GENERATOR", "")

        # Set Python_EXECUTABLE instead if you use PYBIND11_FINDPYTHON
        # EXAMPLE_VERSION_INFO shows you how to pass a value into the C++ code
        # from Python.
        cmake_args = [
            f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={extdir}{os.sep}/trustflow/attestation/verification",
            f"-DPYTHON_EXECUTABLE={sys.executable}",
            f"-DCMAKE_BUILD_TYPE={cfg}",  # not used on MSVC, but no harm
            "-DBUILD_MODE=host",
        ]
        build_args = []
        # Adding CMake arguments set as environment variable
        # (needed e.g. to build for ARM OSx on conda-forge)
        if "CMAKE_ARGS" in os.environ:
            cmake_args += [item for item in os.environ["CMAKE_ARGS"].split(" ") if item]

        if self.compiler.compiler_type != "msvc":
            # Using Ninja-build since it a) is available as a wheel and b)
            # multithreads automatically. MSVC would require all variables be
            # exported for Ninja to pick it up, which is a little tricky to do.
            # Users can override the generator with CMAKE_GENERATOR in CMake
            # 3.15+.
            if not cmake_generator or cmake_generator == "Ninja":
                try:
                    import ninja

                    ninja_executable_path = Path(ninja.BIN_DIR) / "ninja"
                    cmake_args += [
                        "-GNinja",
                        f"-DCMAKE_MAKE_PROGRAM:FILEPATH={ninja_executable_path}",
                    ]
                except ImportError:
                    pass

        else:
            # Single config generators are handled "normally"
            single_config = any(x in cmake_generator for x in {"NMake", "Ninja"})

            # CMake allows an arch-in-generator style for backward compatibility
            contains_arch = any(x in cmake_generator for x in {"ARM", "Win64"})

            # Specify the arch if using MSVC generator, but only if it doesn't
            # contain a backward-compatibility arch spec already in the
            # generator name.
            if not single_config and not contains_arch:
                cmake_args += ["-A", PLAT_TO_CMAKE[self.plat_name]]

            # Multi-config generators have a different way to specify configs
            if not single_config:
                cmake_args += [
                    f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{cfg.upper()}={extdir}"
                ]
                build_args += ["--config", cfg]

        if sys.platform.startswith("darwin"):
            # Cross-compile support for macOS - respect ARCHFLAGS if set
            archs = re.findall(r"-arch (\S+)", os.environ.get("ARCHFLAGS", ""))
            if archs:
                cmake_args += ["-DCMAKE_OSX_ARCHITECTURES={}".format(";".join(archs))]

        # Set CMAKE_BUILD_PARALLEL_LEVEL to control the parallel build level
        # across all generators.
        if "CMAKE_BUILD_PARALLEL_LEVEL" not in os.environ:
            # self.parallel is a Python 3 only way to set parallel jobs by hand
            # using -j in the build_ext call, not supported by pip or PyPA-build.
            if hasattr(self, "parallel") and self.parallel:
                # CMake 3.12+ only.
                build_args += [f"-j{self.parallel}"]

        build_temp = Path(self.build_temp) / ext.name
        if not build_temp.exists():
            build_temp.mkdir(parents=True)

        subprocess.run(
            ["cmake", ext.sourcedir, *cmake_args], cwd=build_temp, check=True
        )
        subprocess.run(
            ["cmake", "--build", ".", *build_args], cwd=build_temp, check=True
        )


class BinaryDistribution(setuptools.Distribution):
    def has_ext_modules(self):
        return True


def build_generator():
    setuptools.setup(
        name=f"trustflow-{TEE_TYPE}-generation",
        version=get_version(),
        author="trustflow",
        author_email="secretflow-contact@service.alipay.com",
        url="https://github.com/asterinas/trustflow",
        description=f"An attestation report generation library for {TEE_TYPE}",
        long_description_content_type="text/markdown",
        long_description=f"An attestation report generation library for {TEE_TYPE}",
        license="Apache 2.0",
        package_dir={"": "pylib"},
        packages=["trustflow/attestation/generation"],
        package_data={
            "": ["*.so"],
        },
        install_requires=read("requirements.txt"),
        ext_modules=[
            BazelExtension(
                "pylib/trustflow/attestation/generation",  # bazel_workspace
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
        name=f"trustflow-verification",
        version=get_version(),
        author="secretflow",
        author_email="secretflow-contact@service.alipay.com",
        url="https://github.com/asterinas/trustflow",
        description=f"An attestation report verification library",
        long_description_content_type="text/markdown",
        long_description="An attestation report verification library",
        license="Apache 2.0",
        packages=["trustflow/attestation/verification"],
        package_data={
            "": ["*.so"],
        },
        install_requires=read("requirements.txt"),
        ext_modules=[CMakeExtension("verifier")],
        cmdclass=dict(build_ext=CMakeBuild),
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
