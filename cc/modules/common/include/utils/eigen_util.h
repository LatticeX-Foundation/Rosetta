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

#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <limits>
#include <random>
#include <regex>
#include <iomanip>
#include <string>
using namespace std;

#ifndef __EIGEN_UTIL_MATRIX__
#define __EIGEN_UTIL_MATRIX__
#include <Eigen/Dense>
namespace Eigen {
typedef Eigen::Matrix<uint64_t, -1, -1> MatrixXul;
typedef Eigen::Matrix<int64_t, -1, -1> MatrixXl;
typedef Eigen::Matrix<uint32_t, -1, -1> MatrixXui;
} // namespace Eigen
using namespace Eigen;
typedef Eigen::MatrixXul MATRIX;
typedef std::vector<MATRIX> MATRIX3D;
#endif

template <typename MAT>
using enable_egine_t = typename std::enable_if<
  std::is_same<typename std::decay<MAT>::type, MatrixXul>::value ||
    std::is_same<typename std::decay<MAT>::type, MatrixXl>::value ||
    std::is_same<typename std::decay<MAT>::type, MatrixXui>::value,
  int>::type;

template <typename MAT, enable_egine_t<MAT> = 0>
inline void print_shape(const MAT& m, std::string tag = "") {
  std::cout << tag << ": shape(" << m.rows() << "," << m.cols() << ")" << std::endl;
}

/**
 * reshape a 2-d martix
 * 
 * from src to dst
 * shape: src.r to shaper, src.c to shapec
 */
template <typename MAT, enable_egine_t<MAT> = 0>
inline void reshape(const MAT& src, MAT& dst, size_t shaper, size_t shapec) {
  int sr = src.rows();
  int sc = src.cols();
  int ss = 0; // ssr * sc + ssc;

  assert((sr * sc) == (shaper * shapec));
  dst.resize(shaper, shapec);
  for (int r = 0; r < shaper; r++) {
    for (int c = 0; c < shapec; c++) {
      int ssr = ss / sc;
      int ssc = ss % sc;
      ss++;
      dst(r, c) = src(ssr, ssc);
    }
  }
}

/**
 * print 2-d martix
 */
template <typename MAT, enable_egine_t<MAT> = 0>
inline void print_mat(const MAT& m, int rows = 5, int cols = 5, size_t width = 4) {
  if (rows < 0)
    rows = m.rows();
  if (cols < 0)
    cols = m.cols();
  for (int r = 0; r < m.rows() && r < rows; r++) {
    cout << "row " << setw(4) << r << ": ";
    for (int c = 0; c < m.cols() && c < cols; c++) {
      cout << setw(width) << m(r, c) << " ";
    }
    cout << endl;
  }
}

/**
 * Normalization & Standardization & Zero-centered
 */
static inline void max_abs_scale(MatrixXd& X, double scalar = 1.0) {
  for (int j = 0; j < X.cols(); j++) {
    MatrixXd feature = X.col(j);
    double dmax = abs(feature.maxCoeff());
    double dmin = abs(feature.minCoeff());
    dmax = (dmax > dmin) ? dmax : dmin;
    if (dmax < 1.0) {
      dmax = 1.0;
      continue;
    }
    feature = feature.unaryExpr([scalar, dmax](double v) { return scalar * v / dmax; });
    X.col(j) = feature;
  }
}

static inline void max_min_scale(MatrixXd& X, double scalar = 1.0) {
  for (int j = 0; j < X.cols(); j++) {
    MatrixXd feature = X.col(j);
    double dmax = feature.maxCoeff();
    double dmin = feature.minCoeff();
    double dmean = feature.mean();
    if (abs(dmax - dmin) < 1.0) {
      dmax = 1.0;
      dmin = 0;
    }
    feature = feature.unaryExpr(
      [scalar, dmax, dmin, dmean](double v) { return scalar * (v - dmin) / (dmax - dmin); });
    X.col(j) = feature;
  }
}

static inline void max_mean_scale(MatrixXd& X, double scalar = 1.0) {
  for (int j = 0; j < X.cols(); j++) {
    MatrixXd feature = X.col(j);
    double dmax = feature.maxCoeff();
    double dmin = feature.minCoeff();
    double dmean = feature.mean();
    if (abs(dmax - dmin) < 1.0) {
      dmax = 1.0;
      dmin = 0;
    }
    feature = feature.unaryExpr(
      [scalar, dmax, dmin, dmean](double v) { return scalar * (v - dmean) / (dmax - dmin); });
    X.col(j) = feature;
  }
}

////////////////////////
template <typename MAT>
static void tofile(const MAT& m, const string& filename) {
  ofstream ofile(filename);
  if (!ofile.good()) {
    cerr << "open " << filename << " failed!" << endl;
    return;
  }
  int prec = std::numeric_limits<double>::digits10;
  for (int r = 0; r < m.rows(); r++) {
    stringstream sss;
    sss.precision(prec);
    for (int c = 0; c < m.cols(); c++) {
      sss << m(r, c) << "\t";
    }
    sss << "\n";
    ofile.write(sss.str().c_str(), sss.str().length());
  }
  ofile.close();
}
