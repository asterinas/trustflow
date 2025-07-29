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

#include "trustflow/proxy/utils/log.h"

#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "yacl/base/exception.h"

namespace trustflow {
namespace proxy {
namespace utils {

namespace {

constexpr char kFormatPattern[] = "%Y-%m-%d %H:%M:%S.%e [%l] [%s:%!:%#] %v";
constexpr std::size_t kMaxLogFileSize = 500 * 1024 * 1024;  // 500 MB
constexpr std::size_t kMaxLogFileCount = 10;  // keep up to 10 max log files

}  // namespace

void LogLevelFromString(const std::string& level,
                        spdlog::level::level_enum* l) {
  const static std::unordered_map<std::string, spdlog::level::level_enum>
      kLevelMap = {{"trace", spdlog::level::trace},
                   {"debug", spdlog::level::debug},
                   {"info", spdlog::level::info},
                   {"warn", spdlog::level::warn},
                   {"err", spdlog::level::err},
                   {"critical", spdlog::level::critical},
                   {"off", spdlog::level::off}};
  auto it = kLevelMap.find(level);
  if (it == kLevelMap.end()) {
    YACL_THROW(fmt::format("None exist log Level [{}]", level));
  }
  *l = it->second;
}

void SetupLogger(const std::string& log_name, const std::string& log_path,
                 const std::string& log_level, bool enable_console_logger) {
  if (spdlog::get(log_name) != nullptr) {
    return;
  }
  auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
      log_path, kMaxLogFileSize, kMaxLogFileCount);

  std::vector<spdlog::sink_ptr> log_sinks{file_sink};
  if (enable_console_logger) {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    log_sinks.push_back(std::move(console_sink));
  }

  auto logger = std::make_shared<spdlog::logger>(log_name, log_sinks.begin(),
                                                 log_sinks.end());

  spdlog::level::level_enum level;
  LogLevelFromString(log_level, &level);
  logger->flush_on(level);
  if (log_name.compare(kAppLog) == 0) {
    logger->set_level(level);
    logger->set_pattern(kFormatPattern);
    spdlog::set_default_logger(logger);
  } else if (log_name.compare(kMonitorLog) == 0) {
    logger->set_level(level);
    logger->set_pattern(kFormatPattern);
    spdlog::register_logger(logger);
  } else {
    YACL_THROW("Unsupported logger type: {}.", log_name);
  }
  SPDLOG_INFO("Initialize logger {} succeed.", log_name);
}

void LogSetup(const LogOptions& opts) {
  SetupLogger(kAppLog, opts.app_log_path, opts.log_level,
              opts.enable_console_logger);
  SetupLogger(kMonitorLog, opts.monitor_log_path, opts.log_level,
              opts.enable_console_logger);
}

void LogShutdown() { spdlog::shutdown(); }

}  // namespace utils
}  // namespace proxy
}  // namespace trustflow
