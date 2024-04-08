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

#include "trustedflow/attestation/verification/wrapper/verifier_wrapper.h"

namespace trustedflow {
namespace attestation {
namespace verification {
namespace pylib {

namespace py = ::pybind11;

PYBIND11_MODULE(verifier, m) {
  m.doc() =
      "Trustedflow Attestation Verifier is a library for verifying attestation "
      "reports from different tee platforms.";

  // bind trustedflow::attestation::Status
  py::class_<trustedflow::attestation::Status>(m, "Status")
      .def(py::init<>())
      .def_readwrite("code", &trustedflow::attestation::Status::code)
      .def_readwrite("message", &trustedflow::attestation::Status::message)
      .def_readwrite("details", &trustedflow::attestation::Status::details);

  m.def("attestation_report_verify",
        &trustedflow::attestation::verification::AttestationReportVerify,
        py::arg("report_json_str"), py::arg("policy_json_str"));
}

}  // namespace pylib
}  // namespace verification
}  // namespace attestation
}  // namespace trustedflow