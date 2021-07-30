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
#pragma once
#include <string>
#include <vector>
#include <unordered_map>

#include "cc/modules/protocol/public/include/protocol_base.h"
#include "cc/modules/protocol/public/include/protocol_ops.h"

namespace rosetta {
class NaiveOpsImpl : public ProtocolOps {
public:
  NaiveOpsImpl(const msg_id_t& msg_id, shared_ptr<ProtocolContext> context) : ProtocolOps(msg_id, context) {}
  ~NaiveOpsImpl() = default;

  int PrivateInput(
    const string& node_id,
    const vector<double>& in_vec,
    vector<string>& out_str_vec);

  int Add(const vector<string>& a,
          const vector<string>& b,
          vector<string>& output,
          const attr_type* attr_info = nullptr);
  
  int Sub(const vector<string>& a,
          const vector<string>& b,
          vector<string>& output,
          const attr_type* attr_info = nullptr);
  
  int Mul(const vector<string>& a,
          const vector<string>& b,
          vector<string>& output,
          const attr_type* attr_info = nullptr);
    
  int Reveal(const vector<string>& a,
              vector<string>& output,
              const attr_type* attr_info = nullptr);

  int Reveal(const vector<string>& a, 
              vector<double>& output, 
              const attr_type* attr_info = nullptr);

    public : shared_ptr<NET_IO> io = nullptr;
};

} // namespace rosetta
