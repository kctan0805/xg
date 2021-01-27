// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/logger.h"

#include "spdlog/sinks/android_sink.h"
#include "spdlog/sinks/msvc_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

namespace xg {

Logger Logger::logger_ = Logger();

Logger::Logger() {
#if defined(_MSC_VER)
  auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
  auto logger = std::make_shared<spdlog::logger>("msvc_logger", sink);
  spdlog::set_default_logger(logger);
  spdlog::set_pattern("[%t:%s:%#] %v");
#elif defined(__ANDROID__)
  auto android_logger = spdlog::android_logger_mt("android");
  spdlog::set_default_logger(android_logger);
  spdlog::set_pattern("[%s:%#] %v");
#endif

  spdlog::set_level(
      static_cast<spdlog::level::level_enum>(SPDLOG_ACTIVE_LEVEL));
}

Logger::~Logger() { spdlog::drop_all(); }

}  // namespace xg
