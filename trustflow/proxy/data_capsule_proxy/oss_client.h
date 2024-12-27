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

#include <string>

#include "alibabacloud/oss/OssClient.h"

namespace trustflow {
namespace proxy {
namespace data_capsule_proxy {

// Download from oss
// Support single file or a directory
void DownloadFromOss(const std::string& endpoint, const std::string& bucket,
                     const std::string& src_path, const std::string& dest_path,
                     const std::string& ak, const std::string& sk,
                     const std::string& sts_token);

// Upload to oss
// Support single file or a directory
void UploadToOss(const std::string& endpoint, const std::string& bucket,
                 const std::string& src_path, const std::string& dest_path,
                 const std::string& ak, const std::string& sk,
                 const std::string& sts_token);
}  // namespace data_capsule_proxy
}  // namespace proxy
}  // namespace trustflow