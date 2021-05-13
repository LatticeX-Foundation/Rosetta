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

// Matrix Multiplication of a*b = c with transpose flags for a,b.
// Output is a share between PARTY_A and PARTY_B.
// a^transpose_a is rows*common_dim and b^transpose_b is common_dim*columns
int MatMul::funcMatMulMPC(
  const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t rows, size_t common_dim,
  size_t columns, size_t transpose_a, size_t transpose_b) {
  if (FOUR_PC) {
    size_t size = rows * columns;
    vector<mpc_t> temp(size);
    vector<mpc_t> a_temp = a;
    vector<mpc_t> b_temp = b;
    getVectorfromPrimary<mpc_t>(a_temp, rows * common_dim, "AS-IS", "NATURAL");
    getVectorfromPrimary<mpc_t>(b_temp, common_dim * columns, "AS-IS", "UNNATURAL");

    EigenMatMul(a_temp, b_temp, c, rows, common_dim, columns, transpose_a, transpose_b);

    if (NON_PRIMARY) {
      populateRandomVector<mpc_t>(temp, size, "COMMON", "NEGATIVE");
      addVectors<mpc_t>(c, temp, c, size);
      sendVector<mpc_t>(c, partner(partyNum), size);
    }

    if (PRIMARY) {
      receiveVector<mpc_t>(temp, partner(partyNum), size);
      addVectors<mpc_t>(c, temp, c, size);
      funcTruncate2PC(c, FLOAT_PRECISION_M, size, PARTY_A, PARTY_B);
    }
  }

  if (THREE_PC) {
    //cout << "-----  " << __FUNCTION__ << " 3PC -----" << endl;
    size_t size = rows * columns;
    size_t size_left = rows * common_dim;
    size_t size_right = common_dim * columns;
    vector<mpc_t> A(size_left, 0), B(size_right, 0), C(size, 0);

    if (HELPER) {
      vector<mpc_t> A1(size_left, 0), A2(size_left, 0), B1(size_right, 0), B2(size_right, 0),
        C1(size, 0), C2(size, 0);

      populateRandomVector<mpc_t>(A1, size_left, "a_1", "POSITIVE");
      populateRandomVector<mpc_t>(A2, size_left, "a_2", "POSITIVE");
      populateRandomVector<mpc_t>(B1, size_right, "a_1", "POSITIVE");
      populateRandomVector<mpc_t>(B2, size_right, "a_2", "POSITIVE");
      populateRandomVector<mpc_t>(C1, size, "a_1", "POSITIVE");

      addVectors<mpc_t>(A1, A2, A, size_left);
      addVectors<mpc_t>(B1, B2, B, size_right);

      EigenMatMul(A, B, C, rows, common_dim, columns, transpose_a, transpose_b);
      subtractVectors<mpc_t>(C, C1, C2, size);
      sendVector<mpc_t>(C2, PARTY_B, size);
    }

    if (PRIMARY) {
      vector<mpc_t> E(size_left), F(size_right);
      vector<mpc_t> temp_E(size_left), temp_F(size_right);
      vector<mpc_t> temp_c(size);

      if (partyNum == PARTY_A) {
        populateRandomVector<mpc_t>(A, size_left, "a_1", "POSITIVE");
        populateRandomVector<mpc_t>(B, size_right, "a_1", "POSITIVE");
        populateRandomVector<mpc_t>(C, size, "a_1", "POSITIVE");
      }

      if (partyNum == PARTY_B) {
        populateRandomVector<mpc_t>(A, size_left, "a_2", "POSITIVE");
        populateRandomVector<mpc_t>(B, size_right, "a_2", "POSITIVE");
        receiveVector<mpc_t>(C, PARTY_C, size);
      }

      // receiveThreeVectors<mpc_t>(A, B, C, PARTY_C, size_left, size_right, size);
      subtractVectors<mpc_t>(a, A, E, size_left);
      subtractVectors<mpc_t>(b, B, F, size_right);

      thread* threads = new thread[2];

      threads[0] = thread(
        &OpBase_::sendTwoVectors<mpc_t>, this, ref(E), ref(F), adversary(partyNum), size_left,
        size_right);
      threads[1] = thread(
        &OpBase_::receiveTwoVectors<mpc_t>, this, ref(temp_E), ref(temp_F), adversary(partyNum),
        size_left, size_right);

      for (int i = 0; i < 2; i++)
        threads[i].join();

      delete[] threads;

      addVectors<mpc_t>(E, temp_E, E, size_left);
      addVectors<mpc_t>(F, temp_F, F, size_right);

      EigenMatMul(a, F, c, rows, common_dim, columns, transpose_a, transpose_b);
      EigenMatMul(E, b, temp_c, rows, common_dim, columns, transpose_a, transpose_b);

      addVectors<mpc_t>(c, temp_c, c, size);
      addVectors<mpc_t>(c, C, c, size);

      if (partyNum == PARTY_A) {
        EigenMatMul(E, F, temp_c, rows, common_dim, columns, transpose_a, transpose_b);
        subtractVectors<mpc_t>(c, temp_c, c, size);
      }

      funcTruncate2PC(c, FLOAT_PRECISION_M, size, PARTY_A, PARTY_B);
    }
    //cout << "-----  " << __FUNCTION__ << " 3PC ok. -----" << endl;
  }
  return 0;
}
} // namespace snn
} // namespace rosetta