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

#include "brpc/server.h"

#include "secretflowapis/v2/sdc/ra_proxy/ra_proxy.pb.h"

namespace trustflow {
namespace proxy {
namespace ra_proxy {

class RaProxyImpl : public ::secretflowapis::v2::sdc::ra_proxy::RaProxy {
 public:
  explicit RaProxyImpl(const std::string& plat, const std::string& cert)
      : plat_(plat), cert_(cert) {}
  void GetRaCert(
      ::google::protobuf::RpcController* cntl_base,
      const ::secretflowapis::v2::sdc::ra_proxy::GetRaCertRequest* request,
      ::secretflowapis::v2::sdc::ra_proxy::GetRaCertResponse* response,
      ::google::protobuf::Closure* done);

 private:
  // TEE platform
  const std::string plat_;
  // x509 cert in PEM format
  const std::string cert_;
};

}  // namespace ra_proxy
}  // namespace proxy
}  // namespace trustflow