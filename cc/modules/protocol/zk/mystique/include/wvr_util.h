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

#include "cc/modules/protocol/public/include/protocol_ops.h"
#include "cc/modules/common/include/utils/rtt_logger.h"
#include "cc/modules/common/include/utils/helper.h"

#include "cc/modules/protocol/zk/mystique/include/zk_int_fp.h"

#include <iostream>
#include "emp-tool/emp-tool.h"
#if defined(__linux__)
#include <sys/time.h>
#include <sys/resource.h>
#elif defined(__APPLE__)
#include <unistd.h>
#include <sys/resource.h>
#include <mach/mach.h>
#endif

#include <cmath>
#include <string>
#include <unordered_map>
#include <vector>
#include <type_traits>
#include <tuple>

using namespace emp;
using namespace std;

#define THREAD_NUM ZK_THREAD_NUM
#define ENDL "\n"

// Also the largest encoded negative value.
#define HALF_PR int64_t((PR - 1) / 2)

extern int zk_party_id;

// ************Note[GeorgeShi]: I know this is ugly, but DO NOT remove these FOR NOW.
#define USE_EXP_APPROX false

#define SUPPORT_FLOAT_NUMBER true

#define ZK_INNER_CHECK true

// ONLY set this be true after setting ZK_INNER_CHECK as true.
#define ZK_INNER_BATCH_CHECK true

#define ZK_F ZK_FLOAT_SIZE

#define ZK_K 52

#define ZK_LL 58

#define ZK_M 12

#define TWO_INV uint64_t((PR + 1) / 2)

#define MINIMUN double(1.0 / (1L << ZK_F))

#define _FLOAT_SCALE_ uint64_t(1UL << ZK_F)

#define _TWO_FLOAT_SCALE_ uint64_t(1UL << (2 * ZK_F))

// temp macro added in 2021-01-26
#define _OUTPUT_WITH_HIGH_SCALE true

#define FloatToFixPoint(a) \
  (int64_t)(((int64_t)(a) << ZK_F) + (int64_t((a - (int64_t)(a)) * (1L << ZK_F))))

inline uint64_t common_mod(int64_t x, u_int64_t MM) {
  if (x > 0) {
    return x % MM;
  } else {
    return (x + MM) % MM;
  }
}

#define FixToFloatPoint(a) ((double((int64_t)(a))) / (1L << ZK_F))

// ************Note[GeorgeShi]
#define humanable false

