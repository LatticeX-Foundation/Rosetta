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

int Synchronize::funcSynchronize() {
  char buf[2] = {0};
  if (partyNum == PARTY_A) {
    sendBuf(PARTY_B, buf, 1);
    sendBuf(PARTY_C, buf, 1);
    receiveBuf(PARTY_B, buf, 1);
    receiveBuf(PARTY_C, buf, 1);
  } else if (partyNum == PARTY_B) {
    receiveBuf(PARTY_A, buf, 1);
    sendBuf(PARTY_A, buf, 1);
    sendBuf(PARTY_C, buf, 1);
    receiveBuf(PARTY_C, buf, 1);
  } else if (partyNum == PARTY_C) {
    receiveBuf(PARTY_A, buf, 1);
    receiveBuf(PARTY_B, buf, 1);
    sendBuf(PARTY_A, buf, 1);
    sendBuf(PARTY_B, buf, 1);
  }
  return 0;
}
} // namespace mpc
} // namespace rosetta