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
#include "cc/modules/common/include/utils/perf_stats.h"
#include "cc/tf/secureops/secure_base_kernel.h"

using namespace std;

#if DO_SECURE_OP_PERFORMANCE_STATISTICS
namespace tensorflow {
mutex secure_op_stats_mtx;
map<string, secure_op_stats*> map_op_stats;
atomic<int64_t> init_exit_counter;
void secure_op_stats_exit_func() {
  size_t max_width = 0;
  for (auto& iter : map_op_stats) {
    auto stat = iter.second;
    max_width = std::max(stat->op.length(), max_width);
  }
  string s_("");
  for (size_t i = 0; i < max_width + 1; i++) {
    s_ += "-";
  }

  std::stringstream ss1;
  // clang-format off
  ss1 << "secureop OPL: only protocol level elapsed; OSL: only secure level elapsed; elapsed: OPL + OSL" << endl;
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
    secure_op_total_elapse += stat->elapse;
    for (auto& iter2 : stat->protocol_op_elapse) {
      protoc_op_total_elapse += iter2.second;
    }
  }

  std::stringstream ss2;
  for (auto& iter : map_op_stats) {
    auto stat = iter.second;
    ss2 << "secureop |" << std::setw(max_width) << stat->op << " |" << std::setw(9) << stat->calls;

    int64_t protocol_level_elapse = 0;
    for (auto& iter2 : stat->protocol_op_elapse) {
      protocol_level_elapse += iter2.second;
    }
    ss2 << " |" << std::setw(14) << std::fixed << protocol_level_elapse / 1e9;
    ss2 << " |" << std::setw(14) << std::fixed << (stat->elapse - protocol_level_elapse) / 1e9;
    ss2 << " |" << std::setw(14) << std::fixed << stat->elapse / 1e9;
    ss2 << " |" << std::setw(14) << std::fixed << stat->elapse * 100.0 / secure_op_total_elapse;
    ss2 << " |" << endl;

    for (auto& iter2 : stat->protocol_op_elapse) {
      ss2 << "secureop |" << std::setw(max_width) << iter2.first << " >" << std::setw(9) << ""
          << " |" << std::setw(14) << std::fixed << iter2.second / 1e9;
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
} // namespace tensorflow
#endif