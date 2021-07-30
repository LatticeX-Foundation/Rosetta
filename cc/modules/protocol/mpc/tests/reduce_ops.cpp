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
   * All basic Reduce OP(s)
   * Sum/Mean/AddN/...
   */

  {
    msg_id_t msgid("All basic Reduce OP(s) (share,share)");
    cout << __FUNCTION__ << " " << msgid << "rows: " << 2 << ", col:" << 5 << endl;

    vector<double> X = {-1.01, -2.00, -3.01, 0, 1.3, 2.02, 3.14, +2, -0.01, 6.54321};
    vector<double> AddN_X = {1.01,  1.1400001, -1.01, -0.01, 7.84321};
    vector<double> Sum_X = {-4.7200003, 13.69321};
    vector<double> Mean_X = {-0.94400007,  2.738642};
    vector<double> Max_X = {1.3, 6.54321};
    vector<double> Min_X = {-3.01, -0.01};
    
    size_t size = X.size();
    vector<string> strX, strY, strZ;
    mpc_proto->GetOps(msgid)->PrivateInput(node_id_0, X, strX);

#define mpc_reduce_f(op)                             \
  do {                                                 \
    string tag(#op);                                   \
    attr_type attr;                                    \
    attr["rows"] = "2";                                \
    attr["cols"] = "5";                                \
    {                                                  \
      mpc_proto->GetOps(msgid)->op(strX, strY, &attr);     \
      vector<double> Y;                                \
      mpc_proto->GetOps(msgid)->Reveal(strY, Y, &reveal_attr);           \
      HD_AROUND_EQUAL_T(Y, op##_X, tag+"=P"+std::to_string(partyid));            \
    }                                                   \
  } while (0)

    mpc_reduce_f(AddN);
    mpc_reduce_f(Sum);
    mpc_reduce_f(Mean);
    mpc_reduce_f(Max);
    mpc_reduce_f(Min);
#undef mpc_reduce_f
  }

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  PROTOCOL_MPC_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);