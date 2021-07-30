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

extern small_mpc_t kAdditionModPrime[PRIME_NUMBER][PRIME_NUMBER];
extern small_mpc_t kMultiplicationModPrime[PRIME_NUMBER][PRIME_NUMBER];

// clang-format off
// void matrixMultEigenx(const vector<mpc_t>& a,const vector<mpc_t>& b,vector<mpc_t>& c,
// 	size_t rows,size_t common_dim,size_t columns,size_t transpose_a,size_t transpose_b);

mpc_t divideMyTypeSA(mpc_t a, mpc_t b);
mpc_t dividePlainSA(mpc_t a, int b);
void dividePlainSA(vector<mpc_t>& vec, int divisor);
mpc_t multiplyMyTypesSA(mpc_t a, mpc_t b, int shift);
size_t partner(size_t party);
size_t adversary(size_t party);

inline small_mpc_t addModPrime(small_mpc_t a, small_mpc_t b) {return kAdditionModPrime[a][b];}
inline small_mpc_t multiplyModPrime(small_mpc_t a, small_mpc_t b) {return kMultiplicationModPrime[a][b];}
small_mpc_t subtractModPrime(small_mpc_t a, small_mpc_t b);
inline small_mpc_t wrapAround(mpc_t a, mpc_t b) {return (a > MINUS_ONE - b);}
void wrapAround(const vector<mpc_t>& a,const vector<mpc_t>& b,vector<small_mpc_t>& c,size_t size);
void populateBitsVector(vector<small_mpc_t>& vec, string r_type, size_t size);

void gen_side_shareOfBits(vector<small_mpc_t>& bit_shares, size_t size, string r_type);
void sharesOfBits(vector<small_mpc_t>& bit_shares_x_1,vector<small_mpc_t>& bit_shares_x_2,const vector<mpc_t>& x,
	size_t size,string r_type);
void sharesOfLSB(vector<small_mpc_t>& share_1,vector<small_mpc_t>& share_2,const vector<mpc_t>& r,
	size_t size,string r_type);
void sharesOfLSB(vector<mpc_t>& share_1,vector<mpc_t>& share_2,const vector<mpc_t>& r,
	size_t size,string r_type);
void sharesOfBitVector(vector<small_mpc_t>& share_1,vector<small_mpc_t>& share_2,const vector<small_mpc_t>& vec,
	size_t size,string r_type);
void sharesOfBitVector(vector<mpc_t>& share_1,vector<mpc_t>& share_2,const vector<small_mpc_t>& vec,
	size_t size,string r_type);

void gen_side_shareOfBitVector(vector<mpc_t>& share_side, size_t size, string r_type);
void splitIntoShares(const vector<mpc_t>& a, vector<mpc_t>& a1, vector<mpc_t>& a2, size_t size);
void XORVectors(const vector<small_mpc_t>& a,const vector<small_mpc_t>& b,vector<small_mpc_t>& c,size_t size);
mpc_t multiplyMyTypes(mpc_t a, mpc_t b, size_t shift);
void log_print(string str);
void error(string str);
void notYet();

void convolutionReshape(const vector<mpc_t>& vec,vector<mpc_t>& vecShaped,
	size_t ih,size_t iw,size_t C,size_t B,size_t fh,size_t fw,size_t sy,size_t sx);
void maxPoolReshape(const vector<mpc_t>& vec,vector<mpc_t>& vecShaped,
	size_t ih,size_t iw,size_t D,size_t B,size_t fh,size_t fw,size_t sy,size_t sx);
void convolutionReshapeBackprop(const vector<mpc_t>& vec,vector<mpc_t>& vecOut,
	size_t imageH,size_t imageW,size_t filterH,size_t filterW,size_t strideY,size_t strideX,
	size_t C,size_t D,size_t B);

void start_m();
void end_m(string str);

void sigmoidSA(const vector<mpc_t>& input, vector<mpc_t>& output, size_t rows, size_t cols);

void compress_C2(const vector<mpc_t>& src_C2, vector<small_mpc_t>& dest_C2);
void restore_C2(const vector<small_mpc_t>& src_C2, vector<mpc_t>& dest_C2);
// clang-format on

// clang-format off
// Template functions
template <typename T>
void populateRandomVector(vector<T>& vec, size_t size, string r_type, string neg_type);

template <typename T>
void addVectors(const vector<T>& a, const vector<T>& b, vector<T>& c, size_t size);

template <typename T>
void subtractVectors(const vector<T>& a, const vector<T>& b, vector<T>& c, size_t size);

template <typename T>
void copyVectors(const vector<T>& a, vector<T>& b, size_t size);

template <typename T1, typename T2>
void addModuloOdd(const vector<T1>& a, const vector<T2>& b, vector<mpc_t>& c, size_t size);

template <typename T1, typename T2>
void subtractModuloOdd(const vector<T1>& a, const vector<T2>& b, vector<mpc_t>& c, size_t size);

template <typename T1, typename T2>
mpc_t addModuloOdd(T1 a, T2 b);

template <typename T1, typename T2>
mpc_t subtractModuloOdd(T1 a, T2 b);

template <typename T>
void sharesModuloOdd(vector<mpc_t>& shares_1, vector<mpc_t>& shares_2, const vector<T>& x,
    size_t size, string r_type);

// Randmoization is passed as an argument here.
template <typename T>
void getVectorfromPrimary(vector<T>& vec, size_t size, string r_mode, string n_mode);
// clang-format on
#include "cc/modules/protocol/mpc/snn/include/snn_tools.hpp"
