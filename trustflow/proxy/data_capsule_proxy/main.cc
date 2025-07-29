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

#include "gflags/gflags.h"
#include "spdlog/spdlog.h"
#include "yaml-cpp/yaml.h"

#include "trustflow/proxy/data_capsule_proxy/capsule_manager_client.h"
#include "trustflow/proxy/data_capsule_proxy/data_capsule_proxy.h"
#include "trustflow/proxy/utils/crypto_util.h"
#include "trustflow/proxy/utils/io_util.h"
#include "trustflow/proxy/utils/log.h"

DEFINE_string(plat, "sim", "platform. sim/tdx/csv");
DEFINE_int32(port, 8010, "TCP Port of this server");
DEFINE_int32(idle_timeout_s, -1,
             "Connection will be closed if there is no "
             "read/write operations during the last `idle_timeout_s'");
DEFINE_string(cert_path, "app.crt", "App certificate path");
DEFINE_string(private_key_path, "app.key", "App private key path");
DEFINE_string(cm_endpoint, "127.0.0.1:8888", "CapsuleManager endpoint");
DEFINE_string(cm_init_config, "", "Init tls asset to get from CapsuleManager");

// log config
DEFINE_string(log_path, "data_capsule_proxy.log", "App log path");
DEFINE_string(monitor_log_path, "data_capsule_proxy_monitor.log",
              "Monitor log path");
DEFINE_string(log_level, "info", "log level");
DEFINE_bool(enable_console_logger, false,
            "Whether logging to stdout while logging to file");

namespace {
constexpr char kScope[] = "scope";
constexpr char kOpName[] = "op_name";
constexpr char kResource[] = "resource";
constexpr char kResourceUri[] = "resource_uri";
constexpr char kColumns[] = "columns";
constexpr char kAtrs[] = "attrs";
constexpr char kEnv[] = "env";
constexpr char kGlobalAttrs[] = "global_attrs";
constexpr char kTlsCertPath[] = "tls_cert_path";
constexpr char kTlsKeyPath[] = "tls_key_path";

secretflowapis::v2::sdc::capsule_manager::ResourceRequest GenResourceRequest(
    const YAML::Node& config, const std::string& cert) {
  secretflowapis::v2::sdc::capsule_manager::ResourceRequest resource_request;
  resource_request.set_initiator_party_id(
      trustflow::proxy::utils::GeneratePartyId(cert));
  resource_request.set_scope(config[kScope].as<std::string>());
  resource_request.set_op_name(config[kOpName].as<std::string>());
  YACL_ENFORCE(config[kResource].IsSequence(), "resource must be a sequence");
  for (const auto& resource_conf : config[kResource]) {
    secretflowapis::v2::sdc::capsule_manager::ResourceRequest::Resource
        resource;
    resource.set_resource_uri(resource_conf[kResourceUri].as<std::string>());
    for (const auto& column : resource_conf[kColumns]) {
      resource.add_columns(column.as<std::string>());
    }
    resource.set_attrs(resource_conf[kAtrs].as<std::string>());

    *(resource_request.add_resources()) = std::move(resource);
  }

  resource_request.set_env(config[kEnv].as<std::string>());
  resource_request.set_global_attrs(config[kGlobalAttrs].as<std::string>());

  return resource_request;
}
}  // namespace

int main(int argc, char* argv[]) {
  try {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    trustflow::proxy::utils::LogOptions log_opts(
        FLAGS_log_path, FLAGS_monitor_log_path, FLAGS_log_level,
        FLAGS_enable_console_logger);
    trustflow::proxy::utils::LogSetup(log_opts);

    const std::string cert = trustflow::proxy::utils::ReadFile(FLAGS_cert_path);
    const std::string private_key =
        trustflow::proxy::utils::ReadFile(FLAGS_private_key_path);

    // Get TLS Asset when cm init config is set
    if (!FLAGS_cm_init_config.empty()) {
      YAML::Node config = YAML::LoadFile(FLAGS_cm_init_config);
      trustflow::proxy::data_capsule_proxy::CapsuleManagerClient
          capsule_manager_client(FLAGS_cm_endpoint);

      auto resource_request = GenResourceRequest(config, cert);

      const auto tls_asset = capsule_manager_client.GetTlsAsset(
          FLAGS_plat, cert, private_key, resource_request);
      trustflow::proxy::utils::WriteFile(config[kTlsCertPath].as<std::string>(),
                                         tls_asset.cert());
      trustflow::proxy::utils::WriteFile(config[kTlsKeyPath].as<std::string>(),
                                         tls_asset.private_key());
    }

    brpc::Server server;

    trustflow::proxy::data_capsule_proxy::DataCapsuleProxyImpl
        data_capsule_proxy_impl(FLAGS_cm_endpoint, FLAGS_plat, cert,
                                private_key);

    if (server.AddService(&data_capsule_proxy_impl,
                          brpc::SERVER_DOESNT_OWN_SERVICE) != 0) {
      SPDLOG_ERROR("Fail to add data_capsule_proxy_impl");
      return -1;
    }

    brpc::ServerOptions options;
    options.idle_timeout_sec = FLAGS_idle_timeout_s;
    if (server.Start(FLAGS_port, &options) != 0) {
      SPDLOG_ERROR("Fail to start data_capsule_proxy_impl");
      return -1;
    }

    SPDLOG_INFO("DataCapsuleProxyImpl is running on port {}", FLAGS_port);
    server.RunUntilAskedToQuit();
  } catch (const std::exception& e) {
    SPDLOG_ERROR(e.what());
    return -1;
  }

  return 0;
}