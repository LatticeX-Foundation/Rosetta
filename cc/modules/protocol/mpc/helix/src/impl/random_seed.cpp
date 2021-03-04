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
int HelixOpsImpl::RandSeed(std::string op_seed, string& out_str) { return RandSeed(); }
uint64_t HelixOpsImpl::RandSeed() {
  vector<uint64_t> seeds(1);
  hi->RandSeed(seeds, 1);
  return seeds[0];
}
uint64_t HelixOpsImpl::RandSeed(vector<uint64_t>& seed) {
  int size = seed.size();
  if (size <= 0)
    size = 1;
  hi->RandSeed(seed, size);
  return seed[0];
}
} // namespace rosetta
