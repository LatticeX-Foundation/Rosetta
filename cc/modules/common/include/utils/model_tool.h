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

#include <iostream>

#include <cassert>
using namespace std;

#include "simple_timer.h"
//#include "defines.h"
#include "logger.h"

#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <random>
#include <regex>
#include <string>
using namespace std;

#include <Eigen/Dense>
using namespace Eigen;

#include "global_defines.h"
#include "helper.h"

#include "logger.h"

#define BUFSIZE RLC_BN_BITS / 8

#define BN_INIT1(V1) \
  bn_t V1;           \
  bn_null(V1);       \
  bn_new(V1)
#define BN_INIT2(V1, V2) \
  BN_INIT1(V1);          \
  BN_INIT1(V2)
#define BN_INIT3(V1, V2, V3) \
  BN_INIT2(V1, V2);          \
  BN_INIT1(V3)
#define BN_INIT4(V1, V2, V3, V4) \
  BN_INIT2(V1, V2);              \
  BN_INIT2(V3, V4)
#define BN_INIT BN_INIT1

inline void print_MATRIX_shape(const MATRIX& m) {
  cout << "shape(" << m.rows() << "," << m.cols() << ")" << endl;
}

// (0,1)
// MatrixXd::Zero(10,10).unaryExpr(ptr_fun(generate_random));
// static double generate_random(double dummy)
// {
//     static default_random_engine e(time(0));
//     // static normal_distribution<double> n(0,10);
//     static normal_distribution<> n;
//     return n(e);
// }

///////// only support basic type
template <typename T>
static void zero_vec2d(vector<vector<T>>& v, int r, int c) {
  v.clear();
  v.resize(r);
  for (int i = 0; i < r; i++) {
    v[i].resize(c, 0);
  }
}

template <typename T>
static void clear_vec2d(vector<vector<T>>& v) {
  v.clear();
  v.resize(0);
  vector<vector<T>>().swap(v);
}

template <typename T>
static void clear_vec1d(vector<T>& v) {
  v.clear();
  v.resize(0);
  vector<T>().swap(v);
}
/////////

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

template <typename T>
static void tofile(const vector<T>& v, const string& filename) {
  ofstream ofile(filename);
  if (!ofile.good()) {
    cerr << "open " << filename << " failed!" << endl;
    return;
  }
  int prec = std::numeric_limits<double>::digits10;
  for (int i = 0; i < v.size(); i++) {
    stringstream sss;
    sss.precision(prec);
    sss << v[i] << "\n";
    ofile.write(sss.str().c_str(), sss.str().length());
  }
  ofile.close();
}

template <typename T>
static void tofile(const vector<vector<T>>& v, const string& filename) {
  int rows = v.size();
  if (rows == 0) {
    cerr << "no any data" << endl;
    return;
  }
  int cols = v[0].size();
  bool do_check = false;
  if (do_check) {
    for (int r = 0; r < rows; r++) {
      if (cols != v[r].size()) {
        cerr << "v[" << r << "] expected " << cols << " but got " << v[r].size() << endl;
        return;
      }
    }
  }

  ofstream ofile(filename, ios::binary | ios::out | ios::trunc);
  if (!ofile.good()) {
    cerr << "open " << filename << " failed!" << endl;
    return;
  }
  ofile.write((const char*)&rows, sizeof(int));
  ofile.write((const char*)&cols, sizeof(int));

  int linesize = cols * sizeof(T);
  for (int r = 0; r < rows; r++) {
    ofile.write((const char*)&v[r][0], linesize);
  }
  ofile.close();
}

static void tofile2(const string& s, const string& filename) {
  ofstream ofile(filename);
  if (!ofile.good()) {
    cerr << "open " << filename << " failed!" << endl;
    return;
  }
  ofile.write(s.data(), s.length());
  ofile.close();
}

