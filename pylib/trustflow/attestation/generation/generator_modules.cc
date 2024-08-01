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

#include "trustflow/attestation/generation/wrapper/generator_wrapper.h"

namespace trustflow {
namespace attestation {
namespace generation {
namespace pylib {

namespace py = ::pybind11;

PYBIND11_MODULE(generator, m) {
  m.doc() =
      "TrustFlow Attestation Generator is a library for generating "
      "attestation report on different tee platforms";

  // bind AttestationGenerator class
  py::class_<trustflow::attestation::generation::AttestationGenerator>(
      m, "AttestationGenerator")
      .def("generate_report_json",
           &trustflow::attestation::generation::AttestationGenerator::
               GenerateReportJson);

  // bind CreateAttestationGenerator function
  m.def("create_attestation_generator",
        &trustflow::attestation::generation::CreateAttestationGenerator,
        py::return_value_policy::take_ownership);
}
}  // namespace pylib
}  // namespace generation
}  // namespace attestation
}  // namespace trustflow
