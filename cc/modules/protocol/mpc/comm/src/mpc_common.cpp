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
#include "cc/modules/protocol/mpc/comm/include/mpc_common.h"

#include <vector>
#include <unordered_map>
#include <iostream>
#include <sstream>

#include <Eigen/Dense> // for matmul
using namespace Eigen;

using namespace std;
using namespace rosetta;

namespace rosetta {
template mpc_t FloatToMpcType<double>(double, int);
template mpc_t FloatToMpcType<int>(int, int);
template mpc_t FloatToMpcType<mpc_t>(mpc_t, int);
template mpc_t FloatToMpcType<small_mpc_t>(small_mpc_t, int);

template double MpcTypeToFloat<mpc_t>(mpc_t, int);

template mpc_t CoffUp<double>(double, int);

ConstPolynomial::ConstPolynomial(
  double init_start,
  double init_end,
  const std::vector<std::vector<double>>& init_poly) {
  __start = init_start;
  __end = init_end;
  for (auto i = 0; i < init_poly.size(); ++i) {
    __inner_poly.push_back(init_poly[i]);
  }
}

bool ConstPolynomial::get_power_list(vector<mpc_t>& out_vec) {
  out_vec.clear();
  for (auto i = 0; i < __inner_poly.size(); ++i) {
    // just converting double to int
    out_vec.push_back(mpc_t(__inner_poly[i][0]));
  }
  return true;
}

bool ConstPolynomial::get_coff_list(vector<mpc_t>& out_vec, int precision) {
  out_vec.clear();
  for (auto i = 0; i < __inner_poly.size(); ++i) {
    // using CoffUp for more precision, remember to CoffDown!
    out_vec.push_back(CoffUp(__inner_poly[i][1], precision));
  }
  return true;
}

string ConstPolynomial::to_string() {
  stringstream ss;
  ss << "[" << __start << "," << __end << "]: ";
  for (auto i = 0; i < __inner_poly.size(); ++i) {
    ss << "+(" << __inner_poly[i][1] << "*x^" << __inner_poly[i][0] << ") ";
  }
  ss << endl;
  return ss.str();
}

typedef unordered_map<std::string, vector<ConstPolynomial>*> FuncPolyFactories;
FuncPolyFactories* func_polynomials_factories() {
  // this is thread-safe after C++11
  static FuncPolyFactories* factory = new FuncPolyFactories;
  return factory;
}

void PolyConfFactory::func_register(
  const std::string& func_name,
  vector<ConstPolynomial>* approx_polys) {
  if (func_polynomials_factories()->find(func_name) != func_polynomials_factories()->end()) {
    (*func_polynomials_factories())[func_name] = approx_polys;
  } else {
    func_polynomials_factories()->insert({func_name, approx_polys});
  }
}

bool PolyConfFactory::get_func_polys(
  const std::string& func_name,
  vector<ConstPolynomial>** approx_polys) {
  FuncPolyFactories* curr_fac = func_polynomials_factories();
  if (curr_fac->find(func_name) != curr_fac->end()) {
    *approx_polys = curr_fac->at(func_name);
    return true;
  } else {
    cout << "ERROR！ can not find" << func_name << endl;
    return false;
  }
}

struct LogFuncRegistrar {
  vector<ConstPolynomial>* log_default_vec = nullptr;
  vector<ConstPolynomial>* log_v1_vec = nullptr;
  vector<ConstPolynomial>* log_v2_vec = nullptr;

