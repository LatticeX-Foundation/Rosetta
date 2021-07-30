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

static void run(int partyid) {
  PROTOCOL_MPC_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  /**
   * Abs/Negative/Square/ReLU/DReLU
   */
  msg_id_t msgid("All Unary OP(s) Snn or (share,share)");
  cout << __FUNCTION__ << " " << msgid << endl;
  vector<double> X = {-10, -1.01, -2.0, 2.3, 2, 0.01, 0, -0.03, 0.031, 100};
  vector<double> Negative_X = {10.0, 1.01 ,  2.   , -2.3  , -2.   , -0.01 , -0.   ,  0.03 , -0.031, -100};
  vector<double> Square_X = {100, 1.0201, 4.0, 5.289999999999999, 4.0, 0.0001, 0.0, 0.0009, 0.0009609999999999999, 10000};
  vector<double> Relu_X = {0, 0.0, 0.0, 2.3, 2.0, 0.01, 0.0, 0.0, 0.031, 100};
  vector<double> ReluPrime_X = {0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 1.0, 1.0};
  vector<double> Sigmoid_X = {0.0, 0.267, 0.119, 0.909, 0.881, 0.502, 0.5, 0.4925, 0.508, 1.0};
  vector<double> Abs_X = {10.0, 1.01, 2.0, 2.3, 2, 0.01, 0, 0.03, 0.031, 100.0};
  vector<string> shareX, shareY;
  mpc_proto->GetOps(msgid)->PrivateInput(node_id_2, X, shareX);
  attr_type attr;
#define mpc_unary_f(op)                                                    \
  do {                                                                       \
    string tag(#op);                                                         \
    {                                                                        \
      mpc_proto->GetOps(msgid)->op(shareX, shareY, &attr);     \
      vector<double> Y;                                \
      mpc_proto->GetOps(msgid)->Reveal(shareY, Y, &reveal_attr);           \
      SIMPLE_AROUND_EQUAL_T(Y, op##_X, tag+"=P"+std::to_string(partyid));\
    }                                                                        \
  } while (0)

  mpc_unary_f(Abs);
  mpc_unary_f(Negative);
  mpc_unary_f(Square);
  mpc_unary_f(Relu);
  mpc_unary_f(ReluPrime);
  mpc_unary_f(Sigmoid);
#undef mpc_unary_f

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  PROTOCOL_MPC_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);