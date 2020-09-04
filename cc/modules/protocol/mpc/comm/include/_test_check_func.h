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
#include <vector>
#include <iostream>
using namespace std;

// clang-format off
static const double EPS = 0.0001;
static inline bool f_equal(double a, double b, double e = EPS) { return (fabs(a - b) < e); }
static inline bool f_notequal(double a, double b, double e = EPS) { return !f_equal(a, b, e); }
static inline bool f_less(double a, double b, double e = EPS) { return !f_equal(a, b, e) && (a < b); }
static inline bool f_lessequal(double a, double b, double e = EPS) { return f_equal(a, b, e) || (a < b); }
static inline bool f_greater(double a, double b, double e = EPS) { return !f_equal(a, b, e) && (a > b); }
static inline bool f_greaterequal(double a, double b, double e = EPS) { return f_equal(a, b, e) || (a > b); }
static inline double f_log2(double a) { return log(a) / log(2); }
// clang-format on

/**
 * 
 * 
 */
void add_check_func(const vector<double>& X, const vector<double>& Y, const vector<double>& Z);
void sub_check_func(const vector<double>& X, const vector<double>& Y, const vector<double>& Z);
void mul_check_func(const vector<double>& X, const vector<double>& Y, const vector<double>& Z);
void floordiv_check_func(const vector<double>& X, const vector<double>& Y, const vector<double>& Z);
void truediv_check_func(const vector<double>& X, const vector<double>& Y, const vector<double>& Z);

void equal_check_func(const vector<double>& X, const vector<double>& Y, const vector<double>& Z);
void notequal_check_func(const vector<double>& X, const vector<double>& Y, const vector<double>& Z);
void less_check_func(const vector<double>& X, const vector<double>& Y, const vector<double>& Z);
void lessequal_check_func(
  const vector<double>& X,
  const vector<double>& Y,
  const vector<double>& Z);
void greater_check_func(const vector<double>& X, const vector<double>& Y, const vector<double>& Z);
void greaterequal_check_func(
  const vector<double>& X,
  const vector<double>& Y,
  const vector<double>& Z);

/**
 * 
 * 
 */
void sce_check_func(const vector<double>& X, const vector<double>& Y, const vector<double>& Z);
void powconst_check_func(const vector<double>& X, const vector<double>& Y, const vector<double>& Z);

/**
 * 
 * 
 */
void square_check_func(const vector<double>& X, const vector<double>& Y);
void negative_check_func(const vector<double>& X, const vector<double>& Y);
void abs_check_func(const vector<double>& X, const vector<double>& Y);
void absprime_check_func(const vector<double>& X, const vector<double>& Y);

void log_check_func(const vector<double>& X, const vector<double>& Y);
void log1p_check_func(const vector<double>& X, const vector<double>& Y);
void hlog_check_func(const vector<double>& X, const vector<double>& Y);
void relu_check_func(const vector<double>& X, const vector<double>& Y);
void reluprime_check_func(const vector<double>& X, const vector<double>& Y);
void sigmoid_check_func(const vector<double>& X, const vector<double>& Y);

/**
 * 
 * 
 */
void mean_check_func(const vector<double>& X, const vector<double>& Y, int rows, int cols);
void sum_check_func(const vector<double>& X, const vector<double>& Y, int rows, int cols);
void addn_check_func(const vector<double>& X, const vector<double>& Y, int rows, int cols);
void max_check_func(const vector<double>& X, const vector<double>& Y, int rows, int cols);
void min_check_func(const vector<double>& X, const vector<double>& Y, int rows, int cols);

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
  int n);
