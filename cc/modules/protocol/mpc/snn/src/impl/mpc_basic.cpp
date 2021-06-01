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

#define PRSS_OPT_VALUE 2

// Compute MSB of a and store it in b
// 3PC: output is shares of MSB in \Z_L
int ComputeMSB::funcComputeMSB3PC(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size) {
  assert(THREE_PC && "funcComputeMSB3PC called in non-3PC mode");

  vector<mpc_t> ri(size);
  vector<small_mpc_t> bit_shares(size * BIT_SIZE);
  vector<mpc_t> LSB_shares(size);
  vector<small_mpc_t> beta(size);
  vector<mpc_t> c(size);
  vector<small_mpc_t> betaP(size);
  vector<small_mpc_t> gamma(size);
  vector<mpc_t> theta_shares(size);

  if (partyNum == PARTY_C) {
    vector<mpc_t> r1(size);
    vector<mpc_t> r2(size);
    vector<mpc_t> r(size);
    vector<small_mpc_t> bit_shares_r_1(size * BIT_SIZE);
    vector<small_mpc_t> bit_shares_r_2(size * BIT_SIZE);
    vector<mpc_t> LSB_shares_1(size);
    vector<mpc_t> LSB_shares_2(size);
    
#if 0
    for (size_t i = 0; i < size; ++i) {
      r1[i] = aes_indep->randModuloOdd();
      r2[i] = aes_indep->randModuloOdd();
    }

    addModuloOdd<mpc_t, mpc_t>(r1, r2, r, size);
    sharesOfBits(bit_shares_r_1, bit_shares_r_2, r, size, "INDEP");
    sharesOfLSB(LSB_shares_1, LSB_shares_2, r, size, "INDEP");

    sendVector<small_mpc_t>(bit_shares_r_1, PARTY_A, size * BIT_SIZE);
    sendVector<small_mpc_t>(bit_shares_r_2, PARTY_B, size * BIT_SIZE);
    sendTwoVectors<mpc_t>(r1, LSB_shares_1, PARTY_A, size, size);
    sendTwoVectors<mpc_t>(r2, LSB_shares_2, PARTY_B, size, size);
#elif PRSS_OPT_VALUE == 1
    populateRandomVector<mpc_t>(r1, size, "a_1", "POSITIVE");
    populateRandomVector<mpc_t>(r2, size, "a_2", "POSITIVE");
    for (size_t i = 0; i < size; ++i) {
      if (r1[i] == MINUS_ONE)
        r1[i] = 0;////[HGF] fix for odd ring Z_{2^128 - 1}, origin: r1[i]--
      if (r2[i] == MINUS_ONE)
        r2[i] = 0;////[HGF] fix for odd ring Z_{2^128 - 1}, origin: r2[i]--
    }

    addModuloOdd<mpc_t, mpc_t>(r1, r2, r, size);
    sharesOfBits(bit_shares_r_1, bit_shares_r_2, r, size, "INDEP");
    sharesOfLSB(LSB_shares_1, LSB_shares_2, r, size, "INDEP");

    sendVector<small_mpc_t>(bit_shares_r_1, PARTY_A, size * BIT_SIZE);
    sendVector<small_mpc_t>(bit_shares_r_2, PARTY_B, size * BIT_SIZE);
    sendVector<mpc_t>(LSB_shares_1, PARTY_A, size);
    sendVector<mpc_t>(LSB_shares_2, PARTY_B, size);
#else
    populateRandomVector<mpc_t>(r1, size, "a_1", "POSITIVE");
    populateRandomVector<mpc_t>(r2, size, "a_2", "POSITIVE");
    for (size_t i = 0; i < size; ++i) {
      if (r1[i] == MINUS_ONE)
        r1[i] = 0;////[HGF] fix for odd ring Z_{2^128 - 1}, origin: r1[i]--
      if (r2[i] == MINUS_ONE)
        r2[i] = 0;////[HGF] fix for odd ring Z_{2^128 - 1}, origin: r2[i]--
    }

    addModuloOdd<mpc_t, mpc_t>(r1, r2, r, size);
    sharesOfBits(bit_shares_r_1, bit_shares_r_2, r, size, "a_1"); //PRSS, user a
    sharesOfLSB2(LSB_shares_1, LSB_shares_2, r, size, "a_1");

    sendVector<small_mpc_t>(bit_shares_r_2, PARTY_B, size * BIT_SIZE);
    sendVector<mpc_t>(LSB_shares_2, PARTY_B, size);//
#endif
  }

  if (PRIMARY) {
#if 0
    vector<mpc_t> temp(size);
    receiveVector<small_mpc_t>(bit_shares, PARTY_C, size * BIT_SIZE);
    receiveTwoVectors<mpc_t>(ri, LSB_shares, PARTY_C, size, size);
#elif PRSS_OPT_VALUE == 1
    vector<mpc_t> temp(size);
    receiveVector<small_mpc_t>(bit_shares, PARTY_C, size * BIT_SIZE);
    receiveVector<mpc_t>(LSB_shares, PARTY_C, size);

    if (partyNum == PARTY_A)
      populateRandomVector<mpc_t>(ri, size, "a_1", "POSITIVE");
    else
      populateRandomVector<mpc_t>(ri, size, "a_2", "POSITIVE");

    for (size_t i = 0; i < size; ++i) {
      if (ri[i] == MINUS_ONE)
        ri[i] -= 1;
    }
#else
    vector<mpc_t> temp(size);
    if (partyNum == PARTY_A)
    {
      populateRandomVector<mpc_t>(ri, size, "a_1", "POSITIVE");
      gen_side_shareOfBits(bit_shares, size, "a_1");
      populateRandomVector<mpc_t>(LSB_shares, size, "a_1", "POSITIVE");
    }
    else
    {
      populateRandomVector<mpc_t>(ri, size, "a_2", "POSITIVE");
      receiveVector<small_mpc_t>(bit_shares, PARTY_C, size * BIT_SIZE);
      receiveVector<mpc_t>(LSB_shares, PARTY_C, size);
    }

    for (size_t i = 0; i < size; ++i) {
      if (ri[i] == MINUS_ONE)
        ri[i] = 0;////[HGF] fix for odd ring Z_{2^128 - 1}, origin: ri[i] -= 1;
    }
#endif

    addModuloOdd<mpc_t, mpc_t>(a, a, c, size);
    addModuloOdd<mpc_t, mpc_t>(c, ri, c, size);

    thread* threads = new thread[2];

    threads[0] = thread(&OpBase_::sendVector<mpc_t>, this, ref(c), adversary(partyNum), size);
    threads[1] = thread(&OpBase_::receiveVector<mpc_t>, this, ref(temp), adversary(partyNum), size);
    for (int i = 0; i < 2; i++)
      threads[i].join();
    delete[] threads;

    addModuloOdd<mpc_t, mpc_t>(c, temp, c, size);
    populateBitsVector(beta, "COMMON", size);
  }

  GetMpcOpInner(PrivateCompare)->Run(bit_shares, c, beta, betaP, size, BIT_SIZE);

  if (partyNum == PARTY_C) {
    vector<mpc_t> theta_shares_1(size);
    vector<mpc_t> theta_shares_2(size);

#if 0
    sharesOfBitVector(theta_shares_1, theta_shares_2, betaP, size, "INDEP");
    sendVector<mpc_t>(theta_shares_1, PARTY_A, size);
    sendVector<mpc_t>(theta_shares_2, PARTY_B, size);
#else
    sharesOfBitVector(theta_shares_1, theta_shares_2, betaP, size, "a_2");
    sendVector<mpc_t>(theta_shares_1, PARTY_A, size);
#endif
  }

  vector<mpc_t> prod(size), temp(size);
  if (PRIMARY) {
    // theta_shares is the same as gamma (in older versions);
    // LSB_shares is the same as delta (in older versions);
#if 0
	  receiveVector<mpc_t>(theta_shares, PARTY_C, size);
#else
    if (partyNum == PARTY_A)
      receiveVector<mpc_t>(theta_shares, PARTY_C, size);
    else
      gen_side_shareOfBitVector(theta_shares, size, "a_2");

#endif

    mpc_t j = 0;
    if (partyNum == PARTY_A)
      j = FloatToMpcType(1);

    for (size_t i = 0; i < size; ++i) {
      theta_shares[i] = (1 - 2 * beta[i]) * theta_shares[i] + j * beta[i];
      LSB_shares[i] = (1 - 2 * (c[i] & 1)) * LSB_shares[i] + j * (c[i] & 1);
    }
  }

  GetMpcOpInner(DotProduct)->Run(theta_shares, LSB_shares, prod, size);

  if (PRIMARY) {
    populateRandomVector<mpc_t>(temp, size, "COMMON", "NEGATIVE");
    for (size_t i = 0; i < size; ++i)
      b[i] = theta_shares[i] + LSB_shares[i] - 2 * prod[i] + temp[i];
  }
  return 0;
}

// 3PC SelectShares
// a,b,c are shared across PARTY_A, PARTY_B
int SelectShares::funcSelectShares3PC(
  const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size) {
  assert(THREE_PC && "funcSelectShares3PC called in non-3PC mode");

  //funcDotProductMPC(a, b, c, size);
  GetMpcOpInner(DotProduct)->Run(a, b, c, size);

  return 0;
}

int Square::funcSquareMPC(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size)
{
  assert(THREE_PC && "funcSelectShares3PC called in non-3PC mode");

  //funcDotProductMPC(a, a, c, size);
  GetMpcOpInner(DotProduct)->Run(a, a, b, size);

  return 0;
}

} // namespace mpc
} // namespace rosetta
