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

#include "trustflow/attestation/generation/wrapper/generator_wrapper.h"

#include "yacl/base/exception.h"

#ifdef TEE_TYPE_SGX2
#include "trustflow/attestation/generation/sgx2/sgx2_generator.h"
#endif

#ifdef TEE_TYPE_TDX
#include "trustflow/attestation/generation/tdx/tdx_generator.h"
#endif

#ifdef TEE_TYPE_CSV
#include "trustflow/attestation/generation/csv/csv_generator.h"
#endif

namespace trustflow {
namespace attestation {
namespace generation {

std::unique_ptr<AttestationGenerator> CreateAttestationGenerator() {
#ifdef TEE_TYPE_SGX2
  return std::make_unique<Sgx2AttestationGenerator>();
#endif

#ifdef TEE_TYPE_TDX
  return std::make_unique<TdxAttestationGenerator>();
#endif

#ifdef TEE_TYPE_CSV
  return std::make_unique<CsvAttestationGenerator>();
#endif

  YACL_THROW("Unsupported TEE type!");
}

}  // namespace generation
}  // namespace attestation
}  // namespace trustflow