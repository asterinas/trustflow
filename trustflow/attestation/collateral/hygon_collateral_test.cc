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

#include "trustflow/attestation/collateral/hygon_collateral.h"

#include "cppcodec/base64_rfc4648.hpp"
#include "csv/attestation/attestation.h"
#include "gtest/gtest.h"

namespace trustflow {
namespace attestation {
namespace collateral {

namespace {
constexpr char kCorrectChipId[] = "NUX0419032405";
constexpr char kWrongChipId[] = "NUX041903";
}  // namespace

TEST(HygonCsvCollateralTest,
     GetHygonCsvCollateral_with_correct_chipid_should_OK) {
  secretflowapis::v2::sdc::HygonCsvCertChain hygon_cert_chain;

  EXPECT_NO_THROW({
    trustflow::attestation::collateral::GetHygonCsvCollateral(kCorrectChipId,
                                                              hygon_cert_chain);
  });

  auto cek_cert =
      cppcodec::base64_rfc4648::decode(hygon_cert_chain.b64_cek_cert());
  auto hsk_cert =
      cppcodec::base64_rfc4648::decode(hygon_cert_chain.b64_hsk_cert());

  EXPECT_EQ(cek_cert.size(), sizeof(CSV_CERT_t));
  EXPECT_EQ(hsk_cert.size(), sizeof(CHIP_ROOT_CERT_t));
}

TEST(HygonCsvCollateralTest,
     GetHygonCsvCollateral_with_wrong_chipid_should_throw) {
  secretflowapis::v2::sdc::HygonCsvCertChain hygon_cert_chain;
  EXPECT_ANY_THROW({
    trustflow::attestation::collateral::GetHygonCsvCollateral(kWrongChipId,
                                                              hygon_cert_chain);
  });
}

}  // namespace collateral
}  // namespace attestation
}  // namespace trustflow