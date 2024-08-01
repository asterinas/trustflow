# TrustFlow Attestation Library

TrustFlow Attestation Library 为不同类型的可信硬件设备提供生成和验证远程认证报告的功能。目前，支持 Intel SGX2、Intel TDX 以及 Hygon CSV。

## Build

### Bazel
Bazel 支持构建 Linux 平台的 generation 和 verification 模块。

**verification 模块编译**
```
bazel build //trustflow/attestation/verification/wrapper:libverification.so -c opt
```
libverification.so 生成路径为：`bazel-bin/trustflow/attestation/verification/wrapper/libverification.so`

**generation 模块编译**

因为generation 需要与特定 TEE 平台绑定，所以在编译时需要制定平台类型：

Intel SGX2 并使用 occlum 运行： `--define tee_type=sgx2`

Intel TDX: `--define tee_type=tdx`

Hygon CSV: `--define tee_type=csv`

例如 Intel TDX：
```
bazel build --define tee_type=tdx //trustflow/attestation/generation/wrapper:libgeneration.so -c opt
```
libgeneration.so 生成路径为：`bazel-bin/trustflow/attestation/generation/wrapper/libgeneration.so`

### CMake
CMake 目前仅支持使用 WebAssembly 编译 verification 模块的能力，用于支持在 Web 上执行远程认证。
```
cmake -H. -Bbuild
cd build && make
```
生成的 JS 文件路径为：`trustflow/attestation/verification/trustflow_verifier.js`
使用该 JS 的sample code 参考[这里](sample/verification/wasm/sample_react_app/README.md)

## 多语言支持

目前支持 C/C++、Python、Rust