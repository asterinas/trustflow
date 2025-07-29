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

#include "secretflowapis/v2/sdc/data_capsule_proxy/data_capsule_proxy.pb.h"

namespace trustflow {
namespace proxy {
namespace data_capsule_proxy {

class DataCapsuleProxyImpl
    : public ::secretflowapis::v2::sdc::data_capsule_proxy::DataCapsuleProxy {
 public:
  explicit DataCapsuleProxyImpl(const std::string& cm_endpoint,
                                const std::string& plat,
                                const std::string& cert,
                                const std::string& private_key)
      : cm_endpoint_(cm_endpoint),
        plat_(plat),
        cert_(cert),
        private_key_(private_key) {}
  void GetInputData(
      ::google::protobuf::RpcController* cntl_base,
      const ::secretflowapis::v2::sdc::data_capsule_proxy::GetInputDataRequest*
          request,
      ::secretflowapis::v2::sdc::data_capsule_proxy::GetInputDataResponse*
          response,
      ::google::protobuf::Closure* done);
  void PutResultData(
      ::google::protobuf::RpcController* cntl_base,
      const ::secretflowapis::v2::sdc::data_capsule_proxy::PutResultDataRequest*
          request,
      ::secretflowapis::v2::sdc::data_capsule_proxy::PutResultDataResponse*
          response,
      ::google::protobuf::Closure* done);

 private:
  // Data capsule proxy will use this endpoint if endpoint is not specified in
  // the request's CmConfig
  const std::string cm_endpoint_;
  // TEE platform
  const std::string plat_;
  // x509 cert in PEM format
  const std::string cert_;
  // pkcs8 private key in PEM format
  const std::string private_key_;
};
}  // namespace data_capsule_proxy
}  // namespace proxy
}  // namespace trustflow