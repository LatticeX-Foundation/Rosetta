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
#include "cc/modules/protocol/mpc/naive/include/naive_impl.h"
#include "cc/modules/protocol/mpc/naive/include/naive_ops_impl.h"
#include "cc/modules/io/include/net_io.h"

#include <stdexcept>
#include <string>
#include <vector>
using namespace std;

namespace rosetta {

shared_ptr<ProtocolOps> NaiveProtocol::GetOps(const string& op_token) {
  auto naive_ops_ptr = make_shared<NaiveOpsImpl>(op_token);
  // In this insecure naive protocol, we pass the party role ID to inner NaiveOpsImpl directly
  // in this inelegant way. For production protocol, please refer to SecureNN implementation. 
  naive_ops_ptr->op_config_map["PID"] = "P" + to_string(my_party_id);
  naive_ops_ptr->io = GetNetHandler();
  return std::dynamic_pointer_cast<ProtocolOps>(naive_ops_ptr);
}

} // namespace rosetta
