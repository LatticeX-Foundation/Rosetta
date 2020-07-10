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

int DotProduct::funcDotProduct(
  const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size) {
  if (FOUR_PC) {
    vector<mpc_t> temp(size);
    vector<mpc_t> a_temp = a;
    vector<mpc_t> b_temp = b;
    getVectorfromPrimary(a_temp, size, "AS-IS", "NATURAL");
    getVectorfromPrimary(b_temp, size, "AS-IS", "UNNATURAL");

    for (size_t i = 0; i < size; ++i)
      c[i] = a_temp[i] * b_temp[i];

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
    vector<mpc_t> A(size, 0), B(size, 0), C(size, 0);

    if (HELPER) {
      vector<mpc_t> A1(size, 0), A2(size, 0), B1(size, 0), B2(size, 0), C1(size, 0), C2(size, 0);

      populateRandomVector<mpc_t>(A1, size, "a_1", "POSITIVE");
      populateRandomVector<mpc_t>(A2, size, "a_2", "POSITIVE");
      populateRandomVector<mpc_t>(B1, size, "b_1", "POSITIVE");
      populateRandomVector<mpc_t>(B2, size, "b_2", "POSITIVE");
      populateRandomVector<mpc_t>(C1, size, "c_1", "POSITIVE");

      addVectors<mpc_t>(A1, A2, A, size);
      addVectors<mpc_t>(B1, B2, B, size);

      for (size_t i = 0; i < size; ++i)
        C[i] = A[i] * B[i];

      subtractVectors<mpc_t>(C, C1, C2, size);
      sendVector<mpc_t>(C2, PARTY_B, size);
      //cout << "partyNum " << partyNum << " C2[0]:" << C2[0] << endl;
    }

    if (PRIMARY) {
      if (partyNum == PARTY_A) {
        populateRandomVector<mpc_t>(A, size, "a_1", "POSITIVE");
        populateRandomVector<mpc_t>(B, size, "b_1", "POSITIVE");
        populateRandomVector<mpc_t>(C, size, "c_1", "POSITIVE");
      }

      if (partyNum == PARTY_B) {
        populateRandomVector<mpc_t>(A, size, "a_2", "POSITIVE");
        populateRandomVector<mpc_t>(B, size, "b_2", "POSITIVE");
        receiveVector<mpc_t>(C, PARTY_C, size);
        //cout << "partyNum " << partyNum << " C[0]:" << C[0] << endl;
      }

      // receiveThreeVectors<mpc_t>(A, B, C, PARTY_C, size, size, size);
      vector<mpc_t> E(size), F(size), temp_E(size), temp_F(size);
      mpc_t temp;

      subtractVectors<mpc_t>(a, A, E, size);
      subtractVectors<mpc_t>(b, B, F, size);

#if 1
      thread* threads = new thread[2];
      threads[0] = thread(
        &OpBase_::sendTwoVectors<mpc_t>, this, ref(E), ref(F), adversary(partyNum), size, size);
      threads[1] = thread(
        &OpBase_::receiveTwoVectors<mpc_t>, this, ref(temp_E), ref(temp_F), adversary(partyNum),
        size, size);

      for (int i = 0; i < 2; i++)
        threads[i].join();

      delete[] threads;
#else
      if (partyNum == PARTY_A) {
        sendTwoVectors<mpc_t>(ref(E), ref(F), adversary(partyNum), size, size);
        receiveTwoVectors<mpc_t>(ref(temp_E), ref(temp_F), adversary(partyNum), size, size);
      } else {
        receiveTwoVectors<mpc_t>(ref(temp_E), ref(temp_F), adversary(partyNum), size, size);
        sendTwoVectors<mpc_t>(ref(E), ref(F), adversary(partyNum), size, size);
      }
#endif

      //cout << "partyNum " << partyNum << "      E[0]:" << E[0] << endl;
      //cout << "partyNum " << partyNum << "      F[0]:" << F[0] << endl;
      //cout << "partyNum " << partyNum << " temp_E[0]:" << temp_E[0] << endl;
      //cout << "partyNum " << partyNum << " temp_F[0]:" << temp_F[0] << endl;

      addVectors<mpc_t>(E, temp_E, E, size);
      addVectors<mpc_t>(F, temp_F, F, size);

      for (size_t i = 0; i < size; ++i) {
        c[i] = a[i] * F[i];
        mpc_t temp = E[i] * b[i];
        c[i] = c[i] + temp;

        if (partyNum == PARTY_A) {
          temp = E[i] * F[i];
          c[i] = c[i] - temp;
        }
      }
      addVectors<mpc_t>(c, C, c, size);
      funcTruncate2PC(c, FLOAT_PRECISION_M, size, PARTY_A, PARTY_B);
    }
  }

  return 0;
}

} // namespace mpc
} // namespace rosetta