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

#pragma once

namespace trustflow {
namespace attestation {

constexpr char kReportVersion[] = "1.0";

constexpr int kReportMaxSize = 40960;

constexpr unsigned int kAttributeMaxSize = 40960;

struct ReportType {
  static constexpr char kReportTypeBgcheck[] = "BackgroundCheck";
  static constexpr char kReportTypePassport[] = "Passport";
};

struct Platform {
  static constexpr char kPlatformSgxDcap[] = "SGX_DCAP";
  static constexpr char kPlatformTdx[] = "TDX";
  static constexpr char kPlatformCsv[] = "CSV";
};

}  // namespace attestation
}  // namespace trustflow