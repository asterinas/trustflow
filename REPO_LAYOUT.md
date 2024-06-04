# Repository Layout

This is a high level overview of how the repository is laid out. Some major folders are listed below:

* [bazel/](bazel/): Configuration for TrustedFlow's use of [Bazel](https://bazel.build/). It includes some remote repositories and rules that we use in our build system.
* [dm_component/](dm_component/): Data management component. 
* [docker/](docker/): Dockerfiles used to build images for compiling or running TrustedFlow.
* [docs/](docs/): Documents of TrustedFlow.
* [trustedflow/](trustedflow/): Some common libs used by TrustedFlow.
  * [attestation/](trustedflow/attestation/): C++ implementations of remote attestation for different platforms.
    * [collateral/](trustedflow/attestation/collateral/): Get quote collateral in Passport type report.
    * [common/](trustedflow/attestation/common/): Common definitions or implementations for attestation.
    * [generation/](trustedflow/attestation/generation/): Remote attestation generation.
    * [sample/](trustedflow/attestation/sample/): Sample code for attestation.
    * [utils/](trustedflow/attestation/utils/): Utilities for attestation.
    * [verification/](trustedflow/attestation/verification/): Remote attestation verification.