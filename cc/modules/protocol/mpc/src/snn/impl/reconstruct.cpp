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
      cout << (signed_mpc_t)out[i] << " ";
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
// Added by SJJ
int Reconstruct2PC::reconstrct_general(
  const vector<mpc_t>& shared_v, size_t size, vector<mpc_t>& plaintext_v, int target_party) {
  if (shared_v.size() < size) {
    size = shared_v.size();
  }

  plaintext_v.resize(size);

  if (target_party == PARTY_A || target_party == PARTY_B) {
    if (!PRIMARY) {
      return 1;
    }
    int tempPartyA = (target_party == PARTY_A) ? PARTY_A : PARTY_B;
    int tempPartyB = (target_party == PARTY_A) ? PARTY_B : PARTY_A;
    if (partyNum == tempPartyB) {
      sendVector<mpc_t>(shared_v, tempPartyA, size);
    }

    if (partyNum == tempPartyA) {
      receiveVector<mpc_t>(plaintext_v, tempPartyB, size);
      addVectors<mpc_t>(plaintext_v, shared_v, plaintext_v, size);
    }
  } else {
    // the receiver is neither P0 nor P1
    if (PRIMARY) {
      sendVector<mpc_t>(shared_v, target_party, size);
    } else if (partyNum == target_party) {
      vector<mpc_t> tmp_in(size, 0);
      receiveVector<mpc_t>(tmp_in, PARTY_A, size);
      receiveVector<mpc_t>(plaintext_v, PARTY_B, size);
      addVectors<mpc_t>(plaintext_v, tmp_in, plaintext_v, size);
    }
  }

  return 0;
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
