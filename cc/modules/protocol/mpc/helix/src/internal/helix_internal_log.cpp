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
#include "cc/modules/protocol/mpc/comm/include/mpc_common.h"

#include <iostream>
#include <vector>
#include <cassert>
#include <string>
#include <unordered_map>

using namespace std;
using namespace rosetta;

namespace rosetta {
namespace helix {

void HelixInternal::PowV2(
  const Share& X,
  const int& k,
  Share& Y) {
    vector<Share> vec_x(1, X);
    vector<Share> vec_y(1);
    PowV2(vec_x, k, vec_y);
    Y = vec_y[0];
}

void HelixInternal::PowV2(const vector<Share>& X, const int& k,
                          vector<Share>& Y) {
  // cout << "vector HelixInternal::PowV3" << endl;
  AUDIT("id:{}, P{} PowV2 input X(Share){}", msgid.get_hex(), player, Vector<Share>(X));
  AUDIT("id:{}, P{} PowV2 input K: {}", msgid.get_hex(), player, k);

  // init curr_Y to '1'
  int vec_size = X.size();
  vector<Share> curr_Y(vec_size);
  vector<double> DOUBLE_ONE(vec_size, 1.0);
  Add(curr_Y, DOUBLE_ONE);
  if (k == 0) {
    Y = curr_Y;
    AUDIT("id:{}, P{} PowV2 output Y(Share){}", msgid.get_hex(), player, Vector<Share>(Y));

    return;  
  }
  if (k == 1) {
    Y = X;
    AUDIT("id:{}, P{} PowV2 output Y(Share){}", msgid.get_hex(), player, Vector<Share>(Y));

    return;
  }

  int curr_k = k;
  // current value in this bit
  int curr_bit = 0;
  // 2's power value
  int curr_p = 1;

  vector<Share> P = X;
  vector<Share> tmp_new_y = curr_Y;
  vector<Share> tmp_new_P = P;
  bool least_bit_covered = false;
  while (curr_k != 0) {
    curr_bit = curr_k % 2;
    if (curr_p != 1) {
      Mul(P, P, tmp_new_P);
      P = tmp_new_P;
    }
    if (curr_bit) {
      // LSB bit, no need to use MPC to multiply const ONE.
      if (!least_bit_covered) {
        curr_Y = P;
        least_bit_covered = true;
      } else {
        Mul(P, curr_Y, tmp_new_y);
        curr_Y = tmp_new_y;
      }
    }
    curr_k = int(curr_k / 2);
    curr_p++;
  }
  Y = curr_Y;
  AUDIT("id:{}, P{} PowV2 output Y(Share){}", msgid.get_hex(), player, Vector<Share>(Y));

  return;
}

void HelixInternal::PowV2(const vector<Share>& X, const vector<int>& k, vector<Share>& Y) {
  assert(X.size() == k.size());
  resize_vector(Y, X.size());
  int vec_size = X.size();
  // speedup with vectorization
  bool is_common_k = true;
  for(int i = 1; i < vec_size; ++i) {
    if(k[i] != k[i-1]) {
      is_common_k = false;
      break;
    }
  }
  if (is_common_k) {
    // cout << "Helix common_k vector pow!" << endl;
    PowV2(X, k[0], Y);
    return;
  }
  // inefficent one 
  for (auto i = 0; i < k.size(); ++i) {
    PowV2(X[i], k[i], Y[i]);
  }
}

void HelixInternal::UniPolynomial(
  const Share& shared_X,
  const vector<mpc_t>& common_power_list,
  const vector<mpc_t>& common_coff_list,
  Share& shared_Y) {
  AUDIT("id:{}, P{} UniPolynomial input X(Share): {}", msgid.get_hex(), player, shared_X);
  AUDIT("id:{}, P{} UniPolynomial input power(mpc_t){}", msgid.get_hex(), player, Vector<mpc_t>(common_power_list));
  AUDIT("id:{}, P{} UniPolynomial input coff(mpc_t){}", msgid.get_hex(), player, Vector<mpc_t>(common_coff_list));
  vector<Share> curr_value(1);
  vector<Share> x_vec(1, shared_X);
  for (auto i = 0; i < common_power_list.size(); ++i) {
    //cout << "power" << i << endl;
    vector<Share> tmp_prod(1);
    vector<mpc_t> curr_coff(1, common_coff_list[i]);
    if (common_power_list[i] == 0) {
      // local const add
      Add(curr_value, curr_coff);
    } else if (common_power_list[i] == 1) {
      // local const multiply
      Mul(x_vec, curr_coff, tmp_prod);
      Add(curr_value, tmp_prod);
    } else {
      Share term_v;
      int curr_k = common_power_list[i];
      PowV2(shared_X, curr_k, term_v);
      vector<Share> tmp_term(1, term_v);
      Mul(tmp_term, curr_coff, tmp_prod);
      Add(curr_value, tmp_prod);
    }
  }
  shared_Y = curr_value[0];
  AUDIT("id:{}, P{} UniPolynomial output Y(Share): {}", msgid.get_hex(), player, shared_Y);
}

// for continuous common_power_list with max order be N, round complexity is N. 
void HelixInternal::UniPolynomial(
  const vector<Share>& shared_X,
  const vector<mpc_t>& common_power_list,
  const vector<mpc_t>& common_coff_list,
  vector<Share>& shared_Y) {
  AUDIT("id:{}, P{} UniPolynomial_batch input X(Share){}", msgid.get_hex(), player, Vector<Share>(shared_X));
  AUDIT("id:{}, P{} UniPolynomial_batch input power(mpc_t){}", msgid.get_hex(), player, Vector<mpc_t>(common_power_list));
  AUDIT("id:{}, P{} UniPolynomial_batch input coff(mpc_t){}", msgid.get_hex(), player, Vector<mpc_t>(common_coff_list));
  int vec_size = shared_X.size();
  // this cache is for reduce calling Mul operation.
  // It works for continuous common_power_list.
  unordered_map<int, vector<Share>> X_pow_cache;
  X_pow_cache[1] = shared_X;

  vector<Share> curr_value(vec_size);
  vector<Share> prod_res(vec_size);
  for (auto i = 0; i < common_power_list.size(); ++i) {
    //cout << "power" << i << endl;
    vector<Share> tmp_prod(vec_size);
    vector<mpc_t> curr_coff(vec_size, common_coff_list[i]);
    if (common_power_list[i] == 0) {
      // local const add
      Add(curr_value, curr_coff);
    } else if (common_power_list[i] == 1) {
      // local const multiply
      Mul(shared_X, curr_coff, tmp_prod, false);
      Add(prod_res, tmp_prod);
    } else {
      vector<Share> term_v(vec_size);
      int curr_k = common_power_list[i];
      if(X_pow_cache.find(curr_k - 1) != X_pow_cache.end()) {
        Mul(shared_X, X_pow_cache[curr_k - 1], term_v);
        X_pow_cache[curr_k] = term_v;
      } else {
        PowV2(shared_X, curr_k, term_v);
        X_pow_cache[curr_k] = term_v;
      }
      Mul(term_v, curr_coff, tmp_prod, false);
      Add(prod_res, tmp_prod);
    }
  }
  Trunc(prod_res, vec_size, GetMpcContext()->FLOAT_PRECISION);
  Add(prod_res, curr_value, shared_Y);
  AUDIT("id:{}, P{} UniPolynomial_batch output Y(Share){}", msgid.get_hex(), player, Vector<Share>(shared_Y));
}

void HelixInternal::LogV2(const vector<Share>& X, vector<Share>& Z) {
  size_t vec_size = X.size();
  Z.resize(vec_size);
  vector<Share> curr_z(Z.size());
  string my_func = "LOG_V2";
  vector<ConstPolynomial>* log_v2_p = NULL;
  if (!PolyConfFactory::get_func_polys(my_func, &log_v2_p)) {
    // TODO: throw exception
    tlog_error<< "ERROR! can not find polynomials for func " << my_func ;

    return;
  }
  auto seg_size = log_v2_p->size();
  if (seg_size == 0) {
    // TODO: throw exception
    tlog_error << "ERROR! empty polynomials in log_v2." ;
    return;
  }

  AUDIT("id:{}, P{} LogV2 input X(Share){}", msgid.get_hex(), player, Vector<Share>(X));
  /** 
	* In this version, we use the optimized style for piecewise function.
	F(x) = f_0(x) + (x>=c_1)(f_1(x)-f_0(x)) + ...+(x>=C_k)(f_k(x)-f_{k-1}(x))
	* note that we will set F(x) = 0, if x < 0
  */
  vector<vector<Share>> compare_res(vec_size, vector<Share>(seg_size));
  vector<vector<Share>> seg_res(vec_size, vector<Share>(seg_size));
  vector<mpc_t> curr_power_list;
  vector<mpc_t> curr_coff_list;
  vector<Share> last_seg_res(vec_size);
  int float_precision = GetMpcContext()->FLOAT_PRECISION;
  for (int i = 0; i < seg_size; ++i) {
    ConstPolynomial curr_seg = log_v2_p->at(i);
    mpc_t seg_init = curr_seg.get_start(float_precision);
    mpc_t seg_end = curr_seg.get_end(float_precision);

    curr_seg.get_power_list(curr_power_list);
    curr_seg.get_coff_list(curr_coff_list, float_precision);

    vector<double> curr_seg_start(vec_size, MpcTypeToFloat(seg_init, float_precision));
    vector<Share> curr_compare_res(vec_size);
    GreaterEqual(X, curr_seg_start, curr_compare_res);
    for (auto j = 0; j < vec_size; ++j) {
      compare_res[j][i] = curr_compare_res[j];
    }

    // get the result in each segment
    vector<Share> curr_seg_res(vec_size);
    UniPolynomial(X, curr_power_list, curr_coff_list, curr_seg_res);
    // f_k(x) - f_{k-1}(x)
    vector<Share> tmp_seg_res(vec_size);
    Sub(curr_seg_res, last_seg_res, tmp_seg_res);
    for (auto j = 0; j < vec_size; ++j) {
      seg_res[j][i] = tmp_seg_res[j];
    }
    last_seg_res = curr_seg_res;
  }
  InnerProducts(compare_res, seg_res, Z, false);

  AUDIT("id:{}, P{} LogV2 output Z(Share){}", msgid.get_hex(), player, Vector<Share>(Z));
}

void HelixInternal::HLog(const vector<Share>& X, vector<Share>& Z) {
  AUDIT("id:{}, P{} HLog input X(Share){}", msgid.get_hex(), player, Vector<Share>(X));
  int vec_size = X.size();
  // Note this is actually a constant: 64
  int LEN = 8 * sizeof(mpc_t); 
  //vector<Share> curr_Y(1);
  // math.log(2) = 0.693147181
  vector<double> DOUBLE_LN_2(vec_size, 0.693147181);
  vector<double> DOUBLE_ZERO(vec_size, 0.0);
  vector<double> DOUBLE_HALF(vec_size, 0.5);
  vector<double> DOUBLE_ONE(vec_size, 1.0);
  vector<double> DOUBLE_NEG_ONE(vec_size, -1.0);
  vector<double> DOUBLE_TWO(vec_size, 2.0);

  vector<Share> shared_val_multiplier(vec_size);
	vector<Share> shared_power_adder(vec_size);
	auto curr_x = X;
	vector<Share> curr_power(vec_size);
  int float_precision = GetMpcContext()->FLOAT_PRECISION;
  // step 1: express x as 2^{m} * {r/2} where r \in [1, 2)
	// 1.1 scale down integer part
	for(int i = 0; i < LEN - float_precision; ++i) {
	  auto curr_x_minus_one = curr_x;
		vector<Share> shared_cmp(vec_size);
		Sub(curr_x, DOUBLE_ONE, curr_x_minus_one);
    DReLU(curr_x_minus_one, shared_cmp);
    Select1Of2(DOUBLE_HALF, DOUBLE_ONE, shared_cmp, shared_val_multiplier);
    Select1Of2(DOUBLE_ONE, DOUBLE_ZERO, shared_cmp, shared_power_adder);
    vector<Share> tmp_v(vec_size);
    Mul(curr_x, shared_val_multiplier, tmp_v);
    curr_x = tmp_v;
    Add(curr_power, shared_power_adder);
  }
	// 1.2 scale up fractional part
  for(int i = 0; i < float_precision; ++i) {
		auto curr_x_minus_half = curr_x;
		vector<Share> shared_cmp(vec_size);
    Sub(curr_x, DOUBLE_HALF, curr_x_minus_half);
    DReLU(curr_x_minus_half, shared_cmp);
    Select1Of2(DOUBLE_ONE, DOUBLE_TWO, shared_cmp, shared_val_multiplier);
    Select1Of2(DOUBLE_ZERO, DOUBLE_NEG_ONE, shared_cmp, shared_power_adder);
    vector<Share> tmp_v(vec_size);
    Mul(curr_x, shared_val_multiplier, tmp_v);
    curr_x = tmp_v;
    Add(curr_power, shared_power_adder);
  }
  // step 2: computer basic ln(r/2) by polynomials interproplation
	vector<mpc_t> power_list;
	vector<mpc_t> coff_list;
  string my_func = "LOG_HD";
	vector<ConstPolynomial>* log_hd_p = NULL;
  if (!PolyConfFactory::get_func_polys(my_func, &log_hd_p)) {
    // TODO: throw exception
    cout << "ERROR! can not find polynomials for func " << my_func <<endl;
    return;
  }
	// we know that this is a one-segment polynomial
  log_hd_p->at(0).get_power_list(power_list);
	log_hd_p->at(0).get_coff_list(coff_list, float_precision);	
	// for (auto i = 0; i< power_list.size(); i++) {
	// 		cout << MpcTypeToFloat(CoffDown(coff_list[i])) << "*X^" << power_list[i] << " + ";
	// }
  vector<Share> basic_val(vec_size);
  UniPolynomial(curr_x, power_list, coff_list, basic_val);
	
  vector<Share> high_part(vec_size);
  Mul(curr_power, DOUBLE_LN_2, high_part);
  Add(basic_val, high_part, Z);

  AUDIT("id:{}, P{} HLog output Z(Share){}", msgid.get_hex(), player, Vector<Share>(Z));
}

} // namespace helix
} // namespace rosetta
