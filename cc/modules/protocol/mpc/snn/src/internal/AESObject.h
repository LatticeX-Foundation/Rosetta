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

#include "cc/modules/protocol/mpc/snn/src/internal/TedKrovetzAesNiWrapperC.h"

class AESObject {
 private:
 public:
  // AES variables
  __m128i pseudoRandomString[RANDOM_COMPUTE];
  unsigned long rCounter = -1;
  AES_KEY_TED aes_key;

  // Extraction variables
  __m128i randomBitNumber{0};
  uint8_t randomBitCounter = 0;
  __m128i random8BitNumber{0};
  uint8_t random8BitCounter = 0;
  __m128i random64BitNumber{0};
  uint8_t random64BitCounter = 0;

  // Private extraction functions
  __m128i newRandomNumber();

  // Private helper functions
  small_mpc_t AES_random(int i);

 public:
  // Constructor
  explicit AESObject(const char* filename);
  explicit AESObject() = default;
  void Init(const std::string& key);

  // Randomness functions
  mpc_t get64Bits();
  small_mpc_t get8Bits();
  small_mpc_t getBit();

  // Other randomness functions
  small_mpc_t randModPrime();
  small_mpc_t randNonZeroModPrime();
  mpc_t randModuloOdd();
  void AES_random_shuffle(vector<small_mpc_t>& vec, size_t begin_offset, size_t end_offset);
};
