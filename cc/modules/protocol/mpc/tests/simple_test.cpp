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


void run(int partyid) {
  PROTOCOL_MPC_TEST_INIT(partyid);
  // mpc_proto->SetFloatPrecision(20);
  log_info << "-------  begin of simple test ------";
  msg_id_t msgid("----unit tests of simple-ops(add,sub,mul)----");
  vector<double> c;

  // input x, y
  vector<double> X = {-1.01, -2.00, -3.01, 0, 1.3, 2.02, 3.14, +2, -0.01, 19.0, 10.0};
  vector<double> Y = {-1.00, -2.01, -3.01, 0, 1.3, 2.03, 3.12, -2, +0.01, 10.0, 10.0};
  vector<double> AddExpect = {-2.01, -4.01, -6.02, 0., 2.6, 4.05, 6.26, 0., 0., 29., 20.};
  vector<double> SubExpect = {-0.01, 0.01, 0., 0., 0., -0.01, 0.02, 4., -0.02, 9., 0};
  vector<double> MulExpect = {1.010, 4.020,  9.060,   0.000, 1.690, 4.101,
                          9.797, -4.000, -0.0001, 190,   100};
  
  
  vector<string> input_str_x, input_str_y, out_str;
  mpc_proto->GetOps(msgid)->PrivateInput(node_id_0, X, input_str_x);
  mpc_proto->GetOps(msgid)->PrivateInput(node_id_0, Y, input_str_y);
  attr_type attr;
  
  
  mpc_proto->GetOps(msgid)->Add(input_str_x, input_str_y, out_str, &attr);
  mpc_proto->GetOps(msgid)->Reveal(out_str, c, &reveal_attr);
  HD_AROUND_EQUAL_T(c, AddExpect, "Add=P" + std::to_string(partyid));

  mpc_proto->GetOps(msgid)->Sub(input_str_x, input_str_y, out_str, &attr);
  mpc_proto->GetOps(msgid)->Reveal(out_str, c, &reveal_attr);
  HD_AROUND_EQUAL_T(c, SubExpect, "Sub=P" + std::to_string(partyid));

  mpc_proto->GetOps(msgid)->Mul(input_str_x, input_str_y, out_str, &attr);
  mpc_proto->GetOps(msgid)->Reveal(out_str, c, &reveal_attr);
  HD_AROUND_EQUAL_T(c, MulExpect, "Mul=P"+std::to_string(partyid));

  log_info << "-------  end of simple test ------";
  PROTOCOL_MPC_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);