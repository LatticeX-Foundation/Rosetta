// ==============================================================================
// Copyright 2020 The LatticeX Foundation
// This file is part of the Rosetta library.
//
// The Rosetta library is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// The Rosetta library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with the Rosetta library. If not, see <http://www.gnu.org/licenses/>.
// ==============================================================================
#include "cc/modules/common/include/utils/logger.h"

#include <stdarg.h>
#include <iostream>
using namespace std;

#ifdef _WIN32
#include <Windows.h>
#include <process.h>
#define getpid _getpid
#else
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#endif

/**
 * this, take large compile time
 */
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

Logger::log_stream::log_stream(
  Logger& logger,
  LogLevel level,
  const char* file,
  const char* func,
  int line)
    : logger_(logger), level_(level) {
#ifndef NDEBUG
  oss_prefix_ << "[" << file << ":" << line << "] ";
#endif
}
Logger::log_stream::log_stream(const log_stream& ls) : logger_(ls.logger_) {}
Logger::log_stream::~log_stream() {
  std::string s(std::move(str()));
  // if (s.empty() || (s[s.length() - 1] != '\n'))
  //   s += "\n";
  if (!s.empty() && (s[s.length() - 1] == '\n'))
    s.pop_back();

#ifndef NDEBUG
  s = oss_prefix_.str() + s;
#endif
  if (logger_.to_stdout_) {
    spdlog::log(static_cast<spdlog::level::level_enum>(level_), s.data());
  }

  auto logger = spdlog::get("Rosetta");
  if (logger_.to_file_ && logger.get()) {
    logger->log(static_cast<spdlog::level::level_enum>(level_), s.data());
    logger->flush();
  }
}

Logger& Logger::Get() {
  static Logger logger;
  return logger;
}

Logger::Logger() {
  pid_ = (int)getpid();
  spdlog::set_automatic_registration(true);
};

void Logger::release() {}

void Logger::set_level(int level) {
  level_ = static_cast<LogLevel>(level);
  spdlog::set_level(static_cast<spdlog::level::level_enum>(level_));
}
void Logger::set_filename(const std::string& filename) {
  if (filename == filename_) {
    return;
  }

  filename_ = filename;
  to_file_ = true;
  char buf[256] = {0};
  //snprintf(buf, 256, "%s.%d", filename_.data(), pid_);
  snprintf(buf, 256, "%s", filename_.data());
  auto logger = spdlog::basic_logger_mt("Rosetta", buf); //it will auto register
}

// c-style format print
void clog_log(LogLevel level, const char* file, const char* func, int line, const char* fmt, ...) {
  if (Logger::Get().Level() > level)
    return;

  va_list args;
  char buf[1024 * 1024] = {0};
  va_start(args, fmt);
  vsprintf(buf, fmt, args);
  va_end(args);

  Logger::Get()(level, file, func, line) << buf;
}
