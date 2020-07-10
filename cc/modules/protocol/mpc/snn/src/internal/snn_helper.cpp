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
#include "cc/modules/protocol/mpc/snn/src/internal/snn_helper.h"
#include "cc/modules/protocol/mpc/snn/include/snn_opsets.h"

using namespace rosetta::mpc;

int NUM_OF_PARTIES; /// only for snn. total parties
int partyNum; /// only for snn. global variable. For this player number

//! only for snn. global variable. For faster DGK computation
small_mpc_t additionModPrime[PRIME_NUMBER][PRIME_NUMBER];
small_mpc_t multiplicationModPrime[PRIME_NUMBER][PRIME_NUMBER];

//! only for snn.
void initializeMPC() {
  // populate offline module prime addition and multiplication tables
  for (int i = 0; i < PRIME_NUMBER; ++i)
    for (int j = 0; j < PRIME_NUMBER; ++j) {
      additionModPrime[i][j] = (i + j) % PRIME_NUMBER;
      multiplicationModPrime[i][j] = (i * j) % PRIME_NUMBER;
    }
}
