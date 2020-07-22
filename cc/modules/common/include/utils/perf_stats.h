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
  std::cout << "-> " #timer ": " << __##timer * 1.0 / 1e9 << std::endl;
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
  std::string vmpeak = "";
  struct __stat {
    // io
    int64_t bytes_sent = 0;
    int64_t bytes_recv = 0;
    int64_t msg_sent = 0;
    int64_t msg_recv = 0;
    double elapsed_sent = 0;
    double elapsed_recv = 0;
    // time
    double clock_seconds = 0;
    double cpu_seconds = 0;
    clock_t start = 0;
    struct timespec req_start, req_end;
  } p, r, a; // prepare,run,all(p+r)
  friend __stat operator+(const __stat& l, const __stat& r) {
    __stat t;
#define _add(v) t.v = l.v + r.v
    _add(bytes_sent);
    _add(bytes_recv);
    _add(msg_sent);
    _add(msg_recv);
    _add(elapsed_sent);
    _add(elapsed_recv);

    _add(clock_seconds);
    _add(cpu_seconds);
#undef _add
    return t;
  }
  void reset() {
    memset(&p, 0, sizeof(__stat));
    memset(&r, 0, sizeof(__stat));
    memset(&a, 0, sizeof(__stat));
    vmpeak = "";
  }
  std::string stats(int party);
  std::string stats2(int party);
};
} // namespace rosetta
