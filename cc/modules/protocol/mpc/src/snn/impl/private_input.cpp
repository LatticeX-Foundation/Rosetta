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

// this will remvoe to rtt/
namespace rosetta {
namespace mpc {

int PrivateInput::funPrivateInput(int party, const vector<double>& v, vector<mpc_t>& shares) {
  if (party == PARTY_C) {
    auto przs = GetMpcOpInner(PRZS);
    przs->Run(PARTY_B, PARTY_C, shares);

    if (partyNum == PARTY_C) {
      vector<mpc_t> vs(v.size());
      convert_double_to_mpctype(v, vs);
      for (size_t i = 0; i < vs.size(); ++i)
        shares[i] = shares[i] + vs[i];

      // send C's to A
      sendVector<mpc_t>(shares, PARTY_A, shares.size());
    } else if (partyNum == PARTY_A) {
      receiveVector<mpc_t>(shares, PARTY_C, shares.size());
    }

    return 0;
  }

  if (PRIMARY) {
    auto przs = GetMpcOpInner(PRZS);
    przs->Run(PARTY_A, PARTY_B, shares);

    if (partyNum == party) {
      vector<mpc_t> vs(v.size());
      convert_double_to_mpctype(v, vs);
      for (size_t i = 0; i < vs.size(); ++i)
        shares[i] = shares[i] + vs[i];
    }
  }
  return 0;
}

} // namespace mpc
} // namespace rosetta