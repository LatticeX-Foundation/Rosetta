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
#pragma once

/**
 * thread-safe log based on spdlog
 */
#include <sstream>
#include <cstring>

namespace LoggerLevel {
enum LogEnum { Trace, Debug, Info, Warn, Error, Fatal };
}
using LogLevel = LoggerLevel::LogEnum;

class Logger {
  class log_stream : public std::ostringstream {
    std::ostringstream oss_prefix_;
    Logger& logger_;
    LogLevel level_;

   public:
    log_stream(Logger& logger, LogLevel level, const char* file, const char* func, int line);
    log_stream(const log_stream& ls);
    ~log_stream();
  };

 private:
  Logger();

 public:
  static Logger& Get();

  virtual ~Logger() { release(); }
  void release();

  virtual log_stream operator()(LogLevel level, const char* file, const char* func, int line) {
    return log_stream(*this, level, file, func, line);
  }

 public:
  LogLevel Level() { return level_; }
  void log_to_stdout(bool flag = true) { to_stdout_ = flag; }
  void set_level(int level);
  void set_filename(const std::string& filename);

 private:
  LogLevel level_ = LoggerLevel::Info;
  int pid_;
  std::string filename_;

  bool to_stdout_ = true; //false;
  bool to_file_ = false;
};

#ifdef _WIN32
#define __FILENAME__ (strrchr(__FILE__, '\\') ? std::strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#define __FILENAME__ (strrchr(__FILE__, '/') ? std::strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

/**
 * Cpp-style iostream
 */
// clang-format off
#define log_trace if (Logger::Get().Level() <= LoggerLevel::Trace) Logger::Get()(LoggerLevel::Trace, __FILENAME__, __FUNCTION__, __LINE__)
#define log_debug if (Logger::Get().Level() <= LoggerLevel::Debug) Logger::Get()(LoggerLevel::Debug, __FILENAME__, __FUNCTION__, __LINE__)
#define log_info  if (Logger::Get().Level() <= LoggerLevel::Info ) Logger::Get()(LoggerLevel::Info,  __FILENAME__, __FUNCTION__, __LINE__)
#define log_warn  if (Logger::Get().Level() <= LoggerLevel::Warn ) Logger::Get()(LoggerLevel::Warn,  __FILENAME__, __FUNCTION__, __LINE__)
#define log_error if (Logger::Get().Level() <= LoggerLevel::Error) Logger::Get()(LoggerLevel::Error, __FILENAME__, __FUNCTION__, __LINE__)
#define log_fatal if (Logger::Get().Level() <= LoggerLevel::Fatal) Logger::Get()(LoggerLevel::Fatal, __FILENAME__, __FUNCTION__, __LINE__)
//#define log_cout  if (Logger::Get().Level() <= LoggerLevel::Cout ) Logger::Get()(LoggerLevel::Cout, __FILENAME__, __FUNCTION__, __LINE__)
#define log_cout  cout
// clang-format on

/**
 * C-style print
 */
void clog_log(LogLevel level, const char* file, const char* func, int line, const char* fmt, ...);
// clang-format off
#define clog_log_trace(...)  clog_log(LoggerLevel::Trace, __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define clog_log_debug(...)  clog_log(LoggerLevel::Debug, __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define clog_log_info(...)   clog_log(LoggerLevel::Info , __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define clog_log_warn(...)   clog_log(LoggerLevel::Warn , __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define clog_log_error(...)  clog_log(LoggerLevel::Error, __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define clog_log_fatal(...)  clog_log(LoggerLevel::Fatal, __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
// clang-format on

#define LOGT clog_log_trace
#define LOGD clog_log_debug
#define LOGI clog_log_info
#define LOGW clog_log_warn
#define LOGE clog_log_error
#define LOGF clog_log_fatal
