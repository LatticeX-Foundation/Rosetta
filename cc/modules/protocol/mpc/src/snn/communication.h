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

#include "snn_helper.h"

// communication options
struct CommOptions {
  int parties; // total party number (now, only supports 3)
  int party_id; // the party index, start with 0
  std::vector<std::string> hosts; // hosts for all parties, in order
  int base_port; // the base port
  //
  string server_cert_;
  string server_prikey_;
  string server_prikey_password_;
};

bool initialize_communication(CommOptions& opt);
void uninitialize_communication();
void synchronize(const msg_id_t& msg_id);
void synchronize(int length);
void synchronize();
