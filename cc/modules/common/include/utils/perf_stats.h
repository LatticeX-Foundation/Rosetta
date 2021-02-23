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
#include "simple_timer.h"
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <iostream>
#include <atomic>
#include <string>
#include <vector>
#include <iomanip>

#define DO_ELAPSED_STATISTIC 0
namespace rosetta {
/**
 * Usage:
 *  @see tests/perf_stats.cpp for details
 *  OR global search these macros
 */

#if DO_ELAPSED_STATISTIC
/**
 * step1 use this macro to define some global(static) timer variables
 */
#define DEFINE_GLOBAL_TIMER_COUNTER(timer) static std::atomic<int64_t> __##timer{0};

/**
 * step2 use these macros to define an atexit function(static) 
 * the following 3 MACROS must be used together, agaist with DEFINE_GLOBAL_TIMER_COUNTER
 */
#define DEFINE_AT_EXIT_FUNCTION_BEG()                \
  static std::atomic<int64_t> __reg_exit_counter{0}; \
  static void __elapsed_atexit_fn() {
#define DEFINE_AT_EXIT_FUNCTION_BODY(timer) \
  std::cout << "-> " << std::setw(30) << #timer "(s): " << __##timer * 1.0 / 1e9 << std::endl;
#define DEFINE_AT_EXIT_FUNCTION_END() }

/**
 * step3 use the following macros to statistic the elapsed
 */
#define ELAPSED_STATISTIC_BEG(timer)        \
  if (rosetta::__reg_exit_counter++ == 0) { \
    atexit(rosetta::__elapsed_atexit_fn);   \
  }                                         \
  SimpleTimer __##timer##_start
#define ELAPSED_STATISTIC_END(timer) \
  __##timer##_start.stop();          \
  rosetta::__##timer += __##timer##_start.ns_elapse()

#else
#define DEFINE_GLOBAL_TIMER_COUNTER(timer)

#define DEFINE_AT_EXIT_FUNCTION_BEG()
#define DEFINE_AT_EXIT_FUNCTION_BODY(timer)
#define DEFINE_AT_EXIT_FUNCTION_END()

#define ELAPSED_STATISTIC_BEG(timer) (void)0
#define ELAPSED_STATISTIC_END(timer) (void)0
#endif
} // namespace rosetta

/**
 * Performance Statitistic. Time, Network, CPU, Memory, etc.
 */
namespace rosetta {
class PerfStats {
 public:
  SimpleTimer timer; // for s.elapse field

 public:
  // name,tag,...
  std::string name = "default";
  struct __stat {
    // io
    uint64_t bytes_sent = 0;
    uint64_t bytes_recv = 0;
    uint64_t msg_sent = 0;
    uint64_t msg_recv = 0;
    double elapsed_sent = 0;
    double elapsed_recv = 0;
    // time
    double clock_seconds = 0;
    double cpu_seconds = 0;
    double elapse = 0;
    // mem
    int64_t max_vmrss = 0; // kB
    // cpu
    double max_cpuusage = 0; // %CPU
    double avg_cpuusage = 0; // %CPU
  } s;
  struct timespec process_cpu_time; // for s.cpu_seconds field

  bool do_memcpu_stats = false;
  /**
   * start timer/mem/cpu/...
   * \param sampling whether to sample statistics for memory, etc.
   */
  void start_perf_stats(bool sampling = false);
  //! \param stop when set to true, statistics will stop
  __stat get_perf_stats(bool stop = false);

  void reset();

  std::string to_console();

  //! return an object. eg: {...}
  std::string to_json(bool pretty = false) const;

  //! return an object-array. eg: {{...},{...},...}
  static std::string to_json(const std::vector<PerfStats>& ps, bool pretty = false);
};

PerfStats operator+(const PerfStats& l, const PerfStats& r);
PerfStats operator-(const PerfStats& l, const PerfStats& r);
PerfStats operator/(const PerfStats& l, int r);

} // namespace rosetta
