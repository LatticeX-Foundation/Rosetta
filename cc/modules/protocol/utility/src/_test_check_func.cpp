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
#include "cc/modules/protocol/utility/include/_test_check_func.h"

/**
 * 
 * 
 */
void add_check_func(const vector<double>& X, const vector<double>& Y, const vector<double>& Z) {
  int size = X.size();
  for (int i = 0; i < size; i++) {
    if (f_equal(X[i] + Y[i], Z[i])) {
      continue;
    }
    cout << "In " << __FUNCTION__ << " i:" << i << " x:" << X[i] << " y:" << Y[i] << " z:" << Z[i]
         << endl;
  }
}

void sub_check_func(const vector<double>& X, const vector<double>& Y, const vector<double>& Z) {
  int size = X.size();
  for (int i = 0; i < size; i++) {
    if (f_equal(X[i] - Y[i], Z[i])) {
      continue;
    }
    cout << "In " << __FUNCTION__ << " i:" << i << " x:" << X[i] << " y:" << Y[i] << " z:" << Z[i]
         << endl;
  }
}

void mul_check_func(const vector<double>& X, const vector<double>& Y, const vector<double>& Z) {
  int size = X.size();
  for (int i = 0; i < size; i++) {
    if (f_equal(X[i] * Y[i], Z[i])) {
      continue;
    }
    cout << "In " << __FUNCTION__ << " i:" << i << " x:" << X[i] << " y:" << Y[i] << " z:" << Z[i]
         << endl;
  }
}

void floordiv_check_func(
  const vector<double>& X,
  const vector<double>& Y,
  const vector<double>& Z) {
  int size = X.size();
  for (int i = 0; i < size; i++) {
    if (f_equal(floor(X[i] / Y[i]), Z[i])) {
      continue;
    }
    cout << "In " << __FUNCTION__ << " i:" << i << " x:" << X[i] << " y:" << Y[i] << " z:" << Z[i]
         << endl;
  }
}

void truediv_check_func(const vector<double>& X, const vector<double>& Y, const vector<double>& Z) {
  int size = X.size();
  for (int i = 0; i < size; i++) {
    if (f_equal(X[i] / Y[i], Z[i])) {
      continue;
    }
    cout << "In " << __FUNCTION__ << " i:" << i << " x:" << X[i] << " y:" << Y[i] << " z:" << Z[i]
         << endl;
  }
}

void equal_check_func(const vector<double>& X, const vector<double>& Y, const vector<double>& Z) {
  int size = X.size();
  for (int i = 0; i < size; i++) {
    // x == y && z == 1
    if (f_equal(X[i], Y[i]) && f_equal(Z[i], 1.0)) {
      continue;
    }
    if (f_notequal(X[i], Y[i]) && f_equal(Z[i], 0)) {
      continue;
    }
    cout << "In " << __FUNCTION__ << " i:" << i << " x:" << X[i] << " y:" << Y[i] << " z:" << Z[i]
         << endl;
  }
}

void notequal_check_func(
  const vector<double>& X,
  const vector<double>& Y,
  const vector<double>& Z) {
  int size = X.size();
  for (int i = 0; i < size; i++) {
    // x != y && z == 1
    if (f_notequal(X[i], Y[i]) && f_equal(Z[i], 1.0)) {
      continue;
    }
    if (f_equal(X[i], Y[i]) && f_equal(Z[i], 0)) {
      continue;
    }
    cout << "In " << __FUNCTION__ << " i:" << i << " x:" << X[i] << " y:" << Y[i] << " z:" << Z[i]
         << endl;
  }
}

void less_check_func(const vector<double>& X, const vector<double>& Y, const vector<double>& Z) {
  int size = X.size();
  for (int i = 0; i < size; i++) {
    // x < y && z == 1
    if (f_less(X[i], Y[i]) && f_equal(Z[i], 1.0)) {
      continue;
    }
    if (f_greaterequal(X[i], Y[i]) && f_equal(Z[i], 0)) {
      continue;
    }
    cout << "In " << __FUNCTION__ << " i:" << i << " x:" << X[i] << " y:" << Y[i] << " z:" << Z[i]
         << endl;
  }
}

void lessequal_check_func(
  const vector<double>& X,
  const vector<double>& Y,
  const vector<double>& Z) {
  int size = X.size();
  for (int i = 0; i < size; i++) {
    // x <= y && z == 1
    if (f_lessequal(X[i], Y[i]) && f_equal(Z[i], 1.0)) {
      continue;
    }
    if (f_greater(X[i], Y[i]) && f_equal(Z[i], 0)) {
      continue;
    }
    cout << "In " << __FUNCTION__ << " i:" << i << " x:" << X[i] << " y:" << Y[i] << " z:" << Z[i]
         << endl;
  }
}

void greater_check_func(const vector<double>& X, const vector<double>& Y, const vector<double>& Z) {
  int size = X.size();
  for (int i = 0; i < size; i++) {
    // x > y && z == 1
    if (f_greater(X[i], Y[i]) && f_equal(Z[i], 1.0)) {
      continue;
    }
    if (f_lessequal(X[i], Y[i]) && f_equal(Z[i], 0)) {
      continue;
    }
    cout << "In " << __FUNCTION__ << " i:" << i << " x:" << X[i] << " y:" << Y[i] << " z:" << Z[i]
         << endl;
  }
}

