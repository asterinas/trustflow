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

#include "pybind11/pybind11.h"

#include "trustflow/attestation/verification/wrapper/verifier_wrapper.h"

namespace trustflow {
namespace attestation {
namespace verification {
namespace pylib {

namespace py = ::pybind11;

PYBIND11_MODULE(verifier, m) {
  m.doc() =
      "TrustFlow Attestation Verifier is a library for verifying attestation "
      "reports from different tee platforms.";

  // bind trustflow::attestation::Status
  py::class_<trustflow::attestation::Status>(m, "Status")
      .def(py::init<>())
      .def_readwrite("code", &trustflow::attestation::Status::code)
      .def_readwrite("message", &trustflow::attestation::Status::message)
      .def_readwrite("details", &trustflow::attestation::Status::details);

  m.def("attestation_report_verify",
        &trustflow::attestation::verification::AttestationReportVerify,
        py::arg("report_json_str"), py::arg("policy_json_str"));
}

}  // namespace pylib
}  // namespace verification
}  // namespace attestation
}  // namespace trustflow