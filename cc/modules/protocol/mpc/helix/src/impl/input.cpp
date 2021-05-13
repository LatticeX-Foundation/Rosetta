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
#include "helix_impl_util.h"

namespace rosetta {
int HelixOpsImpl::PrivateInput(int party_id, const vector<double>& in_x, vector<string>& out_x) {
  vector<Share> shareX(in_x.size());
  hi->Input(party_id, in_x, shareX);
  helix_convert_share_to_string(shareX, out_x);
  return 0;
}


int HelixOpsImpl::Broadcast(int from_party, const string& msg, string& result) {
  hi->Broadcast(from_party, msg, result);
  return 0;
}

int HelixOpsImpl::Broadcast(int from_party, const char* msg, char* result, size_t size) {
  hi->Broadcast(from_party, msg, result, size);
  return 0;
}

} // namespace rosetta
