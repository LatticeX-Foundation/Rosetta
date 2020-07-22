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
#include "cc/modules/protocol/mpc/comm/include/_test_check_func.h"

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
  //! @todo only positive
  for (int i = 0; i < size; i++) {
    if (f_equal(pow(X[i], (int)Y[i]), Z[i])) {
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
    if (f_equal(f_log2(X[i]), Y[i])) {
      continue;
    }
    cout << "In " << __FUNCTION__ << " i:" << i << " x:" << X[i] << " y:" << Y[i] << endl;
  }
}
void log1p_check_func(const vector<double>& X, const vector<double>& Y) {
  int size = X.size();
  for (int i = 0; i < size; i++) {
    if (f_equal(f_log2(X[i] + 1.0), Y[i])) {
      continue;
    }
    cout << "In " << __FUNCTION__ << " i:" << i << " x:" << X[i] << " y:" << Y[i] << endl;
  }
}
void hlog_check_func(const vector<double>& X, const vector<double>& Y) {
  int size = X.size();
  for (int i = 0; i < size; i++) {
    if (f_equal(f_log2(X[i]), Y[i])) {
      continue;
    }
    cout << "In " << __FUNCTION__ << " i:" << i << " x:" << X[i] << " y:" << Y[i] << endl;
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
  //! @todo
  for (int i = 0; i < size; i++) {
    auto r = 1 / (1 + pow(2, X[i]));

    cout << "In " << __FUNCTION__ << " i:" << i << " x:" << X[i] << " y:" << Y[i] << endl;
  }
}

/**
 * 
 * 
 */
void mean_check_func(const vector<double>& X, const vector<double>& Y, int rows, int cols) {
  int size = X.size();
  //! @todo
  for (int i = 0; i < size; i++) {
    cout << "In " << __FUNCTION__ << " i:" << i << " x:" << X[i] << " y:" << Y[i] << endl;
  }
}
void sum_check_func(const vector<double>& X, const vector<double>& Y, int rows, int cols) {
  int size = X.size();
  //! @todo
  for (int i = 0; i < size; i++) {
    cout << "In " << __FUNCTION__ << " i:" << i << " x:" << X[i] << " y:" << Y[i] << endl;
  }
}
void addn_check_func(const vector<double>& X, const vector<double>& Y, int rows, int cols) {
  int size = X.size();
  //! @todo
  for (int i = 0; i < size; i++) {
    cout << "In " << __FUNCTION__ << " i:" << i << " x:" << X[i] << " y:" << Y[i] << endl;
  }
}
void max_check_func(const vector<double>& X, const vector<double>& Y, int rows, int cols) {
  int size = X.size();
  //! @todo
  for (int i = 0; i < size; i++) {
    cout << "In " << __FUNCTION__ << " i:" << i << " x:" << X[i] << " y:" << Y[i] << endl;
  }
}
void min_check_func(const vector<double>& X, const vector<double>& Y, int rows, int cols) {
  int size = X.size();
  //! @todo
  for (int i = 0; i < size; i++) {
    cout << "In " << __FUNCTION__ << " i:" << i << " x:" << X[i] << " y:" << Y[i] << endl;
  }
}
/**
 * 
 * 
 */
void matmul_check_func(
  const vector<double>& X,
  const vector<double>& Y,
  const vector<double>& Z,
  int m,
  int K,
  int n,
  bool t_a,
  bool t_b) {
  int size = Z.size();
  //! @todo
  for (int i = 0; i < size; i++) {
    cout << "In " << __FUNCTION__ << " i:" << i << " x:" << X[i] << " y:" << Y[i] << " z:" << Z[i]
         << endl;
  }
}