template <typename T>
static void fromfile(vector<vector<T>>& v, const string& filename) {
  v.clear();
  v.resize(0);

  ifstream ifile(filename, ios::binary | ios::in);
  if (!ifile.good()) {
    cerr << "open " << filename << " failed!" << endl;
    return;
  }

  int rows, cols;
  ifile.read((char*)&rows, sizeof(int));
  ifile.read((char*)&cols, sizeof(int));
  v.resize(rows);

  int linesize = cols * sizeof(T);
  for (int r = 0; r < rows; r++) {
    v[r].resize(cols);
    ifile.read((char*)&v[r][0], linesize);
  }
  ifile.close();
}

static vector<double> generate_random(int n, int seed = -1) {
  static default_random_engine e;
  if (seed > 0) {
    e.seed(seed);
  } else {
    e.seed(time(0));
  }
  static normal_distribution<> dist;

  vector<double> v;
  for (int i = 0; i < n; i++) {
    v.push_back(dist(e));
  }
  return v;
}

static default_random_engine ee(time(0));
static normal_distribution<double> nnd(0, 10);
static MatrixXd get_random(int c, int r) {
  return MatrixXd::Zero(c, r).unaryExpr([](double dummy) { return nnd(ee); });
}

static MatrixXd get_random2(int c, int r) {
  return MatrixXd::Zero(c, r).unaryExpr([](double dummy) { return (ee() % 10000) / 10000.0; });
}

static MatrixXd get_random3(int c, int r) {
  return MatrixXd::Zero(c, r).unaryExpr(
      [](double dummy) { return (ee() & 0xFFFF) * (0.5 / 0xFFFF); });
}
static std::vector<std::string> split(const std::string& src, char delim) {
  std::stringstream ss(src);
  std::string item;
  std::vector<std::string> elems;
  while (std::getline(ss, item, delim)) {
    elems.push_back(std::move(item));
  }
  return elems;
}

// generate [0, max_size), return the first 'front_size' elements
static void gen_random_index0(vector<int>& indexs_r, int max_size, int front_size = -1) {
  if (front_size < 0)
    front_size = max_size;

  indexs_r.clear();
  indexs_r.resize(front_size);
  for (int i = 0; i < max_size; i++)
    indexs_r[i] = i;

  std::random_shuffle(indexs_r.begin(), indexs_r.end());
}

static void gen_random_index(vector<int>& indexs_r, int max_size, int front_size = -1) {
  if (front_size < 0)
    front_size = max_size;

  vector<int> indexs(max_size);
  for (int i = 0; i < max_size; i++)
    indexs[i] = i;

  std::random_device rd;
  std::mt19937 mt(rd());
  indexs_r.clear();
  indexs_r.resize(front_size);
  for (int i = 0; i < front_size; i++) {
    int j = mt() % indexs.size();
    indexs_r[i] = indexs[j];
    indexs[j] = indexs[indexs.size() - 1];
    indexs.pop_back();
  }
}

static void gen_random_index2(
    vector<int>& indexs_r, int max_size, int front_size = -1, uint32_t seed = 0) {
  if (front_size < 0)
    front_size = max_size;

  vector<int> indexs(max_size);
  for (int i = 0; i < max_size; i++)
    indexs[i] = i;

  if (seed != 0)
    srand(seed);

  indexs_r.clear();
  indexs_r.resize(front_size);
  for (int i = 0; i < front_size; i++) {
    int j = rand() * (seed + 1) % indexs.size();
    indexs_r[i] = indexs[j];
    indexs[j] = indexs[indexs.size() - 1];
    indexs.pop_back();
  }
}

static void standardization(MatrixXd& X) {
  for (int j = 0; j < X.cols(); j++) {
    // MatrixXd feature = X.col(j);
  }
}

static void xavier_uniform(MATRIX& m, size_t fan_in, size_t fan_out, int64_t seed = -1) {
  double d = sqrt(6.0 / (fan_in + fan_out));
  std::uniform_real_distribution<> dist{-d, d};
  std::random_device rd;
  std::default_random_engine e{rd()};
  e.seed((uint32_t)seed);
  for (int r = 0; r < m.rows(); r++) {
    for (int c = 0; c < m.cols(); c++) {
      m(r, c) = floatToDType(dist(e));
    }
  }
}

