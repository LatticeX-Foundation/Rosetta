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
#include <cstring>
#include <iostream>
#include <fstream>
#include <thread>
using namespace std;

#include "cc/modules/protocol/mpc/snn/src/internal/TedKrovetzAesNiWrapperC.h"

#include "cc/modules/protocol/mpc/snn/src/internal/ParallelAESObject.h"

void swapSmallTypes(small_mpc_t* a, small_mpc_t* b) {
  small_mpc_t temp = *a;
  *a = *b;
  *b = temp;
}

void ParallelAESObject::precompute() {
  for (size_t i = 0; i < PC_CALLS_MAX * SHUFFLE_MAX * NO_CORES; ++i)
    randomNumber[i] = 1;

  for (size_t i = 0; i < PC_CALLS_MAX * NONZERO_MAX * NO_CORES; ++i)
    randomNonZero[i] = 1;
}

small_mpc_t ParallelAESObject::randNonZeroModPrime(int t, int& offset) {
  return randomNonZero
      [(counterPC * NONZERO_MAX * NO_CORES + t * NONZERO_MAX + offset++) % PC_CALLS_MAX *
       NONZERO_MAX * NO_CORES];
}

small_mpc_t ParallelAESObject::AES_random(int i, int t, int& offset) {
  small_mpc_t ret;

  do {
    ret = randomNumber
        [(counterPC * SHUFFLE_MAX * NO_CORES + t * SHUFFLE_MAX + offset++) % PC_CALLS_MAX *
         SHUFFLE_MAX * NO_CORES];
  } while (ret >= ((256 / i) * i));

  return ret;
}

void ParallelAESObject::AES_random_shuffle(
    small_mpc_t* vec,
    size_t begin_offset,
    size_t end_offset,
    int t,
    int& offset) {
  auto n = end_offset - begin_offset;

  for (auto i = n - 1; i > 0; --i)
    swapSmallTypes(&vec[begin_offset + i], &vec[begin_offset + AES_random(i + 1, t, offset)]);
}
