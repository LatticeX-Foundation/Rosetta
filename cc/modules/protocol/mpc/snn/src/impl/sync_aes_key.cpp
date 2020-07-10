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

/*
partyA send key to partyB as the public key of A and B
eg:
kab: A --> B
kac: A --> C
kbc: B --> C
*/
int SyncAesKey::funcSyncAesKey(
  int partyA, int partyB, std::string& key_send, std::string& key_recv) {
  if (partyNum == partyA) {
    sendBuf(partyB, key_send.data(), key_send.length(), 0);
  }
  if (partyNum == partyB) {
    receiveBuf(partyA, (char*)key_recv.data(), key_recv.length(), 0);
  }
  return 0;
}
} // namespace mpc
} // namespace rosetta