void greaterequal_check_func(
  const vector<double>& X,
  const vector<double>& Y,
  const vector<double>& Z) {
  int size = X.size();
  for (int i = 0; i < size; i++) {
    // x >= y && z == 1
    if (f_greaterequal(X[i], Y[i]) && f_equal(Z[i], 1.0)) {
      continue;
    }
    if (f_less(X[i], Y[i]) && f_equal(Z[i], 0)) {
      continue;
    }
    cout << "In " << __FUNCTION__ << " i:" << i << " x:" << X[i] << " y:" << Y[i] << " z:" << Z[i]
         << endl;
  }
}

/**
 * 
 * 
 */
void sce_check_func(const vector<double>& X, const vector<double>& Y, const vector<double>& Z) {
  int size = X.size();
  //! @todo max(logit, 0) - logit * label + log(1 + exp(-abs(logits)))
  for (int i = 0; i < size; i++) {
    cout << "In " << __FUNCTION__ << " i:" << i << " x:" << X[i] << " y:" << Y[i] << " z:" << Z[i]
         << endl;
  }
}

void powconst_check_func(
  const vector<double>& X,
  const vector<double>& Y,
  const vector<double>& Z) {
  int size = X.size();
  for (int i = 0; i < size; i++) {
    auto r = pow(X[i], (int)Y[i]);
    if (f_equal(r, Z[i], 0.001)) {
      continue;
    }
    cout << "In " << __FUNCTION__ << " i:" << i << " x:" << X[i] << " y:" << Y[i] << " z:" << Z[i]
         << " r:" << r << endl;
  }
}

/**
 * 
 * 
 */
void square_check_func(const vector<double>& X, const vector<double>& Y) {
  int size = X.size();
  for (int i = 0; i < size; i++) {
    if (f_equal(pow(X[i], 2.0), Y[i])) {
      continue;
    }
    cout << "In " << __FUNCTION__ << " i:" << i << " x:" << X[i] << " y:" << Y[i] << endl;
  }
}
void negative_check_func(const vector<double>& X, const vector<double>& Y) {
  int size = X.size();
  for (int i = 0; i < size; i++) {
    if (f_equal(-X[i], Y[i])) {
      continue;
    }
    cout << "In " << __FUNCTION__ << " i:" << i << " x:" << X[i] << " y:" << Y[i] << endl;
  }
}
void abs_check_func(const vector<double>& X, const vector<double>& Y) {
  int size = X.size();
  for (int i = 0; i < size; i++) {
    if (f_equal(abs(X[i]), Y[i])) {
      continue;
    }
    cout << "In " << __FUNCTION__ << " i:" << i << " x:" << X[i] << " y:" << Y[i] << endl;
  }
}
void absprime_check_func(const vector<double>& X, const vector<double>& Y) {
  int size = X.size();
  for (int i = 0; i < size; i++) {
    if (f_greaterequal(X[i], 0) && (f_equal(Y[i], 1))) {
      continue;
    }
    if (f_less(X[i], 0) && (f_equal(Y[i], -1))) {
      continue;
    }
    cout << "In " << __FUNCTION__ << " i:" << i << " x:" << X[i] << " y:" << Y[i] << endl;
  }
}

void log_check_func(const vector<double>& X, const vector<double>& Y) {
  int size = X.size();
  for (int i = 0; i < size; i++) {
    auto r = f_log2(X[i]);
    if (f_equal(r, Y[i])) {
      continue;
    }
    cout << "In " << __FUNCTION__ << " i:" << i << " x:" << X[i] << " y:" << Y[i] << " r:" << r
         << endl;
  }
}
void log1p_check_func(const vector<double>& X, const vector<double>& Y) {
  int size = X.size();
  for (int i = 0; i < size; i++) {
    auto r = f_log2(X[i] + 1.0);
    if (f_equal(r, Y[i])) {
      continue;
    }
    cout << "In " << __FUNCTION__ << " i:" << i << " x:" << X[i] << " y:" << Y[i] << " r:" << r
         << endl;
  }
}
void hlog_check_func(const vector<double>& X, const vector<double>& Y) {
  int size = X.size();
  for (int i = 0; i < size; i++) {
    auto r = f_log2(X[i]);
    if (f_equal(r, Y[i])) {
      continue;
    }
    cout << "In " << __FUNCTION__ << " i:" << i << " x:" << X[i] << " y:" << Y[i] << " r:" << r
         << endl;
  }
}

void relu_check_func(const vector<double>& X, const vector<double>& Y) {
  int size = X.size();
  for (int i = 0; i < size; i++) {
    if (f_less(X[i], 0) && (f_equal(Y[i], 0))) {
      continue;
    }
    if (f_greaterequal(X[i], 0) && (f_equal(Y[i], X[i]))) {
      continue;
    }
    cout << "In " << __FUNCTION__ << " i:" << i << " x:" << X[i] << " y:" << Y[i] << endl;
  }
}

