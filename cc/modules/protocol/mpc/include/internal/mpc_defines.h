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

// global_defines
// some MACRO defined in mpc/CMakeLists.txt
#include "global_defines.h"
// global_defines

#include "simple_timer.h"

#include <emmintrin.h>
#include <cassert>
#include <climits>
#include <vector>
#include <string>
using namespace std;

// clang-format off

// Typedefs and others
// SML_USE_* defined in mpc/CMakeLists.txt
#if SML_USE_UINT32 
typedef uint32_t mpc_t; // unsigned
typedef int32_t signed_mpc_t; // signed
#define __builtin_umul_overflow __builtin_umul_overflow
#define __builtin_uadd_overflow __builtin_uadd_overflow
#elif SML_USE_UINT64
typedef uint64_t mpc_t;
typedef int64_t signed_mpc_t;
#define __builtin_umul_overflow __builtin_umulll_overflow
#define __builtin_uadd_overflow __builtin_uaddll_overflow
#elif SML_USE_INT64
typedef int64_t mpc_t;
typedef int64_t signed_mpc_t;
#define __builtin_umul_overflow __builtin_umulll_overflow
#define __builtin_uadd_overflow __builtin_uaddll_overflow
#else
typedef uint64_t mpc_t;
typedef int64_t signed_mpc_t;
#define __builtin_umul_overflow __builtin_umulll_overflow
#define __builtin_uadd_overflow __builtin_uaddll_overflow
#endif
typedef uint8_t small_mpc_t;
typedef __m128i superLongType;


// openmp for parallel
#if (defined _OPENMP)
#define USE_OPENMP 1
#include "omp.h"
#else
#define USE_OPENMP 0
#endif


// globals for parties
extern int NUM_OF_PARTIES;
#define STANDALONE (NUM_OF_PARTIES == 1)
#define THREE_PC (NUM_OF_PARTIES == 3)
#define FOUR_PC (NUM_OF_PARTIES == 4)
#define PARTY_A 0
#define PARTY_B 1
#define PARTY_C 2
#define PARTY_D 3
#define PARTY_S 4
// global varible parsed from config file to specify which parties
//  to store the plain result with MpcSaveV2:
  // By default, the local ciphertext values are saved in model.
  // Currently, only support it as 3-bit bitmap:[P2 P1 P0]
  //  0: none, all local ciphertext
  //  1: P0,
  //  2: P1,
  //  4: P2,
  //  3: P0 and P1
  //  5: P0 and P2
  //  6: P1 and P2
  //  7: P0, P1 and P2
extern int SAVER_MODE;

#define PRIMARY ((partyNum == PARTY_A) || (partyNum == PARTY_B))
#define	NON_PRIMARY ((partyNum == PARTY_C) || (partyNum == PARTY_D))
#define HELPER (partyNum == PARTY_C)
#define MPC (FOUR_PC || THREE_PC)


// helper macros
#define _aligned_malloc(size, alignment) aligned_alloc(alignment, size)
#define _aligned_free free
#define getrandom(min, max) ((rand() % (int)(((max) + 1) - (min))) + (min))

// split into integer and decimal parts, then add the two parts
#define FloatToMpcType(a) ((mpc_t)( \
    (((signed_mpc_t)(a)) << FLOAT_PRECISION) \
    + (signed_mpc_t)(((a) - (signed_mpc_t)(a)) * (1L << FLOAT_PRECISION))))
#define MpcTypeToFloat(a) ((double((signed_mpc_t)(a))) / (1L << FLOAT_PRECISION))

#define USE_CAST_SCHEMA 0
#define USE_BITCOPY_SCHEMA 0
#if USE_BITCOPY_SCHEMA
union mpc_f {
  double d;
  mpc_t t;
};
static inline mpc_t FloatToMpcTypeBC(double d){
  mpc_f f;
  f.d = d;
  return f.t;
  
  // mpc_t t = 0; 
  // memcpy((char*)&t, (const char*)&d, sizeof(d));
  // return t;
}
static inline double MpcTypeToFloatBC(mpc_t t){
  mpc_f f;
  f.t = t;
  return f.d;

  // double d = 0; 
  // memcpy((char*)&d, (const char*)&t, sizeof(t));
  // return d;
}
#elif USE_CAST_SCHEMA
#define MpcTypeToFloatBC(a) (double((signed_mpc_t)(a)))
#define FloatToMpcTypeBC(a) ((mpc_t)(((signed_mpc_t)(a))))
#else
#define FloatToMpcTypeBC FloatToMpcType
#define MpcTypeToFloatBC MpcTypeToFloat
#endif


/* 
    @brief: Customized for polynomial interpolation coefficients so that 
    we have higher precision (4 more significant decimal points)!
    @Note: the original float number shoud not (usually) be too large.
*/
#define CoffUp(a) ((mpc_t)(signed_mpc_t)(double(a) * (1L << (FLOAT_PRECISION+13))))
#define CoffDown(a) ((double((signed_mpc_t)(a))) / (1L << 13))

#define DELETE_WITH_CHECK(X) if(X) {delete X; X = NULL;} (void)0
#define DELETE_ARRAY_WITH_CHECK(X) if(X) {delete []X; X = NULL;}(void)0


// AES and other globals
#define RANDOM_COMPUTE 1024 //Size of buffer for random elements
#define FIXED_KEY_AES "43739841701238781571456410093f43"
#define STRING_BUFFER_SIZE 256
#define true 1
#define false 0
#define DEBUG_CONST 32
#define DEBUG_INDEX 0
#define DEBUG_PRINT "SIGNED"
#define CPP_ASSEMBLY 1
#define PARALLEL false
#define NO_CORES 4
#define PRIME_NUMBER 127


const int BIT_SIZE = (sizeof(mpc_t) * CHAR_BIT);
const mpc_t LARGEST_NEG = ((mpc_t)1 << (BIT_SIZE - 1));
const mpc_t MINUS_ONE = (mpc_t)-1;
const small_mpc_t BOUNDARY = (256 / PRIME_NUMBER) * PRIME_NUMBER;

const __m128i BIT1 = _mm_setr_epi8(1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
const __m128i BIT2 = _mm_setr_epi8(2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
const __m128i BIT4 = _mm_setr_epi8(4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
const __m128i BIT8 = _mm_setr_epi8(8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
const __m128i BIT16 = _mm_setr_epi8(16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
const __m128i BIT32 = _mm_setr_epi8(32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
const __m128i BIT64 = _mm_setr_epi8(64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
const __m128i BIT128 = _mm_setr_epi8(128, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);


// for debug
#define MPC_DEBUG_USE_FIXED_AESKEY  1
#define MPC_DEBUG                   0

// for basic operations
#define USE_PIECE_WISE_SIGMOID 1

// for mpcop
#define OPEN_MPCOP_DEBUG_AND_CHECK 0

// for use IO with msg_id_t
#define USE_NETIO_WITH_MESSAGEID 1

// some flags
#define MPC_LOG_DEBUG 		0
#define MPC_CHECK_OVERFLOW	0
#define MPC_HAS_OPENMP		USE_OPENMP
#define MPC_USE_INIT_KEYS2	1

// clang-format on
