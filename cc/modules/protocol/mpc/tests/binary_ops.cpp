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
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  /**
   * All basic Binary OP(s), eg.
   * Add/Sub/Mul/Div/Compare(s)
   */
  {
    msg_id_t msgid("All basic Binary OP(s) (share,share)");
    cout << __FUNCTION__ << " " << msgid << endl;

    vector<double> X = {-1.01, -2.00, -3.01, 1.3, 2.02, 3.14, +2, -0.01, 19.0, 10.0};
    vector<double> Y = {-1.00, -2.01, -3.01, 1.3, 2.03, 3.12, -2, +0.01, 10.0, 10.0};
    vector<double> CONST_POW_N = {0, 1, 2, 4, 2, 3, 2, 0, 3, 2};
    vector<double> Add_X = {-2.01, -4.01, -6.02, 2.6 , 4.05, 6.26, 0. , 0. , 29. , 20.};
    vector<double> Sub_X = {-0.01,  0.01,  0. ,0. , -0.01, 0.02, 4., -0.02, 9.,  0};
    vector<double> Pow_X = {1.000, -2.000,  9.060, 2.856,  4.080,  31,  4.000, 1.000,  6859,  100};
    vector<double> Mul_X = {1.010,  4.020,  9.060, 1.690,  4.101,  9.797, -4.000, -0.0001,  190,  100};
    vector<double> Truediv_X = {1.01,  0.99502486,  1.,  1., 0.9950739, 1.0064104, -1., -1.,  1.9, 1.}; //the fourth `nan` set to -2
    vector<double> Floordiv_X = {1.,  0.,  1.,  1.,  0.,  1., -1., -1.,  1.,  1};//the fourth `nan` set to -2
    vector<double> Div_X = {1.01,  0.99502488,  1.,  1., 0.99507389, 1.00641026, -1., -1.,  1.9, 1.};//the fourth `nan` set to -2

    vector<double> Greater_X = {0, 1, 0, 0, 0, 1, 1, 0, 1, 0};
    vector<double> GreaterEqual_X = {0, 1, 1, 1, 0, 1, 1, 0, 1, 1};
    vector<double> Equal_X = {0, 0, 1, 1, 0, 0, 0, 0, 0, 1};
    vector<double> NotEqual_X = {1, 1, 0, 0, 1, 1, 1, 1, 1, 0};
    vector<double> LessEqual_X = {1, 0, 1, 1, 1, 0, 0, 1, 0, 1};
    vector<double> Less_X = {1, 0, 0, 0, 1, 0, 0, 1, 0, 0};
    
    size_t size = X.size();

    vector<string> strX, strY, strZ;
    mpc_proto->GetOps(msgid)->PrivateInput(node_id_0, X, strX);
    mpc_proto->GetOps(msgid)->PrivateInput(node_id_1, Y, strY);

    // print_vec(X, 20, "input X");
    // print_vec(Y, 20, "input Y");

    int float_precision = mpc_proto->GetMpcContext()->FLOAT_PRECISION;
    vector<string> literalX, literalY, const_pow_literalY;
    convert_double_to_literal_str(X, literalX, float_precision);
    convert_double_to_literal_str(Y, literalY, float_precision);
    convert_double_to_literal_str(CONST_POW_N, const_pow_literalY, float_precision);

#define snn_binary_f(op)                                        \
  do {                                                            \
    string tag(#op);                                              \
    {                                                             \
      mpc_proto->GetOps(msgid)->op(strX, strY, strZ);                 \
      vector<double> Z;                                           \
      mpc_proto->GetOps(msgid)->Reveal(strZ, Z, &reveal_attr);                      \
      SIMPLE_AROUND_EQUAL_T(Z, op##_X, tag + "(private,private)" + "=P" + std::to_string(partyid)); \
    }                                                             \
    {                                                             \
      attr_type attr;                                             \
      attr["lh_is_const"] = "1";                                  \
      mpc_proto->GetOps(msgid)->op(literalX, strY, strZ, &attr);             \
      vector<double> Z;                                           \
      mpc_proto->GetOps(msgid)->Reveal(strZ, Z, &reveal_attr);                      \
      SIMPLE_AROUND_EQUAL_T(Z, op##_X, tag + "(const,private)" + "=P" + std::to_string(partyid)); \
    }                                                                                                                      \
    {                                                             \
      attr_type attr;                                             \
      attr["rh_is_const"] = "1";                                  \
      mpc_proto->GetOps(msgid)->op(strX, literalY, strZ, &attr);      \
      vector<double> Z;                                           \
      mpc_proto->GetOps(msgid)->Reveal(strZ, Z, &reveal_attr);                      \
      SIMPLE_AROUND_EQUAL_T(Z, op##_X, tag + "(private,const)" + "=P" + std::to_string(partyid)); \
    }                                                                                                                   \
  } while (0)


  #define snn_binary_f_rh_const(op)                                        \
  do {                                                            \
    string tag(#op);                                              \
    {                                                             \
      attr_type attr;                                             \
      attr["rh_is_const"] = "1";                                  \
      mpc_proto->GetOps(msgid)->op(strX, const_pow_literalY, strZ, &attr);      \
      vector<double> Z;                                           \
      mpc_proto->GetOps(msgid)->Reveal(strZ, Z, &reveal_attr);                      \
      AROUND_EQUAL_T(Z, op##_X, 0.1, tag + "(private,const)" + "=P" + std::to_string(partyid)); \
    }                                                                                                                   \
  } while (0)

    // /***********    basic binary ops test  ***********/
    snn_binary_f(Add);
    snn_binary_f(Sub);
    snn_binary_f(Mul);
    
    // // Todo: two reasons for commenting this for now: 
    // // 1. the case for 0 / 0 is undefined for Div,
    // // 2. we implement with Mul when one-side is constant and this may incur additional precision loss.
    // // check the independent 
    snn_binary_f(Floordiv);
    snn_binary_f(Truediv);
    snn_binary_f(Div);

    snn_binary_f_rh_const(Pow);

    /***********    basic compare binary ops  ***********/
    snn_binary_f(Equal);
    snn_binary_f(NotEqual);
    snn_binary_f(Less);
    snn_binary_f(LessEqual);
    snn_binary_f(Greater);
    snn_binary_f(GreaterEqual);

#undef snn_binary_f
#undef snn_binary_f_rh_const
  }

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  PROTOCOL_MPC_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);