void reluprime_check_func(const vector<double>& X, const vector<double>& Y) {
  int size = X.size();
  for (int i = 0; i < size; i++) {
    if (f_less(X[i], 0) && (f_equal(Y[i], 0))) {
      continue;
    }
    if (f_greaterequal(X[i], 0) && (f_equal(Y[i], 1))) {
      continue;
    }
    cout << "In " << __FUNCTION__ << " i:" << i << " x:" << X[i] << " y:" << Y[i] << endl;
  }
}

void sigmoid_check_func(const vector<double>& X, const vector<double>& Y) {
  int size = X.size();
  for (int i = 0; i < size; i++) {
    auto r = 1 / (1 + pow(2.718281828459, -X[i]));
    if (f_equal(Y[i], r, 0.02)) {
      continue;
    }
    cout << "In " << __FUNCTION__ << " i:" << i << " x:" << X[i] << " y:" << Y[i] << " r:" << r
         << endl;
  }
}

/**
 * 
 * 
 */
void mean_check_func(const vector<double>& X, const vector<double>& Y, int rows, int cols) {
  vector<double> T(rows, 0);
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      T[r] += X[r * cols + c];
    }
    T[r] /= cols;
  }
  for (int i = 0; i < rows; i++) {
    if (f_equal(Y[i], T[i])) {
      continue;
    }
    cout << "In " << __FUNCTION__ << " i:" << i << " x:" << X[i] << " y:" << Y[i] << " t:" << T[i]
         << endl;
  }
}
void sum_check_func(const vector<double>& X, const vector<double>& Y, int rows, int cols) {
  vector<double> T(rows, 0);
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      T[r] += X[r * cols + c];
    }
  }
  for (int i = 0; i < rows; i++) {
    if (f_equal(Y[i], T[i])) {
      continue;
    }
    cout << "In " << __FUNCTION__ << " i:" << i << " x:" << X[i] << " y:" << Y[i] << " t:" << T[i]
         << endl;
  }
}
void addn_check_func(const vector<double>& X, const vector<double>& Y, int rows, int cols) {
  vector<double> T(cols, 0);
  for (int c = 0; c < cols; c++) {
    for (int r = 0; r < rows; r++) {
      T[c] += X[r * cols + c];
    }
  }
  for (int i = 0; i < cols; i++) {
    if (f_equal(Y[i], T[i])) {
      continue;
    }
    cout << "In " << __FUNCTION__ << " i:" << i << " x:" << X[i] << " y:" << Y[i] << " t:" << T[i]
         << endl;
  }
}
void max_check_func(const vector<double>& X, const vector<double>& Y, int rows, int cols) {
  vector<double> T(rows, 0);
  for (int r = 0; r < rows; r++) {
    T[r] = X[r * cols];
    for (int c = 1; c < cols; c++) {
      T[r] = std::max(X[r * cols + c], T[r]);
    }
  }
  for (int i = 0; i < rows; i++) {
    if (f_equal(Y[i], T[i])) {
      continue;
    }
    cout << "In " << __FUNCTION__ << " i:" << i << " x:" << X[i] << " y:" << Y[i] << " t:" << T[i]
         << endl;
  }
}
void min_check_func(const vector<double>& X, const vector<double>& Y, int rows, int cols) {
  vector<double> T(rows, 0);
  for (int r = 0; r < rows; r++) {
    T[r] = X[r * cols];
    for (int c = 1; c < cols; c++) {
      T[r] = std::min(X[r * cols + c], T[r]);
    }
  }
  for (int i = 0; i < rows; i++) {
    if (f_equal(Y[i], T[i])) {
      continue;
    }
    cout << "In " << __FUNCTION__ << " i:" << i << " x:" << X[i] << " y:" << Y[i] << " t:" << T[i]
         << endl;
  }
}
/**
 * 
 * 
 */
#include <Eigen/Dense> // for matmul
using namespace Eigen;
void matmul_check_func(
  const vector<double>& X,
  const vector<double>& Y,
  const vector<double>& Z,
  int m,
  int K,
  int n) {
  MatrixXd mat_a = MatrixXd(m, K);
  MatrixXd mat_b = MatrixXd(K, n);
  MatrixXd mat_c = MatrixXd(m, n);

  for (int i = 0; i < m; i++) {
    for (int j = 0; j < K; j++) {
      mat_a(i, j) = X[i * K + j];
    }
  }

  for (int i = 0; i < K; i++) {
    for (int j = 0; j < n; j++) {
      mat_b(i, j) = Y[i * n + j];
    }
  }

  mat_c = mat_a * mat_b;

  vector<double> T(Z.size());
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
      T[i * n + j] = mat_c(i, j);
    }
  }

  int size = Z.size();
  for (int i = 0; i < size; i++) {
    if (f_equal(Z[i], T[i], 0.0003)) {
      continue;
    }
    cout << "In " << __FUNCTION__ << " i:" << i << " x:" << X[i] << " y:" << Y[i] << " z:" << Z[i]
         << " t:" << T[i] << endl;
  }
}