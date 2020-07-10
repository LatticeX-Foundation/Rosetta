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
#include <string>
using namespace std;

#include "eigen_util.h"
#include "file_directory.h"

#include "cc/modules/common/include/utils/simple_timer.h"
#include "cc/modules/common/include/utils/logger.h"
#include "cc/modules/common/include/utils/helper.h"

#include "random_util.h"

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

  static double rmse(MatrixXd& X, MatrixXd& Y, MatrixXd& W) { return sqrt(mse(X, Y, W)); }

  static double r2_score(MatrixXd& X, MatrixXd& Y, MatrixXd& W) {
    double y_mean = Y.mean();
    MatrixXd Y_mean = Y.unaryExpr([y_mean](double dummy) { return dummy - y_mean; });
    double SST = Y_mean.array().pow(2).sum();
    MatrixXd Y_ = X * W;
    MatrixXd Y__mean = Y_.unaryExpr([y_mean](double dummy) { return dummy - y_mean; });
    //double SSR = Y__mean.array().pow(2).sum();
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
    const vector<double>& Y,
    const vector<double>& predY,
    const char* tag,
    metrics& me) {
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
    const vector<double>& Y,
    const vector<double>& predY,
    const char* tag = "") {
    metrics me;
    me.threshold = 0.5;
    accuracy_(Y, predY, tag, me);
    LOGI("accuracy %s %s\n", me.toString().c_str(), tag);
    return me.acc;
  }

  static void roc(
    const vector<double>& Y,
    const vector<double>& predY,
    vector<double>& tprs,
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
    const vector<double>& Y,
    const vector<double>& predY,
    vector<double>& recs,
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

  static double precision(MatrixXd& X, MatrixXd& Y, MatrixXd& W) { return 0; }
  static double recall(MatrixXd& X, MatrixXd& Y, MatrixXd& W) { return 0; }
};

class model_select {
 public:
  // default split samples to train:test = 0.7:0.3
  template <typename MAT>
  static bool train_test_split(
    MAT& X,
    MAT& X_train,
    MAT& X_test,
    MAT& Y,
    MAT& Y_train,
    MAT& Y_test,
    double test_size = 0.3,
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
    //size_t c = 0;
    MATRIX Xtmp = X;
    MATRIX Ytmp = Y;
    shuffle(Xtmp, Ytmp, X, Y, rand);
  }

  static void read_batch(
    const MATRIX& X,
    const MATRIX& Y,
    MATRIX& Xb,
    MATRIX& Yb,
    size_t& counter,
    size_t batch,
    size_t shape,
    bool rand = false) {
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