// to kickoff this project, we just use the similar naive protocol code
namespace rosetta {
namespace zk {

class ZkUint64;
// tmp for local simulation
int convert_string_to_mac(const std::string* a, uint64_t* b, size_t size, bool human = humanable);
int convert_string_to_mac(const std::string* a, ZkUint64* b, size_t size, bool human = humanable);

// unwrap the IT-MAC value [128-bit block] from string
/// [GeorgeShi]: I know using `return value` as scale_multiplier in this style is very ugly. I will try to fix it soon...
int convert_string_to_mac(
  const std::vector<std::string>& a,
  std::vector<__uint128_t>& b,
  bool human = humanable,
  void* wvr_impl = nullptr);

int convert_string_to_mac(
  const std::vector<std::string>& a,
  std::vector<ZkIntFp>& b,
  bool human = humanable,
  void* wvr_impl = nullptr);

int convert_string_to_mac(
  const std::string* a,
  __uint128_t* b,
  size_t size,
  bool human = humanable,
  void* wvr_impl = nullptr);

static inline void convert_string_to_double(
  const vector<std::string>& a,
  double* b,
  bool human = humanable) {
  size_t size = a.size();
  if (human) { //double string : 1.001
    for (int i = 0; i < size; i++) {
#if !use_literal_value_binary_version
      memcpy((char*)&b[i], a[i].c_str(), sizeof(double));
#else
      b[i] = to_double(a[i].data());
#endif
    }
  } else {
    for (int i = 0; i < size; i++) {
      char* p = (char*)&b[i];
      memcpy(p, a[i].data(), sizeof(double));
    }
  }
}

static inline void convert_string_to_double(
  const vector<std::string>& a,
  vector<double>& b,
  bool human = humanable) {
  size_t size = a.size();
  b.resize(size);
  convert_string_to_double(a, b.data(), human);
}

// temp for local simulation
void convert_mac_to_string(
  const vector<uint64_t>& a,
  std::vector<std::string>& b,
  bool human,
  int scale_multiplier);

void convert_mac_to_string(
  const uint64_t* a,
  std::vector<std::string>& b,
  size_t size,
  bool human = humanable,
  int scale_multiplier = 1);

void convert_mac_to_string(
  const vector<ZkUint64>& a,
  std::vector<std::string>& b,
  bool human = humanable,
  int scale_multiplier = 1);

// wrap the IT-MAC value in a string
/// [GeorgeShi]: I know using `scale_multiplier` parameter in this style is very ugly. I will try to fix it soon...
void convert_mac_to_string(
  const std::vector<__uint128_t>& a,
  std::vector<std::string>& b,
  bool human = humanable,
  int scale_multiplier = 1);

void convert_mac_to_string(
  const vector<ZkIntFp>& a,
  std::vector<std::string>& b,
  bool human = humanable,
  int scale_multiplier = 1);

void convert_mac_to_string(
  const __uint128_t* a,
  std::vector<std::string>& b,
  size_t size,
  bool human = humanable,
  int scale_multiplier = 1);

// convert plaintext values to internal x \in field F_p
template <typename T>
void zk_encode(const std::vector<T>& a, std::vector<uint64_t>& b) {
  b.clear();
  b.resize(a.size());
  for (auto i = 0; i < a.size(); ++i) {
// auto plain_x = a[i];
// convert float to fix-point!
#if SUPPORT_FLOAT_NUMBER
    auto plain_x = FloatToFixPoint(a[i]);
#else
    auto plain_x = a[i];
#endif

    int64_t y = int64_t(plain_x);
    if (y < -HALF_PR) {
      cout << "WARNING!: out of range! too small value!:" << y;
      y = -HALF_PR;
    } else if (y > HALF_PR) {
      cout << "WARNING!: out of range! too Large value!:" << y;
      y = HALF_PR;
    }
    b[i] = mod(y + PR);
  }
}

template <typename T>
void zk_encode_fp(const T* a, size_t size, uint64_t* b) {
  for (auto i = 0; i < size; ++i) {
// auto plain_x = a[i];
// convert float to fix-point!
#if SUPPORT_FLOAT_NUMBER
    auto plain_x = FloatToFixPoint(a[i]);
#else
    auto plain_x = a[i];
#endif

    int64_t y = int64_t(plain_x);
    if (y < -HALF_PR) {
      cout << "WARNING!: out of range! too small value!:" << y;
      y = -HALF_PR;
    } else if (y > HALF_PR) {
      cout << "WARNING!: out of range! too Large value!:" << y;
      y = HALF_PR;
    }
    b[i] = mod(y + PR);
  }
}

static inline void zk_encode_float32(const vector<float>& a, vector<uint64_t>& b) {
  b.resize(a.size());
  for (auto i = 0; i < a.size(); ++i) {
// auto plain_x = a[i];
// convert float to fix-point!
#if SUPPORT_FLOAT_NUMBER
    auto plain_x = FloatToFixPoint(a[i]);
#else
    auto plain_x = a[i];
#endif

    int64_t y = int64_t(plain_x);
    if (y < -HALF_PR) {
      cout << "WARNING!: out of range! too small value!:" << y;
      y = -HALF_PR;
    } else if (y > HALF_PR) {
      cout << "WARNING!: out of range! too Large value!:" << y;
      y = HALF_PR;
    }
    b[i] = mod(y + PR);
    // I dont know why. but this seems work.
    if (b[i] > HALF_PR && (b[i] % 2 == 1)) {
#if DEBUG_MODE
      log_info << "attention, offset added";
#endif
      b[i] = b[i] + 1;
    }
  }
}

// convert internal values \in field F_p to plaintext.
template <typename T>
void zk_decode(const std::vector<uint64_t>& a, std::vector<T>& b) {
  b.clear();
  b.resize(a.size());
  for (auto i = 0; i < a.size(); ++i) {
    auto tmp = int64_t(a[i]);
    if (a[i] > HALF_PR) {
      tmp = int64_t(a[i] - PR);
    }
    // convert fix-point to float-point number
    if (std::is_floating_point<T>::value) {
// cout << "Wow, float!" << endl;
#if SUPPORT_FLOAT_NUMBER
      b[i] = FixToFloatPoint(tmp);
#else
      b[i] = T(tmp);
#endif
    } else {
      // cout << "Not float!" << endl;
      b[i] = T(tmp);
    }
  }
}

// convert internal values \in field F_p to plaintext.[]
template <typename T>
void zk_decode(const uint64_t* a, T* b, size_t size) {
  log_debug << "debug zk_decode for pointer parameter";

  for (auto i = 0; i < size; ++i) {
    auto tmp = int64_t(a[i]);
    if (a[i] > HALF_PR) {
      tmp = int64_t(a[i] - PR);
    }
    // convert fix-point to float-point number
    if (std::is_floating_point<T>::value) {
// cout << "Wow, float!" << endl;
#if SUPPORT_FLOAT_NUMBER
      b[i] = FixToFloatPoint(tmp);
#else
      b[i] = T(tmp);
#endif
    } else {
      // cout << "Not float!" << endl;
      b[i] = T(tmp);
    }
  }
}

template <typename T>
static inline void zk_decode_(const vector<uint64_t>& a, vector<T>& b, int scale_n = 1) {
  int size = a.size();
  b.resize(size);

  for (int i = 0; i < size; ++i) {
    auto tmp = int64_t(a[i]);
    if (a[i] > HALF_PR) {
      tmp = int64_t(a[i] - PR);
    }
    b[i] = ((T((int64_t)(tmp))) / (1L << (scale_n * ZK_F)));
  }
}

// only for Prover's IT-MAC
// If need_trunc is true, we perform another truncation at last
void zk_decode(const std::vector<__uint128_t>& a, std::vector<double>& b, bool need_trunc = false);
void zk_decode(const std::vector<ZkIntFp>& a, std::vector<double>& b, bool need_trunc = false);
void zk_decode(const ZkIntFp* a, size_t size, double* b, bool need_trunc = false);
void zk_decode(const ZkIntFp* a, size_t size, uint64_t* b);

// adapter for internal arith2bool<BoolIO<ZKNetIO>> with too large size.
inline void zk_arith2bool(const vector<ZkIntFp>& in, vector<Integer>& out, int size) {
  assert((in.size() == size));
  int ONCE_MAX = 50000; //99999;
  sync_zk_bool<BoolIO<ZKNetIO>>();
  // Note[GeorgeShi]: when the underlying need to re-generate some bits buffer,
  //                  this will be a little costly.
  auto start = clock_start();
  if (size <= ONCE_MAX) {
    arith2bool<BoolIO<ZKNetIO>>(out.data(), (IntFp*)in.data(), size);
    // for debuging:
    // uint64_t plain_fp_value = 0;
    // ZkIntFp tmp_v = in[0];
    // tmp_v.open(plain_fp_value);
    // log_debug << "Fp :" << plain_fp_value << "--> Boolean Int:" << out[0].reveal<uint64_t>(PUBLIC);
  } else {
    out.clear();
    vector<Integer> curr_iter_out(ONCE_MAX);
    int iters = floor(size / (ONCE_MAX * 1.0));
    for (int i = 0; i < iters; ++i) {
      log_debug << i << "-th iter arith2bool<BoolIO<ZKNetIO>> [" << i * ONCE_MAX << ", "
                << (i + 1) * ONCE_MAX << ")";
      vector<ZkIntFp> curr_iter_in(in.cbegin() + i * ONCE_MAX, in.cbegin() + (i + 1) * ONCE_MAX);
      curr_iter_out.clear();
      curr_iter_out.resize(ONCE_MAX);
      arith2bool<BoolIO<ZKNetIO>>(curr_iter_out.data(), (IntFp*)curr_iter_in.data(), ONCE_MAX);
      out.insert(out.end(), curr_iter_out.begin(), curr_iter_out.end());
      sync_zk_bool<BoolIO<ZKNetIO>>();
      // for debuging:
      // uint64_t plain_fp_value = 0;
      // ZkIntFp tmp_v = curr_iter_in[0];
      // tmp_v.open(plain_fp_value);
      // log_debug << "Fp :" << plain_fp_value << "--> Boolean Int:" << curr_iter_out[0].reveal<uint64_t>(PUBLIC);
    }
    int leftover_num = size - iters * ONCE_MAX;
    if (leftover_num > 0) {
      log_debug << "remaining arith2bool<BoolIO<ZKNetIO>> [" << iters * ONCE_MAX << ", " << size
                << ")";
      vector<ZkIntFp> curr_iter_in(in.cbegin() + iters * ONCE_MAX, in.cend());
      curr_iter_out.clear();
      curr_iter_out.resize(leftover_num);
      arith2bool<BoolIO<ZKNetIO>>(curr_iter_out.data(), (IntFp*)curr_iter_in.data(), leftover_num);
      out.insert(out.end(), curr_iter_out.begin(), curr_iter_out.end());
      sync_zk_bool<BoolIO<ZKNetIO>>();
      // for debuging:
      // uint64_t plain_fp_value = 0;
      // ZkIntFp tmp_v = curr_iter_in[0];
      // tmp_v.open(plain_fp_value);
      // log_debug << "Fp :" << plain_fp_value << "--> Boolean Int:" << curr_iter_out[0].reveal<uint64_t>(PUBLIC);
    }
  }
  sync_zk_bool<BoolIO<ZKNetIO>>();
  auto switch_time = time_from(start);
  log_debug << "batch arith2bool<BoolIO<ZKNetIO>> ok, costing " << switch_time / 1000
            << " for party " << zk_party_id << ENDL;
}

/**
  Prover should prove to verifier that:
    a != 0
  in ZK style.
  (return 0 mean success, otherwise is fail)
*/
int _proof_and_check_nonzero(const std::vector<ZkIntFp>& a);

/**
  Prover should prove to verifier that:
    a >= 0
  in ZK style.
  (return 0 mean success, otherwise is fail)
*/
int _proof_and_check_non_neg(const std::vector<ZkIntFp>& a);

/**
  Prover should prove to verifier that:
    a <= 0
  in ZK style.
  (return 0 mean success, otherwise is fail)
*/
int _proof_and_check_non_pos(const std::vector<ZkIntFp>& a);

/*
  Prover should prove to verifier that:
    abs(a-a_prime) < diff_bound
  in ZK style.
  @note: This is accomplished  by checking [abs(a-prime) - diff_bound] \in [0, diff_bound)  \bigcup (PR-diff_bound, PR] within field F_p.
  (return 0 mean success, otherwise is fail)
*/
int _inner_proof_and_check(
  const std::vector<ZkIntFp>& a,
  const std::vector<ZkIntFp>& a_prime,
  uint64_t diff_bound = _FLOAT_SCALE_);
int _inner_proof_and_check(
  const ZkIntFp* a,
  const ZkIntFp* a_prime,
  size_t size,
  uint64_t diff_bound = _FLOAT_SCALE_);

/*
Prover should prove to verifier that:
 [c] = [a] / [b]
by checking : abs([a]*2^s - [b][c]) \le abs([b]) in ZK style.
*/
int _inner_div_proof_and_check(
  const std::vector<ZkIntFp>& a,
  const std::vector<ZkIntFp>& b,
  const std::vector<ZkIntFp>& c);

/*
  Prover should prove to verifier that:
    |a * a_inv - 2^{2s}| < |a| && a != 0
  in ZK style.
  (return 0 mean success, otherwise is fail)
*/
int _proof_and_check_inv(const std::vector<ZkIntFp>& a, const std::vector<ZkIntFp>& a_inv);

/*
  Prover should prove to verifier that:
    c = \sqrt{a}
  in ZK style 
  (return 0 mean success, otherwise is fail)
*/
int _inner_sqrt_proof_and_check(const std::vector<ZkIntFp>& a, const std::vector<ZkIntFp>& c);

static inline std::ostream& operator<<(std::ostream& os, const __uint128_t& mac_v) {
  uint64_t val = uint64_t(mac_v >> 64);
  uint64_t mac = uint64_t(mac_v & 0xFFFFFFFFFFFFFFFFULL);
  os << "[" << val << "," << mac << "]";
  return os;
}

// element-wise Maximum operation
// Attention: in_a and out_c can NOT refer to same value
// Attention! Please call sync_zk_bool<BoolIO<ZKNetIO>>() before and after calling this function!
static inline void zk_max(
  const std::vector<ZkIntFp>& in_a,
  const std::vector<ZkIntFp>& in_b,
  std::vector<ZkIntFp>& out_c) {
  int vec_size = in_a.size();
  assert(vec_size == in_b.size());
  out_c.clear();
  out_c.resize(vec_size);
  // sync_zk_bool<BoolIO<ZKNetIO>>();
  vector<Integer> bin_a(vec_size); //, Integer(ZK_INT_LEN, 0, PUBLIC));
  vector<Integer> bin_b(vec_size); //, Integer(ZK_INT_LEN, 0, PUBLIC));
  vector<Integer> bin_c(vec_size);
  arith2bool<BoolIO<ZKNetIO>>(bin_a.data(), (IntFp*)in_a.data(), vec_size);
  // sync_zk_bool<BoolIO<ZKNetIO>>();
  arith2bool<BoolIO<ZKNetIO>>(bin_b.data(), (IntFp*)in_b.data(), vec_size);
  // sync_zk_bool<BoolIO<ZKNetIO>>();
  Integer NEG_SIGN(ZK_INT_LEN, HALF_PR + 1, PUBLIC);
  Integer tmp_max_one;
  Integer tmp_max_two;
  Bit sign_a;
  Bit sign_b;
  Bit compare_sign;
  Bit a_and_b_same_sign;

  for (auto i = 0; i < vec_size; ++i) {
    sign_a = bin_a[i].geq(NEG_SIGN);
    sign_b = bin_b[i].geq(NEG_SIGN);
    a_and_b_same_sign = (sign_a ^ sign_b);
    // case 1, if a and b have same sign, we get the larger encoded one.
    compare_sign = bin_b[i].geq(bin_a[i]);
    tmp_max_one = bin_a[i].select(compare_sign, bin_b[i]);
    // case 2. if a and b have different sign, and a has positive sign, we choose a.
    tmp_max_two = bin_a[i].select(sign_a, bin_b[i]);
    // choose case based on sign-XOR
    bin_c[i] = tmp_max_one.select(a_and_b_same_sign, tmp_max_two);
  }
  // sync_zk_bool<BoolIO<ZKNetIO>>();
  bool2arith<BoolIO<ZKNetIO>>(out_c.data(), bin_c.data(), vec_size);
  // sync_zk_bool<BoolIO<ZKNetIO>>();
}

static inline string get_attr_value(const attr_type* attr, string tag, string default_value) {
  if (attr && attr->count(tag) > 0)
    default_value = attr->at(tag);
  return default_value;
}

static inline int get_attr_value(const attr_type* attr, string tag, int default_value) {
  string val = get_attr_value(attr, tag, to_string(default_value));
  return std::stoi(val);
}

// Note[GeorgeShi]: This does NOT apply to float-converted Value.
static inline void field_truncation(const vector<uint64_t>& vec_a, vector<uint64_t>& vec_out) {
  uint64_t U = (1LL << (ZK_K - 1));
  int size = vec_a.size();
  vec_out.clear();
  vec_out.resize(size);
  // 2^{-M}
  uint64_t temp = uint64_t(1ULL << (MERSENNE_PRIME_EXP - ZK_M));

  for (auto i = 0; i < size; ++i) {
    vec_out[i] = mod(U + vec_a[i]);
    uint64_t b1 = common_mod(vec_out[i], 1 << ZK_M);
    b1 = common_mod(vec_a[i] - b1, PR);
    cout << "b1:" << b1 << endl;
    vec_out[i] = mult_mod(b1, temp);
  }
}

// Extended Euclid Algorithm in Z_p
// get a * `inverse_a` + `placeholder_p` * p = gcd(a, p), i.e. the returned value, which should be '1'
// This implementation is borrowed from https://cp-algorithms.com/algebra/extended-euclid-algorithm.html
static inline uint64_t egcd(
  const uint64_t& a,
  const uint64_t& p,
  uint64_t& inverse_a,
  uint64_t& placeholder_p) {
  inverse_a = 1;
  placeholder_p = 0;
  uint64_t x1 = 0, y1 = 1, a1 = a, b1 = p;
  while (b1) {
    uint64_t q = floor(a1 / b1);
    tie(inverse_a, x1) = make_tuple(x1, inverse_a - q * x1);
    tie(placeholder_p, y1) = make_tuple(y1, placeholder_p - q * y1);
    tie(a1, b1) = make_tuple(b1, a1 - q * b1);
  }
  return a1;
}

} // namespace zk
} // namespace rosetta
