// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_LOGGER_H_
#define XG_LOGGER_H_

#include "spdlog/spdlog.h"
#ifdef _WIN32  // workaround build fail bug
#undef CreateWindow
#undef MemoryBarrier
#endif

namespace xg {

#define XG_TRACE SPDLOG_TRACE
#define XG_DEBUG SPDLOG_DEBUG
#define XG_INFO SPDLOG_INFO
#define XG_WARN SPDLOG_WARN
#define XG_ERROR SPDLOG_ERROR
#define XG_CRITICAL SPDLOG_CRITICAL

class Logger {
 public:
  static Logger& Get() {
    return logger_;
  }

  constexpr bool IsTraceEnabled() const {
    return SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_TRACE;
  }

  constexpr bool IsDebugEnabled() const {
    return SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_DEBUG;
  }

  constexpr bool IsInfoEnabled() const {
    return SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_INFO;
  }

  constexpr bool IsWarnEnabled() const {
    return SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_WARN;
  }

  constexpr bool IsErrorEnabled() const {
    return SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_ERROR;
  }

  constexpr bool IsCriticalEnabled() const {
    return SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_CRITICAL;
  }

 private:
  Logger();
  ~Logger();
  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;
  Logger(Logger&&) = delete;
  Logger& operator=(Logger&&) = delete;

  static Logger logger_;
};

}  // namespace xg

#endif  // XG_LOGGER_H_
