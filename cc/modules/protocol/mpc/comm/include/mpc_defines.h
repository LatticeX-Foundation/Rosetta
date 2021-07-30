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

// NOTE THAT THIS FILE IS USED FOR PROTOCOL SNN NOW.
// WE WILL RENAME THIS AS SOON AS POSSIBLE.

#include <emmintrin.h>
#include <cassert>
#include <climits>
#include <vector>
#include <string>
using namespace std;

// clang-format off

#include "cc/modules/protocol/mpc/comm/include/mpc_common.h" 
using namespace rosetta;

typedef __m128i superLongType;

#define __builtin_umul_overflow __builtin_umulll_overflow
#define __builtin_uadd_overflow __builtin_uaddll_overflow

// openmp for parallel
#if (defined _OPENMP)
#define USE_OPENMP 1
#include "omp.h"
#else
#define USE_OPENMP 0
#endif

// 3PC helper macro for roles
#define STANDALONE (num_of_parties == 1)
#define THREE_PC (num_of_parties == 3)
#define FOUR_PC (num_of_parties == 4)
#define PARTY_A 0
#define PARTY_B 1
#define PARTY_C 2
#define PARTY_D 3
#define PARTY_S 4


#define PRIMARY ((partyNum == PARTY_A) || (partyNum == PARTY_B))
#define	NON_PRIMARY ((partyNum == PARTY_C) || (partyNum == PARTY_D))
#define HELPER (partyNum == PARTY_C)
#define MPC (FOUR_PC || THREE_PC)


static inline bool is_primary(int role_id) {
  return role_id == PARTY_A && role_id == PARTY_B;
}

static inline bool is_party_a(int role_id) {
  return role_id == PARTY_A;
}

static inline bool is_party_b(int role_id) {
  return role_id == PARTY_B;
}

static inline bool is_party_c(int role_id) {
  return role_id == PARTY_C;
}

static inline bool is_helper(int role_id) {
  return role_id == PARTY_C;
}

// helper macros
#define _aligned_malloc(size, alignment) aligned_alloc(alignment, size)
#define _aligned_free free
#define getrandom(min, max) ((rand() % (int)(((max) + 1) - (min))) + (min))

#define DELETE_WITH_CHECK(X) if(X) {delete X; X = NULL;} (void)0
#define DELETE_ARRAY_WITH_CHECK(X) if(X) {delete []X; X = NULL;}(void)0


// AES and other globals
#define RANDOM_COMPUTE 1024 //Size of buffer for random elements
#define FIXED_KEY_AES "43739841701238781571456410093f43"
#define STRING_BUFFER_SIZE 256
#define DEBUG_CONST 32
#define DEBUG_INDEX 0
#define DEBUG_PRINT "SIGNED"
#define CPP_ASSEMBLY 1
#define PARALLEL false
#define NO_CORES 4

#if ROSETTA_MPC_128
//avoid overflow error in PrivateCompare when prime is not large enough
#define PRIME_NUMBER 251
#else
#define PRIME_NUMBER 127
#endif


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
#define MPC_DEBUG_USE_FIXED_AESKEY  0
#define MPC_DEBUG                   0

// for basic operations
#define USE_PIECE_WISE_SIGMOID 1

// for mpcop
#define OPEN_MPCOP_DEBUG_AND_CHECK 0

// for use IO with msg_id_t
#define USE_NETIO_WITH_MESSAGEID 1
 
/*
// If there are some large values during its computation process,
// the probability of the error in MPC arithematic truncation will NOT be negligible any more.
// [Refer to Theorem 1 in SecureML paper for details]
// So we may need to fix it in production application.
// But, in the same time, this fixup will bring extra communication complexity [One round added].
// 
// Therefore we provide the solution to this problem as an option, 
// and by default we will not use this fixup. 
// If you really need this in your specific application, you can set this macro to '1'. 
*/
#define FIX_SHARE_TRUNCATION_PROBABILISTIC_ERROR 0

// some flags
#define MPC_LOG_DEBUG 		  0
#define MPC_CHECK_OVERFLOW	0
#define MPC_HAS_OPENMP		USE_OPENMP
#define MPC_USE_INIT_KEYS2	1

//for reciprocal division method
#define ITERATION_TIME     10

// clang-format on