static void xavier_uniform2(vector<float>& v, size_t fan_in, size_t fan_out, int64_t seed = -1) {
  double d = sqrt(6.0 / (fan_in + fan_out));
  std::uniform_real_distribution<> dist{-d, d};
  std::random_device rd;
  std::default_random_engine e{rd()};
  if (seed != -1)
    e.seed((uint32_t)seed);
  for (int i = 0; i < v.size(); i++) {
    v[i] = dist(e);
  }
}

static void uniform2(vector<float>& v, float low, float up, int64_t seed = -1) {
  std::uniform_real_distribution<> dist{low, up};
  std::random_device rd;
  std::default_random_engine e{rd()};
  if (seed != -1)
    e.seed((uint32_t)seed);
  for (int i = 0; i < v.size(); i++) {
    v[i] = dist(e);
  }
}

static void uniform2(MatrixXd& m, float low, float up, int64_t seed = -1) {
  std::uniform_real_distribution<> dist{low, up};
  std::random_device rd;
  std::default_random_engine e{rd()};
  if (seed != -1)
    e.seed((uint32_t)seed);
  for (int r = 0; r < m.rows(); r++) {
    for (int c = 0; c < m.cols(); c++) {
      m(r, c) = dist(e);
    }
  }
}

static void uniform2(MatrixXui& m, int32_t low, int32_t up, int64_t seed = -1) {
  std::uniform_int_distribution<int32_t> dist{low, up};
  std::random_device rd;
  std::default_random_engine e{rd()};
  if (seed != -1)
    e.seed((uint32_t)seed);
  for (int r = 0; r < m.rows(); r++) {
    for (int c = 0; c < m.cols(); c++) {
      m(r, c) = dist(e);
    }
  }
}

static void uniform2(MatrixXul& m, int64_t low, int64_t up, int64_t seed = -1) {
  std::uniform_int_distribution<int64_t> dist{low, up};
  std::random_device rd;
  std::default_random_engine e{rd()};
  if (seed != -1)
    e.seed((uint32_t)seed);
  for (int r = 0; r < m.rows(); r++) {
    for (int c = 0; c < m.cols(); c++) {
      m(r, c) = dist(e);
    }
  }
}

static void uniform2(MatrixXl& m, int64_t low, int64_t up, int64_t seed = -1) {
  std::uniform_int_distribution<int64_t> dist{low, up};
  std::random_device rd;
  std::default_random_engine e{rd()};
  if (seed != -1)
    e.seed((uint32_t)seed);
  for (int r = 0; r < m.rows(); r++) {
    for (int c = 0; c < m.cols(); c++) {
      m(r, c) = dist(e);
    }
  }
}

static void uniform2(vector<vector<uint64_t>>& m, int64_t low, int64_t up, int64_t seed = -1) {
  std::uniform_int_distribution<int64_t> dist{low, up};
  std::random_device rd;
  std::default_random_engine e{rd()};
  if (seed != -1)
    e.seed((uint32_t)seed);
  for (int r = 0; r < m.size(); r++) {
    for (int c = 0; c < m[r].size(); c++) {
      m[r][c] = dist(e);
    }
  }
}
static void uniform2(vector<vector<int64_t>>& m, int64_t low, int64_t up, int64_t seed = -1) {
  std::uniform_int_distribution<int64_t> dist{low, up};
  std::random_device rd;
  std::default_random_engine e{rd()};
  if (seed != -1)
    e.seed((uint32_t)seed);
  for (int r = 0; r < m.size(); r++) {
    for (int c = 0; c < m[r].size(); c++) {
      m[r][c] = dist(e);
    }
  }
}
static void uniform2(vector<uint64_t>& m, int64_t low, int64_t up, int64_t seed = -1) {
  std::uniform_int_distribution<int64_t> dist{low, up};
  std::random_device rd;
  std::default_random_engine e{rd()};
  if (seed != -1)
    e.seed((uint32_t)seed);
  for (int r = 0; r < m.size(); r++) {
    m[r] = dist(e);
  }
}

