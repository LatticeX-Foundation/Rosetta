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
  log_info << "-------  begin of contributed-ops test ------";
  msg_id_t msgid("----unit tests of contributed-ops (Reciprocaldiv, Rsqrt, Sqrt)----");
  vector<double> c;

  // input x, y
  vector<double> X = {-1.01, -2.00, -3.01, 1.3, 2.02, 3.14, +2, -0.01, 19.0, 10.0};
  vector<double> Y = {-1.00, -2.01, -3.01, 1.3, 2.03, 3.12, -2, +0.01, 10.0, 10.0};
  
  vector<string> input_str_x, input_str_y, out_str;
  mpc_proto->GetOps(msgid)->PrivateInput(node_id_0, X, input_str_x);
  mpc_proto->GetOps(msgid)->PrivateInput(node_id_0, Y, input_str_y);
  attr_type attr;
  
  /////////   contributed ops tests /////////
  mpc_proto->GetOps(msgid)->Exp(input_str_x, out_str, &attr);
  mpc_proto->GetOps(msgid)->Reveal(out_str, c, &reveal_attr);
  // vector<double> ExpExpect = {0.364,  0.135, 0.049, 3.669,       7.538,
  //                             23.104, 7.389, 0.990, 178482301.0, 22026.466};
  vector<double> ExpExpect = {0.364,  0.135, 0.049, 3.669,       7.538,
                              23.104, 7.389, 0.990};
  c.resize(c.size()-2);
  AROUND_EQUAL_T(c, ExpExpect, 0.5, "Exp=P"+std::to_string(partyid));

  vector<double> ReciprocaldivExpect = {1.01,  0.99502488,  1.0, 1., 0.99507389,  1.00641026, -1.0,  -1.0,  1.9, 1.0};//forth element divides zero!!!
  mpc_proto->GetOps(msgid)->Reciprocaldiv(input_str_x, input_str_y, out_str, &attr);
  mpc_proto->GetOps(msgid)->Reveal(out_str, c, &reveal_attr);
  SIMPLE_AROUND_EQUAL_T(c, ReciprocaldivExpect, "Reciprocaldiv=P"+std::to_string(partyid));


  // ///// update input with positive inputs ///
  vector<double> X_Positive = {0.4, 0.86, 1.2, 1, 1.3, 2.02, 3.14, +2, 4, 19.0, 100.0, 225.0, 256.0};
  vector<string> input_str_x2;
  mpc_proto->GetOps(msgid)->PrivateInput(node_id_0, X_Positive, input_str_x2);

  vector<double> RsqrtExpect = {1.58113883, 1.07832773, 0.91287093, 1., 0.87705802, 0.70359754, 0.56433265, 0.70710678, 0.5, 0.22941573, 0.1, 1.0/15.0, 1.0/16.0};
  vector<double> SqrtExpect = {0.63245553, 0.92736185, 1.09544512, 1., 1.14017543, 1.42126704, 1.77200451, 1.41421356, 2., 4.35889894, 10, 15.0, 16.0};

  mpc_proto->GetOps(msgid)->Sqrt(input_str_x2, out_str, &attr);
  mpc_proto->GetOps(msgid)->Reveal(out_str, c, &reveal_attr);
  SIMPLE_AROUND_EQUAL_T(c, SqrtExpect, "Sqrt=P"+std::to_string(partyid));

  mpc_proto->GetOps(msgid)->Rsqrt(input_str_x2, out_str, &attr);
  mpc_proto->GetOps(msgid)->Reveal(out_str, c, &reveal_attr);
  SIMPLE_AROUND_EQUAL_T(c, RsqrtExpect, "Rsqrt=P"+std::to_string(partyid));

  log_info << "-------  end of contributed-ops test ------";
  PROTOCOL_MPC_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);