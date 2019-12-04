// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/logger.h"

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

namespace xg {

Logger Logger::logger_ = Logger();

Logger::Logger() {
  spdlog::set_level(
      static_cast<spdlog::level::level_enum>(SPDLOG_ACTIVE_LEVEL));
  spdlog::set_pattern("[%t]%+");
}

Logger::~Logger() { spdlog::drop_all(); }

}  // namespace xg