static void uniform2(vector<int64_t>& m, int64_t low, int64_t up, int64_t seed = -1) {
  std::uniform_int_distribution<int64_t> dist{low, up};
  std::random_device rd;
  std::default_random_engine e{rd()};
  if (seed != -1)
    e.seed((uint32_t)seed);
  for (int r = 0; r < m.size(); r++) {
    m[r] = dist(e);
  }
}

static void max_abs_scale(MatrixXd& X, double scalar = 1.0) {
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
static void max_min_scale(MatrixXd& X, double scalar = 1.0) {
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
static void max_mean_scale(MatrixXd& X, double scalar = 1.0) {
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

static void normalize(MatrixXd& X, double scalar = 1.0) {
  max_abs_scale(X, scalar);
}

static void normalize(MatrixXul& X, double scalar = 1.0) {}
static void normalize(MatrixXl& X, double scalar = 1.0) {}

class model_metrics {
 public:
  static double mae(MatrixXd& X, MatrixXd& Y, MatrixXd& W) {
    MatrixXd YY = (Y - X * W).array().abs();
    return YY.sum() / X.rows();
  }

  static double mse(MatrixXd& X, MatrixXd& Y, MatrixXd& W) {
    MatrixXd YY = (Y - X * W).array().pow(2);
    return YY.sum() / X.rows();
  }

  static double rmse(MatrixXd& X, MatrixXd& Y, MatrixXd& W) {
    return sqrt(mse(X, Y, W));
  }

  static double r2_score(MatrixXd& X, MatrixXd& Y, MatrixXd& W) {
    double y_mean = Y.mean();
    MatrixXd Y_mean = Y.unaryExpr([y_mean](double dummy) { return dummy - y_mean; });
    double SST = Y_mean.array().pow(2).sum();
    MatrixXd Y_ = X * W;
    MatrixXd Y__mean = Y_.unaryExpr([y_mean](double dummy) { return dummy - y_mean; });
    double SSR = Y__mean.array().pow(2).sum();
    double SSE = (Y - Y_).array().pow(2).sum();
    double R2 = 1 - (SSE / SST); // R2 = SSR / SST;
    return R2;
  }
  static double r2_adjust(MatrixXd& X, MatrixXd& Y, MatrixXd& W) {
    double R2 = r2_score(X, Y, W);
    double R2_adjust = 1 - (1 - pow(R2, 2)) * (X.rows() - 1) / (X.rows() - X.cols() - 1);
    return R2_adjust;
  }
  static void linear_evaluation(MatrixXd& X, MatrixXd& Y, MatrixXd& W) {
    print("linear regression evaluation report");
    print(" mae", mae(X, Y, W));
    print(" mse", mse(X, Y, W));
    print("rmse", rmse(X, Y, W));
    print("  R2", r2_score(X, Y, W));
    print("R2-A", r2_adjust(X, Y, W));
  }
  static double accuracy(MatrixXd& X, MatrixXd& Y, MatrixXd& W) {
    // for test, this, only for mnsit
    MatrixXd YY = X * W;
    int n = YY.rows();
    int n_postive = 0;
    for (int i = 0; i < n; i++) {
      // if (i % 100 == 0)
      //    print("I", i, n_postive);
      if (Y(i, 0) < 0.05 && YY(i, 0) < 0.05)
        n_postive++;
      else if (Y(i, 0) >= 0.05 && YY(i, 0) >= 0.05)
        n_postive++;
    }
    return n_postive * 1.0 / n;
  }

  struct metrics {
    int FP = 0;
    int FN = 0;
    int TP = 0;
    int TN = 0;

    double acc = 0.0;
    double pre = 0.0;
    double rec = 0.0;
    double fs1 = 0.0;
    double tpr = 0.0;
    double fpr = 0.0;

    double loss = 0.0;
    double threshold = 0.5;

    double totaly = 0.0;
    double totalpredy = 0.0;

    string toString() {
      char buf[256] = {0};
      snprintf(
          buf, 256,
          "THRESHOLD:%.6f LOSS:%.6f TP:%03d TN:%03d FP:%03d FN:%03d "
          "ACC:%.6f PRE:%.6f "
          "REC:%.6f F1:%f TPR:%.6f FPR:%.6f T:%.6f P:%.6f",
          threshold, loss, TP, TN, FP, FN, acc, pre, rec, fs1, tpr, fpr, totaly, totalpredy);
      return string(buf);
    }
  };

  static void accuracy_(
      const vector<double>& Y, const vector<double>& predY, const char* tag, metrics& me) {
    assert(Y.size() > 0);
    assert(Y.size() == predY.size());

    size_t size = Y.size();

    // loss logy' + (1-t)log(1-y')
    double loss = 0;

    // True False Positive Negative
    int& FP = me.FP = 0;
    int& FN = me.FN = 0;
    int& TP = me.TP = 0;
    int& TN = me.TN = 0;
    double threshold = me.threshold;
    for (int i = 0; i < Y.size(); i++) {
      double y = Y[i]; // true value
      double predy = predY[i]; // predict value
      // LOGI("accuracy index:%03d predict:%f label:%f\n", i, predy, y);
      if (predy > 0.99999)
        predy = 0.99999;
      if (predy < 0.00001)
        predy = 0.00001;

      me.totaly += y;
      me.totalpredy += predy;

      if ((y < threshold) && (predy >= threshold))
        FP++;
      else if ((y >= threshold) && (predy >= threshold))
        TP++;
      else if ((y >= threshold) && (predy < threshold))
        FN++;
      else if ((y < threshold) && (predy < threshold))
        TN++;
      else
        cerr << "never enter here!" << endl;

      loss += -y * log(predy) - (1 - y) * log(1 - predy);
    }

    me.acc = (1.0f * (TP + TN)) / (TP + TN + FP + FN); // accuracy
    me.pre = ((1.0f * TP) / (TP + FP + 1e-6)) + 1e-6; // precision
    me.rec = ((1.0f * TP) / (TP + FN + 1e-6)) + 1e-6; // recall
    me.fs1 = 2.0 / ((1 / me.pre) + (1 / me.rec)); // F-score(beta=1)
    me.tpr = ((1.0f * TP) / (TP + FN + 1e-6)) + 1e-6; // True Positive Rate
    me.fpr = ((1.0f * FP) / (FP + TN + 1e-6)) + 1e-6; // False Positive Rate
    me.loss = loss / size;
  }

  static double accuracy(
      const vector<double>& Y, const vector<double>& predY, const char* tag = "") {
    metrics me;
    me.threshold = 0.5;
    accuracy_(Y, predY, tag, me);
    LOGI("accuracy %s %s\n", me.toString().c_str(), tag);
    return me.acc;
  }

  static void roc(
      const vector<double>& Y, const vector<double>& predY, vector<double>& tprs,
      vector<double>& fprs) {
    const char* tag = "ROC";

    tprs.clear();
    fprs.clear();

    vector<double> tmpy(predY.begin(), predY.end());
    sort(tmpy.begin(), tmpy.end());
    LOGI("accuracy \n");
    for (int i = 0; i < tmpy.size(); i++) {
      metrics me;
      me.threshold = tmpy[i];
      accuracy_(Y, predY, tag, me);
      LOGI("accuracy %s %s\n", me.toString().c_str(), tag);
      tprs.push_back(me.tpr);
      fprs.push_back(me.fpr);
    }
  }
  static void pr(
      const vector<double>& Y, const vector<double>& predY, vector<double>& recs,
      vector<double>& pres) {
    const char* tag = "PR";

    recs.clear();
    pres.clear();

    vector<double> tmpy(predY.begin(), predY.end());
    sort(tmpy.begin(), tmpy.end());
    LOGI("accuracy \n");
    for (int i = 0; i < tmpy.size(); i++) {
      metrics me;
      me.threshold = tmpy[i];
      accuracy_(Y, predY, tag, me);
      LOGI("accuracy %s %s\n", me.toString().c_str(), tag);
      recs.push_back(me.rec);
      pres.push_back(me.pre);
    }
  }

  static double precision(MatrixXd& X, MatrixXd& Y, MatrixXd& W) {
    return 0;
  }
  static double recall(MatrixXd& X, MatrixXd& Y, MatrixXd& W) {
    return 0;
  }
};

class secure_ml {
 public:
  static void gen_01(
      MatrixXd& X, MatrixXd& X0, MatrixXd& X1, MatrixXd& Y, MatrixXd& Y0, MatrixXd& Y1) {
    assert(X.rows() > 0 && X.cols() > 0 && Y.rows() > 0 && Y.cols() > 0 && X.rows() == Y.rows());
    int n = X.rows();
    int d = X.cols();

    // X0 = MatrixXd::Zero(n, d).unaryExpr([](double dummy) { return nn(ee); });
    // Y0 = MatrixXd::Zero(n, 1).unaryExpr([](double dummy) { return nn(ee); });
    X0 = get_random2(n, d);
    Y0 = get_random2(n, 1);
    normalize(X0);

    X1 = X - X0;
    Y1 = Y - Y0;
  }
};

class model_select {
 public:
  // default split samples to train:test = 0.7:0.3

  // default split samples to train:test = 0.7:0.3
  template <typename MAT>
  static bool train_test_split(
      MAT& X, MAT& X_train, MAT& X_test, MAT& Y, MAT& Y_train, MAT& Y_test, double test_size = 0.3,
      uint32_t seed = 0) {
    int n = X.rows();
    int d = X.cols();

    int n_test = n * test_size;
    int n_train = n - n_test;
    print("n_test", n_test, "n_train", n_train);
    assert(n_test > 0 && n_train > 0);
    if (!(n_test > 0 && n_train > 0)) {
      cout << "!(n_test > 0 && n_train > 0)" << endl;
      return false;
    }

    SimpleTimer timer;
    X_train = MAT::Zero(n_train, d);
    X_test = MAT::Zero(n_test, d);
    Y_train = MAT::Zero(n_train, 1);
    Y_test = MAT::Zero(n_test, 1);

    vector<int> indexs_r(n);
    if (seed == 0) {
      gen_random_index0(indexs_r, n);
    } else {
      gen_random_index2(indexs_r, n, -1, seed);
    }

    for (int i = 0; i < n_train; i++) {
      X_train.row(i) = X.row(indexs_r[i]);
      Y_train.row(i) = Y.row(indexs_r[i]);
    }
    for (int i = 0; i < n_test; i++) {
      X_test.row(i) = X.row(indexs_r[i]);
      Y_test.row(i) = Y.row(indexs_r[i]);
    }
    print("train_test_split cost(us)", timer.us_elapse());
    return true;
  }

  static void shuffle(const MATRIX& X, const MATRIX& Y, MATRIX& X_, MATRIX& Y_, bool rand = true) {
    size_t c = 0;
    read_batch(X, Y, X_, Y_, c, X.rows(), X.cols(), rand);
  }

  static void shuffle_local(MATRIX& X, MATRIX& Y, bool rand = true) {
    size_t c = 0;
    MATRIX Xtmp = X;
    MATRIX Ytmp = Y;
    shuffle(Xtmp, Ytmp, X, Y, rand);
  }

  static void read_batch(
      const MATRIX& X, const MATRIX& Y, MATRIX& Xb, MATRIX& Yb, size_t& counter, size_t batch,
      size_t shape, bool rand = false) {
    /*size_t counter = 0;
    size_t batch = 0;
    size_t shape = 0;*/

    assert(Xb.rows() == batch);
    assert(Xb.cols() == shape);
    assert(Yb.rows() == batch);
    assert(Yb.cols() == 1);

    if (rand)
      counter = 0;

    size_t size = X.rows();
    if ((size > counter) && (size - counter >= batch)) {
      if (rand) {
        vector<int> indexs_r(size);
        gen_random_index2(indexs_r, size);
        for (int i = 0; i < batch; i++) {
          Xb.row(i) = X.row(indexs_r[i]);
          Yb.row(i) = Y.row(indexs_r[i]);
        }
      } else {
        Xb = X.block(counter, 0, batch, shape);
        Yb = Y.block(counter, 0, batch, 1);
        counter += batch;
        if (counter >= size) {
          counter -= size;
        }
      }
    } else {
      size_t diff = size - counter; // diff < batch
      Xb.block(0, 0, diff, shape) = X.block(counter, 0, diff, shape);
      Yb.block(0, 0, diff, 1) = Y.block(counter, 0, diff, 1);
      counter = 0;

      size_t start = diff;
      size_t remain = batch - diff;
      while (remain > size) {
        Xb.block(start, 0, size, shape) = X.block(0, 0, size, shape);
        Yb.block(start, 0, size, 1) = Y.block(0, 0, size, 1);
        remain -= size;
        start += size;
      }
      Xb.block(start, 0, remain, shape) = X.block(0, 0, remain, shape);
      Yb.block(start, 0, remain, 1) = Y.block(0, 0, remain, 1);
      counter += remain;
    }
  }
};

class model_tool {
 public:
  // size: bytes
  static string fmt_size(uint64_t size) {
    stringstream sss;
    if (size > 1024 * 1024 * 1024) // G
    {
      sss << 1.0 * size / (1024 * 1024 * 1024) << "G ";
    }
    if (size > 1024 * 1024) // M
    {
      sss << 1.0 * size / (1024 * 1024) << "M ";
    }
    if (size > 1024) // K
    {
      sss << 1.0 * size / 1024 << "K ";
    }
    sss << size << "B";
    return sss.str();
  }
  static string fmt_time(int64_t us) {
    stringstream sss;
    sss << setw(11) << us << "(us) [ ";
    if (us > 3600 * 1000 * 1000L) // Hour
    {
      sss << us / (3600 * 1000 * 1000L) << " h ";
      us %= (3600 * 1000 * 1000L);
    }
    if (us > 60 * 1000 * 1000) // minutes
    {
      sss << us / (60 * 1000 * 1000) << " m ";
      us %= (60 * 1000 * 1000);
    }
    if (us > 1000 * 1000) // second
    {
      sss << us / (1000 * 1000) << " s ";
      us %= (1000 * 1000);
    }
    if (us > 1000) // millisecond
    {
      sss << us / 1000 << " ms ";
      us %= 1000;
    }
    if (us > 0) // microsecond
    {
      sss << us << " us ";
    }
    sss << "]";
    return sss.str();
  }

 public:
  static void printShape(const string& msg, const MATRIX& m) {
    cout << "MATRIX[" << msg << "] SHAPE: (" << m.rows() << "," << m.cols() << ")" << endl;
  }
  static void printShape(const string& msg, const MATRIX3D& m) {
    cout << "MATRIX[" << msg << "] SIZE: " << m.size();
    if (m.size() > 0) {
      cout << " SHAPE: (" << m[0].rows() << "," << m[0].cols() << ")";
    }
    cout << endl;
  }
  static void printMatrix1(const MATRIX& m, bool print = true) {
    if (!print)
      return;
    for (int r = 0; r < m.rows(); r++) {
      for (int c = 0; c < m.cols(); c++) {
        cout << "m(" << r << "," << c << ") = " << m(r, c) << endl;
      }
    }
  }
  static void printMatrix2(
      const MATRIX& m, int rows = 32, int cols = 32, size_t width = 4, bool print = true) {
    // return;

    if (!print)
      return;
    if (rows < 0)
      rows = m.rows();
    if (cols < 0)
      cols = m.cols();
    for (int r = 0; r < m.rows() && r < rows; r++) {
      cout << "row" << setw(4) << r << ": ";
      for (int c = 0; c < m.cols() && c < cols; c++) {
        cout << setw(width) << (DType2)m(r, c) << " ";
      }
      cout << endl;
    }
  }
  static void reshapeMatrix(const MATRIX& src, MATRIX& dst, size_t shaper, size_t shapec) {
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
};
