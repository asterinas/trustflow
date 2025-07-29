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
#include "src/butil/logging.h"

#include "trustflow/proxy/ra_proxy/ra_proxy.h"
#include "trustflow/proxy/utils/io_util.h"
#include "trustflow/proxy/utils/log.h"

DEFINE_string(plat, "sim", "platform. sim/tdx/csv");
DEFINE_int32(port, 8010, "TCP Port of this server");
DEFINE_int32(idle_timeout_s, -1,
             "Connection will be closed if there is no "
             "read/write operations during the last `idle_timeout_s'");
DEFINE_string(cert_path, "app.crt", "Certificate path");

// log config
DEFINE_string(log_path, "ra_proxy.log", "App log path");
DEFINE_string(monitor_log_path, "ra_proxy_monitor.log", "Monitor log path");
DEFINE_string(log_level, "info", "log level");
DEFINE_bool(enable_console_logger, false,
            "Whether logging to stdout while logging to file");

int main(int argc, char* argv[]) {
  try {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    trustflow::proxy::utils::LogOptions log_opts(
        FLAGS_log_path, FLAGS_monitor_log_path, FLAGS_log_level,
        FLAGS_enable_console_logger);
    trustflow::proxy::utils::LogSetup(log_opts);

    brpc::Server server;

    const std::string cert = trustflow::proxy::utils::ReadFile(FLAGS_cert_path);

    trustflow::proxy::ra_proxy::RaProxyImpl ra_proxy_impl(FLAGS_plat, cert);

    if (server.AddService(&ra_proxy_impl, brpc::SERVER_DOESNT_OWN_SERVICE) !=
        0) {
      SPDLOG_ERROR("Fail to add ra_proxy_impl");
      return -1;
    }

    brpc::ServerOptions options;
    options.idle_timeout_sec = FLAGS_idle_timeout_s;
    if (server.Start(FLAGS_port, &options) != 0) {
      SPDLOG_ERROR("Fail to start ra_proxy_impl");
      return -1;
    }

    SPDLOG_INFO("RaProxyImpl is running on port {}", FLAGS_port);
    server.RunUntilAskedToQuit();
  } catch (const std::exception& e) {
    SPDLOG_ERROR(e.what());
    return -1;
  }

  return 0;
}