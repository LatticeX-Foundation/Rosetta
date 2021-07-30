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
  {
    msg_id_t msgid("All basic Binary OP(s) (VV/VC/CV)");
    cout << __FUNCTION__ << " " << msgid << endl;

#define helix_binary_f(lh, rh, op, strX, strY, check_func, X, Y)                  \
  do {                                                                            \
    string tag(#op);                                                              \
    cout << "\nTEST AND CHECK:" << tag << string((lh == 1) ? " lh_is_const" : "") \
         << string((rh == 1) ? " rh_is_const" : "") << endl;                      \
    if_print_vec(strX, 10, "strX");                                               \
    if_print_vec(strY, 10, "strY");                                               \
    if_print_vec(X, 10, "X");                                                     \
    if_print_vec(Y, 10, "Y");                                                     \
                                                                                  \
    attr_type attr;                                                               \
    attr["lh_is_const"] = to_string(lh);                                          \
    attr["rh_is_const"] = to_string(rh);                                          \
    vector<string> strZ;                                                          \
    mpc_proto->GetOps(msgid)->op(strX, strY, strZ, &attr);                        \
    if_print_vec(strZ, 10, "strX " + tag + " strY = strZ");                       \
    vector<double> Z;                                                             \
    mpc_proto->GetOps(msgid)->Reveal(strZ, Z, &reveal_attr);                      \
    if_print_vec(Z, 10, tag + " Z");                                              \
    if (partyid == 2)                                                             \
      check_func(X, Y, Z);                                                        \
  } while (0)

    // initialize
    vector<double> X, Y; // variables
    vector<double> CX, CY; // constants
    vector<string> strX, strY; // variables
    vector<string> strCX, strCY; // constants

    int k = 10;
    random_vector(X, k, -3, 3);
    random_vector(Y, k, -3, 3);
    {
      CX.resize(k);
      CY.resize(k);
      strCX.resize(k);
      strCY.resize(k);
      srand(1);
      for (int i = 0; i < k; i++) {
        CX[i] = (rand() % 300) / 100.0 + 3.0;
        strCX[i] = std::to_string(CX[i]);
      }
      srand(11);
      for (int i = 0; i < k; i++) {
        CY[i] = (rand() & 600) / 200.0 + 3.0;
        strCY[i] = std::to_string(CY[i]);
      }
    }
    mpc_proto->GetOps(msgid)->PrivateInput(net_io->GetNodeId(2), X, strX);
    mpc_proto->GetOps(msgid)->PrivateInput(net_io->GetNodeId(2), Y, strY);

    //
    //  general random tests and check (VV)
    helix_binary_f(0, 0, Add, strX, strY, add_check_func, X, Y);
    helix_binary_f(0, 0, Sub, strX, strY, sub_check_func, X, Y);
    helix_binary_f(0, 0, Mul, strX, strY, mul_check_func, X, Y);
    helix_binary_f(0, 0, Floordiv, strX, strY, floordiv_check_func, X, Y);
    helix_binary_f(0, 0, Truediv, strX, strY, truediv_check_func, X, Y);

    helix_binary_f(0, 0, Equal, strX, strY, equal_check_func, X, Y);
    helix_binary_f(0, 0, NotEqual, strX, strY, notequal_check_func, X, Y);
    helix_binary_f(0, 0, Less, strX, strY, less_check_func, X, Y);
    helix_binary_f(0, 0, LessEqual, strX, strY, lessequal_check_func, X, Y);
    helix_binary_f(0, 0, Greater, strX, strY, greater_check_func, X, Y);
    helix_binary_f(0, 0, GreaterEqual, strX, strY, greaterequal_check_func, X, Y);

    // general random tests and check (CV)
    helix_binary_f(1, 0, Add, strCX, strY, add_check_func, CX, Y);
    helix_binary_f(1, 0, Sub, strCX, strY, sub_check_func, CX, Y);
    helix_binary_f(1, 0, Mul, strCX, strY, mul_check_func, CX, Y);
    helix_binary_f(1, 0, Floordiv, strCX, strY, floordiv_check_func, CX, Y);
    helix_binary_f(1, 0, Truediv, strCX, strY, truediv_check_func, CX, Y);

    helix_binary_f(1, 0, Equal, strCX, strY, equal_check_func, CX, Y);
    helix_binary_f(1, 0, NotEqual, strCX, strY, notequal_check_func, CX, Y);
    helix_binary_f(1, 0, Less, strCX, strY, less_check_func, CX, Y);
    helix_binary_f(1, 0, LessEqual, strCX, strY, lessequal_check_func, CX, Y);
    helix_binary_f(1, 0, Greater, strCX, strY, greater_check_func, CX, Y);
    helix_binary_f(1, 0, GreaterEqual, strCX, strY, greaterequal_check_func, CX, Y);

    // general random tests and check (VC)
    helix_binary_f(0, 1, Add, strX, strCY, add_check_func, X, CY);
    helix_binary_f(0, 1, Sub, strX, strCY, sub_check_func, X, CY);
    helix_binary_f(0, 1, Mul, strX, strCY, mul_check_func, X, CY);
    helix_binary_f(0, 1, Floordiv, strX, strCY, floordiv_check_func, X, CY);
    helix_binary_f(0, 1, Truediv, strX, strCY, truediv_check_func, X, CY);

    helix_binary_f(0, 1, Equal, strX, strCY, equal_check_func, X, CY);
    helix_binary_f(0, 1, NotEqual, strX, strCY, notequal_check_func, X, CY);
    helix_binary_f(0, 1, Less, strX, strCY, less_check_func, X, CY);
    helix_binary_f(0, 1, LessEqual, strX, strCY, lessequal_check_func, X, CY);
    helix_binary_f(0, 1, Greater, strX, strCY, greater_check_func, X, CY);
    helix_binary_f(0, 1, GreaterEqual, strX, strCY, greaterequal_check_func, X, CY);

    // special cases for some ops' tests and check
    {
      vector<double> X = {-1.01, -2.00, -3.01, 0, 1.3, 2.02, 3.14, +2, -0.01};
      vector<double> Y = {-1.00, -2.01, -3.01, 0, 1.3, 2.03, 3.12, -2, +0.01};
      vector<string> strX, strY;
      mpc_proto->GetOps(msgid)->PrivateInput(net_io->GetNodeId(2), X, strX);
      mpc_proto->GetOps(msgid)->PrivateInput(net_io->GetNodeId(2), Y, strY);

      helix_binary_f(0, 0, Add, strX, strY, add_check_func, X, Y);
      helix_binary_f(0, 0, Sub, strX, strY, sub_check_func, X, Y);
      helix_binary_f(0, 0, Mul, strX, strY, mul_check_func, X, Y);
      helix_binary_f(0, 0, Floordiv, strX, strY, floordiv_check_func, X, Y);
      helix_binary_f(0, 0, Truediv, strX, strY, truediv_check_func, X, Y);

      helix_binary_f(0, 0, Equal, strX, strY, equal_check_func, X, Y);
      helix_binary_f(0, 0, NotEqual, strX, strY, notequal_check_func, X, Y);
      helix_binary_f(0, 0, Less, strX, strY, less_check_func, X, Y);
      helix_binary_f(0, 0, LessEqual, strX, strY, lessequal_check_func, X, Y);
      helix_binary_f(0, 0, Greater, strX, strY, greater_check_func, X, Y);
      helix_binary_f(0, 0, GreaterEqual, strX, strY, greaterequal_check_func, X, Y);
    }

    {
      // special for sce
      helix_binary_f(0, 0, SigmoidCrossEntropy, strX, strY, sce_check_func, X, Y);
    }
    {
      // special for pow
      vector<double> C; // constants
      vector<string> strC; // constants
      {
        C.resize(k);
        strC.resize(k);
        srand(1);
        for (int i = 0; i < k; i++) {
          C[i] = (rand() % 300) / 100.0 + 3.0;
          strC[i] = std::to_string(C[i]);
        }
      }
      helix_binary_f(0, 1, Pow, strX, strC, powconst_check_func, X, C);
    }

#undef helix_binary_f
  }

  {
    msg_id_t msgid("All basic Unary OP(s) (share)");
    cout << __FUNCTION__ << " " << msgid << endl;

#define helix_unary_f(op, strX, check_func, X)       \
  do {                                               \
    string tag(#op);                                 \
    cout << "\nTEST AND CHECK:" << tag << endl;      \
    if_print_vec(strX, 10, "strX");                  \
    if_print_vec(X, 10, "X");                        \
                                                     \
    attr_type attr;                                  \
    vector<string> strY;                             \
    mpc_proto->GetOps(msgid)->op(strX, strY, &attr); \
    if_print_vec(strY, 10, tag + " strX = strY");    \
    vector<double> Y;                                \
    mpc_proto->GetOps(msgid)->Reveal(strY, Y, &reveal_attr);       \
    if_print_vec(Y, 10, tag + " Y");                 \
    if (partyid == 2)                                \
      check_func(X, Y);                              \
  } while (0)

    // initialize
    vector<double> X;
    vector<string> strX;

    int k = 10;
    random_vector(X, k, -3, 3);
    mpc_proto->GetOps(msgid)->PrivateInput(net_io->GetNodeId(2), X, strX);

    //
    //  general random tests and check
    helix_unary_f(Square, strX, square_check_func, X);
    helix_unary_f(Negative, strX, negative_check_func, X);
    helix_unary_f(Abs, strX, abs_check_func, X);
    helix_unary_f(AbsPrime, strX, absprime_check_func, X);
    helix_unary_f(Log, strX, log_check_func, X);
    helix_unary_f(Log1p, strX, log1p_check_func, X);
    helix_unary_f(HLog, strX, hlog_check_func, X);
    helix_unary_f(Relu, strX, relu_check_func, X);
    helix_unary_f(ReluPrime, strX, reluprime_check_func, X);
    helix_unary_f(Sigmoid, strX, sigmoid_check_func, X);

#undef helix_unary_f
  }

  {
    msg_id_t msgid("All basic Reduce OP(s) (share)");
    cout << __FUNCTION__ << " " << msgid << endl;

#define helix_reduce_f(op, strX, r, c, check_func, X) \
  do {                                                \
    string tag(#op);                                  \
    cout << "\nTEST AND CHECK:" << tag << endl;       \
    if_print_vec(strX, 10, "strX");                   \
    if_print_vec(X, 10, "X");                         \
                                                      \
    attr_type attr;                                   \
    attr["rows"] = to_string(r);                      \
    attr["cols"] = to_string(c);                      \
    vector<string> strY;                              \
    mpc_proto->GetOps(msgid)->op(strX, strY, &attr);  \
    if_print_vec(strY, 10, tag + " strX = strY");     \
    vector<double> Y;                                 \
    mpc_proto->GetOps(msgid)->Reveal(strY, Y, &reveal_attr);        \
    if_print_vec(Y, 10, tag + " Y");                  \
    if (partyid == 2)                                 \
      check_func(X, Y, r, c);                         \
  } while (0)

    // initialize
    vector<double> X;
    vector<string> strX;

    int r = 3, c = 4;
    int k = r * c;
    random_vector(X, k, -3, 3);
    mpc_proto->GetOps(msgid)->PrivateInput(net_io->GetNodeId(2), X, strX);

    //
    //  general random tests and check
    helix_reduce_f(Mean, strX, r, c, mean_check_func, X);
    helix_reduce_f(Sum, strX, r, c, sum_check_func, X);
    helix_reduce_f(AddN, strX, r, c, addn_check_func, X);
    helix_reduce_f(Max, strX, r, c, max_check_func, X);
    helix_reduce_f(Min, strX, r, c, min_check_func, X);
#undef helix_reduce_f
  }

  {
    msg_id_t msgid("MatMul(share,share)");
    cout << __FUNCTION__ << " " << msgid << endl;

#define helix_matmul_f(op, strX, strY, m, K, n, check_func, X, Y) \
  do {                                                            \
    string tag(#op);                                              \
    cout << "\nTEST AND CHECK:" << tag << endl;                   \
    if_print_vec(strX, 10, "strX");                               \
    if_print_vec(strY, 10, "strY");                               \
    if_print_vec(X, 10, "X");                                     \
    if_print_vec(Y, 10, "Y");                                     \
                                                                  \
    attr_type attr;                                               \
    attr["m"] = to_string(m);                                     \
    attr["k"] = to_string(K);                                     \
    attr["n"] = to_string(n);                                     \
    vector<string> strZ;                                          \
    mpc_proto->GetOps(msgid)->op(strX, strY, strZ, &attr);        \
    if_print_vec(strZ, 10, "strX " + tag + " strY = strZ");       \
    vector<double> Z;                                             \
    mpc_proto->GetOps(msgid)->Reveal(strZ, Z, &reveal_attr);      \
    if_print_vec(Z, 10, tag + " Z");                              \
    if (partyid == 2)                                             \
      check_func(X, Y, Z, m, K, n);                               \
  } while (0)

    // initialize
    vector<double> X, Y; // variables
    vector<string> strX, strY; // variables
    int m = 10, K = 21, n = 10;
    random_vector(X, m * K, -3, 3);
    random_vector(Y, K * n, -3, 3);

    mpc_proto->GetOps(msgid)->PrivateInput(net_io->GetNodeId(2), X, strX);
    mpc_proto->GetOps(msgid)->PrivateInput(net_io->GetNodeId(2), Y, strY);

    //
    // general random tests and check
    helix_matmul_f(Matmul, strX, strY, m, K, n, matmul_check_func, X, Y);

    // special cases here
    {
      //
    }

#undef helix_matmul_f
  }

  {
    msg_id_t msgid("Other OPs added here");
    cout << __FUNCTION__ << " " << msgid << endl;
  }
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  PROTOCOL_MPC_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);
