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

#include <vector>
#include <string>
#include <cstdint>

const static int FLOAT_PRECISION_DEFAULT = 16;

#if ROSETTA_MPC_128
#if __SIZEOF_INT128__
typedef unsigned __int128 uint128_t;
typedef __int128 int128_t;
typedef uint128_t mpc_t;
typedef uint8_t small_mpc_t;
typedef int128_t signed_mpc_t;
typedef uint8_t bit_t;
#define GCC_SUPPORT_INT128 1
#else
#error your gcc not support int128 or uint128
#endif //if__SIZEOF_INT128__
#else
typedef uint64_t mpc_t;
typedef uint8_t small_mpc_t;
typedef int64_t signed_mpc_t;
typedef uint8_t bit_t;
#endif //ROSETTA_MPC_128

namespace rosetta {

// Attention! Note that the FLOAT_PRECISION_M will be initialized AFTER
//      initializing protocol, so DO NOT use it or its related functions before
//      initializing protocol with config!
// #define FloatToMpcType(a)                        
//   ((mpc_t)(                                      
//     (((signed_mpc_t)(a)) << FLOAT_PRECISION_M) + 
//     (signed_mpc_t)(((a) - (signed_mpc_t)(a)) * (1L << FLOAT_PRECISION_M))))

template<typename T>
mpc_t FloatToMpcType(T a, int precision) {
  return ((mpc_t)(
    (((signed_mpc_t)(a)) << precision) + 
    (signed_mpc_t)(((a) - (signed_mpc_t)(a)) * (1L << precision))));
}
extern template mpc_t FloatToMpcType<double>(double, int);
extern template mpc_t FloatToMpcType<int>(int, int);
extern template mpc_t FloatToMpcType<mpc_t>(mpc_t, int);
extern template mpc_t FloatToMpcType<small_mpc_t>(small_mpc_t, int);
  
// #define MpcTypeToFloat(a) ((double((signed_mpc_t)(a))) / (1L << FLOAT_PRECISION_M))
template<typename T>
double MpcTypeToFloat(T a, int precision) {
  return ((double((signed_mpc_t)(a))) / (1L << precision));
}
extern template double MpcTypeToFloat<mpc_t>(mpc_t, int);

// TODO(george): to re-implement this
/* 
    @brief: Customized for polynomial interpolation coefficients so that 
    we have higher precision (more significant decimal points)!
    @Note: the original float number should not (usually) be too large.
*/
// #define CoffUp(a)
//  ((mpc_t)(signed_mpc_t)(double(a) * (1L << (FLOAT_PRECISION_M + FLOAT_PRECISION_M))))
template<typename T>
mpc_t CoffUp(T a, int precision) {
  return ((mpc_t)(
    (((signed_mpc_t)(a)) << precision) + 
    (signed_mpc_t)(((a) - (signed_mpc_t)(a)) * (1L << precision))));
}
extern template mpc_t CoffUp<double>(double, int);
// only used in protocol SecureNN currenlty. please use 'trunc' (and Scale) in protocol Helix.
// #define CoffDown(a) ((double((signed_mpc_t)(a))) / (1L << FLOAT_PRECISION_M))
#define CoffDown(a) a
///////**************************some internal functionalities ********************************
/*
	****Polynomials******************
*/
/*
	for example:
		y = 1 + 2 * X + 5 * X^ 3 , \in (0, 4)
		get_power_list --> [0, 1, 3]
		get_coff_list --> [1, 2, 5]
		get_start --> 0
		get_end --> 4
*/

using std::vector;
using std::string;

class ConstPolynomial {
 public:
  //ConstPolynomial(): __start(0), __end(0) {}
  explicit ConstPolynomial(
    double init_start,
    double init_end,
    const vector<vector<double>>& init_poly);

  bool get_power_list(vector<mpc_t>& out_vec);
  bool get_coff_list(vector<mpc_t>& out_vec, int precision);

  mpc_t get_start(int precision) { return FloatToMpcType(__start, precision); };
  mpc_t get_end(int precision) { return FloatToMpcType(__end, precision); };
  string to_string();

 private:
  // internal presentation for initialization
  vector<vector<double>> __inner_poly;
  // Note: if __end == __start, this function is for all X, [-\inf, +\inf].
  double __start = 0.0; // >=
  double __end = 0.0; // <
};

/*
	@brief: function approximation registering entry for mapping from func_name to its
	segmental polynomials.
	// TODO: make this as singleton
*/
struct PolyConfFactory {
  // eg : "log_v1" --> A1
  // 		"log_v2" --> [B1, B2, B3]
  // TODO: add mutex_lock or RW lock
 public:
  static void func_register(
    const string& func_name,
    vector<ConstPolynomial>* approx_polys);

  static bool get_func_polys(
    const string& func_name,
    vector<ConstPolynomial>** approx_polys);
  //private:
  //	static unordered_map<string, vector<ConstPolynomial>> FUNC_POLY_MAP;
};

/**
 * 
 */
void EigenMatMul(
  const vector<mpc_t>& a,
  const vector<mpc_t>& b,
  vector<mpc_t>& c,
  size_t rows,
  size_t common_dim,
  size_t columns,
  bool transpose_a,
  bool transpose_b);

} // namespace rosetta
