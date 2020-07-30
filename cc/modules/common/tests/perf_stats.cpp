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
#include "cc/modules/common/tests/test.h"
#include "cc/modules/common/include/utils/perf_stats.h"

namespace rosetta {
// step1 define the global(static) timer variables
DEFINE_GLOBAL_TIMER_COUNTER(my_timer1)
DEFINE_GLOBAL_TIMER_COUNTER(my_timer2)

// step2 define an atexit function(static)
DEFINE_AT_EXIT_FUNCTION_BEG()
DEFINE_AT_EXIT_FUNCTION_BODY(my_timer1)
DEFINE_AT_EXIT_FUNCTION_BODY(my_timer2)
DEFINE_AT_EXIT_FUNCTION_END()

// step3 use ELAPSED_STATISTIC_BEG/ELAPSED_STATISTIC_END in the program
} // namespace rosetta

#include <thread>
#include <chrono>
TEST_CASE("utils perf_stats", "[common][utils]") {
  // step3 use my_timer1
  ELAPSED_STATISTIC_BEG(my_timer1);
  std::this_thread::sleep_for(std::chrono::milliseconds(113));
  ELAPSED_STATISTIC_END(my_timer1);

  // step3 use my_timer2
  ELAPSED_STATISTIC_BEG(my_timer2);
  std::this_thread::sleep_for(std::chrono::milliseconds(579));
  ELAPSED_STATISTIC_END(my_timer2);
}
