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

int Reconstruct2PC::funcReconstruct2PC(const vector<mpc_t>& a, size_t size, string str) {
  if (!PRIMARY)
    return 1;
  assert((partyNum == PARTY_A || partyNum == PARTY_B) && "Reconstruct called by spurious parties");

  if (a.size() < size)
    size = a.size();

  vector<mpc_t> out;
  funcReconstruct2PC(a, size, out, PARTY_A);

  if (partyNum == PARTY_A) {
    cout << str << "[shared]: ";
    for (size_t i = 0; i < size; ++i) {
      cout << to_readable_dec((signed_mpc_t)out[i]) << " ";
    }
    cout << endl;
    cout << str << "[plain]: ";
    for (size_t i = 0; i < size; ++i) {
      cout << MpcTypeToFloat(out[i]) << " ";
    }
    cout << endl;
  }

  return 0;
}

int Reconstruct2PC::funcReconstruct2PC(
  const vector<mpc_t>& a, size_t size, vector<mpc_t>& out, int recv_party) {
  if (!PRIMARY)
    return 1;
  assert((partyNum == PARTY_A || partyNum == PARTY_B) && "Reconstruct called by spurious parties");
  if (a.size() < size)
    size = a.size();

  //cout << "----  recons 2222" << endl;
  out.resize(size, 0);
  int tempPartyA = (recv_party == PARTY_A) ? PARTY_A : PARTY_B;
  int tempPartyB = (recv_party == PARTY_A) ? PARTY_B : PARTY_A;
  if (partyNum == tempPartyB) {
    sendVector<mpc_t>(a, tempPartyA, size);
  }

  if (partyNum == tempPartyA) {
    receiveVector<mpc_t>(out, tempPartyB, size);
    addVectors<mpc_t>(out, a, out, size);
  }

  return 0;
}

/**
 * Reveal, arithmetic
 * 
 * \param plain the result (fixpoint for arithmetic), sets to p
 * \param p bit-wised, which party will get the plaintext
 * 
 * p --> 0x 0 1 1 1
 * P -->      2 1 0
 * eg.
 * p ==> 0x ... 0000 0001 --> P0
 * p ==> 0x ... 0000 0101 --> P2 & P0
 * p ==> 0x ... 0000 0111 --> P2 & P1 & P0
 * and so on.
 * 
 * for balancing traffic:
 * reveal P0: P1 sends A1 to P0
 * reveal P1: P2 sends A0 to P1
 * reveal P2: P0 sends delta to P2
 * 
 * @note
 * <T1,T2> ==>  <Share, mpc_t> or <BitShare, bit_t>
 */
int Reconstruct2PC::funcReconstruct2PC_ex(
  const vector<mpc_t>& a, size_t size, vector<mpc_t>& out, int recv_party) {
  if (recv_party > 7 || recv_party <= 0)
  {
    cout << "!! bad receive_party, should be 1-7, Notice: one bit represent for one part\n" << endl;
    return -1;
  }

  if (a.size() < size)
    size = a.size();

  out.resize(size, 0);
 
  bool reveal_a = recv_party & 0x00000001 ? true : false;
  bool reveal_b = recv_party & 0x00000002 ? true : false;
  bool reveal_c = recv_party & 0x00000004 ? true : false;

  if (reveal_a) {
    if (partyNum == PARTY_A) {
      receiveVector<mpc_t>(out, PARTY_B, size);
      addVectors<mpc_t>(out, a, out, size);
    }
    if (partyNum == PARTY_B) {
      sendVector<mpc_t>(a, PARTY_A, size);
    }
  }

  if (reveal_b) {
    if (partyNum == PARTY_A) {
      sendVector<mpc_t>(a, PARTY_B, size);
    }
    if (partyNum == PARTY_B) {
      receiveVector<mpc_t>(out, PARTY_A, size);
      addVectors<mpc_t>(out, a, out, size);
    }
  }

  if (reveal_c) {
    if (partyNum == PARTY_A) {
      sendVector<mpc_t>(a, PARTY_C, size);
    }
    if (partyNum == PARTY_B) {
      sendVector<mpc_t>(a, PARTY_C, size);
    }
    if (partyNum == PARTY_C) {
      receiveVector<mpc_t>(out, PARTY_A, size);

      vector<mpc_t> b_secret(size, 0);
      receiveVector<mpc_t>(b_secret, PARTY_B, size);
      addVectors<mpc_t>(out, b_secret, out, size);
    }
  }

  return 0;
}

