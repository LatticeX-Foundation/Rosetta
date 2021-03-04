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

#include "cc/modules/protocol/mpc/helix/include/helix_internal.h"

#include <iostream>
#include <vector>
#include <cassert>
#include <cmath>
#include <string>

using namespace std;

namespace rosetta {
namespace helix {
/**
 * all input/output are scaled bit shares.
 * AND (x and y) = x*y
 * OR  (x or y)  = x + y - x*y
 * XOR (x xor y) = x + y - 2*x*y
 * NOT (not x)   = 1 - x
 * Z[i] = X[i] (OP) Y[i]
 */
void HelixInternal::AND(
  const vector<Share>& X,
  const vector<Share>& Y,
  vector<Share>& Z,
  bool scaled) {
  assert(X.size() == Y.size());
  Mul(X, Y, Z, scaled);
}
void HelixInternal::OR(
  const vector<Share>& X,
  const vector<Share>& Y,
  vector<Share>& Z,
  bool scaled) {
  assert(X.size() == Y.size());
  int size = X.size();
  vector<Share> sum, prod;
  Add(X, Y, sum);
  Mul(X, Y, prod, scaled);
  Sub(sum, prod, Z);
}

void HelixInternal::XOR(
  const vector<Share>& X,
  const vector<Share>& Y,
  vector<Share>& Z,
  bool scaled) {
  assert(X.size() == Y.size());
  int size = X.size();
  vector<Share> sum, prod, tmp;
  vector<mpc_t> two(size, 2);
  Add(X, Y, sum);
  Mul(X, Y, prod, scaled);
  Mul(prod, two, tmp, false);
  Sub(sum, tmp, Z);
}

void HelixInternal::NOT(const vector<Share>& X, vector<Share>& Z, bool scaled) {
  int size = X.size();
  if (scaled) {
    vector<mpc_t> one(size, 1 << FLOAT_PRECISION_M);
    Sub(one, X, Z);
  } else {
    vector<mpc_t> one(size, 1);
    Sub(one, X, Z);
  }
}

/////////////////
/////////////////
/////////////////
void HelixInternal::AND(
  const vector<Share>& X,
  const vector<double>& C,
  vector<Share>& Z,
  bool scaled) {
  assert(X.size() == C.size());
  vector<Share> scaledX = X;
  if (!scaled) {
    Scale(scaledX);
  }
  Mul(scaledX, C, Z);
}
void HelixInternal::OR(
  const vector<Share>& X,
  const vector<double>& C,
  vector<Share>& Z,
  bool scaled) {
  assert(X.size() == C.size());
  vector<Share> scaledX = X;
  if (!scaled) {
    Scale(scaledX);
  }

  int size = X.size();
  vector<Share> sum, prod;
  Add(X, C, sum);
  Mul(X, C, prod);
  Sub(sum, prod, Z);
}
void HelixInternal::XOR(
  const vector<Share>& X,
  const vector<double>& C,
  vector<Share>& Z,
  bool scaled) {
  assert(X.size() == C.size());
  vector<Share> scaledX = X;
  if (!scaled) {
    Scale(scaledX);
  }

  int size = X.size();
  vector<Share> sum, prod, tmp;
  vector<double> two(size, 2.0);
  Add(X, C, sum);
  Mul(X, C, prod);
  Mul(prod, two, tmp);
  Sub(sum, tmp, Z);
}
void HelixInternal::AND(
  const vector<double>& C,
  const vector<Share>& Y,
  vector<Share>& Z,
  bool scaled) {
  AND(Y, C, Z, scaled);
}
void HelixInternal::OR(
  const vector<double>& C,
  const vector<Share>& Y,
  vector<Share>& Z,
  bool scaled) {
  OR(Y, C, Z, scaled);
}
void HelixInternal::XOR(
  const vector<double>& C,
  const vector<Share>& Y,
  vector<Share>& Z,
  bool scaled) {
  XOR(Y, C, Z, scaled);
}
/////////////////
/////////////////
/////////////////

// K-*, log(n)
void HelixInternal::logical_op_r_(
  _Lfunc f,
  const vector<vector<Share>>& XX,
  vector<Share>& Z,
  bool scaled) {
  Z.clear();
  int k = XX.size();
  int d = log2ceil(k);
  if (k == 0) {
    Z.resize(0);
    return;
  }
  if (k == 1) {
    Z = XX[0];
    return;
  }
  //cout << "k:" << k << endl;

  int size = XX[0].size();
  vector<Share> hX, hY, hZ; // vectorization
  if (k & 1 == 1) { // odd
    for (int i = 0; i < (k - 1) / 2; i++) {
      hX.insert(hX.end(), XX[i * 2].begin(), XX[i * 2].end());
      hY.insert(hY.end(), XX[i * 2 + 1].begin(), XX[i * 2 + 1].end());
    }
    (this->*f)(hX, hY, hZ, scaled);
    vector<vector<Share>> hXX;
    for (int i = 0; i < k / 2; i++) {
      vector<Share> X(hZ.begin() + size * i, hZ.begin() + size * (i + 1));
      hXX.push_back(X);
    }
    hXX.push_back(XX[k - 1]);
    logical_op_r_(f, hXX, Z, scaled);
  } else {
    for (int i = 0; i < k / 2; i++) {
      hX.insert(hX.end(), XX[i * 2].begin(), XX[i * 2].end());
      hY.insert(hY.end(), XX[i * 2 + 1].begin(), XX[i * 2 + 1].end());
    }
    (this->*f)(hX, hY, hZ, scaled);
    vector<vector<Share>> hXX;
    for (int i = 0; i < k / 2; i++) {
      vector<Share> X(hZ.begin() + size * i, hZ.begin() + size * (i + 1));
      hXX.push_back(X);
    }
    logical_op_r_(f, hXX, Z, scaled);
  }
}

void HelixInternal::AND(const vector<vector<Share>>& XX, vector<Share>& Z, bool scaled) {
  logical_op_r_(&HelixInternal::AND, XX, Z, scaled);
}
void HelixInternal::OR(const vector<vector<Share>>& XX, vector<Share>& Z, bool scaled) {
  logical_op_r_(&HelixInternal::OR, XX, Z, scaled);
}
void HelixInternal::XOR(const vector<vector<Share>>& XX, vector<Share>& Z, bool scaled) {
  logical_op_r_(&HelixInternal::XOR, XX, Z, scaled);
}

} // namespace helix
} // namespace rosetta
