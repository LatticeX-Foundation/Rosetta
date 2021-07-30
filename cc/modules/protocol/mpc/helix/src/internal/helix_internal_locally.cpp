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
using namespace std;

namespace rosetta {
namespace helix {

/**
 * non-scale to scaled
 */
void HelixInternal::Scale(vector<Share>& X, size_t power) {
  for (int i = 0; i < X.size(); ++i) {
    X[i].s0.A0 *= (1UL << power);
    X[i].s1.A1 *= (1UL << power);
  }
}

void HelixInternal::Scale(const vector<Share>& X, vector<Share>& Y, size_t power) {
  Y.assign(X.begin(), X.end());
  Scale(Y, power);
}

//! @todo for all the following opearators, replace with template

/**
 * Add/Sub, for basic type (mpc_t/bit_t)
 * Z = X + Y
 */
void HelixInternal::Add(const vector<mpc_t>& X, const vector<mpc_t>& Y, vector<mpc_t>& Z) {
  Z.assign(X.begin(), X.end());
  Add(Z, Y);
}
/**
 * X += Y
 */
void HelixInternal::Add(vector<mpc_t>& X, const vector<mpc_t>& Y) {
  size_t size = X.size();
  for (int i = 0; i < size; i++) {
    X[i] += Y[i];
  }
}
/**
 * Z = X ^ Y
 */
void HelixInternal::Add(const vector<bit_t>& X, const vector<bit_t>& Y, vector<bit_t>& Z) {
  Z.assign(X.begin(), X.end());
  Add(Z, Y);
}
/**
 * X ^= Y
 */
void HelixInternal::Add(vector<bit_t>& X, const vector<bit_t>& Y) {
  size_t size = X.size();
  for (int i = 0; i < size; i++) {
    X[i] ^= Y[i];
  }
}

/**
 * Z = X - Y
 */
void HelixInternal::Sub(const vector<mpc_t>& X, const vector<mpc_t>& Y, vector<mpc_t>& Z) {
  Z.assign(X.begin(), X.end());
  Sub(Z, Y);
}
/**
 * X -= Y
 */
void HelixInternal::Sub(vector<mpc_t>& X, const vector<mpc_t>& Y) {
  size_t size = X.size();
  for (int i = 0; i < size; i++) {
    X[i] -= Y[i];
  }
}
/**
 * Z = X ^ Y
 */
void HelixInternal::Sub(const vector<bit_t>& X, const vector<bit_t>& Y, vector<bit_t>& Z) {
  Z.assign(X.begin(), X.end());
  Sub(Z, Y);
}
/**
 * X ^= Y
 */
void HelixInternal::Sub(vector<bit_t>& X, const vector<bit_t>& Y) {
  size_t size = X.size();
  for (int i = 0; i < size; i++) {
    X[i] ^= Y[i];
  }
}

/**
 * Add, locally
 * Z = X + C
 * \param C constants (fixpoint, scaled)
 */
void HelixInternal::Add(const vector<Share>& X, const vector<mpc_t>& C, vector<Share>& Z) {
  Z.assign(X.begin(), X.end());
  Add(Z, C);
}

/**
 * Add, locally
 * X += C
 * \param C constants (fixpoint, scaled)
 */
void HelixInternal::Add(vector<Share>& X, const vector<mpc_t>& C) {
  assert(X.size() == C.size());
  size_t size = X.size();

  if (is_primary()) {
    for (int i = 0; i < size; i++) {
      X[i].s0.delta += C[i];
    }
  }
}

/**
 * Add, locally, float version
 */
void HelixInternal::Add(const vector<Share>& X, const vector<double>& C, vector<Share>& Z) {
  Z.assign(X.begin(), X.end());
  Add(Z, C);
}
void HelixInternal::Add(vector<Share>& X, const vector<double>& C) {
  vector<mpc_t> fpC;
  convert_plain_to_fixpoint(C, fpC, GetMpcContext()->FLOAT_PRECISION);
  Add(X, fpC);
}

/**
 * Sub, locally
 * Z = X - C
 * \param C constants (fixpoint, scaled)
 */
void HelixInternal::Sub(const vector<Share>& X, const vector<mpc_t>& C, vector<Share>& Z) {
  Z.assign(X.begin(), X.end());
  Sub(Z, C);
}

/**
 * Sub, locally
 * X -= C
 * \param C constants (fixpoint, scaled)
 */
void HelixInternal::Sub(vector<Share>& X, const vector<mpc_t>& C) {
  assert(X.size() == C.size());
  size_t size = X.size();

  if (is_primary()) {
    for (int i = 0; i < size; i++) {
      X[i].s0.delta -= C[i];
    }
  }
}

/**
 * Sub, locally, float version
 */
void HelixInternal::Sub(const vector<Share>& X, const vector<double>& C, vector<Share>& Z) {
  Z.assign(X.begin(), X.end());
  Sub(Z, C);
}
void HelixInternal::Sub(vector<Share>& X, const vector<double>& C) {
  vector<mpc_t> fpC;
  convert_plain_to_fixpoint(C, fpC, GetMpcContext()->FLOAT_PRECISION);
  Sub(X, fpC);
}

/**
 * Share op Share
 */
void HelixInternal::Add(const vector<Share>& X, const vector<Share>& Y, vector<Share>& Z) {
  Z.assign(X.begin(), X.end());
  Add(Z, Y);
}
void HelixInternal::Add(vector<Share>& X, const vector<Share>& Y) {
  assert(X.size() == Y.size());
  size_t size = X.size();
  for (int i = 0; i < size; i++) {
    X[i].s0.A0 += Y[i].s0.A0;
    X[i].s1.A1 += Y[i].s1.A1;
  }
}
void HelixInternal::Add(const vector<Share>& X, const Share& Y, vector<Share>& Z) {
  Z.assign(X.begin(), X.end());
  Add(Z, Y);
}
void HelixInternal::Add(vector<Share>& X, const Share& Y) {
  size_t size = X.size();
  for (int i = 0; i < size; i++) {
    X[i].s0.A0 += Y.s0.A0;
    X[i].s1.A1 += Y.s1.A1;
  }
}
void HelixInternal::Add(Share& X, const Share& Y) {
  X.s0.A0 += Y.s0.A0;
  X.s1.A1 += Y.s1.A1;
}
void HelixInternal::Add(const Share& X, const Share& Y, Share& Z) {
  Z.s0.A0 = X.s0.A0 + Y.s0.A0;
  Z.s1.A1 = X.s1.A1 + Y.s1.A1;
}

void HelixInternal::Sub(const vector<Share>& X, const vector<Share>& Y, vector<Share>& Z) {
  Z.assign(X.begin(), X.end());
  Sub(Z, Y);
}
void HelixInternal::Sub(vector<Share>& X, const vector<Share>& Y) {
  assert(X.size() == Y.size());
  size_t size = X.size();
  for (int i = 0; i < size; i++) {
    X[i].s0.A0 -= Y[i].s0.A0;
    X[i].s1.A1 -= Y[i].s1.A1;
  }
}
void HelixInternal::Sub(const vector<Share>& X, const Share& Y, vector<Share>& Z) {
  Z.assign(X.begin(), X.end());
  Sub(Z, Y);
}
void HelixInternal::Sub(vector<Share>& X, const Share& Y) {
  size_t size = X.size();
  for (int i = 0; i < size; i++) {
    X[i].s0.A0 -= Y.s0.A0;
    X[i].s1.A1 -= Y.s1.A1;
  }
}

void HelixInternal::Sub(Share& X, const Share& Y) {
  X.s0.A0 -= Y.s0.A0;
  X.s1.A1 -= Y.s1.A1;
}

void HelixInternal::Sub(const Share& X, const Share& Y, Share& Z) {
  Z.s0.A0 = X.s0.A0 - Y.s0.A0;
  Z.s1.A1 = X.s1.A1 - Y.s1.A1;
}

/**
 * constants Add/Sub sharing
 */
void HelixInternal::Add(const vector<mpc_t>& C, const vector<Share>& X, vector<Share>& Z) {
  assert(X.size() == C.size());
  size_t size = X.size();

  Z.assign(X.begin(), X.end());
  if (is_primary()) {
    for (int i = 0; i < size; i++) {
      Z[i].s0.delta += C[i];
    }
  }
}
void HelixInternal::Add(const vector<double>& C, const vector<Share>& X, vector<Share>& Z) {
  vector<mpc_t> fpC;
  convert_plain_to_fixpoint(C, fpC, GetMpcContext()->FLOAT_PRECISION);
  Add(fpC, X, Z);
}
void HelixInternal::Sub(const vector<mpc_t>& C, const vector<Share>& X, vector<Share>& Z) {
  assert(X.size() == C.size());
  size_t size = X.size();

  Negative(X, Z);
  if (is_primary()) {
    for (int i = 0; i < size; i++) {
      Z[i].s0.delta += C[i];
    }
  }
}
void HelixInternal::Sub(const vector<double>& C, const vector<Share>& X, vector<Share>& Z) {
  vector<mpc_t> fpC;
  convert_plain_to_fixpoint(C, fpC, GetMpcContext()->FLOAT_PRECISION);
  Sub(fpC, X, Z);
}

/**
 * Sum
 * 
 * Y = X[0] + X[1] + ...
 */
void HelixInternal::Sum(const vector<Share>& X, Share& Y) {
  size_t size = X.size();
  for (int i = 0; i < size; i++) {
    Y.s0.A0 += X[i].s0.A0;
    Y.s1.A1 += X[i].s1.A1;
  }
}

/**
 * Negative
 * 
 * Y = -1 * X
 */
void HelixInternal::Negative(const vector<Share>& X, vector<Share>& Y) {
  size_t size = X.size();
  vector<mpc_t> negOne(size, (mpc_t)-1L);
  Mul(X, negOne, Y, false); // locally multiply constants (no-scaled)
}

} // namespace helix
} // namespace rosetta