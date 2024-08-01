# Repository Layout

This is a high level overview of how the repository is laid out. Some major folders are listed below:

* [bazel/](bazel/): Configuration for TrustFlow's use of [Bazel](https://bazel.build/). It includes some remote repositories and rules that we use in our build system.
* [dm_component/](dm_component/): Data management component. 
* [docker/](docker/): Dockerfiles used to build images for compiling or running TrustFlow.
* [docs/](docs/): Documents of TrustFlow.
* [trustflow/](trustflow/): Some common libs used by TrustFlow.
  * [attestation/](trustflow/attestation/): C++ implementations of remote attestation for different platforms.
    * [collateral/](trustflow/attestation/collateral/): Get quote collateral in Passport type report.
    * [common/](trustflow/attestation/common/): Common definitions or implementations for attestation.
    * [generation/](trustflow/attestation/generation/): Remote attestation generation.
    * [sample/](trustflow/attestation/sample/): Sample code for attestation.
    * [utils/](trustflow/attestation/utils/): Utilities for attestation.
    * [verification/](trustflow/attestation/verification/): Remote attestation verification.