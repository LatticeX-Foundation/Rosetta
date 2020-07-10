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

/*
All operations here are compute locally
See opsets_local.h
*/
namespace rosetta {
namespace snn {

// Share Truncation, truncate shares of a by power (in place) (power is logarithmic)
void funcTruncate2PC(vector<mpc_t>& a, size_t power, size_t size, size_t party_1, size_t party_2) {
  if (!PRIMARY)
    return;
  assert((partyNum == party_1 || partyNum == party_2) && "Truncate called by spurious parties");

  if (partyNum == party_1) {
    for (size_t i = 0; i < size; ++i)
      a[i] = static_cast<mpc_t>(static_cast<int64_t>(a[i]) >> power);
  }

  if (partyNum == party_2) {
    for (size_t i = 0; i < size; ++i)
      a[i] = -static_cast<mpc_t>(static_cast<int64_t>(-a[i]) >> power);
  }
}

void funcTruncateElem2PC(mpc_t& a, size_t power, size_t party_1, size_t party_2) {
  assert((partyNum == party_1 || partyNum == party_2) && "Truncate called by spurious parties");

  if (partyNum == party_1)
    a = static_cast<mpc_t>(static_cast<int64_t>(a) >> power);

  if (partyNum == party_2)
    a = -static_cast<mpc_t>(static_cast<int64_t>(-a) >> power);
}

mpc_t funcTruncateElem2PCConst(const mpc_t& a, size_t power, size_t party_1, size_t party_2) {
  assert((partyNum == party_1 || partyNum == party_2) && "Truncate called by spurious parties");
  mpc_t r = a;
  if (partyNum == party_1)
    r = static_cast<mpc_t>(static_cast<int64_t>(r) >> power);

  if (partyNum == party_2)
    r = -static_cast<mpc_t>(static_cast<int64_t>(-r) >> power);
  return r;
}

// XOR shares with a public bit into output.
void funcXORModuloOdd2PC(
  vector<small_mpc_t>& bit, vector<mpc_t>& shares, vector<mpc_t>& output, size_t size) {
  if (!PRIMARY)
    return;
  if (partyNum == PARTY_A) {
    for (size_t i = 0; i < size; ++i) {
      if (bit[i] == 1)
        output[i] = subtractModuloOdd<small_mpc_t, mpc_t>(1, shares[i]);
      else
        output[i] = shares[i];
    }
  }

  if (partyNum == PARTY_B) {
    for (size_t i = 0; i < size; ++i) {
      if (bit[i] == 1)
        output[i] = subtractModuloOdd<small_mpc_t, mpc_t>(0, shares[i]);
      else
        output[i] = shares[i];
    }
  }
}

void funcConditionalSet2PC(
  const vector<mpc_t>& a, const vector<mpc_t>& b, vector<small_mpc_t>& c, vector<mpc_t>& u,
  vector<mpc_t>& v, size_t size) {
  assert(
    (partyNum == PARTY_C || partyNum == PARTY_D) && "ConditionalSet called by spurious parties");

  for (size_t i = 0; i < size; ++i) {
    if (c[i] == 0) {
      u[i] = a[i];
      v[i] = b[i];
    } else {
      u[i] = b[i];
      v[i] = a[i];
    }
  }
}

} // namespace mpc
} // namespace rosetta