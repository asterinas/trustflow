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

#include <spdlog/spdlog.h>

#include <string>

#include "fmt/format.h"

namespace trustflow {
namespace proxy {
namespace utils {

constexpr char kMonitorLog[] = "monitor_log";
constexpr char kAppLog[] = "app_log";

struct LogOptions {
  std::string app_log_path;
  std::string monitor_log_path;
  std::string log_level;
  bool enable_console_logger;

  LogOptions(std::string app_log_path_, std::string monitor_log_path_,
             std::string log_level_, bool enable_console_logger_ = false)
      : app_log_path(std::move(app_log_path_)),
        monitor_log_path(std::move(monitor_log_path_)),
        log_level(std::move(log_level_)),
        enable_console_logger(enable_console_logger_) {}
};

void LogLevelFromString(const std::string& level, spdlog::level::level_enum* l);

void LogSetup(const LogOptions& opts);

// shutdown logger
void LogShutdown();

}  // namespace utils
}  // namespace proxy
}  // namespace trustflow
