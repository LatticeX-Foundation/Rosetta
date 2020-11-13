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
#include <stdexcept>

// this will remvoe to rtt/
namespace rosetta {
namespace snn {

int Broadcast::funcBroadcast(int from_party, const string& msg, string& result) {
  log_debug << "snn public input msg, size: " << msg.size();

  result.clear();
  vector<int> peers = get_mpc_peers(from_party);
  if (from_party == partyNum) {
    //send msg to peers
    for (size_t i = 0; i < peers.size(); ++i)
      sendBuf(peers[i], msg.data(), msg.size(), 0);
  } else {
    // receive msg
    result.resize(msg.size());
    receiveBuf(from_party, (char*)result.data(), msg.size(), 0);
  }

  log_debug << "snn public input msg ok.";
  return 0;
}

} // namespace snn
} // namespace rosetta