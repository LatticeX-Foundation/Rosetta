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
#include <string>
#include <chrono>
using namespace std;

// For details usage, see `cc/modules/common/tests/perf_stats_op.cpp`.

#define DO_SECURE_OP_PERFORMANCE_STATISTICS 1
#if DO_SECURE_OP_PERFORMANCE_STATISTICS
// ////////////////////// PERFORMANCE STATISTICS
void __add_base_op(const std::string& baseop);
void __baseop_calls_increase(const string& baseop);
void __baseop_elapse_add(const string& baseop, int64_t e);
void __subop_calls_increase(const string& baseop, const string& subop);
void __subop_elapse_add(const string& baseop, const string& subop, int64_t e);
// ////////////////////// PERFORMANCE STATISTICS
#define SECURE_OP_ADD_BASE_OP(baseop) __add_base_op(baseop)

#define SECURE_OP_KERNEL_BASE_CLASS_COMPUTE_STATS_BEG(baseop) \
  __baseop_calls_increase(baseop);                            \
  auto ___begin = std::chrono::steady_clock::now()
#define SECURE_OP_KERNEL_BASE_CLASS_COMPUTE_STATS_END(baseop)                        \
  int64_t e = std::chrono::duration_cast<std::chrono::duration<int64_t, std::nano>>( \
                std::chrono::steady_clock::now() - ___begin)                         \
                .count();                                                            \
  __baseop_elapse_add(baseop, e)

#define SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG_(baseop, opname) \
  {                                                           \
    __subop_calls_increase(baseop, #opname);                  \
    auto __##opname##_begin = std::chrono::steady_clock::now()

#define SECURE_OP_CALL_PROTOCOL_OP_STATS_END_(baseop, opname)                                    \
  auto __##opname##_end = std::chrono::steady_clock::now();                                      \
  auto __##opname##_dur = std::chrono::duration_cast<std::chrono::duration<int64_t, std::nano>>( \
                            __##opname##_end - __##opname##_begin)                               \
                            .count();                                                            \
  __subop_elapse_add(baseop, #opname, __##opname##_dur);                                         \
  }                                                                                              \
  (void)0

// ////////////////////// PERFORMANCE STATISTICS
#else
#define SECURE_OP_ADD_BASE_OP(baseop) (void)0
#define SECURE_OP_KERNEL_BASE_CLASS_COMPUTE_STATS_BEG(baseop) (void)0
#define SECURE_OP_KERNEL_BASE_CLASS_COMPUTE_STATS_END(baseop) (void)0
#define SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG_(baseop, baseop) (void)0
#define SECURE_OP_CALL_PROTOCOL_OP_STATS_END_(baseop, baseop) (void)0
#endif
// ////////////////////// PERFORMANCE STATISTICS
