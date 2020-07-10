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

/*
** thread-safe log based on spdlog
*/

#include <cstring>
#include <string>
#include <sstream>
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

#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

#ifdef _WIN32
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

namespace LoggerLevel
{
enum LogEnum { Trace, Debug, Info, Warn, Error, Fatal };
}
using LogLevel = LoggerLevel::LogEnum;

class Logger {
  class log_stream : public std::ostringstream {
    std::ostringstream oss_prefix_;
    Logger& logger_;
    LogLevel level_;

   public:
    log_stream(Logger& logger, LogLevel level, const char* file, const char* func, int line)
        : logger_(logger), level_(level) {
#ifndef NDEBUG
      oss_prefix_ << "[" << file << ":" << line << "] ";
#endif
      }

    log_stream(const log_stream& ls) : logger_(ls.logger_) {}
    ~log_stream() {
      std::string s(std::move(str()));
      // if (s.empty() || (s[s.length() - 1] != '\n'))
      //   s += "\n";
      
#ifndef NDEBUG
      s = oss_prefix_.str() + s;
#endif
      if (logger_.to_stdout) {
        //std::cout << str() << std::endl;
        spdlog::log(static_cast<spdlog::level::level_enum>(level_), s.data());
      }

      auto logger = spdlog::get("Rosetta"); 
      if (logger_.to_file && logger.get())
        logger->log(static_cast<spdlog::level::level_enum>(level_), s.data());
    }
  };

 public:
  static Logger& Get() {
    static Logger logger;
    return logger;
  }

 public:
  void log_to_stdout(bool flag = true) { to_stdout = flag; }
  void set_level(int level) { 
    level_ = static_cast<LogLevel>(level);
    spdlog::set_level(static_cast<spdlog::level::level_enum>(level_));
  }

  void set_filename(const std::string& filename) {
    if (filename == filename_) {
      return;
    }

    filename_ = filename;
    to_file = true;
    char buf[256] = {0};
    snprintf(buf, 256, "%s.%d", filename_.data(), pid_);
    auto logger = spdlog::basic_logger_mt("Rosetta", buf);//it will auto register
  }

 private:
  Logger() {
    //mkdir("log", 0777);
    pid_ = (int)getpid();
    spdlog::set_automatic_registration(true);
  };

 public:
  virtual ~Logger() { release(); }
  void release() {}

  virtual log_stream operator()(LogLevel level, const char* file, const char* func, int line) {
    return log_stream(*this, level, file, func, line);
  }

  LogLevel Level() { return level_; }

private:
  LogLevel level_ = LoggerLevel::Info;
  int pid_;
  std::string filename_;
public:
  bool to_stdout = true; //false;
  bool to_file = false;
};

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

#include <stdarg.h>
static void clog_log(
  LogLevel level,
  const char* file,
  const char* func,
  int line,
  const char* fmt,
  ...) {
  if (Logger::Get().Level() > level)
    return;

  va_list args;
  char buf[1024 * 1024] = {0};
  va_start(args, fmt);
  vsprintf(buf, fmt, args);
  va_end(args);

  Logger::Get()(level, file, func, line) << buf;
}

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
