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
** thread-safe
*/

#include "safe_queue.h"

#include <ctime>
#include <mutex>
#include <thread>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
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

#ifdef _WIN32
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

namespace utils {
// temporary/simple cope
static inline std::string local_time_for_log() {
  time_t t = time(NULL);
  struct tm* lt = localtime(&t);
  struct timeval tval;
  gettimeofday(&tval, NULL);

  char buf[32] = {0};
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", lt);
  sprintf(buf, "%s.%03ld", buf, tval.tv_usec / 1000);
  return std::string(buf);
}

static inline std::string base_name(const char* filepath) {
  std::string s(filepath);
  size_t pos = std::string::npos;
  if ((pos = s.find_last_of("/")) != std::string::npos) {
    s = s.substr(pos + 1);
  }
  return s;
}

// create_directories("/aa/bb/cc/")
static int32_t create_directories(const std::string& directoryPath) {
  char tmpDirPath[256] = {0};
  for (uint32_t i = 0; i < directoryPath.length(); ++i) {
    tmpDirPath[i] = directoryPath[i];
    if (tmpDirPath[i] == '\\' || tmpDirPath[i] == '/') {
      if (access(tmpDirPath, 0) != 0) {
        int32_t ret = mkdir(tmpDirPath, 0777);
        if (ret != 0) {
          return ret;
        }
      }
    }
  }
  return 0;
}
} // namespace utils

#define USE_LOGGER_QUEUE 0
typedef safe_queue<std::string> log_queue;

enum class LogLevel { Cout, Trace, Debug, Info, Warn, Error, Fatal };
static const char* slog_level[] = {"Cout ", "Trace", "Debug", "Info ", "Warn ", "Error", "Fatal"};

class Logger {
  class log_stream : public std::ostringstream {
    std::ostringstream oss_prefix_;

   public:
    log_stream(Logger& logger, LogLevel level, const char* file, const char* func, int line)
        : logger_(logger) {
      // FORMAT:
      // *this << "[DATE TIME][PROCESS ID][THREAD ID][LOG LEVEL][FILENAME][FUNCTION][LINE] MESSAGE";
      oss_prefix_ << "[" << utils::local_time_for_log() << "]"
                  << "[" << logger_.pid_ << "]"
                  << "[" << std::this_thread::get_id() << "]"
                  << "[" << slog_level[static_cast<int>(level)] << "]"
                  << "[" << utils::base_name(file) << "]"
                  << "[" << func << "]"
                  << "[" << line << "] ";
    }
    log_stream(const log_stream& ls) : logger_(ls.logger_) {}
    ~log_stream() {
      //*this << std::endl; // can remove this line

      std::string s(std::move(str()));
      if (s[s.length() - 1] != '\n')
        s += "\n";

      if (logger_.to_stdout)
        std::cout << s; // maybe not to stdout

      s = oss_prefix_.str() + s;
#if USE_LOGGER_QUEUE
      logger_.queue_.push(std::move(s));
#endif
      {
        std::unique_lock<std::mutex> lck(logger_.to_file_mtx_);
        logger_.to_file(std::move(s));
      }
    }

   private:
    Logger& logger_;
  };

 public:
  static Logger& Get() {
    static Logger logger;
    return logger;
  }

 public:
  ofstream ofile;
  bool to_stdout = false;

 public:
  void log_to_stdout(bool flag = true) {
    to_stdout = flag;
  }
  void set_filename(const std::string& filename) {
    if (filename.empty() || (filename == filename_)) {
      return;
    }
    std::unique_lock<std::mutex> lck(to_file_mtx_);
    if (filename == filename_) {
      return;
    }
    utils::create_directories(filename);
    string filenametmp = filename + "-" + std::to_string(pid_);
    if (false) {
      // try open
      ofstream ofiletemp;
      ofiletemp.open(filenametmp);
      if (ofiletemp.bad()) {
        // set failed
        return;
      }
      ofiletemp.close();
    }
    filename_ = filenametmp;
    if (ofile.is_open()) {
      ofile.flush();
      ofile.clear();
      ofile.close();
      usleep(1000);
    }
    ofile.open(filename_);
  }
  std::string filename_ = "default";

 public:
  Logger() {
    mkdir("log", 0777);
    pid_ = (int)getpid();
    //ofile.open("default-" + std::to_string(pid_) + ".log");

#if USE_LOGGER_QUEUE
    thread_ = std::thread(&Logger::pop_from_queue, this);
    thread_.detach();
#endif
  };
  virtual ~Logger() {
    release();
  }
  void release() {
    ofile.close();
  }

  virtual log_stream operator()(LogLevel level, const char* file, const char* func, int line) {
    return log_stream(*this, level, file, func, line);
  }

  void pop_from_queue() {
    while (true) {
      std::string s;
      queue_.pop(s);
    }
  }
  void to_file(const std::string& s) {
    if (ofile.good()) {
      ofile << s;
      ofile.flush();
    }
  }

  LogLevel Level() {
    return level_;
  }

  LogLevel level_ = LogLevel::Debug;

  std::thread thread_;
  std::mutex to_file_mtx_;
  log_queue queue_;
  int pid_;
};

// clang-format off
#define log_trace if (Logger::Get().Level() <= LogLevel::Trace) Logger::Get()(LogLevel::Trace, __FILE__, __FUNCTION__, __LINE__)
#define log_debug if (Logger::Get().Level() <= LogLevel::Debug) Logger::Get()(LogLevel::Debug, __FILE__, __FUNCTION__, __LINE__)
#define log_info  if (Logger::Get().Level() <= LogLevel::Info ) Logger::Get()(LogLevel::Info,  __FILE__, __FUNCTION__, __LINE__)
#define log_warn  if (Logger::Get().Level() <= LogLevel::Warn ) Logger::Get()(LogLevel::Warn,  __FILE__, __FUNCTION__, __LINE__)
#define log_error if (Logger::Get().Level() <= LogLevel::Error) Logger::Get()(LogLevel::Error, __FILE__, __FUNCTION__, __LINE__)
#define log_fatal if (Logger::Get().Level() <= LogLevel::Fatal) Logger::Get()(LogLevel::Fatal, __FILE__, __FUNCTION__, __LINE__)
//#define log_cout  if (Logger::Get().Level() <= LogLevel::Cout ) Logger::Get()(LogLevel::Cout, __FILE__, __FUNCTION__, __LINE__)
#define log_cout  cout
// clang-format on

/*
Usage:
log_debug << "1" << 2 << 2.718f << 3.1415926 << false;
log_info << "log_info" << "";
log_warn << "log_warn" << "";
log_error << "log_error" << "";
log_fatal << "log_fatal" << "";
*/

#include <stdarg.h>
static void clog_log(
  LogLevel level, const char* file, const char* func, int line, const char* fmt, ...) {
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
#define clog_log_trace(...)  clog_log(LogLevel::Trace, __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define clog_log_debug(...)  clog_log(LogLevel::Debug, __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define clog_log_info(...)   clog_log(LogLevel::Info , __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define clog_log_warn(...)   clog_log(LogLevel::Warn , __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define clog_log_error(...)  clog_log(LogLevel::Error, __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define clog_log_fatal(...)  clog_log(LogLevel::Fatal, __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
// clang-format on

#define LOGT clog_log_trace
#define LOGD clog_log_debug
#define LOGI clog_log_info
#define LOGW clog_log_warn
#define LOGE clog_log_error
#define LOGF clog_log_fatal
