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

// PARTY_A, PARTY_B hold shares in a, want shares of RELU in b.
int Relu::funcRELUMPC(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size) {
  if (THREE_PC) {
    vector<mpc_t> reluPrime(size);

    //funcRELUPrime3PC(a, reluPrime, size);
    GetMpcOpInner(ReluPrime)->Run3PC(a, reluPrime, size);

    //funcSelectShares3PC(a, reluPrime, b, size);
    GetMpcOpInner(SelectShares)->Run3PC(a, reluPrime, b, size);
  }

  return 0;
}

// 3PC: PARTY_A, PARTY_B hold shares in a, want shares of RELU' in b.
int ReluPrime::funcRELUPrime3PC(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size) {
  assert(THREE_PC && "funcRELUPrime3PC called in non-3PC mode");

  vector<mpc_t> twoA(size, 0);
  mpc_t j = 0;

  for (size_t i = 0; i < size; ++i)
    twoA[i] = (a[i] << 1);

  //funcShareConvertMPC(twoA, size);
  GetMpcOpInner(ShareConvert)->Run(twoA, size);

  //funcComputeMSB3PC(twoA, b, size);
  GetMpcOpInner(ComputeMSB)->Run3PC(twoA, b, size);

  if (partyNum == PARTY_A)
    j = FloatToMpcType(1);

  if (PRIMARY) {
    for (size_t i = 0; i < size; ++i)
      b[i] = j - b[i];
  }

  return 0;
}
} // namespace mpc
} // namespace rosetta
