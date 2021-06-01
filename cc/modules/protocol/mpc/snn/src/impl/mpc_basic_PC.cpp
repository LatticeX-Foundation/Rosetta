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

int PrivateCompare::funcPrivateCompareMPC(
  const vector<small_mpc_t>& share_m, const vector<mpc_t>& r, const vector<small_mpc_t>& beta,
  vector<small_mpc_t>& betaPrime, size_t size, size_t dim) {
  assert(THREE_PC && "funcShareConvertMPC called in non-3PC mode");
  assert(dim == BIT_SIZE && "Private Compare assert issue");
  size_t sizeLong = size * dim;
  size_t index3, index2;
  size_t PARTY = PARTY_D;

  if (THREE_PC)
    PARTY = PARTY_C;
  else if (FOUR_PC)
    PARTY = PARTY_D;

  if (PRIMARY) {
    small_mpc_t bit_r, a, tempM;
    vector<small_mpc_t> c(sizeLong);
    mpc_t valueX;

    if (PARALLEL) {
      // I have removed the parallel for private compare, by yyl, 2020.03.12
    } else {
      // Check the security of the first if condition
      for (size_t index2 = 0; index2 < size; ++index2) {
        if (beta[index2] == 1 and r[index2] != MINUS_ONE)
          valueX = r[index2] + 1;
        else
          valueX = r[index2];

        if (beta[index2] == 1 and r[index2] == MINUS_ONE) {
          // One share of zero and other shares of 1
          // Then multiply and shuffle
          for (size_t k = 0; k < dim; ++k) {
            index3 = index2 * dim + k;
            c[index3] = aes_common->randModPrime();
            if (partyNum == PARTY_A)
              c[index3] = subtractModPrime((k != 0), c[index3]);

            c[index3] = multiplyModPrime(c[index3], aes_common->randNonZeroModPrime());
          }
        } else {
          // Single for loop
          a = 0;
          for (size_t k = 0; k < dim; ++k) {
            index3 = index2 * dim + k;
            c[index3] = a;
            tempM = share_m[index3];

            bit_r = (small_mpc_t)((valueX >> (BIT_SIZE - 1 - k)) & 1);

            if (bit_r == 0)
              a = addModPrime(a, tempM);
            else
              a = addModPrime(a, subtractModPrime((partyNum == PARTY_A), tempM));

            if (!beta[index2]) {
              if (partyNum == PARTY_A)
                c[index3] = addModPrime(c[index3], 1 + bit_r);
              c[index3] = subtractModPrime(c[index3], tempM);
            } else {
              if (partyNum == PARTY_A)
                c[index3] = addModPrime(c[index3], 1 - bit_r);
              c[index3] = addModPrime(c[index3], tempM);
            }

            c[index3] = multiplyModPrime(c[index3], aes_common->randNonZeroModPrime());
          }
        }
        aes_common->AES_random_shuffle(c, index2 * dim, (index2 + 1) * dim);
      }
    }
    sendVector<small_mpc_t>(c, PARTY, sizeLong);
  }

  if (partyNum == PARTY) {
    vector<small_mpc_t> c1(sizeLong);
    vector<small_mpc_t> c2(sizeLong);

    receiveVector<small_mpc_t>(c1, PARTY_A, sizeLong);
    receiveVector<small_mpc_t>(c2, PARTY_B, sizeLong);

    for (size_t index2 = 0; index2 < size; ++index2) {
      betaPrime[index2] = 0;
      for (int k = 0; k < dim; ++k) {
        index3 = index2 * dim + k;
        if (addModPrime(c1[index3], c2[index3]) == 0) {
          betaPrime[index2] = 1;
          break;
        }
      }
    }
  }

  return 0;
}

} // namespace mpc
} // namespace rosetta
