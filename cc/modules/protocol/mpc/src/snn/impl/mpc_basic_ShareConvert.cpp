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
#include "op_impl.h"

namespace rosetta {
namespace mpc {

// Convert shares of a in \Z_L to shares in \Z_{L-1} (in place)
// a \neq L-1
int ShareConvert::funcShareConvertMPC(vector<mpc_t>& a, size_t size) {
  // LOGI("funcShareConvertMPC");

  vector<mpc_t> r(size);
  vector<small_mpc_t> etaDP(size);
  vector<small_mpc_t> alpha(size);
  vector<small_mpc_t> betai(size);
  vector<small_mpc_t> bit_shares(size * BIT_SIZE);
  vector<mpc_t> delta_shares(size);
  vector<small_mpc_t> etaP(size);
  vector<mpc_t> eta_shares(size);
  vector<mpc_t> theta_shares(size);
  size_t PARTY = PARTY_D;

  if (THREE_PC)
    PARTY = PARTY_C;
  else if (FOUR_PC)
    PARTY = PARTY_D;

  if (PRIMARY) {
    vector<mpc_t> r1(size);
    vector<mpc_t> r2(size);
    vector<mpc_t> a_tilde(size);

    populateRandomVector<mpc_t>(r1, size, "COMMON", "POSITIVE");
    populateRandomVector<mpc_t>(r2, size, "COMMON", "POSITIVE");
    addVectors<mpc_t>(r1, r2, r, size);

    if (partyNum == PARTY_A)
      wrapAround(r1, r2, alpha, size);

    if (partyNum == PARTY_A) {
      addVectors<mpc_t>(a, r1, a_tilde, size);
      wrapAround(a, r1, betai, size);
    }
    if (partyNum == PARTY_B) {
      addVectors<mpc_t>(a, r2, a_tilde, size);
      wrapAround(a, r2, betai, size);
    }

    populateBitsVector(etaDP, "COMMON", size);
    sendVector<mpc_t>(a_tilde, PARTY_C, size);
  }

  if (partyNum == PARTY_C) {
    vector<mpc_t> x(size);
    vector<small_mpc_t> delta(size);
    vector<mpc_t> a_tilde_1(size);
    vector<mpc_t> a_tilde_2(size);
    vector<small_mpc_t> bit_shares_x_1(size * BIT_SIZE);
    vector<small_mpc_t> bit_shares_x_2(size * BIT_SIZE);
    vector<mpc_t> delta_shares_1(size);
    vector<mpc_t> delta_shares_2(size);

    receiveVector<mpc_t>(a_tilde_1, PARTY_A, size);
    receiveVector<mpc_t>(a_tilde_2, PARTY_B, size);
    addVectors<mpc_t>(a_tilde_1, a_tilde_2, x, size);
    wrapAround(a_tilde_1, a_tilde_2, delta, size);

#if 0
		sharesOfBits(bit_shares_x_1, bit_shares_x_2, x, size, "INDEP");
		sendVector<small_mpc_t>(bit_shares_x_1, PARTY_A, size*BIT_SIZE);
		sendVector<small_mpc_t>(bit_shares_x_2, PARTY_B, size*BIT_SIZE);
		sharesModuloOdd<small_mpc_t>(delta_shares_1, delta_shares_2, delta, size, "INDEP");
		sendVector<mpc_t>(delta_shares_1, PARTY_A, size);
		sendVector<mpc_t>(delta_shares_2, PARTY_B, size);
#else
    sharesOfBits(bit_shares_x_1, bit_shares_x_2, x, size, "a_1");
    sendVector<small_mpc_t>(bit_shares_x_2, PARTY_B, size * BIT_SIZE);
    sharesModuloOdd<small_mpc_t>(delta_shares_1, delta_shares_2, delta, size, "a_1");
    sendVector<mpc_t>(delta_shares_2, PARTY_B, size);
#endif
  }

  if (PRIMARY) {
#if 0
		receiveVector<small_mpc_t>(bit_shares, PARTY_C, size*BIT_SIZE);
		receiveVector<mpc_t>(delta_shares, PARTY_C, size);
#else
    if (partyNum == PARTY_A) {
      gen_side_shareOfBits(bit_shares, size, "a_1");
      gen_side_sharesModuloOdd<small_mpc_t>(delta_shares, size, "a_1");
    } else {
      receiveVector<small_mpc_t>(bit_shares, PARTY_C, size * BIT_SIZE);
      receiveVector<mpc_t>(delta_shares, PARTY_C, size);
    }
#endif
  }

  ////funcPrivateCompareMPC(bit_shares, r, etaDP, etaP, size, BIT_SIZE);
  GetMpcOpInner(PrivateCompare)->Run(bit_shares, r, etaDP, etaP, size, BIT_SIZE);

  if (partyNum == PARTY) {
    vector<mpc_t> eta_shares_1(size);
    vector<mpc_t> eta_shares_2(size);

    //huanggaofeng note: not required in the paper, cause relu-prime(0) failed
    // for (size_t i = 0; i < size; ++i)
    //   etaP[i] = 1 - etaP[i];

    sharesModuloOdd<small_mpc_t>(eta_shares_1, eta_shares_2, etaP, size, "INDEP");
    sendVector<mpc_t>(eta_shares_1, PARTY_A, size);
    sendVector<mpc_t>(eta_shares_2, PARTY_B, size);
  }

  if (PRIMARY) {
    receiveVector<mpc_t>(eta_shares, PARTY, size);
    funcXORModuloOdd2PC(etaDP, eta_shares, theta_shares, size);
    addModuloOdd<mpc_t, small_mpc_t>(theta_shares, betai, theta_shares, size);
    subtractModuloOdd<mpc_t, mpc_t>(theta_shares, delta_shares, theta_shares, size);

    if (partyNum == PARTY_A)
      subtractModuloOdd<mpc_t, small_mpc_t>(theta_shares, alpha, theta_shares, size);

    subtractModuloOdd<mpc_t, mpc_t>(a, theta_shares, a, size);
  }
  return 0;
}

} // namespace mpc
} // namespace rosetta
