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

#include "cc/modules/protocol/mpc/comm/include/mpc_helper.h"
#include "cc/modules/protocol/mpc/snn/include/opsets_base.h"

#include <vector>
#include <string>
using namespace std;

namespace rosetta {
namespace snn {
// clang-format off
void funcTruncate2PC(vector<mpc_t> &a, size_t power, size_t size, size_t party_1, size_t party_2);
void funcTruncate2PC_many(vector<mpc_t>& a, vector<size_t> power, size_t size, size_t party_1, size_t party_2);
void funcTruncateElem2PC(mpc_t &a, size_t power, size_t party_1, size_t party_2);
mpc_t funcTruncateElem2PCConst(const mpc_t &a, size_t power = FLOAT_PRECISION_M, size_t party_1 = PARTY_A, size_t party_2 = PARTY_B);
void funcXORModuloOdd2PC(vector<small_mpc_t> &bit, vector<mpc_t> &shares, vector<mpc_t> &output, size_t size);
void funcConditionalSet2PC(const vector<mpc_t> &a, const vector<mpc_t> &b, vector<small_mpc_t> &c, vector<mpc_t> &u, vector<mpc_t> &v, size_t size);
// clang-format on
} // namespace mpc
} // namespace rosetta