  LogFuncRegistrar() {
    // Note that we use the same piecewise polynomials to fit Log function of both secureNN and Helix protocol.
    ///////*****************LOG INTPRETATION ****************
    /// OPTION for general High Resolution%
    /// this is for [0.5, 1]
    const std::vector<std::vector<double>> FUNC_APPROX_LOG_OPTION_HD = {
      {0, -2.6145621}, {1, 6.92508636},  {2, -9.48726206},
      {3, 8.57287151}, {4, -4.31242379}, {5, 0.91630145}};

    /// OPTION A (not used any more): single polynomial
    /// This approximation is best for x \in [0.3, 1.8)
    const std::vector<std::vector<double>> FUNC_APPROX_LOG_OPTION_A = {
      {0, -3.35674972}, {1, 12.79333646},  {2, -26.18955259},
      {3, 30.24596692}, {4, -17.30367641}, {5, 3.82474222}};

    const std::vector<std::vector<double>> FUNC_APPROX_LOG_OPTION_B_SEGMENTS = {{}};
    /// OPTION B: 3-segment polynomial
    // This approximation is best for x in [1.2, 10]
    const std::vector<std::vector<double>> FUNC_APPROX_LOG_OPTION_B_3 = {
      {0, -0.147409486}, {1, 0.463403306}, {2, -0.022636005}};

    // This approximation is best for x in [0.05, 1.2]
    const std::vector<std::vector<double>> FUNC_APPROX_LOG_OPTION_B_2 = {
      {0, -3.0312942}, {1, 8.253302766}, {2, -8.668159044}, {3, 3.404663323}};

    // This approximation is best for x in [0.0001, 0.05]
    const std::vector<std::vector<double>> FUNC_APPROX_LOG_OPTION_B_1 = {
      {0, -6.805568387}, {1, 284.0022382}, {2, -8360.491679}, {3, 85873.96716}};

    ConstPolynomial log_default_appro_poly = ConstPolynomial(0, 0, FUNC_APPROX_LOG_OPTION_HD);
    log_default_vec = new vector<ConstPolynomial>({log_default_appro_poly});
    PolyConfFactory::func_register(string("LOG_HD"), log_default_vec);

    ConstPolynomial log_v1_appro_poly = ConstPolynomial(0, 0, FUNC_APPROX_LOG_OPTION_A);
    vector<ConstPolynomial>* log_v1_vec = new vector<ConstPolynomial>({log_v1_appro_poly});
    PolyConfFactory::func_register(string("LOG_V1"), log_v1_vec);
    // Note: Attention! The result may become worse when X < 0.0001.
    //		In Machine Learning, It will be better to clip it.
    ConstPolynomial log_v2_appro_poly_1 = ConstPolynomial(0.0001, 0.05, FUNC_APPROX_LOG_OPTION_B_1);
    ConstPolynomial log_v2_appro_poly_2 = ConstPolynomial(0.05, 1.2, FUNC_APPROX_LOG_OPTION_B_2);
    // Note: Attention! The result may become worse when X > 10.
    //		It will be better than ZERO. So we set it to be a VERY BIG NUMBER!
    ConstPolynomial log_v2_appro_poly_3 = ConstPolynomial(
      1.2,
      10000, // 2^40
      FUNC_APPROX_LOG_OPTION_B_3);
    log_v2_vec =
      new vector<ConstPolynomial>({log_v2_appro_poly_1, log_v2_appro_poly_2, log_v2_appro_poly_3});
    PolyConfFactory::func_register(string("LOG_V2"), log_v2_vec);
    // cout << "DEBUG" << "LOG_V2 registered!" << endl;
  }
  ~LogFuncRegistrar() {
    delete log_default_vec;
    log_default_vec = nullptr;
    delete log_v1_vec;
    log_v1_vec = nullptr;
    delete log_v2_vec;
    log_v2_vec = nullptr;
  }
};

static LogFuncRegistrar log_func_registrar;

struct CELogFuncRegistrar {
  vector<ConstPolynomial>* ce_log_vec = nullptr;
  CELogFuncRegistrar() {
    /*
    		This approximation is for approximating CELog with x in [0.0, 7.1].
    		The Polynomial is 0.69056255 - 0.49805477*X + 0.13973045*X^2 - 0.01761977*X^3 - 0.00083065*X^4
    		if  x > 7.1 we set its value to Fixed 0.0003.
		*/
    const std::vector<std::vector<double>> MPC_CE_LOG_OPTION = {
      {0, 0.69056225}, {1, -0.49805477}, {2, 0.13973045}, {3, -0.01761977}, {4, 0.00083065}};
    ConstPolynomial ce_log_appro_poly = ConstPolynomial(0, 7.1, MPC_CE_LOG_OPTION);
    ce_log_vec = new vector<ConstPolynomial>({ce_log_appro_poly});
    PolyConfFactory::func_register(string("LOG_CE"), ce_log_vec);
  }
  ~CELogFuncRegistrar() {
    delete ce_log_vec;
    ce_log_vec = nullptr;
  }
};

static CELogFuncRegistrar ce_log_func_registrar;

// helpers
// ref snn
void EigenMatMul(
  const vector<mpc_t>& a,
  const vector<mpc_t>& b,
  vector<mpc_t>& c,
  size_t rows,
  size_t common_dim,
  size_t columns,
  bool transpose_a,
  bool transpose_b) {
  assert(rows * common_dim == a.size() && "a vector sizes is incorrect!!!");
  assert(common_dim * columns == b.size() && "b vector sizes is incorrect!!!");
  assert(rows * columns == c.size() && "c vector sizes is incorrect!!!");

  size_t a_rows, a_cols;
  size_t b_rows, b_cols;
  a_rows = transpose_a ? common_dim : rows;
  a_cols = transpose_a ? rows : common_dim;
  b_rows = transpose_b ? columns : common_dim;
  b_cols = transpose_b ? common_dim : columns;
  Matrix<mpc_t, Dynamic, Dynamic, RowMajor> eigen_a(a_rows, a_cols);
  Matrix<mpc_t, Dynamic, Dynamic, RowMajor> eigen_b(b_rows, b_cols);
  Matrix<mpc_t, Dynamic, Dynamic, RowMajor> eigen_c(rows, columns);

  assert(a.size() == rows * common_dim);
  assert(b.size() == common_dim * columns);
  assert(c.size() == rows * columns);

  assert(eigen_a.size() == rows * common_dim);
  assert(eigen_b.size() == common_dim * columns);
  assert(eigen_c.size() == rows * columns);

  // memcpy((void *)eigen_a.data(), a.data(), a.size() * sizeof(mpc_t));
  for (int i = 0; i < a_rows; i++)
    for (int j = 0; j < a_cols; j++)
      eigen_a(i, j) = a[i * a_cols + j];
  if (transpose_a)
    eigen_a.transposeInPlace();

  // memcpy((void *)eigen_b.data(), b.data(), b	.size() * sizeof(mpc_t));
  for (int i = 0; i < b_rows; i++)
    for (int j = 0; j < b_cols; j++)
      eigen_b(i, j) = b[i * b_cols + j];
  if (transpose_b)
    eigen_b.transposeInPlace();

#if MPC_CHECK_OVERFLOW
  checkOverflow(a, b, rows, common_dim, columns, transpose_a, transpose_b);
#endif
  eigen_c = eigen_a * eigen_b;

  // memcpy(c.data(), eigen_c.data(), eigen_c.size() * sizeof(mpc_t));
  for (int i = 0; i < rows; i++)
    for (int j = 0; j < columns; j++)
      c[i * columns + j] = eigen_c(i, j);
}

//! @todo optimized
void EigenMatMul2(
  const vector<mpc_t>& a,
  const vector<mpc_t>& b,
  vector<mpc_t>& c,
  size_t rows,
  size_t common_dim,
  size_t columns,
  bool transpose_a,
  bool transpose_b) {
  assert(rows * common_dim == a.size() && "a vector sizes is incorrect!!!");
  assert(common_dim * columns == b.size() && "b vector sizes is incorrect!!!");
  assert(rows * columns == c.size() && "c vector sizes is incorrect!!!");

  size_t a_rows, a_cols;
  size_t b_rows, b_cols;
  a_rows = transpose_a ? common_dim : rows;
  a_cols = transpose_a ? rows : common_dim;
  b_rows = transpose_b ? columns : common_dim;
  b_cols = transpose_b ? common_dim : columns;

  Matrix<mpc_t, Dynamic, Dynamic, RowMajor> eigen_a(a_rows, a_cols);
  Matrix<mpc_t, Dynamic, Dynamic, RowMajor> eigen_b(b_rows, b_cols);
  Matrix<mpc_t, Dynamic, Dynamic, RowMajor> eigen_c(rows, columns);

  assert(a.size() == rows * common_dim);
  assert(b.size() == common_dim * columns);
  assert(c.size() == rows * columns);

  assert(eigen_a.size() == rows * common_dim);
  assert(eigen_b.size() == common_dim * columns);
  assert(eigen_c.size() == rows * columns);

#if 1
  for (int i = 0; i < a_rows; i++)
    for (int j = 0; j < a_cols; j++)
      eigen_a(i, j) = a[i * a_cols + j];
  if (transpose_a)
    eigen_a.transposeInPlace();

  for (int i = 0; i < b_rows; i++)
    for (int j = 0; j < b_cols; j++)
      eigen_b(i, j) = b[i * b_cols + j];
  if (transpose_b)
    eigen_b.transposeInPlace();

  eigen_c = eigen_a * eigen_b;

  for (int i = 0; i < rows; i++)
    for (int j = 0; j < columns; j++)
      c[i * columns + j] = eigen_c(i, j);
#else
  memcpy((void*)eigen_a.data(), a.data(), a.size() * sizeof(mpc_t));
  if (transpose_a)
    eigen_a.transposeInPlace();

  memcpy((void*)eigen_b.data(), b.data(), b.size() * sizeof(mpc_t));
  if (transpose_b)
    eigen_b.transposeInPlace();

  eigen_c = eigen_a * eigen_b;

  memcpy(c.data(), eigen_c.data(), eigen_c.size() * sizeof(mpc_t));
#endif
}

} // namespace rosetta
