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
#include "cc/modules/common/include/utils/perf_stats_op.h"

#include <chrono>
#include <thread>

void op_add() { std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 1000)); }
void op_sub() { std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 1000)); }
void op_mul() { std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 3000)); }
void my_big_op() {
  SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG_("MyBigOp", OpAdd);
  op_add();
  SECURE_OP_CALL_PROTOCOL_OP_STATS_END_("MyBigOp", OpAdd);

  SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG_("MyBigOp", OpSub);
  op_sub();
  SECURE_OP_CALL_PROTOCOL_OP_STATS_END_("MyBigOp", OpSub);

  SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG_("MyBigOp", OpMul);
  op_mul();
  SECURE_OP_CALL_PROTOCOL_OP_STATS_END_("MyBigOp", OpMul);

  SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG_("MyBigOp", OpMul);
  op_mul();
  SECURE_OP_CALL_PROTOCOL_OP_STATS_END_("MyBigOp", OpMul);

  SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG_("MyBigOp", OpSub);
  op_sub();
  SECURE_OP_CALL_PROTOCOL_OP_STATS_END_("MyBigOp", OpSub);
}

TEST_CASE("utils perf_stats_op", "[common][utils]") {
  // init
  SECURE_OP_ADD_BASE_OP("MyBigOp");

  // the whole perfmance of `MyBigOp`
  SECURE_OP_KERNEL_BASE_CLASS_COMPUTE_STATS_BEG("MyBigOp");
  my_big_op();
  SECURE_OP_KERNEL_BASE_CLASS_COMPUTE_STATS_END("MyBigOp");
}
