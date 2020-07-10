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

#define RELIC_BUFSIZE RLC_BN_BITS / 8

#define BN_INIT1(V1) \
  bn_t V1;           \
  bn_null(V1);       \
  bn_new(V1)
#define BN_INIT2(V1, V2) \
  BN_INIT1(V1);          \
  BN_INIT1(V2)
#define BN_INIT3(V1, V2, V3) \
  BN_INIT2(V1, V2);          \
  BN_INIT1(V3)
#define BN_INIT4(V1, V2, V3, V4) \
  BN_INIT2(V1, V2);              \
  BN_INIT2(V3, V4)
#define BN_INIT BN_INIT1
