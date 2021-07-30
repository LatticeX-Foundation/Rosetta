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

#include <iostream>
#include <string>
#include <vector>

using namespace std;
namespace rosetta {

// AND/OR/XOR see math_binary_op.cpp
// NOT
int HelixOpsImpl::NOT(const vector<string>& a, vector<string>& c, const attr_type* attr_info) {
  vector<Share> shareA, shareB, shareC;
  helix_convert_string_to_share(a, shareA);
  AUDIT("id:{}, NOT P{} input{}", _op_msg_id.get_hex(), hi->party_id(), Vector<Share>(shareA));

  hi->NOT(shareA, shareC);
  helix_convert_share_to_string(shareC, c);
  AUDIT("id:{}, NOT P{} output{}", _op_msg_id.get_hex(), hi->party_id(), Vector<Share>(shareC));

  return 0;
}
} // namespace rosetta
