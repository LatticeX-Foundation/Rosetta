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

// this will remvoe to rtt/
namespace rosetta {
namespace snn {

int PRZS::funPRZS(int party0, int party1, vector<mpc_t>& shares) {
  string r_type = "COMMON";
  if (
    ((party0 == PARTY_A) && (party1 == PARTY_B)) || ((party0 == PARTY_B) && (party1 == PARTY_A))) {
    r_type = "COMMON";
  } else if (
    ((party0 == PARTY_A) && (party1 == PARTY_C)) || ((party0 == PARTY_C) && (party1 == PARTY_A))) {
    r_type = "a_1";
  } else if (
    ((party0 == PARTY_B) && (party1 == PARTY_C)) || ((party0 == PARTY_C) && (party1 == PARTY_B))) {
    r_type = "a_2";
  } else {
    notYet();
  }

  populateRandomVector2<mpc_t>(shares, shares.size(), r_type, "NEGATIVE");
  return 0;
}

} // namespace mpc
} // namespace rosetta