/**
 * Reveal, arithmetic, the input is in Z_{L-1}
 * 
 * \param plain the result (fixpoint for arithmetic), sets to p
 * \param p bit-wised, which party will get the plaintext
 * 
 * p --> 0x 0 1 1 1
 * P -->      2 1 0
 * eg.
 * p ==> 0x ... 0000 0001 --> P0
 * p ==> 0x ... 0000 0101 --> P2 & P0
 * p ==> 0x ... 0000 0111 --> P2 & P1 & P0
 * and so on.
 * 
 * for balancing traffic:
 * reveal P0: P1 sends A1 to P0
 * reveal P1: P2 sends A0 to P1
 * reveal P2: P0 sends delta to P2
 * 
 * @note
 * <T1,T2> ==>  <Share, mpc_t> or <BitShare, bit_t>
 */
int Reconstruct2PC::funcReconstruct2PC_ex_mod_odd(
  const vector<mpc_t>& a, vector<mpc_t>& out, int recv_party) {
  if (recv_party > 7 || recv_party <= 0)
  {
    cout << "!! bad receive_party, should be 1-7, Notice: one bit represent for one part\n" << endl;
    return -1;
  }

  size_t size = a.size();

  out.resize(size, 0);
 
  bool reveal_a = recv_party & 0x00000001 ? true : false;
  bool reveal_b = recv_party & 0x00000002 ? true : false;
  bool reveal_c = recv_party & 0x00000004 ? true : false;

  if (reveal_a) {
    if (partyNum == PARTY_A) {
      receiveVector<mpc_t>(out, PARTY_B, size);
      addModuloOdd<mpc_t, mpc_t>(out, a, out, size);
    }
    if (partyNum == PARTY_B) {
      sendVector<mpc_t>(a, PARTY_A, size);
    }
  }

  if (reveal_b) {
    if (partyNum == PARTY_A) {
      sendVector<mpc_t>(a, PARTY_B, size);
    }
    if (partyNum == PARTY_B) {
      receiveVector<mpc_t>(out, PARTY_A, size);
      addModuloOdd<mpc_t, mpc_t>(out, a, out, size);
    }
  }

  if (reveal_c) {
    if (partyNum == PARTY_A) {
      sendVector<mpc_t>(a, PARTY_C, size);
    }
    if (partyNum == PARTY_B) {
      sendVector<mpc_t>(a, PARTY_C, size);
    }
    if (partyNum == PARTY_C) {
      receiveVector<mpc_t>(out, PARTY_A, size);

      vector<mpc_t> b_secret(size, 0);
      receiveVector<mpc_t>(b_secret, PARTY_B, size);
      addModuloOdd<mpc_t, mpc_t>(out, b_secret, out, size);
    }
  }

  return 0;
}

// Added by SJJ
int Reconstruct2PC::reconstruct_general(
  const vector<mpc_t>& shared_v, size_t size, vector<mpc_t>& plaintext_v, int target_party) {
  // Note: [HGF] fix this
  if (target_party > PARTY_C || target_party < PARTY_A) {
    target_party = PARTY_A;
  }

  return funcReconstruct2PC_ex(shared_v, size, plaintext_v, 0x00000001 << target_party);
}

int ReconstructBit2PC::funcReconstructBit2PC(
  const vector<small_mpc_t>& a, size_t size, string str) {
  if (!PRIMARY)
    return 1;
  assert((partyNum == PARTY_A || partyNum == PARTY_B) && "Reconstruct called by spurious parties");

  vector<small_mpc_t> temp(size);
  if (partyNum == PARTY_B)
    sendVector<small_mpc_t>(a, PARTY_A, size);

  if (partyNum == PARTY_A) {
    receiveVector<small_mpc_t>(temp, PARTY_B, size);
    XORVectors(temp, a, temp, size);

    cout << str << ": ";
    for (size_t i = 0; i < size; ++i)
      cout << (int)temp[i] << " ";
    cout << endl;
  }
  return 0;
}

} // namespace mpc
} // namespace rosetta
