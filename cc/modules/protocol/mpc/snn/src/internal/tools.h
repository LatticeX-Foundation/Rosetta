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

#include <cmath>
#include <ctime>
#include <cstdio>
#include <wmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <openssl/sha.h>
#include "cc/modules/protocol/mpc/snn/src/internal/Config.h"
#include "cc/modules/protocol/mpc/snn/src/internal/TedKrovetzAesNiWrapperC.h"
#include "cc/modules/protocol/mpc/snn/src/internal/secCompMultiParty.h"
#include "cc/modules/protocol/mpc/snn/src/internal/main_gf_funcs.h"
#include "cc/modules/protocol/mpc/snn/src/internal/AESObject.h"
#include "cc/modules/protocol/mpc/snn/src/internal/ParallelAESObject.h"
#include "cc/modules/protocol/mpc/snn/src/internal/snn_helper.h"


extern small_mpc_t kAdditionModPrime[PRIME_NUMBER][PRIME_NUMBER];
extern small_mpc_t kMultiplicationModPrime[PRIME_NUMBER][PRIME_NUMBER];

#if MULTIPLICATION_TYPE == 0
#define MUL(x, y) gfmul(x, y)
#define MULT(x, y, ans) gfmul(x, y, &ans)

#ifdef OPTIMIZED_MULTIPLICATION
#define MULTHZ(x, y, ans) \
  gfmulHalfZeros(x, y, &ans) // optimized multiplication when half of y is zeros
#define MULHZ(x, y) gfmulHalfZeros(x, y) // optimized multiplication when half of y is zeros
#else
#define MULTHZ(x, y, ans) gfmul(x, y, &ans)
#define MULHZ(x, y) gfmul(x, y)
#endif
#define SET_ONE _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1)

#else
#define MUL(x, y) gfmul3(x, y)
#define MULT(x, y, ans) gfmul3(x, y, &ans)
#ifdef OPTIMIZED_MULTIPLICATION
#define MULTHZ(x, y, ans) \
  gfmul3HalfZeros(x, y, &ans) // optimized multiplication when half of y is zeros
#define MULHZ(x, y) gfmul3HalfZeros(x, y) // optimized multiplication when half of y is zeros
#else
#define MULTHZ(x, y, ans) gfmul3(x, y, &ans)
#define MULHZ(x, y) gfmul3(x, y)
#endif
#define SET_ONE _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1)
#endif

//
// field zero
#define SET_ZERO _mm_setzero_si128()
// the partynumber(+1) embedded in the field
#define SETX(j) _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, j + 1) // j+1
// Test if 2 __m128i variables are equal
#define EQ(x, y) _mm_test_all_ones(_mm_cmpeq_epi8(x, y))
// Add 2 field elements in GF(2^128)
#define ADD(x, y) _mm_xor_si128(x, y)
// Subtraction and addition are equivalent in characteristic 2 fields
#define SUB ADD
// Evaluate x^n in GF(2^128)
#define POW(x, n) fastgfpow(x, n)
// Evaluate x^2 in GF(2^128)
#define SQ(x) square(x)
// Evaluate x^(-1) in GF(2^128)
#define INV(x) inverse(x)
// Evaluate P(x), where p is a given polynomial, in GF(2^128)
#define EVAL(x, poly, ans) fastPolynomEval(x, poly, &ans) // polynomEval(SETX(x),y,z)
// Reconstruct the secret from the shares
#define RECON(shares, deg, secret) reconstruct(shares, deg, &secret)
// returns a (pseudo)random __m128i number using AES-NI
#define RAND LoadSeedNew
// returns a (pseudo)random bit using AES-NI
#define RAND_BIT LoadBool

// the encryption scheme
#define PSEUDO_RANDOM_FUNCTION(seed1, seed2, index, numberOfBlocks, result) \
  pseudoRandomFunctionwPipelining(seed1, seed2, index, numberOfBlocks, result);

// The degree of the secret-sharings before multiplications
extern int degSmall;
// The degree of the secret-sharing after multiplications (i.e., the degree of the secret-sharings
// of the PRFs)
extern int degBig;
// The type of honest majority we assume
extern int majType;

// bases for interpolation
extern __m128i* baseReduc;
extern __m128i* baseRecon;
// saved powers for evaluating polynomials
extern __m128i* powers;

// one in the field
extern const __m128i ONE;
// zero in the field
extern const __m128i ZERO;

typedef struct polynomial {
  int deg;
  __m128i* coefficients;
} Polynomial;

void gfmul(__m128i a, __m128i b, __m128i* res);
// This function works correctly only if all the upper half of b is zeros
void gfmulHalfZeros(__m128i a, __m128i b, __m128i* res);
// multiplies a and b
__m128i gfmul(__m128i a, __m128i b);
// This function works correctly only if all the upper half of b is zeros
__m128i gfmulHalfZeros(__m128i a, __m128i b);
__m128i gfpow(__m128i x, int deg);
__m128i fastgfpow(__m128i x, int deg);
__m128i square(__m128i x);
__m128i inverse(__m128i x);
string _sha256hash_(char* input, int length);
string sha256hash(char* input, int length);
void printError(string error);
string __m128i_toHex(__m128i var);
string __m128i_toString(__m128i var);
__m128i stringTo__m128i(string str);

unsigned int charValue(char c);
double diff(timespec start, timespec end);
string convertBooltoChars(bool* input, int length);
string toHex(string s);
string convertCharsToString(char* input, int size);

void print(__m128i* arr, int size);
void print128_num(__m128i var);
void print_usage(const char* bin);
void start_time();
void end_time(string str);
void start_rounds();
void end_rounds(string str);

void print_myType(mpc_t var, string message, string type);
void print_linear(mpc_t var, string type);

#include "cc/modules/protocol/mpc/snn/include/snn_tools.h"
