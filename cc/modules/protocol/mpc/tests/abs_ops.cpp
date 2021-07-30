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
// only for disable vscode warnings
#ifndef PROTOCOL_MPC_TEST
#define PROTOCOL_MPC_TEST_SNN 1
#endif

#include "cc/modules/protocol/mpc/tests/test.h"

static vector<double> expect_abs(const vector<double>& x) {
  vector<double> expect(x.size(), 0.0);
  for (size_t i = 0; i < x.size(); i++)
  {
    expect[i] = std::abs(x[i]);
  }
  
  return expect;
}

static vector<double> expect_abs_prime(const vector<double>& x) {
  vector<double> expect(x.size(), 0.0);
  for (size_t i = 0; i < x.size(); i++)
  {
    expect[i] = x[i] >= 0.0 ? 1 : -1;
  }
  
  return expect;
}

void run(int partyid) {
  PROTOCOL_MPC_TEST_INIT(partyid);
  msg_id_t msgid("----unit tests of abs-ops----");
  // input a b
  vector<double> x = {-10.0, -3.0, 10.0, 3.0, 0.003, -0.02};
  vector<string> input_str, out_str;
  mpc_proto->GetOps(msgid)->PrivateInput(node_id_0, x, input_str);
  attr_type attr;
  
  // case 1: Abs
  mpc_proto->GetOps(msgid)->Abs(input_str, out_str, &attr);
  // reveal c
  vector<double> c;
  mpc_proto->GetOps(msgid)->Reveal(out_str, c, &reveal_attr);
  auto expect_abs_ = expect_abs(x);
  // print_vec(c, c.size(), "Abs result plain:");
  // print_vec(expect_abs_, expect_abs_.size(), "Abs result expect:");
  HD_AROUND_EQUAL_T(c, expect_abs_, "Abs=P"+std::to_string(partyid));

  // case 2: AbsPrime
  mpc_proto->GetOps(msgid)->AbsPrime(input_str, out_str, &attr);
  // reveal c
  mpc_proto->GetOps(msgid)->Reveal(out_str, c, &reveal_attr);
  auto expect_abs_prime_ = expect_abs_prime(x);
  HD_AROUND_EQUAL_T(c, expect_abs_prime_, "AbsPrime=P"+std::to_string(partyid));
  

  PROTOCOL_MPC_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);