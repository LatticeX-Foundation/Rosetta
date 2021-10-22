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
#include "cc/modules/protocol/zk/mystique/include/wvr_util.h"

#include "cc/modules/protocol/zk/mystique/include/zk_int_fp.h"
#include "cc/modules/protocol/zk/mystique/include/zk_int_fp_eigen.h"
#include <iostream>
#include "emp-tool/emp-tool.h"

#include <iostream>

#include "cc/modules/protocol/zk/mystique/include/mystique_ops_impl.h"
#include "cc/modules/protocol/zk/mystique/include/mystique_plain_internal.h"
#include "cc/modules/protocol/zk/mystique/include/zk_uint64_t.h"

#include <stdexcept>
#include <string>
#include <vector>
#include <iomanip>
using namespace std;

using namespace emp;
using emp::block;
using rosetta::zk::ZkIntFp;



int zk_party_id;
#include <atomic>
extern std::atomic<int64_t> convert_2_str_time;
extern std::atomic<int64_t> convert_2_mac_time;
namespace rosetta {
namespace zk {

// prefer to use the ZkIntFp one.
int convert_string_to_mac(
  const std::vector<std::string>& a,
  std::vector<__uint128_t>& b,
  bool human,
  void* wvr_impl) {
  SimpleTimer timer;
  size_t size = a.size();
  b.clear();
  b.resize(size);
  if(size == 0) {
    return -1;
  }

  int scale_multiplier = convert_string_to_mac(a.data(), b.data(), size, human, wvr_impl);
  convert_2_mac_time += timer.ns_elapse();
  return scale_multiplier;
}

// This is the preferred one.
int convert_string_to_mac(
  const std::vector<std::string>& a,
  std::vector<ZkIntFp>& b,
  bool human,
  void* wvr_impl) {
  SimpleTimer timer;
  size_t size = a.size();
  b.clear();
  b.resize(size);
  if(size == 0) {
    return -1;
  }

  int scale_multiplier = convert_string_to_mac(a.data(), (__uint128_t*)b.data(), size, human, wvr_impl);
  convert_2_mac_time += timer.ns_elapse();
  return scale_multiplier;
}


// tmp for local simulation
int convert_string_to_mac(const std::string* a, ZkUint64* b, size_t size, bool human) {
  // defult as 1, meaning Fp is normally scaled.
  int scale_multiplier = 1;
  if (human) {
    for(auto i = 0; i < size; ++i) {
      if (a[i].length() == sizeof(ZkIntFp)*2 + 1 && a[i].back() == '#') {// # private input or secure compute output string
        uint64_t val = from_hex_str<uint64_t>(std::string(a[i].begin(), a[i].begin() + 2 * sizeof(uint64_t)));
        b[i].value = val;
      } else if (a[i].length() == sizeof(ZkIntFp)*2 + 1 && a[i].back() == '+') {
        uint64_t val = from_hex_str<uint64_t>(std::string(a[i].begin(), a[i].begin() + 2 * sizeof(uint64_t)));
        b[i].value = val;
        scale_multiplier = 2;
      } else if (a[i].length() == sizeof(double) + 1 && a[i].back() == '$') {
        vector<double> d(1,0);
        log_error << "encoding plain float-inout local simulate A!";
        memcpy((char*)&d[0], a[i].c_str(), sizeof(double));
        zk_encode_fp(d.data(), 1, &b[i].value);
      }
      else {
        vector<double> d(1,0);
        log_error << "encoding plain float-inout local simulate B!";
        d[0] = to_double(a[i].c_str());
        zk_encode_fp(d.data(), 1, &b[i].value);
      }
    }
  } else {// binary values
    const static size_t fix_binary_len = sizeof(uint64_t) + 1;
    for(auto i = 0; i < size; ++i) {
      if (a[i].length() == fix_binary_len && a[i].back() == '#') {// # private input or secure compute output string
        memcpy(&b[i].value, a[i].data(), fix_binary_len-1);
      } else if (a[i].length() == fix_binary_len && a[i].back() == '+') {
        memcpy(&b[i].value, a[i].data(), fix_binary_len-1);
        scale_multiplier = 2;
      } else if (a[i].length() == sizeof(double) + 1 && a[i].back() == '$') {// const input
        vector<double> d(1,0);
        log_error << "encoding plain float-inout local simulate C!";
        memcpy((char*)&d[0], a[i].c_str(), sizeof(double));
        zk_encode_fp(d.data(), 1, &b[i].value);
      } else {
        vector<double> d(1,0);
        // log_error << "encoding plain float-inout local simulate D!";
        d[0] = to_double(a[i].c_str());
        zk_encode_fp(d.data(), 1, &b[i].value);
      }
    }
  }
  return scale_multiplier;
}
int convert_string_to_mac(const std::string* a, uint64_t* b, size_t size, bool human) {
  // defult as 1, meaning Fp is normally scaled.
  int scale_multiplier = 1;
  if (human) {
    for(auto i = 0; i < size; ++i) {
      if (a[i].length() == sizeof(ZkIntFp)*2 + 1 && a[i].back() == '#') {// # private input or secure compute output string
        uint64_t val = from_hex_str<uint64_t>(std::string(a[i].begin(), a[i].begin() + 2 * sizeof(uint64_t)));
        b[i] = val;
      } else if (a[i].length() == sizeof(ZkIntFp)*2 + 1 && a[i].back() == '+') {
        uint64_t val = from_hex_str<uint64_t>(std::string(a[i].begin(), a[i].begin() + 2 * sizeof(uint64_t)));
        b[i] = val;
        scale_multiplier = 2;
      } else if (a[i].length() == sizeof(double) + 1 && a[i].back() == '$') {
        vector<double> d(1,0);
        log_error << "encoding plain float-inout local simulate A!";
        memcpy((char*)&d[0], a[i].c_str(), sizeof(double));
        zk_encode_fp(d.data(), 1, &b[i]);
      }
      else {
        vector<double> d(1,0);
        log_error << "encoding plain float-inout local simulate B!";
        d[0] = to_double(a[i].c_str());
        zk_encode_fp(d.data(), 1, &b[i]);
      }
    }
  } else {// binary values
    const static size_t fix_binary_len = sizeof(uint64_t) + 1;
    for(auto i = 0; i < size; ++i) {
      if (a[i].length() == fix_binary_len && a[i].back() == '#') {// # private input or secure compute output string
        memcpy(&b[i], a[i].data(), fix_binary_len-1);
      } else if (a[i].length() == fix_binary_len && a[i].back() == '+') {
        memcpy(&b[i], a[i].data(), fix_binary_len-1);
        scale_multiplier = 2;
      } else if (a[i].length() == sizeof(double) + 1 && a[i].back() == '$') {// const input
        vector<double> d(1,0);
        log_error << "encoding plain float-inout local simulate C!";
        memcpy((char*)&d[0], a[i].c_str(), sizeof(double));
        zk_encode_fp(d.data(), 1, &b[i]);
      } else {
        vector<double> d(1,0);
        // log_error << "encoding plain float-inout local simulate D!";
        d[0] = to_double(a[i].c_str());
        zk_encode_fp(d.data(), 1, &b[i]);
      }
    }
  }
  return scale_multiplier;
}

int convert_string_to_mac(
  const std::string* a, 
  __uint128_t* b, 
  size_t size,
  bool human, 
  void* wvr_impl) {
  if(size == 0) {
    throw std::runtime_error("empty input to convert !");
  }
  // defult as 1, meaning Fp is normally scaled.
  int scale_multiplier = 1;
  if (human) {
    for(auto i = 0; i < size; ++i) {
      if (a[i].length() == sizeof(ZkIntFp)*2 + 1 && a[i].back() == '#') {// # private input or secure compute output string
        uint64_t val = from_hex_str<uint64_t>(std::string(a[i].begin(), a[i].begin() + 2 * sizeof(uint64_t)));
        uint64_t mac = from_hex_str<uint64_t>(std::string(a[i].begin() + 2 * sizeof(uint64_t), a[i].begin() + 4 * sizeof(uint64_t)));
        b[i] = (__uint128_t)makeBlock(val, mac);
      } else if (a[i].length() == sizeof(ZkIntFp)*2 + 1 && a[i].back() == '+') {
        uint64_t val = from_hex_str<uint64_t>(std::string(a[i].begin(), a[i].begin() + 2 * sizeof(uint64_t)));
        uint64_t mac = from_hex_str<uint64_t>(std::string(a[i].begin() + 2 * sizeof(uint64_t), a[i].begin() + 4 * sizeof(uint64_t)));
        b[i] = (__uint128_t)makeBlock(val, mac);
        scale_multiplier = 2;
      } else if (a[i].length() == sizeof(double) + 1 && a[i].back() == '$') {
        double d(0);
        memcpy((char*)&d, a[i].c_str(), sizeof(double));
        b[i] = ZkIntFp(d, PUBLIC).value;
      }
      else {
        b[i] = ZkIntFp(to_double(a[i].c_str()), PUBLIC).value;

      }
    }
  } else {// binary values
    const static size_t fix_binary_len = sizeof(uint64_t)*2 + 1;
    for(auto i = 0; i < size; ++i) {
      if (a[i].length() == fix_binary_len && a[i].back() == '#') {// # private input or secure compute output string
        memcpy(&b[i], a[i].data(), fix_binary_len-1);
      } else if (a[i].length() == fix_binary_len && a[i].back() == '+') {
        memcpy(&b[i], a[i].data(), fix_binary_len-1);
        scale_multiplier = 2;
      } else if (a[i].length() == sizeof(double) + 1 && a[i].back() == '$') {// const input
        double d(0);
        memcpy((char*)&d, a[i].c_str(), sizeof(double));
        b[i] = ZkIntFp(d, PUBLIC).value;
      } else {
        b[i] = ZkIntFp(to_double(a[i].c_str()), PUBLIC).value;
      }
    }
  }
  return scale_multiplier;
}

// tmp for local simulation
void convert_mac_to_string(
  const vector<uint64_t>& a, 
  std::vector<std::string>& b, 
  bool human,
  int scale_multiplier) {
  convert_mac_to_string(a.data(), b, a.size(), human, scale_multiplier);
}

void convert_mac_to_string(
  const uint64_t* a, 
  std::vector<std::string>& b, 
  size_t size,
  bool human,
  int scale_multiplier) {
  SimpleTimer timer;
  b.resize(size);
  char fp_postfix = '#';
  if(scale_multiplier == 2) {
    fp_postfix = '+';
  }  else if(scale_multiplier > 2) {
    throw std::runtime_error("scale_multiplier should not be :!" + std::to_string(scale_multiplier));
  }

  if(human) {
    string tmp_s = "";
    uint64_t val_mac[2];
    for(auto i = 0; i < size; ++i) {
      tmp_s += get_hex_str(val_mac[0]);
      tmp_s.push_back(fp_postfix);
      b[i] = tmp_s;
    }
  } else {
    log_debug << " mac to string in local simulation No humanable debug stub " << ENDL;
#if !LOCAL_SIMULATE_V2
    for(auto i = 0; i < size; ++i) {
      b[i].resize(sizeof(uint64_t) + 1, fp_postfix);
      memcpy((char*)b[i].data(), &a[i], sizeof(a[i]));
    }
#else
    vector<ZkIntFp> azk(size);
    plain_batch_feed(azk.data(), a, size);
    for(auto i = 0; i < size; ++i)
    {
      b[i].resize(sizeof(azk[i]) + 1, fp_postfix);
      memcpy((char*)b[i].data(), &azk[i], sizeof(azk[i]));
    }
#endif
  }
  convert_2_str_time += timer.ns_elapse();
}

// tmp for local simulation
void convert_mac_to_string(
  const vector<ZkUint64>& a, 
  std::vector<std::string>& b, 
  bool human,
  int scale_multiplier) {
  convert_mac_to_string((uint64_t*)a.data(), b, a.size(), human, scale_multiplier);
}

void convert_mac_to_string(
  const __uint128_t* a, 
  std::vector<std::string>& b, 
  size_t size,
  bool human,
  int scale_multiplier) {
  SimpleTimer timer;
  b.resize(size);
  char fp_postfix = '#';
  if(scale_multiplier == 2) {
    fp_postfix = '+';
  }  else if(scale_multiplier > 2) {
    throw std::runtime_error("scale_multiplier should not be :!" + std::to_string(scale_multiplier));
  }

  if(human) {
    string tmp_s = "";
    uint64_t val_mac[2];
    for(auto i = 0; i < size; ++i) {
      tmp_s.clear();
      // for prover:   [x, M_x]
      // for verifier: [0, K_x]
      val_mac[0] = uint64_t(a[i] >> 64);
      val_mac[1] = uint64_t(a[i] & 0xFFFFFFFFFFFFFFFFULL);
      {
        tmp_s += get_hex_str(val_mac[0]);
        tmp_s += get_hex_str(val_mac[1]);
        tmp_s.push_back(fp_postfix);
      }
      b[i] = tmp_s;
    }
  } else {
    
    for(auto i = 0; i < size; ++i) {
      b[i].resize(sizeof(uint64_t)*2+1, fp_postfix);
      memcpy((char*)b[i].data(), &a[i], sizeof(a[i]));
    }
  }
  convert_2_str_time += timer.ns_elapse();
}


void convert_mac_to_string(
  const ZkIntFp* a,
  std::vector<std::string>& b,
  size_t size,
  bool human,
  int scale_multiplier) {
  convert_mac_to_string((const __uint128_t*)a, b, size, human, scale_multiplier);
}

void convert_mac_to_string(
  const vector<ZkIntFp>& a,
  std::vector<std::string>& b,
  bool human,
  int scale_multiplier) {
  convert_mac_to_string(a.data(), b, a.size(), human, scale_multiplier);
}

void zk_decode(const std::vector<__uint128_t>& a, std::vector<double>& b, bool need_trunc) {
  b.clear();
  b.resize(a.size());
  int64_t val = 0;
  for (auto i = 0; i < a.size(); ++i) {
    uint64_t u_val = uint64_t(a[i] >> 64);
    if( u_val > HALF_PR) {
      val = int64_t(u_val - PR);
    } else {
      val = int64_t(u_val);
    }
    // convert fix-point to float-point number
    log_debug << "u128 decoding" << ENDL;

    #if SUPPORT_FLOAT_NUMBER
      b[i] = FixToFloatPoint(val);
      if(need_trunc) {
        log_debug << "Mul need trunc again" << ENDL;
        b[i] = b[i ] / (1L << ZK_F);
      }
    #else
      b[i] = double(val);
    #endif
  }
}

void zk_decode(const std::vector<ZkIntFp>& a, std::vector<double>& b, bool need_trunc) {
  b.clear();
  b.resize(a.size());
  int64_t val = 0;
  for (auto i = 0; i < a.size(); ++i) {
    uint64_t u_val = uint64_t(a[i].value >> 64);
    if( u_val > HALF_PR) {
      val = int64_t(u_val - PR);
    } else {
      val = int64_t(u_val);
    }
    // convert fix-point to float-point number
    log_debug << "u128 decoding" << ENDL;

    #if SUPPORT_FLOAT_NUMBER
      b[i] = FixToFloatPoint(val);
      if(need_trunc) {
        log_debug << "Mul need trunc again" << ENDL;
        b[i] = b[i ] / (1L << ZK_F);
      }
    #else
      b[i] = double(val);
    #endif
  }

}

void zk_decode(const ZkIntFp* a, size_t size, double* b, bool need_trunc) {
  int64_t val = 0;
  for (auto i = 0; i < size; ++i) {
    uint64_t u_val = uint64_t(a[i].value >> 64);
    if(u_val > HALF_PR) {
      val = int64_t(u_val - PR);
    } else {
      val = int64_t(u_val);
    }
    // convert fix-point to float-point number
    log_debug << "u128 decoding" << ENDL;

    #if SUPPORT_FLOAT_NUMBER
      b[i] = FixToFloatPoint(val);
      if(need_trunc) {
        log_debug << "Mul need trunc again" << ENDL;
        b[i] = b[i] / (1L << ZK_F);
      }
    #else
      b[i] = double(val);
    #endif
  }
}

void zk_decode(const ZkIntFp* a, size_t size, uint64_t* b) {
  for (size_t i = 0; i < size; ++i) {
    b[i] = uint64_t(a[i].value >> 64);
  }
}


/**
  Prover should prove to verifier that:
    abs(a-a_prime) < diff_bound
  in ZK style.
  @note: This is accomplished  by checking [abs(a-prime) - diff_bound] \in [0, diff_bound)  \bigcup (PR-diff_bound, PR] within field F_p.
*/

int _inner_proof_and_check(const std::vector<ZkIntFp>& a, const std::vector<ZkIntFp>& a_prime, uint64_t diff_bound) {
  return _inner_proof_and_check(a.data(), a_prime.data(), a.size(), diff_bound);
}

int _inner_proof_and_check(const ZkIntFp* a, const ZkIntFp* a_prime, size_t size, uint64_t diff_bound) {
  // 1. get d = abs(a - a_prime)
  auto start = clock_start();
  vector<ZkIntFp> field_diff(size);
  for(int i = 0; i < size; ++i) {
    field_diff[i] = a[i] - a_prime[i];
  }
  
  vector<Integer> bin_diff(size, Integer(ZK_INT_LEN, 0, PUBLIC));
  // zk_arith2bool(field_diff, bin_diff, size);
  sync_zk_bool<BoolIO<ZKNetIO>>();
  // Note[GeorgeShi]: when the underlying need to re-generate some bits buffer,
  //                  this will be a little costly.
  log_debug << "begin arith2bool<BoolIO<ZKNetIO>> for ZK MUL with size:" << size;
  arith2bool<BoolIO<ZKNetIO>>(bin_diff.data(), (IntFp*)field_diff.data(), size);
  // sync_zk_bool<BoolIO<ZKNetIO>>();
  auto switch_time = time_from(start);
  log_debug << "arith2bool<BoolIO<ZKNetIO>> for ZK MUL ok, costing " << switch_time/1000 << " for party " << zk_party_id << ENDL;
  
  Integer POS_DIFF_BOUND(ZK_INT_LEN, diff_bound, PUBLIC);
  Integer NEG_DIFF_BOUND(ZK_INT_LEN, PR - diff_bound, PUBLIC);
  // Note that default construction of Bit is Public false.
  vector<Bit> pos_ge_res_bin(size);
  vector<Bit> neg_ge_res_bin(size);
  vector<Bit> res_bin(size);
  Bit all_res;
  log_debug << "try to Proof ZK MUL in batch-style" << ENDL;
  for (size_t i = 0; i < size; ++i) {
    // Because we have encoded the negative float-point values outside, 
    // we check the error difference in [0, bound), or (PR-bound,PR].
    // bin_diff[i] = bin_diff[i].abs();
    pos_ge_res_bin[i] = bin_diff[i].geq(POS_DIFF_BOUND); // should be 0, or
    neg_ge_res_bin[i] = bin_diff[i].geq(NEG_DIFF_BOUND); // should be 1
    res_bin[i] =  pos_ge_res_bin[i] & !neg_ge_res_bin[i];  // !pos_ge_res_bin[i] | neg_ge_res_bin[i];
    all_res = all_res | res_bin[i]; // OR of all zeros should be zero. 0|0|0
  }
  // sync_zk_bool<BoolIO<ZKNetIO>>();
  bool batch_mul_check_res = !all_res.reveal<bool>(PUBLIC);
  if(!batch_mul_check_res) {
      log_error << "Fail to batch proof ZK Mul with detailed context:";
      for (int i = 0; i < size; ++i) {
        bool curr_res = !res_bin[i].reveal<bool>(PUBLIC);
        if(!curr_res) {
          log_error <<  i << "-th ZK Mul fail!:";
          ZkIntFp tmp_v = a[i];
          uint64_t plain_fp_value = 0; 
          tmp_v.open(plain_fp_value);
          log_error << " input a:" << plain_fp_value;
          tmp_v = a_prime[i];
          tmp_v.open(plain_fp_value);
          log_error << " input a_prime:" << plain_fp_value;
          log_error << "expected bound:" << diff_bound;
          tmp_v = field_diff[i];
          tmp_v.open(plain_fp_value);
          log_error << "with difference in Boolean:" << bin_diff[i].reveal<uint64_t>(PUBLIC);
          log_error << "with difference in Fp:" << plain_fp_value;
        }
      }
      throw std::runtime_error("CHECK ZK MUL FAIL!");
  }
  sync_zk_bool<BoolIO<ZKNetIO>>();
  auto reveal_time = time_from(start);
  log_debug << "check ZK MUL DONE, With time for batch check: " << reveal_time / 1000 << " for party: " << zk_party_id << " " << ENDL;

  return 0;
}

int _inner_div_proof_and_check(const std::vector<ZkIntFp>& a_zks, const std::vector<ZkIntFp>& b_zks, const std::vector<ZkIntFp>& c_zks) {
  int size = a_zks.size();
  vector<ZkIntFp> a_scaled_zks(size);
  vector<ZkIntFp> bc_zks(size);
  vector<ZkIntFp> field_diff(size);
  vector<ZkIntFp> neg_b_zks(size);
  for(int i = 0; i < size; ++i) {
		// step 4.1: calc 2^s * a
    a_scaled_zks[i] = a_zks[i] * _FLOAT_SCALE_;
    // step 4.2: calc b * c
    bc_zks[i] = b_zks[i] * c_zks[i];
    field_diff[i] = a_scaled_zks[i] - bc_zks[i];
    // encoded as PR-b
    neg_b_zks[i] = -b_zks[i];
    //cerr << i << "-th proof data: " << (__uint128_t)(a_scaled_zks[i].value >> 64) << " VS " <<  (__uint128_t)(bc_zks[i].value >> 64);
  }
  a_scaled_zks.clear();
  bc_zks.clear();

  vector<Integer> bin_diff(size, Integer(ZK_INT_LEN, 0, PUBLIC));
  vector<Integer> bin_b(size, Integer(ZK_INT_LEN, 0, PUBLIC));
  vector<Integer> bin_neg_b(size, Integer(ZK_INT_LEN, 0, PUBLIC));
  // to determine whether the original Fp in negative encoded value.
  Integer NEG_SIGN(ZK_INT_LEN, HALF_PR, PUBLIC);
  sync_zk_bool<BoolIO<ZKNetIO>>();
  // Note[GeorgeShi]: when the underlying need to re-generate some bits buffer,
  //                  this will be a little costly.
  auto start = clock_start();
  log_debug << "begin arith2bool<BoolIO<ZKNetIO>> for ZK DIV";
  arith2bool<BoolIO<ZKNetIO>>(bin_diff.data(), (IntFp*)field_diff.data(), size);
  arith2bool<BoolIO<ZKNetIO>>(bin_b.data(), (IntFp*)b_zks.data(), size);
  arith2bool<BoolIO<ZKNetIO>>(bin_neg_b.data(), (IntFp*)neg_b_zks.data(), size);
  // sync_zk_bool<BoolIO<ZKNetIO>>();
  auto switch_time = time_from(start);
  log_debug << "arith2bool<BoolIO<ZKNetIO>> for ZK DIV ok, costing " << switch_time/1000 << " for party " << zk_party_id << ENDL;
  
  // Note that default construction of Bit is Public false.
  // Note: only batch-style for now.
  Bit temp_sign;
  Bit pos_ge_res_bin;
  Bit neg_ge_res_bin;
  Bit all_res;
  log_debug << "try to Proof MUL in batch-style" << ENDL;
  for (size_t i = 0; i < size; ++i) {
    // Because we have encoded the negative float-point values outside, 
    // we check the error difference in [0, |b|), or (PR-|b|,PR].
    // log_debug << "Here stub";
    //1. if b is positive
    temp_sign = bin_b[i].geq(NEG_SIGN);
    pos_ge_res_bin = bin_diff[i].geq(bin_b[i]) & (!temp_sign);
    pos_ge_res_bin = (!bin_diff[i].geq(bin_neg_b[i])) & pos_ge_res_bin;
    //2. if b is negative
    neg_ge_res_bin = (bin_diff[i].geq(bin_neg_b[i])) & temp_sign;
    neg_ge_res_bin = (!bin_diff[i].geq(bin_b[i])) & neg_ge_res_bin;
    // should be 0, or
    all_res = all_res | (pos_ge_res_bin | neg_ge_res_bin); // OR of all zeros should be zero. 0|0|0
  }
  // sync_zk_bool<BoolIO<ZKNetIO>>();
  bool batch_mul_check_res = !all_res.reveal<bool>(PUBLIC);
  if(!batch_mul_check_res) {
      log_error << "Fail to batch proof ZK DIV with context:";
      for (int i = 0; i < size; ++i) {
        log_error <<  i << "-th ZK DIV, with difference "
            << bin_diff[i].reveal<uint64_t>(PUBLIC);
        log_error << "while target Upper bound diff:" << bin_b[i].reveal<uint64_t>(PUBLIC);
      }
      throw std::runtime_error("CHECK ZK DIV FAIL!");
  }
  sync_zk_bool<BoolIO<ZKNetIO>>();
  auto reveal_time = time_from(start);
  log_debug << "Proof ZK DIV DONE, With time for batch check: " << reveal_time / 1000 << " for party: " << zk_party_id << " " << ENDL;
  return 0;
}

int _proof_and_check_inv(const std::vector<ZkIntFp>& a, const std::vector<ZkIntFp>& a_inv) {
  // 1. get res_zks = a * a_inv - 2^{2s} < abs(a)
  int size = a.size();
  ZkIntFp TWO_SCALE(_TWO_FLOAT_SCALE_, PUBLIC);
  vector<ZkIntFp> res_zks(size);
  vector<ZkIntFp> neg_a_zks(size);
  for(int i = 0; i < size; ++i) {
    res_zks[i] = a[i] * a_inv[i] - TWO_SCALE;
    neg_a_zks[i] = -a[i];
  }

  vector<Integer> bin_diff(size, Integer(ZK_INT_LEN, 0, PUBLIC));
  vector<Integer> bin_a(size, Integer(ZK_INT_LEN, 0, PUBLIC));
  vector<Integer> bin_neg_a(size, Integer(ZK_INT_LEN, 0, PUBLIC));
  // to determine whether the original Fp in negative encoded value.
  Integer NEG_SIGN(ZK_INT_LEN, HALF_PR, PUBLIC);

  sync_zk_bool<BoolIO<ZKNetIO>>();
  auto start = clock_start();
  log_debug << "begin arith2bool<BoolIO<ZKNetIO>> for ZK INV";
  arith2bool<BoolIO<ZKNetIO>>(bin_diff.data(), (IntFp*)res_zks.data(), size);
  arith2bool<BoolIO<ZKNetIO>>(bin_a.data(), (IntFp*)a.data(), size);
  arith2bool<BoolIO<ZKNetIO>>(bin_neg_a.data(), (IntFp*)neg_a_zks.data(), size);
  // sync_zk_bool<BoolIO<ZKNetIO>>();
  auto switch_time = time_from(start);
  log_debug << "arith2bool<BoolIO<ZKNetIO>> for ZK INV ok, costing " << switch_time/1000 << " for party" << zk_party_id << ENDL;

  Bit temp_sign;
  Bit pos_ge_res_bin;
  Bit neg_ge_res_bin;
  Bit all_res;
  log_debug << "try to Proof INV in batch-style" << ENDL;
  for (size_t i = 0; i < size; ++i) {
    // Because we have encoded the negative float-point values outside, 
    // we check the error difference in [0, |a|), or (PR-|a|,PR].
    // log_debug << "debug stub check inv abs";
    temp_sign = bin_a[i].geq(NEG_SIGN); // true means a is negative
    pos_ge_res_bin = bin_diff[i].geq(bin_a[i]) & (!temp_sign);
    pos_ge_res_bin = (!bin_diff[i].geq(bin_neg_a[i])) & pos_ge_res_bin; 
  
    neg_ge_res_bin = !bin_diff[i].geq(bin_a[i]) & temp_sign;
    neg_ge_res_bin = (bin_diff[i].geq(bin_neg_a[i])) & neg_ge_res_bin;
    // should be 0, or
    all_res = all_res | (pos_ge_res_bin | neg_ge_res_bin); // OR of all zeros should be zero. 0|0|0
  }

  // 2. check d < diff_bound
  // sync_zk_bool<BoolIO<ZKNetIO>>();
  bool batch_inv_check_res = !all_res.reveal<bool>(PUBLIC);
  // sync_zk_bool<BoolIO<ZKNetIO>>();
  if(!batch_inv_check_res) {
      log_error << batch_inv_check_res << ", Fail to batch proof ZK Invert with context:";
      for (int i = 0; i < size; ++i) {
        log_error <<  i << "-th ZK INV, with difference "
            << bin_diff[i].reveal<uint64_t>(PUBLIC);
        log_error << "while target Upper bound diff:" << bin_a[i].reveal<uint64_t>(PUBLIC);
      }
      throw std::runtime_error("CHECK ZK INVERT FAIL!");
  }

  sync_zk_bool<BoolIO<ZKNetIO>>();
  // 3. check a != 0
  _proof_and_check_nonzero(a);
  auto reveal_time = time_from(start);
  log_debug << "Proof ZK INV DONE, with time for batch check: " << reveal_time / 1000 << " for party: " << zk_party_id << " " << ENDL;
  return 0;
}

int _proof_and_check_nonzero(const std::vector<ZkIntFp>& a) {
  log_debug << "--> check non-zero..." << ENDL;

  int size = a.size();
  Integer Zero(ZK_INT_LEN, 0, PUBLIC);
  vector<Bit> zero_res_bin(size);
  vector<Integer> bin_a(size, Integer(ZK_INT_LEN, 0, PUBLIC));
  sync_zk_bool<BoolIO<ZKNetIO>>();
  arith2bool<BoolIO<ZKNetIO>>(bin_a.data(), (IntFp*)a.data(), size);
  auto start = clock_start();
  
  Bit all_res;
  for (size_t i = 0; i < size; ++i) {
    zero_res_bin[i] = bin_a[i].equal(Zero);
    all_res = all_res | zero_res_bin[i]; // should all be 0, 
  }
  bool nonzero_check = !all_res.reveal<bool>(PUBLIC);
  if (!nonzero_check) {
    log_error << "Batch non-zero check failed! with context:";
      for (int i = 0; i < size; ++i) {
        log_error <<  i << "-th element is zero? : "
            << zero_res_bin[i].reveal<bool>(PUBLIC);
      }
    throw std::runtime_error("CHECK NON-ZERO FAIL!");
  }
  auto reveal_time = time_from(start);  
  log_debug << "time for NON-ZERO check: " << reveal_time / 1000 << " for party: " << zk_party_id << " " << ENDL;
  log_debug << "check non-zero DONE!" << ENDL;
  sync_zk_bool<BoolIO<ZKNetIO>>();
  return 0;
}

/*
  Prover should prove to verifier that:
    [c] = [\sqrt{a}]
  in ZK style 
  (return 0 mean success, otherwise is fail)
*/
int _inner_sqrt_proof_and_check(const std::vector<ZkIntFp>& a, const std::vector<ZkIntFp>& c) {
  log_debug << "--> check Sqrt..." << ENDL;
  int ret = 0;
  int size = a.size();
  // 1. checking c >= 0
  sync_zk_bool<BoolIO<ZKNetIO>>();
  ret = _proof_and_check_non_neg(c);
  sync_zk_bool<BoolIO<ZKNetIO>>();
  log_debug << " 1. checking c>=0 :" << (bool(1-ret)? "OK " : "FAIL");
  ZkIntFp ONE(uint64_t(1), PUBLIC);
  // 2. checking c^2 - a * 2^s <= 0,
  vector<ZkIntFp> check_diff_right(size);
  //    and 0 <= c^2 -a * 2^s + 2c + 1 
  vector<ZkIntFp> check_diff_left(size);
  for(int i = 0; i < size; ++i) {
    check_diff_right[i] = c[i] * c[i];
    check_diff_right[i] = check_diff_right[i] - a[i] * _FLOAT_SCALE_;
    check_diff_left[i] = check_diff_right[i] + ONE;
    check_diff_left[i] = check_diff_left[i] + c[i] * 2;
  }
  sync_zk_bool<BoolIO<ZKNetIO>>();
  ret = ret + _proof_and_check_non_pos(check_diff_right);
  log_debug << " 2. checking c^2 - a * 2^s <= 0 :" << (bool(1-ret)? "OK " : " FAIL ");
  ret = ret + _proof_and_check_non_neg(check_diff_left);
  sync_zk_bool<BoolIO<ZKNetIO>>();
  log_debug << " 3. checking 0 <= c^2 -a * 2^s + 2c + 1 :" << (bool(1-ret)? "OK " : " FAIL ");
  log_debug << "check SQRT result DONE!" << ENDL;
  return 0;
}


int _proof_and_check_non_neg(const std::vector<ZkIntFp>& a) {
  log_debug << "--> check non-negative..." << ENDL;

  int size = a.size();
  Integer Zero(ZK_INT_LEN, 0, PUBLIC);
  Integer MAXIMUM_PLUS_ONE(ZK_INT_LEN, HALF_PR + 1, PUBLIC);
  vector<Bit> zero_res_bin(size);
  vector<Integer> bin_a(size, Integer(ZK_INT_LEN, 0, PUBLIC));
  // sync_zk_bool<BoolIO<ZKNetIO>>();
  arith2bool<BoolIO<ZKNetIO>>(bin_a.data(), (IntFp*)a.data(), size);
  auto start = clock_start();
  
  Bit all_res(true);
  for (size_t i = 0; i < size; ++i) {
    zero_res_bin[i] = !bin_a[i].geq(MAXIMUM_PLUS_ONE);
    zero_res_bin[i] = zero_res_bin[i] | bin_a[i].equal(Zero);
    all_res = all_res & zero_res_bin[i]; // should all be 1 
  }
  bool nonzero_check = all_res.reveal<bool>(PUBLIC);
  if (!nonzero_check) {
    uint64_t curr_fp_value = 0;
    ZkIntFp curr_fp;
    log_error << "Batch non-negative check failed! with context:";
      for (int i = 0; i < size; ++i) {
        log_error <<  i << "-th element is non-negative? : "
            << zero_res_bin[i].reveal<bool>(PUBLIC);
        
        curr_fp = a[i];
        curr_fp.open(curr_fp_value);
        log_error << "Curr Diff: Fp :" << curr_fp_value << "--> Boolean Int:" << bin_a[i].reveal<uint64_t>(PUBLIC);
      }
    throw std::runtime_error("CHECK NON-NEG FAIL!");
  }
  auto reveal_time = time_from(start);  
  log_debug << "time for NON-NEG check: " << reveal_time / 1000 << " for party: " << zk_party_id << " " << ENDL;
  log_debug << "check non-negative DONE!" << ENDL;
  // sync_zk_bool<BoolIO<ZKNetIO>>();
  return 0;
}


int _proof_and_check_non_pos(const std::vector<ZkIntFp>& a) {
  log_debug << "--> check non-positive..." << ENDL;

  int size = a.size();
  Integer Zero(ZK_INT_LEN, 0, PUBLIC);
  Integer MINIMUM(ZK_INT_LEN, HALF_PR + 1, PUBLIC);
  vector<Bit> zero_res_bin(size);
  vector<Integer> bin_a(size, Integer(ZK_INT_LEN, 0, PUBLIC));
  // sync_zk_bool<BoolIO<ZKNetIO>>();
  arith2bool<BoolIO<ZKNetIO>>(bin_a.data(), (IntFp*)a.data(), size);
  auto start = clock_start();
  
  Bit all_res(true);
  for (size_t i = 0; i < size; ++i) {
    // Todo: maybe we can optimize this...
    zero_res_bin[i] = bin_a[i].geq(MINIMUM); // check < 0
    zero_res_bin[i] = zero_res_bin[i] | (bin_a[i].equal(Zero)); // check <=0
    all_res = all_res & zero_res_bin[i]; // should all be 1 =1&1&1 
  }
  bool nonzero_check = all_res.reveal<bool>(PUBLIC);
  if (!nonzero_check) {
    uint64_t curr_fp_value = 0;
    ZkIntFp curr_fp;
    log_error << "Batch non-positive check failed! with context:";
      for (int i = 0; i < size; ++i) {
        log_error <<  i << "-th element is non-positive? : "
            << zero_res_bin[i].reveal<bool>(PUBLIC);
        curr_fp = a[i];
        curr_fp.open(curr_fp_value);
        log_error << "Curr Diff: Fp :" << curr_fp_value << "--> Boolean Int:" << bin_a[i].reveal<uint64_t>(PUBLIC);
      }
    throw std::runtime_error("CHECK NON-POS FAIL!");
  }

  auto reveal_time = time_from(start);  
  log_debug << "time for NON-POS check: " << reveal_time / 1000 << " for party: " << zk_party_id << " " << ENDL;
  log_debug << "check non-positive DONE!" << ENDL;
  // sync_zk_bool<BoolIO<ZKNetIO>>();
  return 0;
}

} // namespace zk

} // namespace rosetta
