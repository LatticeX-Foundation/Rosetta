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
#include "cc/modules/protocol/mpc/snn/src/impl/op_impl.h"

namespace rosetta {
namespace snn {

int LogicalAND::funcLogicalOp(
  const vector<mpc_t> X,
  const vector<mpc_t> Y,
  vector<mpc_t>& Z,
  size_t size) {
  Z.resize(size);
  GetMpcOpInner(DotProduct)->Run(X, Y, Z, size);
  return 0;
}
int LogicalAND::funcLogicalOp(
  const vector<mpc_t> X,
  const vector<string> sY,
  vector<mpc_t>& Z,
  size_t size) {
  Z.resize(size);
  vector<double> dY(size, 0);
  vector<mpc_t> Y(size, 0);
  rosetta::convert::from_double_str(sY, dY);
  convert_double_to_mpctype(dY, Y);

  ///////////////////////////////////
  for (size_t i = 0; i < size; i++) {
    Z[i] = X[i] * Y[i];
  }
  if (PRIMARY)
    funcTruncate2PC(Z, FLOAT_PRECISION_M, size, PARTY_A, PARTY_B);
  ///////////////////////////////////
  return 0;
}

int LogicalOR::funcLogicalOp(
  const vector<mpc_t> X,
  const vector<mpc_t> Y,
  vector<mpc_t>& Z,
  size_t size) {
  vector<mpc_t> sum(size, 0);
  addVectors<mpc_t>(X, Y, sum, size);
  vector<mpc_t> prod(size, 0);
  GetMpcOpInner(DotProduct)->Run(X, Y, prod, size);
  subtractVectors(sum, prod, Z, size);

#if 0 // internal test [yl]
  {
    vector<vector<mpc_t>> XX;
    vector<mpc_t> ZZ, RR;
    XX.push_back(X);
    XX.push_back(Y);
    XX.push_back(Z);
    XX.push_back(Y);
    XX.push_back(Z);
    ZZ = Z;
    GetMpcOpInner(LogicalXOR)->Run(XX, ZZ);
    RR.resize(ZZ.size());
    GetMpcOpInner(Reconstruct2PC)->Run(ZZ, RR);
    cout << "RR[0]:" << RR[0] << endl;
  }
#endif

  return 0;
}
int LogicalOR::funcLogicalOp(
  const vector<mpc_t> X,
  const vector<string> sY,
  vector<mpc_t>& Z,
  size_t size) {
  Z.resize(size);
  vector<double> dY(size, 0);
  vector<mpc_t> Y(size, 0);
  rosetta::convert::from_double_str(sY, dY);
  convert_double_to_mpctype(dY, Y);

  ///////////////////////////////////
  vector<mpc_t> sum = X;
  if (partyNum == PARTY_A) {
    // X add Y in P0, X add 0 in P1/P2
    addVectors<mpc_t>(X, Y, sum, size);
  }

  vector<mpc_t> prod(size, 0);
  for (size_t i = 0; i < size; i++) {
    prod[i] = X[i] * Y[i];
  }
  if (PRIMARY)
    funcTruncate2PC(prod, FLOAT_PRECISION_M, size, PARTY_A, PARTY_B);

  subtractVectors(sum, prod, Z, size);
  ///////////////////////////////////
  return 0;
}

int LogicalXOR::funcLogicalOp(
  const vector<mpc_t> X,
  const vector<mpc_t> Y,
  vector<mpc_t>& Z,
  size_t size) {
  Z.resize(size);

  vector<mpc_t> sum(size, 0);
  addVectors<mpc_t>(X, Y, sum, size);
  vector<mpc_t> prod(size, 0);
  GetMpcOpInner(DotProduct)->Run(X, Y, prod, size);
  if (PRIMARY) {
    for (int i = 0; i < size; ++i) {
      prod[i] = prod[i] << 1;
    }
  }
  subtractVectors(sum, prod, Z, size);

  return 0;
}

int LogicalXOR::funcLogicalOp(
  const vector<mpc_t> X,
  const vector<string> sY,
  vector<mpc_t>& Z,
  size_t size) {
  Z.resize(size);
  vector<double> dY(size, 0);
  vector<mpc_t> Y(size, 0);
  rosetta::convert::from_double_str(sY, dY);
  convert_double_to_mpctype(dY, Y);

  ///////////////////////////////////
  vector<mpc_t> sum = X;
  if (partyNum == PARTY_A) {
    // X add Y in P0, X add 0 in P1/P2
    addVectors<mpc_t>(X, Y, sum, size);
  }

  vector<mpc_t> prod(size, 0);
  for (size_t i = 0; i < size; i++) {
    prod[i] = X[i] * Y[i];
  }
  if (PRIMARY)
    funcTruncate2PC(prod, FLOAT_PRECISION_M, size, PARTY_A, PARTY_B);
  if (PRIMARY) {
    for (int i = 0; i < size; ++i) {
      prod[i] = prod[i] << 1;
    }
  }
  subtractVectors(sum, prod, Z, size);
  ///////////////////////////////////
  return 0;
}

int LogicalNOT::funcLogicalOp(const vector<mpc_t> X, vector<mpc_t>& Z, size_t size) {
  Z.resize(size);
  vector<mpc_t> one(size, 1 << (FLOAT_PRECISION_M - 1));
  GetMpcOpInner(Sub)->Run(one, X, Z, size);
  return 0;
}

//////////
int LogicalOp::funcLogicalOp_(const vector<vector<mpc_t>> XX, vector<mpc_t>& Z) {
  Z.clear();
  int k = XX.size();
  int d = log2ceil(k);
  if (k == 0) {
    Z.resize(0);
    return 0;
  }
  if (k == 1) {
    Z = XX[0];
    return 0;
  }
  //cout << "k:" << k << endl;

  int size = XX[0].size();
  vector<mpc_t> hX, hY, hZ; // verctorization
  if (k & 1 == 1) { // odd
    for (int i = 0; i < (k - 1) / 2; i++) {
      hX.insert(hX.end(), XX[i * 2].begin(), XX[i * 2].end());
      hY.insert(hY.end(), XX[i * 2 + 1].begin(), XX[i * 2 + 1].end());
    }
    funcLogicalOp(hX, hY, hZ, hX.size());
    vector<vector<mpc_t>> hXX;
    for (int i = 0; i < k / 2; i++) {
      vector<mpc_t> X(hZ.begin() + size * i, hZ.begin() + size * (i + 1));
      hXX.push_back(X);
    }
    hXX.push_back(XX[k - 1]);
    funcLogicalOp_(hXX, Z);
  } else {
    for (int i = 0; i < k / 2; i++) {
      hX.insert(hX.end(), XX[i * 2].begin(), XX[i * 2].end());
      hY.insert(hY.end(), XX[i * 2 + 1].begin(), XX[i * 2 + 1].end());
    }
    funcLogicalOp(hX, hY, hZ, hX.size());
    vector<vector<mpc_t>> hXX;
    for (int i = 0; i < k / 2; i++) {
      vector<mpc_t> X(hZ.begin() + size * i, hZ.begin() + size * (i + 1));
      hXX.push_back(X);
    }
    funcLogicalOp_(hXX, Z);
  }

  return 0;
}

} // namespace snn
} // namespace rosetta