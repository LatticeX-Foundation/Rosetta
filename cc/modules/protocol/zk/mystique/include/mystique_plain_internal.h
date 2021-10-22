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

#include "cc/modules/protocol/zk/mystique/include/zk_int_fp_eigen.h"
#include "emp-tool/emp-tool.h"
#include "emp-tool/circuits/integer.h"

#include "emp-zk/emp-zk.h"

#include "cc/modules/protocol/zk/mystique/include/wvr_util.h"

using emp::Integer;
using rosetta::zk::ZkUint64;
using std::vector;
#include <atomic>


extern int zk_party_id;
#define ZK_IS_ALICE (zk_party_id == 1)
#define ZK_IS_BOB (zk_party_id == 2)

#define USE_INNER_PRODUCT_OPT 1

namespace rosetta {
namespace zk {

static inline void plain_batch_feed(IntFp *labels, const uint64_t *value, int len) {
  for (size_t i = 0; i < len; i++)
  {
    uint64_t fpv = mod(value[i]);
    labels[i].value = (__uint128_t)makeBlock(fpv, 0);
  }
}

static inline void plain_zk_fp(const IntFp *zks, ZkUint64 *result, int size) {
  for (size_t i = 0; i < size; i++) // get high 64 bits
  {
    result[0].value = (uint64_t)(zks[i].value >> 64);
  }
}

static inline ZkUint64 mystique_plain_add(const ZkUint64& a, const ZkUint64& b) {
  return a+b;
}

static inline ZkUint64 mystique_plain_sub(const ZkUint64& a, const ZkUint64& b) {
  return a-b;
}

static inline ZkUint64 mystique_plain_neg(const ZkUint64& a) {
  return -a;
}

static inline ZkUint64 mystique_plain_mul(const ZkUint64& a, const ZkUint64& b) {
  return a*b;
}

static inline void mystique_plain_truncate(ZkUint64* in, size_t size) {
  // out.resize(in.size());
  SimpleTimer timer;
  vector<Integer> bc(size);
  // scale 2^{2s} to float
  vector<uint64_t> in_fps(size);
  for (size_t i = 0; i < size; i++)
  {
    in_fps[i] = in[i].value;
  }
  
  vector<float> float_in(in_fps.size());
  zk_decode_(in_fps, float_in, 2*ZK_F);
  zk_encode_float32(float_in, in_fps);

  for (size_t i = 0; i < size; i++)
  {
    in[i].value = in_fps[i];
  }

  // for (size_t i = 0; i < size; i++)
  // {
  //   float fl = in[i].value >> (2*ZK_FLOAT_SIZE);
  //   // scale back with 2^{s}
  //   in[i] = (ZkUint64)(fl / (1<<ZK_FLOAT_SIZE));
  // }
}

static inline void mystique_plain_relu(const vector<ZkUint64>& in, vector<ZkUint64>& out, int scale_count=1) {
  size_t size = in.size();
  // vector<ZkUint64> bin = in;
  vector<uint64_t> in_fps(in.size());
  if (scale_count == 1) {
    // do nothing
  } else if (scale_count == 2) {
    for (size_t i = 0; i < in.size(); i++)
    {
      in_fps[i] = in[i].value;
    }
    
    vector<float> float_in(in.size());
    zk_decode_(in_fps, float_in, 2*ZK_F);
    zk_encode_float32(float_in, in_fps);
    // for (size_t i = 0; i < bin.size(); i++)
    // {
    //   // float fl = bin[i].value >> (2*ZK_FLOAT_SIZE);
    //   // bin[i].value = (uint64_t)(fl / (1<<ZK_FLOAT_SIZE));

    // }
  } else {
    log_error << "bad scale_count:  " << scale_count;
    throw std::runtime_error("bad scale_count");
  }
  
  ZkUint64 zero(0);
  auto &relu = out;
  ZkUint64 smallest_neg((uint64_t)((PR-1)/2)+1);
  for (size_t i = 0; i < size; ++i)
    relu[i] = (in_fps[i] >= smallest_neg.value ? zero : in_fps[i]);
}

static inline void mystique_plain_max(const vector<ZkUint64>& in, ZkUint64& out) {
  if (in.empty())
    throw std::runtime_error("null input, mystique_plain_max");
  
  // simple bubble 
  size_t size = in.size();
  auto &max = out;
  ZkUint64 zero(0);
  ZkUint64 smallest_neg((uint64_t)((PR-1)/2)+1);
  ZkUint64 same_sign_as_bigger_one;
  ZkUint64 a_positive_as_bigger_one;
  bool sign_a = false;
  bool sign_b = false;
  bool compare_sign = false;
  bool a_and_b_not_same_sign = false;
  max = in[0];
  for(int i = 1; i < size; ++i) {
    // auto delta = mod(in[i] + (PR - max));
    // if (delta < smallest_neg)// bigger
    //   max = in[i];

    sign_a = max >= smallest_neg;// is negative
    sign_b = in[i] >= smallest_neg; // is negative

    a_and_b_not_same_sign = (sign_a ^ sign_b);// xor
    // case 1, if a and b have same sign, we get the larger encoded one.
    compare_sign = (in[i] >= max);
    same_sign_as_bigger_one = compare_sign ? in[i] : max;
    // case 2. if a and b have different sign, and a has positive sign, we choose a.
    a_positive_as_bigger_one = sign_a ? in[i] : max;
    // choose case based on sign-XOR
    max = a_and_b_not_same_sign ? a_positive_as_bigger_one : same_sign_as_bigger_one;
  }
}

// get max elements of matrix, axis=1, get max of each row
static inline void mystique_plain_max_matrix(const vector<ZkUint64>& in, size_t rows, size_t cols, vector<ZkUint64>& out) {
  if (in.size() != rows*cols)
    throw std::runtime_error("input size ! = rows*cols, mystique_plain_max_matrix");
  
  log_debug << "mystique_plain_max_matrix, rows: " << rows << ", cols: " << cols;
  // simple bubble to get max element of each row
  out.resize(rows);
  
  size_t size = rows*cols;
  vector<ZkUint64> &bin_c = out;
 
  ZkUint64 NEG_SIGN(HALF_PR + 1);// the smallest negative value
  // a as the formal one, b as the latter one
  ZkUint64 same_sign_as_bigger_one;
  ZkUint64 a_positive_as_bigger_one;
  bool sign_a;
  bool sign_b;
  bool compare_sign;
  bool a_and_b_not_same_sign;
  
  // log_info << "mystique_plain_max_matrix, start run...";
  // get a bigger one
  for(size_t i = 0; i < rows; ++i) {
    // log_info << "mystique_plain_max_matrix, start run-" << i << " row...";
    bin_c[i] = in[i*cols];
    for (size_t j = 1; j < cols; ++j) {
      sign_a = (bin_c[i] >= NEG_SIGN);// is negative
      sign_b = (in[i*cols + j] >= NEG_SIGN); // is negative

      a_and_b_not_same_sign = (sign_a ^ sign_b);// xor
      // case 1, if a and b have same sign, we get the larger encoded one.
      compare_sign = in[i*cols + j] >= (bin_c[i]);
      same_sign_as_bigger_one = (compare_sign ? in[i*cols + j] : bin_c[i]);
      // case 2. if a and b have different sign, and a has positive sign, we choose a.
      a_positive_as_bigger_one = (sign_a ? in[i*cols + j] : bin_c[i]);
      // choose case based on sign-XOR
      bin_c[i] = (a_and_b_not_same_sign ? a_positive_as_bigger_one : same_sign_as_bigger_one);
    }
  }
  
  log_debug << "mystique_plain_max_matrix ok.";
}

// matmul proof with right operhand constant
static inline int mystique_plain_matmul_proof_r_const(const ZkU64Matrix& a, const DoubleMatrix& b, ZkU64Matrix& c, bool need_truncate=true) {
  assert(a.size() != 0 && b.size() != 0);
  if (c.size() == 0)
    c.resize(a.rows(), b.cols());
  
  ZkU64Matrix b_fix(b.rows(), b.cols());
  zk_encode_fp(b.data(), b.rows() * b.cols(), (uint64_t*)b_fix.data());

  zk_eigen_const_const_matmul(a, b_fix, c);//[c] = [a] * b

  // TODO: truncation to Fp or just encode as emp::Float
  if (need_truncate) {
    mystique_plain_truncate(c.data(), c.size());
  }

  return 0;
}

// matmul proof (not private_input float encoded a*b )
static inline int mystique_plain_matmul_proof(const ZkU64Matrix& a, const ZkU64Matrix& b, ZkU64Matrix& c, bool need_truncate=true) {
  assert(a.size() != 0 && b.size() != 0);
  
  c = a * b;
  // TODO: truncation to Fp or just encode as emp::Float
  if (need_truncate) {
    mystique_plain_truncate(c.data(), c.size());
  }
  
  log_info << "party: " << zk_party_id << "mystique_plain_matmul_fp_with_proof ok.";
  return 0;
}

}//zk
}//rosetta