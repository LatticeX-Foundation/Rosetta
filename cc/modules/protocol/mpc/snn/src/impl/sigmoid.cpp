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
#include "cc/modules/protocol/mpc/snn/src/impl/op_impl.h"

namespace rosetta {
namespace snn {

/*
** basic functions for sigmoid
*/
int Sigmoid::funcPrivateCompareMPCEx(
  const vector<mpc_t>& a, const vector<mpc_t>& r, vector<mpc_t>& b, size_t size) {
  // LOGI("funcDotProductMPC start");
  if (FOUR_PC) {
    LOGW("not support 4PC now !");
    return 1;
  } else if (THREE_PC) {
    //<x_j - j*r>_i
    vector<mpc_t> v(a.size(), 0);
    for (size_t i = 0; i < size; i++) {
      v[i] = a[i] - partyNum * r[i];
    }
    //funcRELUPrime3PC(v, b, size);
    GetMpcOpInner(ReluPrime)->Run3PC(v, b, size);

  } else {
    LOGE("funcPrivateCompareMPCEx should not be here!");
    return 1;
  }

  // LOGI("sigmoid.private.compare OK");
  return 0;
}

//compare two a commond value r with secret-sharing value a,
//it functions the same as operaotr>=() ">=", if r >= a, returns 8192 (mean "1"), otherwise returns 0
int Sigmoid::funcPrivateCompareMPCEx2(
  const vector<mpc_t>& r, const vector<mpc_t>& a, vector<mpc_t>& b, size_t size) {
  // LOGI("funcDotProductMPC start");
  if (FOUR_PC) {
    LOGW("not support 4PC now !");
    return 1;
  } else if (THREE_PC) {
    //<x_j - j*r>_i
    vector<mpc_t> v(a.size(), 0);
    for (size_t i = 0; i < size; i++) {
      v[i] = partyNum * r[i] - a[i];
    }
    //funcRELUPrime3PC(v, b, size);
    GetMpcOpInner(ReluPrime)->Run3PC(v, b, size);
  } else {
    LOGE("funcPrivateCompareMPCEx should not be here!");
    return 1;
  }

  // LOGI("sigmoid.private.compare OK");
  return 0;
}

int Sigmoid::funcLinearMPC(
  const vector<mpc_t>& x, mpc_t a, mpc_t b, vector<mpc_t>& out, size_t size) {
  for (size_t i = 0; i < size; i++) {
    out[i] = a * x[i];
    funcTruncateElem2PC(out[i], FLOAT_PRECISION_M, PARTY_A, PARTY_B);
    out[i] += b;
  }
  return 0;
}

int Sigmoid::funcCubeMPC(const vector<mpc_t>& a, vector<mpc_t>& cube, size_t size) {
  LOGI("funcCubeMPC run");
  // x * x
  vector<mpc_t> square(size, 0);
  //funcDotProductMPC(a, a, square, size);
  GetMpcOpInner(DotProduct)->Run(a, a, square, size);

  //(x*x) * x
  //funcDotProductMPC(square, a, cube, size);
  GetMpcOpInner(DotProduct)->Run(square, a, cube, size);

  LOGI("funcSquareMPC OK");
  return 0;
}

//----     x^3    ------
int Sigmoid::funcFastPow3MPC(const vector<mpc_t>& x, vector<mpc_t>& out, size_t size) {
  LOGD("funcFastPow3MPC ...");

  if (FOUR_PC) {
    LOGE("not support 4PC fast_pow3 !");
    return 1;
  }

  if (THREE_PC) {
    vector<mpc_t> A(size, 0), D(size, 0), C(size, 0);
    /*
    P0 and P2 use k0 to generate random numbers a0, c0
    P1 and P2 use k1 to generate random numbers a1, d1
    P2 computes a = a0 + a1, then computes c1 = a^2 - c0 , d0 = a^3 - d1, and sends
    c1, d0 to P1,P0 respectively

    For i in {0,1}, P-i computes: ei = xi - ai , and reconstruct e by exchange the value.
    For i in {0,1}, P-i outputs: i * e^3 + 3*e^2*ai + 3*e*ci + di
     */
    if (HELPER) {
      vector<mpc_t> A0(size, 0), A1(size, 0), C0(size, 0), C1(size, 0), D0(size, 0), D1(size, 0);

      populateRandomVector<mpc_t>(A0, size, "a_1", "POSITIVE");
      populateRandomVector<mpc_t>(A1, size, "a_2", "POSITIVE");
      populateRandomVector<mpc_t>(C0, size, "b_1", "POSITIVE");
      populateRandomVector<mpc_t>(D1, size, "b_2", "POSITIVE");

      addVectors<mpc_t>(A0, A1, A, size);
      for (size_t i = 0; i < size; i++) {
        C1[i] = A[i] * A[i] - C0[i];
        D0[i] = A[i] * A[i] * A[i] - D1[i];
      }

      sendVector<mpc_t>(C1, PARTY_B, size);
      sendVector<mpc_t>(D0, PARTY_A, size);
    }

    if (PRIMARY) {
      if (partyNum == PARTY_A) {
        populateRandomVector<mpc_t>(A, size, "a_1", "POSITIVE");
        populateRandomVector<mpc_t>(C, size, "b_1", "POSITIVE");

        // recv d0 from party-2
        receiveVector<mpc_t>(D, PARTY_C, size);
      }

      if (partyNum == PARTY_B) {
        populateRandomVector<mpc_t>(A, size, "a_2", "POSITIVE");
        populateRandomVector<mpc_t>(D, size, "b_2", "POSITIVE");
        // recv c1 from party-2
        receiveVector<mpc_t>(C, PARTY_C, size);
      }

      // P-i computes: ei = xi - ai , and reconstruct e by exchange the value.
      vector<mpc_t> E(size), temp_E(size);
      // ei = xi - ai
      subtractVectors<mpc_t>(x, A, E, size);

      // send and recv with seperated threads
      thread* threads = new thread[2];
      threads[0] = thread(&OpBase_::sendVector<mpc_t>, this, ref(E), adversary(partyNum), size);
      threads[1] =
        thread(&OpBase_::receiveVector<mpc_t>, this, ref(temp_E), adversary(partyNum), size);

      for (int i = 0; i < 2; i++)
        threads[i].join();

      delete[] threads;

      // recovery e
      addVectors<mpc_t>(E, temp_E, E, size);

      // P-i outputs: i * e^3 + 3*e^2*ai + 3*e*ci + di
      vector<mpc_t> plain_e_square(size), plain_e_cube(size), temp(size);

      for (size_t i = 0; i < size; ++i) {
        plain_e_square[i] = (E[i] * E[i]);
        plain_e_cube[i] = (plain_e_square[i] * E[i]);
      }

      for (size_t i = 0; i < size; ++i) {
        if (partyNum == PARTY_A) {
          out[i] = 3 * plain_e_square[i] * A[i] + 3 * E[i] * C[i] + D[i];
        } else if (partyNum == PARTY_B) {
          out[i] = plain_e_cube[i] + 3 * plain_e_square[i] * A[i] + 3 * E[i] * C[i] + D[i];
        }
        funcTruncateElem2PC(out[i], 2 * FLOAT_PRECISION_M, PARTY_A, PARTY_B);
      }
    }
  }

  LOGI("funcFastPow3MPC OK.");
  return 0;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/*
** sigmoid approximate function
** g3(x) = Comp(x, -8)*(0.5 + 0.15012* x - 0.00159301 * x^3)
** + Comp(x,8) *(0.5 - 0.15012 * x + 0.00159301 *x^3), for x in [-8,8]
*/
int Sigmoid::funcSigmoidG3MPC(
  const vector<mpc_t>& a, vector<mpc_t>& b, size_t size, bool use_fastPow3 /*= true*/) {
  LOGI("funcDotProductMPC start");

  // #ifndef NDEBUG // TEST TO RECORD COMMUNICATION
  //   uint64_t init_sent_bytes = commObject.getSent();
  //   uint64_t init_recv_bytes = commObject.getRecv();
  //   uint64_t pow3_recv_bytes = 0;
  //   uint64_t pow3_sent_bytes = 0;
  // #endif

  if (FOUR_PC) {
    LOGW("4PC is not support !");
    return 1;
  } else if (THREE_PC) {
    // y = 0, for x in (-inf, -4)
    // y = g3(x), for x in [-4, 4]
    // y = 1, for x in (4, +inf)
    // y = Comp(x, -4)*(0.5 + 0.15012* x - 0.00159301 * x^3) + Comp(x,4) *(0.5 - 0.15012 * x +
    // 0.00159301 *x^3)
    vector<mpc_t> neg_eights(size, FloatToMpcType(-4));
    vector<mpc_t> eights(size, FloatToMpcType(4));
    vector<mpc_t> cmp1(size), cmp2(size);
    vector<mpc_t> result(size, 0);

    mpc_t half = FloatToMpcType(0.5) / 2; // for secret-sharing we should split 0.5 to two 0.25
    mpc_t p1 = FloatToMpcType(0.15012);
    mpc_t p2 = FloatToMpcType(0.00159301);

    // calculate Comp values
    funcPrivateCompareMPCEx(a, neg_eights, cmp1, size);
    funcPrivateCompareMPCEx(a, eights, cmp2, size);

    vector<mpc_t> A1 = a;
    for (size_t i = 0; i < size; i++)
      A1[i] *= p1; //==> (x * 0.15012)
    if (PRIMARY)
      funcTruncate2PC(A1, FLOAT_PRECISION_M, size, PARTY_A, PARTY_B);

    vector<mpc_t> A2(size);
    //==> x^3
    // #ifndef NDEBUG
    //     pow3_sent_bytes = commObject.getSent();
    //     pow3_recv_bytes = commObject.getRecv();
    // #endif
    if (!use_fastPow3)
      funcCubeMPC(a, A2, size);
    else
      funcFastPow3MPC(a, A2, size);
    // #ifndef NDEBUG
    //     pow3_sent_bytes = commObject.getSent() - pow3_sent_bytes;
    //     pow3_recv_bytes = commObject.getRecv() - pow3_recv_bytes;
    //     LOGI(
    //         "*** %s: sent: %llu, recv: %llu Bytes ***",
    //         use_fastPow3 ? "FastPow3" : "Cube",
    //         pow3_sent_bytes,
    //         pow3_recv_bytes);
    // #endif

    for (size_t i = 0; i < size; i++)
      A2[i] *= p2; //=> 0.00159301 * (x^3)
    if (PRIMARY)
      funcTruncate2PC(A2, FLOAT_PRECISION_M, size, PARTY_A, PARTY_B);

    b.resize(size);
    for (size_t i = 0; i < size; i++) {
      b[i] = (half + A1[i] - A2[i]); //==> (0.5 + 0.15012* x - 0.00159301 * x^3)
      A2[i] = (half - A1[i] + A2[i]); //==> (0.5 - 0.15012 * x + 0.00159301 *x^3);
    }

    // Comp(x, -8)*(0.5 + 0.15012* x - 0.00159301 * x^3)
    ////funcDotProductMPC(cmp1, b, A1, size);
    GetMpcOpInner(DotProduct)->Run(cmp1, b, A1, size);

    // Comp(x,8) *(0.5 - 0.15012 * x + 0.00159301 *x^3)
    ////funcDotProductMPC(cmp2, A2, b, size);
    GetMpcOpInner(DotProduct)->Run(cmp2, A2, b, size);

    // add
    for (size_t i = 0; i < size; i++)
      b[i] += A1[i];
  } else {
    LOGE("num of parties is: %d, not support !", NUM_OF_PARTIES);
    return 1;
  }

  string party = "A";
  if (PRIMARY)
    party = partyNum == PARTY_A ? "A" : "B";
  else
    party = "C";

  LOGI("sigmoid for g3(x) ok.");
  // #ifndef NDEBUG // TEST TO RECORD COMMUNICATION
  //   LOGI(
  //       "*** g3x, (%s) sent: %llu Bytes, recv: %llu Bytes  ***",
  //       use_fastPow3 ? "FastPow3" : "Cube",
  //       commObject.getSent() - init_sent_bytes,
  //       commObject.getRecv() - init_recv_bytes);
  // #endif
  return 0;
}

/*
** Derivative of approximate sigmoid
** g3(x)' = Comp(x, -8)*(0.15012 - 0.00477903 * x^2) + Comp(x,8) *(-0.15012 + 0.00477903*x^2)
*/
int Sigmoid::funcSigmoidG3PrimeMPC(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size) {
  LOGD("funcSigmoidG3PrimeMPC ...");

  if (FOUR_PC) {
    LOGW("4PC is not support !");
    return 1;
  } else if (THREE_PC) {
    // y = 0, for x in (-inf, -8)
    // y = g3(x), for x in [-8, 8]
    // y = 1, for x in (8, +inf)
    // y = Comp(x, -8)*(0.15012 - 0.00477903 * x^2) + Comp(x,8) *(-0.15012 + 0.00477903*x^2)
    vector<mpc_t> neg_eights(size, FloatToMpcType(-8));
    vector<mpc_t> eights(size, FloatToMpcType(8));
    vector<mpc_t> cmp1(size), cmp2(size);
    vector<mpc_t> result(size, 0);

    mpc_t p0 = FloatToMpcType(0.15012) / 2; // for secret-sharing  divide 2
    mpc_t p2 = FloatToMpcType(0.00477903);

    // calculate Comp values
    funcPrivateCompareMPCEx(a, neg_eights, cmp1, size);
    funcPrivateCompareMPCEx(a, eights, cmp2, size);

    vector<mpc_t> A2(size);
    //==> x^2
    //funcDotProductMPC(a, a, A2, size);
    GetMpcOpInner(DotProduct)->Run(a, a, A2, size);

    for (size_t i = 0; i < size; i++)
      A2[i] *= p2; //=> 0.00477903 * (x^2)
    if (PRIMARY)
      funcTruncate2PC(A2, FLOAT_PRECISION_M, size, PARTY_A, PARTY_B);

    b.resize(size);
    for (size_t i = 0; i < size; i++) {
      b[i] = (p0 - A2[i]); //==> (0.15012/2 - 0.00477903 * x^2)
      A2[i] = (-p0 + A2[i]); //==> (-0.15012/2 + 0.00477903*x^2)
    }

    vector<mpc_t> A1(size);
    ////funcDotProductMPC(cmp1, b, A1, size);
    GetMpcOpInner(DotProduct)->Run(cmp1, b, A1, size);
    ////funcDotProductMPC(cmp2, A2, b, size);
    GetMpcOpInner(DotProduct)->Run(cmp2, A2, b, size);

    // add
    for (size_t i = 0; i < size; i++)
      b[i] += A1[i];
  } else {
    LOGE("num of parties is: %d, not support !", NUM_OF_PARTIES);
    return 1;
  }

  string party = "A";
  if (PRIMARY)
    party = partyNum == PARTY_A ? "A" : "B";
  else
    party = "C";

  LOGI("--- party-%s: sigmoid for g3(x) with share values below   ---", party.data());
  LOGI("funcSigmoidG3PrimeMPC OK.");
  return 0;
}

/**
* @brief approximate sigmoid implemented with 6-pieces function
* @param y input of ZkReal type in range (-inf,+inf)
* @return sigmoid of ZkReal input
* @note sigmoid(y) equals:
0, for y in range (-inf, -4);
0.0484792 * y + 0.1998976, for y in range [-4, -2); 
0.1928931 * y + 0.4761351, for y in range [-2, 0);
0.1928931 * y + 0.5238649, for y in range [0, 2);
0.0484792 * y + 0.8001024, for y in range [2,4);
1, for y in range [4, +inf) 
*/
int Sigmoid::funcSigmoidPieceWiseMPC(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size) {
  LOGD("funcSigmoidPieceWiseMPC start");

  if (FOUR_PC) {
    LOGW("4PC is not support !");
    return 1;
  } else if (THREE_PC) {
    // vector<mpc_t> y = a;
    vector<mpc_t> y = a;

    //(0.0484792, 0.1998976),  (0.1928931, 0.4761351), (0.1928931, 0.5238649),  (0.0484792,
    // 0.8001024)
    mpc_t a1 = FloatToMpcType(0.0484792);
    mpc_t b1 = FloatToMpcType(0.1998976) / 2;
    mpc_t a2 = FloatToMpcType(0.1928931);
    mpc_t b2 = FloatToMpcType(0.4761351) / 2;
    mpc_t a3 = FloatToMpcType(0.1928931);
    mpc_t b3 = FloatToMpcType(0.5238649) / 2;
    mpc_t a4 = FloatToMpcType(0.0484792);
    mpc_t b4 = FloatToMpcType(0.8001024) / 2;

    //[-4,4]: (0.0484792, 0.1998976),  (0.1928931, 0.4761351), (0.1928931, 0.5238649),  (0.0484792,
    // 0.8001024)
    mpc_t y1 = FloatToMpcType(-4);
    mpc_t y2 = FloatToMpcType(-2);
    mpc_t y3 = FloatToMpcType(0);
    mpc_t y4 = FloatToMpcType(2);
    mpc_t y5 = FloatToMpcType(4);
    vector<mpc_t> y1_cmp(size, y1);
    vector<mpc_t> y2_cmp(size, y2);
    vector<mpc_t> y3_cmp(size, y3);
    vector<mpc_t> y4_cmp(size, y4);
    vector<mpc_t> y5_cmp(size, y5);
    funcPrivateCompareMPCEx2(y1_cmp, y, y1_cmp, size);
    funcPrivateCompareMPCEx2(y2_cmp, y, y2_cmp, size);
    funcPrivateCompareMPCEx2(y3_cmp, y, y3_cmp, size);
    funcPrivateCompareMPCEx2(y4_cmp, y, y4_cmp, size);
    funcPrivateCompareMPCEx2(y5_cmp, y, y5_cmp, size);
    // if (PRIMARY)
    // {
    // 	funcReconstruct2PC(y1_cmp, size, "y1_cmp");
    // 	funcReconstruct2PC(y2_cmp, size, "y2_cmp");
    // 	funcReconstruct2PC(y3_cmp, size, "y3_cmp");
    // 	funcReconstruct2PC(y4_cmp, size, "y4_cmp");
    // 	funcReconstruct2PC(y5_cmp, size, "y5_cmp");
    // }
    mpc_t lastOne = FloatToMpcType(1) / 2; // add last 1

    vector<mpc_t> linear_temp(size);
    vector<mpc_t> mul_temp(size);
    vector<mpc_t> out(size, 0);

    if (PRIMARY)
      funcLinearMPC(y, 0 - a1, 0 - b1, linear_temp, size);
    ////funcDotProductMPC(y1_cmp, linear_temp, mul_temp, size);
    GetMpcOpInner(DotProduct)->Run(y1_cmp, linear_temp, mul_temp, size);
    addVectors(out, mul_temp, out, size);

    if (PRIMARY)
      funcLinearMPC(y, a1 - a2, b1 - b2, linear_temp, size);
    ////funcDotProductMPC(y2_cmp, linear_temp, mul_temp, size);
    GetMpcOpInner(DotProduct)->Run(y2_cmp, linear_temp, mul_temp, size);
    addVectors(out, mul_temp, out, size);

    if (PRIMARY)
      funcLinearMPC(y, a2 - a3, b2 - b3, linear_temp, size);
    ////funcDotProductMPC(y3_cmp, linear_temp, mul_temp, size);
    GetMpcOpInner(DotProduct)->Run(y3_cmp, linear_temp, mul_temp, size);
    addVectors(out, mul_temp, out, size);

    if (PRIMARY)
      funcLinearMPC(y, a3 - a4, b3 - b4, linear_temp, size);
    ////funcDotProductMPC(y4_cmp, linear_temp, mul_temp, size);
    GetMpcOpInner(DotProduct)->Run(y4_cmp, linear_temp, mul_temp, size);
    addVectors(out, mul_temp, out, size);

    if (PRIMARY)
      funcLinearMPC(y, a4 /*-0*/, b4 - lastOne, linear_temp, size);
    ////funcDotProductMPC(y5_cmp, linear_temp, mul_temp, size);
    GetMpcOpInner(DotProduct)->Run(y5_cmp, linear_temp, mul_temp, size);
    addVectors(out, mul_temp, out, size);

    for (size_t i = 0; i < size; i++) {
      b[i] = out[i] + lastOne;
    }
  }

  string party = "A";
  if (PRIMARY)
    party = partyNum == PARTY_A ? "A" : "B";
  else
    party = "C";

  LOGD("party-%s funcSigmoidPieceWiseMPC ok.", party.data());
  return 0;
}

/*
** @brief sigmoid approximate function 5-pieces-functions of ali
** @detail:
** sigmoid(x) =
10^−4, x <= −5
0.02776 * x + 0.145, −5 < x <= −2.5
0.17 * x + 0.5, −2.5 < x <= 2.5
0.02776 * x + 0.85498, 2.5 < x <= 5
1 − 10^−4, x > 5
*/
int Sigmoid::funcSigmoidAliPieceWiseMPC(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size) {
  LOGD("funcSigmoidAliPieceWiseMPC start");

  if (FOUR_PC) {
    LOGW("4PC is not support !");
    return 1;
  } else if (THREE_PC) {
    // vector<mpc_t> y = a;
    vector<mpc_t> y = a;

    mpc_t a1 = FloatToMpcType(0.02776);
    mpc_t b1 = FloatToMpcType(0.145) / 2;
    mpc_t a2 = FloatToMpcType(0.17);
    mpc_t b2 = FloatToMpcType(0.5) / 2;
    mpc_t a3 = FloatToMpcType(0.02776);
    mpc_t b3 = FloatToMpcType(0.85498) / 2;

    mpc_t y1 = FloatToMpcType(-5);
    mpc_t y2 = FloatToMpcType(-2.5);
    //mpc_t y3 = FloatToMpcType(0);
    mpc_t y3 = FloatToMpcType(2.5);
    mpc_t y4 = FloatToMpcType(5);
    vector<mpc_t> y1_cmp(size, y1);
    vector<mpc_t> y2_cmp(size, y2);
    vector<mpc_t> y3_cmp(size, y3);
    vector<mpc_t> y4_cmp(size, y4);
    //vector<mpc_t> y5_cmp(size, y5);
    funcPrivateCompareMPCEx2(y1_cmp, y, y1_cmp, size);
    funcPrivateCompareMPCEx2(y2_cmp, y, y2_cmp, size);
    funcPrivateCompareMPCEx2(y3_cmp, y, y3_cmp, size);
    funcPrivateCompareMPCEx2(y4_cmp, y, y4_cmp, size);
    //funcPrivateCompareMPCEx2(y5_cmp, y, y5_cmp, size);
    // if (PRIMARY)
    // {
    // 	funcReconstruct2PC(y1_cmp, size, "y1_cmp");
    // 	funcReconstruct2PC(y2_cmp, size, "y2_cmp");
    // 	funcReconstruct2PC(y3_cmp, size, "y3_cmp");
    // 	funcReconstruct2PC(y4_cmp, size, "y4_cmp");
    // 	funcReconstruct2PC(y5_cmp, size, "y5_cmp");
    // }
    mpc_t lastOne = 0; //FloatToMpcType(0.9999) / 2; // add last 1
    mpc_t minor = 0;
    if (PARTY_A == partyNum) {
      lastOne = FloatToMpcType(0.99987);
      minor = FloatToMpcType(0.00013);
    } else {
      lastOne = 0;
      minor = 0;
    }
    vector<mpc_t> linear_temp(size);
    vector<mpc_t> mul_temp(size);
    vector<mpc_t> out(size, 0);

    if (PRIMARY)
      funcLinearMPC(y, 0 - a1, minor - b1, linear_temp, size);
    ////funcDotProductMPC(y1_cmp, linear_temp, mul_temp, size);
    GetMpcOpInner(DotProduct)->Run(y1_cmp, linear_temp, mul_temp, size);
    addVectors(out, mul_temp, out, size);

    if (PRIMARY)
      funcLinearMPC(y, a1 - a2, b1 - b2, linear_temp, size);
    ////funcDotProductMPC(y2_cmp, linear_temp, mul_temp, size);
    GetMpcOpInner(DotProduct)->Run(y2_cmp, linear_temp, mul_temp, size);
    addVectors(out, mul_temp, out, size);

    if (PRIMARY)
      funcLinearMPC(y, a2 - a3, b2 - b3, linear_temp, size);
    ////funcDotProductMPC(y3_cmp, linear_temp, mul_temp, size);
    GetMpcOpInner(DotProduct)->Run(y3_cmp, linear_temp, mul_temp, size);
    addVectors(out, mul_temp, out, size);

    if (PRIMARY)
      funcLinearMPC(y, a3 - 0, b3 - lastOne, linear_temp, size);
    ////funcDotProductMPC(y4_cmp, linear_temp, mul_temp, size);
    GetMpcOpInner(DotProduct)->Run(y4_cmp, linear_temp, mul_temp, size);
    addVectors(out, mul_temp, out, size);

    // if (PRIMARY)
    //   funcLinearMPC(y, a4 /*-0*/, b4 - lastOne, linear_temp, size);
    // funcDotProductMPC(y5_cmp, linear_temp, mul_temp, size);
    // addVectors(out, mul_temp, out, size);

    for (size_t i = 0; i < size; i++) {
      b[i] = out[i] + lastOne;
    }
  }

  string party = "A";
  if (PRIMARY)
    party = partyNum == PARTY_A ? "A" : "B";
  else
    party = "C";

  LOGD("party-%s funcSigmoidPieceWiseMPCAli ok.", party.data());

  return 0;
}

} // namespace mpc
} // namespace rosetta