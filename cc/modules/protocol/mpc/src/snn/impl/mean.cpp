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
#include "op_impl.h"

namespace rosetta {
namespace mpc {
int Mean::funcMean(const vector<mpc_t>& a, vector<mpc_t>& b, size_t rows, size_t cols) {
  if (PRIMARY) {
    for (int i = 0; i < rows; i++) {
      mpc_t v = 0;
      for (int j = 0; j < cols; j++) {
        int idx = i * cols + j;
        v += a[idx];
      }
      b[i] = v * FloatToMpcType(1.0 / cols);
    }
    funcTruncate2PC(b, FLOAT_PRECISION, rows, PARTY_A, PARTY_B);
  }
  return 0;
}
} // namespace mpc
} // namespace rosetta