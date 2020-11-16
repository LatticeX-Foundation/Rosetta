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
#include "cc/modules/protocol/mpc/comm/include/mpc_defines.h"
#include "cc/modules/io/include/ex.h"
#include "cc/modules/common/include/utils/simple_timer.h"
#include "cc/modules/common/include/utils/helper.h"

// clang-format off
void convert_mpctype_to_double(const vector<mpc_t>& a, vector<double>& b);
void convert_double_to_mpctype(const vector<double>& a, vector<mpc_t>& b);

void convert_string_to_mpctype(const vector<std::string>& a, vector<mpc_t>& b, bool human = true);
void convert_mpctype_to_string(const vector<mpc_t>& a, vector<std::string>& b, bool human = true);

vector<int> get_mpc_peers(int party);
// clang-format on
