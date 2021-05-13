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
/*
    For implemention of general MPC polynomials and approximating non-arithematic
    functionalities. 
*/

#include "cc/modules/protocol/mpc/snn/src/impl/op_impl.h"

#include <unordered_map>
#include <string>
using namespace std;
using namespace rosetta;

namespace rosetta {
namespace snn {
void Polynomial::mpc_pow_const(
  const vector<mpc_t>& shared_X,
  mpc_t common_k,
  vector<mpc_t>& shared_Y) {
  // cout << "DEBUG new mpc_pow_const" << endl;
  int vec_size = shared_X.size();
  shared_Y.resize(vec_size);
  // init curr_Y to '1'
  vector<mpc_t> curr_Y(vec_size, FloatToMpcType(1.0/2));

  if (common_k == 0) {
    shared_Y = curr_Y;
    return;
  }
  if (common_k == 1) {
    shared_Y = shared_X;
    return;
  }

  int curr_k = common_k;
  // current value in this bit
  int curr_bit = 0;
  // 2's power value
  int curr_p = 1;

  vector<mpc_t> P = shared_X;
  vector<mpc_t> tmp_new_y = curr_Y;
  vector<mpc_t> tmp_new_P = P;
  bool least_bit_covered = false;
  while (curr_k != 0) {
    curr_bit = curr_k % 2;
    if (curr_p != 1) {
      GetMpcOpInner(DotProduct)->Run(P, P, tmp_new_P, vec_size);
      P = tmp_new_P;
    }
    if (curr_bit) {
      // LSB bit, no need to use MPC to multiply const ONE.
      if (!least_bit_covered) {
        curr_Y = P;
        least_bit_covered = true;
      } else {
        GetMpcOpInner(DotProduct)->Run(P, curr_Y, tmp_new_y, vec_size);
        curr_Y = tmp_new_y;
      }
    }
    curr_k = int(curr_k / 2);
    curr_p++;
  }
  shared_Y = curr_Y;
  return;
}

void Polynomial::mpc_pow_const(const mpc_t& shared_X, mpc_t common_k, mpc_t& shared_Y) {
	vector<mpc_t> vec_x(1, shared_X);
	vector<mpc_t> vec_y(1);
	mpc_pow_const(vec_x, common_k, vec_y);
	shared_Y = vec_y[0];
}

void Polynomial::local_const_mul(
  const vector<mpc_t>& shared_X,
  mpc_t common_V,
  vector<mpc_t>& shared_Y) {
  int vec_size = shared_X.size();
  shared_Y.resize(vec_size);
  if (PRIMARY) {
    for (int i = 0; i < vec_size; ++i) {
      shared_Y[i] = common_V * shared_X[i];
      funcTruncateElem2PC(shared_Y[i], FLOAT_PRECISION_M, PARTY_A, PARTY_B);
    }
  }
}

// for continuous common_power_list with max order be N, round complexity is N. 
void Polynomial::mpc_uni_polynomial(
  const vector<mpc_t>& shared_X,
  const vector<mpc_t>& common_power_list,
  const vector<mpc_t>& common_coff_list,
  vector<mpc_t>& shared_Y) {
  int vec_size = shared_X.size();
  shared_Y.resize(vec_size);
  // this cache is for reduce calling Mul operation.
  // It works for continuous common_power_list.
  unordered_map<int, vector<mpc_t>> X_pow_cache;
  X_pow_cache[1] = shared_X;

  // Step one:
  vector<mpc_t> local_value(vec_size, 0);
  for (auto i = 0; i < common_power_list.size(); ++i) {
    // cout << "power" << i << endl;
    vector<mpc_t> tmp_coff_vec(vec_size, common_coff_list[i]);
    vector<mpc_t> tmp_prod(vec_size);
    if (common_power_list[i] == 0) {
      if (partyNum == PARTY_A) {
        addVectors(local_value, tmp_coff_vec, local_value, vec_size);
      }
    } else if (common_power_list[i] == 1) {
      if (PRIMARY) {
        // local const multiply
        mpc_t coff_v = common_coff_list[i];
        local_const_mul(shared_X, coff_v, tmp_prod);
        addVectors(local_value, tmp_prod, local_value, vec_size);
      }
    } else {
      vector<mpc_t> term_v(vec_size);
      mpc_t curr_k = common_power_list[i];
      if(X_pow_cache.find(curr_k - 1) != X_pow_cache.end()) {
        GetMpcOpInner(DotProduct)->Run(shared_X, X_pow_cache[curr_k - 1], term_v, vec_size);
        X_pow_cache[curr_k] = term_v;
      } else {
        mpc_pow_const(shared_X, curr_k, term_v);
        X_pow_cache[curr_k] = term_v;
      }
      if (PRIMARY) {
        // local const multiply
        mpc_t coff_v = common_coff_list[i];
        local_const_mul(term_v, common_coff_list[i], tmp_prod);
        addVectors(local_value, tmp_prod, local_value, vec_size);
      }
    }
  }
  shared_Y = local_value;
}

void Polynomial::mpc_uni_polynomial(
  const mpc_t& shared_X,
  const vector<mpc_t>& common_power_list,
  const vector<mpc_t>& common_coff_list,
  mpc_t& shared_Y) {
  // Step one:
  mpc_t local_value = 0;
  for (auto i = 0; i < common_power_list.size(); ++i) {
    vector<mpc_t> tmp_prod(1);
    if (common_power_list[i] == 0) {
      if (partyNum == PARTY_A) {
        local_value += common_coff_list[i];
      }
    } else if (common_power_list[i] == 1) {
      if (PRIMARY) {
        mpc_t coff_v = common_coff_list[i];
        // local const multiply
        mpc_t term_v = shared_X * coff_v;
        tmp_prod[0] = term_v;
        funcTruncate2PC(tmp_prod, FLOAT_PRECISION_M, 1, PARTY_A, PARTY_B);
        local_value += tmp_prod[0];
      }
    } else {
      mpc_t term_v;
      mpc_t curr_k = common_power_list[i];
      mpc_pow_const(shared_X, curr_k, term_v);
      if (PRIMARY) {
        // local const multiply
        term_v = term_v * common_coff_list[i];
        tmp_prod[0] = term_v;
        funcTruncate2PC(tmp_prod, FLOAT_PRECISION_M, 1, PARTY_A, PARTY_B);
        local_value += tmp_prod[0];
      }
    }
  }
  shared_Y = local_value;
}

} // namespace snn
} // namespace rosetta
