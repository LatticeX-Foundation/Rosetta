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
#include "cc/modules/common/include/utils/perf_stats_op.h"

#include <atomic>
#include <map>
#include <mutex>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <iostream>
using namespace std;

#if DO_SECURE_OP_PERFORMANCE_STATISTICS

struct __op_stats {
  atomic<int64_t> calls{0};
  atomic<int64_t> elapse{0};
};
struct op_stats {
  op_stats(const string& _op) : op(_op) {}
  string op; // secure op name
  __op_stats secure_op_stats; // secure op
  map<string, __op_stats> protocol_op_stats; // protocol op
};

mutex op_stats_mtx;
map<string, op_stats*> map_op_stats;
atomic<int64_t> init_exit_counter;
std::chrono::time_point<std::chrono::steady_clock> ops_wall_clock;

void __op_stats_exit_func();
void __add_base_op(const std::string& baseop) {
  if (init_exit_counter++ == 0) {
    atexit(__op_stats_exit_func);
    ops_wall_clock = std::chrono::steady_clock::now();
  }
  unique_lock<mutex> lck(op_stats_mtx);
  auto iter = map_op_stats.find(baseop);
  if (iter == map_op_stats.end()) {
    map_op_stats[baseop] = new op_stats(baseop);
  }
}
void __baseop_calls_increase(const string& baseop) {
  map_op_stats[baseop]->secure_op_stats.calls++;
}
void __baseop_elapse_add(const string& baseop, int64_t e) {
  map_op_stats[baseop]->secure_op_stats.elapse += e;
}
void __subop_calls_increase(const string& baseop, const string& subop) {
  map_op_stats[baseop]->protocol_op_stats[subop].calls++;
}
void __subop_elapse_add(const string& baseop, const string& subop, int64_t e) {
  map_op_stats[baseop]->protocol_op_stats[subop].elapse += e;
}

void __op_stats_exit_func() {
  size_t max_width = strlen("(%)Summary");
  for (auto& iter : map_op_stats) {
    auto stat = iter.second;
    max_width = std::max(stat->op.length(), max_width);
    for (auto& iter2 : stat->protocol_op_stats) {
      max_width = std::max(iter2.first.length(), max_width);
    }
  }
  string s_("");
  for (size_t i = 0; i < max_width + 1; i++) {
    s_ += "-";
  }

  int64_t ops_wall_elapsed = std::chrono::duration_cast<std::chrono::duration<int64_t, std::nano>>(
                               std::chrono::steady_clock::now() - ops_wall_clock)
                               .count();

  std::stringstream ss1;
  // clang-format off
  ss1 << "secureop      : ALL OPs WALL CLOCK ELAPSED(s): " << std::fixed << ops_wall_elapsed / 1e9;
  ss1 << " (counting from the time the first OP is created)" << endl;
  ss1 << "secureop  name: the protocol level op (NAME >) called by the secure level op (NAME |)" << endl;
  ss1 << "secureop calls: number of times different OP are called" << endl;
  ss1 << "secureop   OPL: only protocol level elapsed; OSL: only secure level elapsed; elapsed: OPL + OSL" << endl;
  ss1 << "secureop  Note: the elapsed(OPL/OSL/elapsed) is the sum of all parallel elapse for each OP" << endl;
  ss1 << "secureop +" << s_ << "+----------+---------------+---------------+---------------+---------------+" << endl;
  ss1 << "secureop |" << setw(max_width+1) << "name "        << "|" << setw(10) << "calls "  
      << "|" << setw(15) << "OPL(s) "  << "|" << setw(15) << "OSL(s) "  
      << "|" << setw(15) << "elapsed(s) "<< "|" << setw(15) << "percent(%) " << "|" << endl;
  ss1 << "secureop +" << s_ << "+----------+---------------+---------------+---------------+---------------+" << endl;
  // clang-format on

  int64_t secure_op_total_elapse = 0;
  int64_t protoc_op_total_elapse = 0;
  for (auto& iter : map_op_stats) {
    auto stat = iter.second;
    secure_op_total_elapse += stat->secure_op_stats.elapse;
    for (auto& iter2 : stat->protocol_op_stats) {
      protoc_op_total_elapse += iter2.second.elapse;
    }
  }

  std::stringstream ss2;
  for (auto& iter : map_op_stats) {
    auto stat = iter.second;

    int64_t protocol_level_calls = 0;
    int64_t protocol_level_elapse = 0;
    for (auto& iter2 : stat->protocol_op_stats) {
      protocol_level_calls += iter2.second.calls;
      protocol_level_elapse += iter2.second.elapse;
    }

    // secure op
    auto& sos = stat->secure_op_stats;
    //string s_calls = to_string(sos.calls) + "[" + to_string(protocol_level_calls) + "]";
    string s_calls = "[" + to_string(sos.calls) + "]";
    ss2 << "secureop |" << std::setw(max_width) << stat->op << " |" << std::setw(9) << s_calls;
    ss2 << " |" << std::setw(14) << std::fixed << protocol_level_elapse / 1e9;
    ss2 << " |" << std::setw(14) << std::fixed << (sos.elapse - protocol_level_elapse) / 1e9;
    ss2 << " |" << std::setw(14) << std::fixed << sos.elapse / 1e9;
    ss2 << " |" << std::setw(14) << std::fixed << sos.elapse * 100.0 / secure_op_total_elapse;
    ss2 << " |" << endl;

    // protocol op(s)
    for (auto& iter2 : stat->protocol_op_stats) {
      auto& pos = iter2.second;
      string s_calls = to_string(pos.calls); // "[" + to_string(pos.calls) + "]";
      ss2 << "secureop |" << std::setw(max_width) << iter2.first << " >" << std::setw(9) << s_calls
          << " |" << std::setw(14) << std::fixed << pos.elapse / 1e9;
      ss2 << " |" << std::setw(14) << ""
          << " |" << std::setw(14) << ""
          << " |" << std::setw(14) << ""
          << " |" << endl;
    }

    delete stat;
  }
  ss2 << "secureop +" << s_
      << "+----------+---------------+---------------+---------------+---------------+" << endl;

  ss2 << "secureop |" << std::setw(max_width) << "Summary"
      << " |" << std::setw(9) << "-"
      << " |" << std::setw(14) << std::fixed << protoc_op_total_elapse / 1e9 << " |"
      << std::setw(14) << std::fixed << (secure_op_total_elapse - protoc_op_total_elapse) / 1e9
      << " |" << std::setw(14) << std::fixed << secure_op_total_elapse / 1e9 << " |"
      << std::setw(14) << ""
      << " |" << endl;
  ss2 << "secureop |" << std::setw(max_width) << "(%)Summary"
      << " |" << std::setw(9) << "-"
      << " |" << std::setw(14) << std::fixed
      << protoc_op_total_elapse * 100.0 / secure_op_total_elapse << " |" << std::setw(14)
      << std::fixed
      << (secure_op_total_elapse - protoc_op_total_elapse) * 100.0 / secure_op_total_elapse << " |"
      << std::setw(14) << ""
      << " |" << std::setw(14) << std::fixed
      << secure_op_total_elapse * 100.0 / secure_op_total_elapse << " |" << endl;

  ss2 << "secureop +" << s_
      << "+----------+---------------+---------------+---------------+---------------+" << endl;
  std::cout << ss1.str() << ss2.str();
}
#endif
