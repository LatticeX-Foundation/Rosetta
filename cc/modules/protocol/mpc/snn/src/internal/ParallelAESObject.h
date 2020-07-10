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
#include <algorithm>
#include "cc/modules/protocol/mpc/snn/src/internal/snn_helper.h"

#define NONZERO_MAX 100
#define SHUFFLE_MAX 100
#define PC_CALLS_MAX 100

class ParallelAESObject {
 private:
  // Private helper functions
  small_mpc_t AES_random(int i, int t, int& offset);

  // precomputed random numbers
  small_mpc_t randomNumber[PC_CALLS_MAX * SHUFFLE_MAX * NO_CORES];
  small_mpc_t randomNonZero[PC_CALLS_MAX * NONZERO_MAX * NO_CORES];

  uint64_t counterPC = 0;

 public:
  // Constructor
  ParallelAESObject(const char* filename){};
  void precompute();

  // Other randomness functions
  small_mpc_t randNonZeroModPrime(int t, int& offset);
  void AES_random_shuffle(
      small_mpc_t* vec,
      size_t begin_offset,
      size_t end_offset,
      int t,
      int& offset);
  void counterIncrement() {
    counterPC++;
  };
};
