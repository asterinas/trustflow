// Copyright 2023 Ant Group Co., Ltd.
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

#include "secretflowapis/v2/sdc/capsule_manager/capsule_manager.grpc.pb.h"

namespace trustflow {
namespace proxy {
namespace data_capsule_proxy {

class CapsuleManagerClient {
 public:
  explicit CapsuleManagerClient(const std::string& capsule_manager_endpoint);

  std::string GetRaCert();
  std::vector<secretflowapis::v2::sdc::capsule_manager::DataKey> GetDataKeys(
      const std::string& plat, const std::string& cert,
      const std::string& private_key,
      secretflowapis::v2::sdc::capsule_manager::ResourceRequest& resource_req)
      const;
  secretflowapis::v2::sdc::capsule_manager::TlsAsset GetTlsAsset(
      const std::string& plat, const std::string& cert,
      const std::string& private_key,
      secretflowapis::v2::sdc::capsule_manager::ResourceRequest& resource_req)
      const;
  void CreateResultDataKey(const std::string& plat, const std::string& cert,
                           const std::string& private_key,
                           secretflowapis::v2::sdc::capsule_manager::
                               CreateResultDataKeyRequest::Body& body) const;

 private:
  std::unique_ptr<
      secretflowapis::v2::sdc::capsule_manager::CapsuleManager::Stub>
      capsule_manager_stub_;
  // init in GetRaCert
  std::string capsule_manager_cert_;
};

}  // namespace data_capsule_proxy
}  // namespace proxy
}  // namespace trustflow