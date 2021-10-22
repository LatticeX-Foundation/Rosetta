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

#include "emp-tool/emp-tool.h"
#include "emp-zk/emp-zk.h"

#include "cc/modules/protocol/zk/mystique/include/zk_netio.h"

#define ZK_INT_LEN 62
#define ZK_THREAD_NUM 5

#define LOCAL_SIMULATE false

#define DEBUG_MODE false

extern int zk_party_id;

#define ZK_HALF_PR int64_t((PR - 1) / 2)
#define ZK_FLOAT_SIZE 16
#define ZKFloatToFixPoint(a) \
  (int64_t)(((int64_t)(a) << ZK_FLOAT_SIZE) + (int64_t((a - (int64_t)(a)) * (1L << ZK_FLOAT_SIZE))))

using namespace emp;

#include <atomic>
extern std::atomic<int64_t> mul_gate_counter;
extern std::atomic<int64_t> mul_const_gate_counter;

namespace rosetta {
namespace zk {

class ZkIntFp : public IntFp {
 public:
  ZkIntFp() : IntFp(0, PUBLIC) {}

  ZkIntFp(const ZkIntFp& obj) { this->value = obj.value; }

  ZkIntFp(int64_t input, int party) {
    // encode
    uint64_t input_u64 = zk_fp_encode(input);

    // feed
    if (party == PUBLIC) {
      value = ZKFpExec::zk_exec->pub_label(input_u64);
    } else {
      ZKFpExec::zk_exec->feed(value, input_u64);
    }
  }

  ZkIntFp(double input, int party) {
    // encode
    uint64_t input_u64 = zk_fp_encode(input);

    // feed
    if (party == PUBLIC) {
      value = ZKFpExec::zk_exec->pub_label(input_u64);
    } else {
      ZKFpExec::zk_exec->feed(value, input_u64);
    }
  }

  ZkIntFp(int input) : ZkIntFp((int64_t)input, PUBLIC) {}

  ZkIntFp(uint64_t input, int party = PUBLIC) : IntFp(input, party) {}

  void feed(uint64_t input, int party) {
    if (party == PUBLIC) {
      this->value = ZKFpExec::zk_exec->pub_label(input);
    } else {
      ZKFpExec::zk_exec->feed(this->value, input);
    }
  }

  static inline uint64_t zk_fp_encode(double input) {
    // convert float to fix-point!
    int64_t plain_x = ZKFloatToFixPoint(input);
    int64_t y = plain_x;
    if (y < -ZK_HALF_PR) {
      cout << "WARNING!: out of range! too small value!:" << y;
      y = -ZK_HALF_PR;
    } else if (y > ZK_HALF_PR) {
      cout << "WARNING!: out of range! too Large value!:" << y;
      y = ZK_HALF_PR;
    }

    return mod((uint64_t)(y + (int64_t)PR));
  }

  static inline void zk_fp_encode(const double* input, uint64_t* output, size_t size) {
    // convert float to fix-point!
    int64_t plain_x = 0;
    for (size_t i = 0; i < size; i++) {
      plain_x = ZKFloatToFixPoint(input[i]);
      if (plain_x < -ZK_HALF_PR) {
        cout << "WARNING!: out of range! too small value!:" << input[i] << ", Integer: " << plain_x;
        plain_x = -ZK_HALF_PR;
      } else if (plain_x > ZK_HALF_PR) {
        cout << "WARNING!: out of range! too Large value!:" << input[i] << ", Integer: " << plain_x;
        plain_x = ZK_HALF_PR;
      }

      output[i] = mod((uint64_t)(plain_x + (int64_t)PR));
    }
  }

  static inline uint64_t zk_fp_encode(int64_t input) {
    int64_t y = (int64_t)input;
    if (y < -ZK_HALF_PR) {
      cout << "WARNING!: out of range! too small value!:" << y;
      y = -ZK_HALF_PR;
    } else if (y > ZK_HALF_PR) {
      cout << "WARNING!: out of range! too Large value!:" << y;
      y = ZK_HALF_PR;
    }
    return mod((uint64_t)(y + (int64_t)PR));
  }

  static inline void zk_batch_get_fp(const ZkIntFp* a, size_t size, uint64_t* b) {
    for (auto i = 0; i < size; ++i) {
      // auto tmp = int64_t(a[i].value >> 64);
      // if (tmp > ZK_HALF_PR) {
      // 	tmp = int64_t(tmp - PR);
      // }

      // b[i] = uint64_t(tmp);

      b[i] = (uint64_t)(a[i].value >> 64);
    }
  }

  bool reveal_u64(const uint64_t expected) {
    // cout << "zk prover secret value: " << uint64_t(value >> 64) << endl;
    ZKFpExec::zk_exec->reveal(&this->value, (uint64_t*)&expected, 1);
    return true;
  }

  bool reveal(const int64_t expected) {
    // encode
    uint64_t expect_u64 = zk_fp_encode(expected);
    return reveal_u64(expect_u64);
  }

  bool reveal(const double expected) {
    // encode
    uint64_t expect_u64 = zk_fp_encode(expected);
    cout << "reveal expect u64 value: " << expect_u64 << endl;
    return reveal_u64(expect_u64);
  }

  static inline bool batch_reveal(const ZkIntFp* inputs, size_t size) {
    vector<uint64_t> plains(size);
    for (int i = 0; i < size; ++i) {
      plains[i] = uint64_t(inputs[i].value >> 64);
    }
    if (zk_party_id == ALICE) {
      ((ZKFpExecPrv<BoolIO<ZKNetIO>>*)(ZKFpExec::zk_exec))
        ->io->send_data(plains.data(), size * sizeof(uint64_t));
    } else {
      ((ZKFpExecPrv<BoolIO<ZKNetIO>>*)(ZKFpExec::zk_exec))
        ->io->recv_data(plains.data(), size * sizeof(uint64_t));
    }

    return batch_reveal_check((IntFp*)inputs, (uint64_t*)plains.data(), (int)size);
  }

  // actually, this is 'Open'.
  bool open(uint64_t& res) {
    uint64_t plain = 0;
    plain = uint64_t(this->value >> 64);
    if (zk_party_id == ALICE) {
      ((ZKFpExecPrv<BoolIO<ZKNetIO>>*)(ZKFpExec::zk_exec))->io->send_data(&plain, sizeof(uint64_t));
    } else {
      ((ZKFpExecPrv<BoolIO<ZKNetIO>>*)(ZKFpExec::zk_exec))->io->recv_data(&plain, sizeof(uint64_t));
    }
    res = plain;
    return reveal_u64(plain);
  }

  ZkIntFp operator+(const ZkIntFp& rhs) const {
    ZkIntFp res(*this);
    res.value = ZKFpExec::zk_exec->add_gate(this->value, rhs.value);
    return res;
  }

  ZkIntFp operator*(const ZkIntFp& rhs) const {
    mul_gate_counter++;
    ZkIntFp res(*this);
    res.value = ZKFpExec::zk_exec->mul_gate(this->value, rhs.value);
    return res;
  }

  ZkIntFp operator*(const uint64_t& rhs) const {
    mul_const_gate_counter++;
    ZkIntFp res(*this);
    res.value = ZKFpExec::zk_exec->mul_const_gate(this->value, rhs);
    return res;
  }

  // add
  ZkIntFp& operator+=(const ZkIntFp& rhs) {
    this->value = ZKFpExec::zk_exec->add_gate(this->value, rhs.value);
    return *this;
  }

  // sub
  ZkIntFp& operator-=(const ZkIntFp& rhs) {
    this->value = zk_fp_sub(*this, rhs, zk_party_id).value;
    return *this;
  }

  // substract
  ZkIntFp operator-(const ZkIntFp& rhs) const { return zk_fp_sub(*this, rhs, zk_party_id); }

  // divide
  ZkIntFp operator/(const ZkIntFp& rhs) const {
    // [kelvin] TODO: I will implement this interface
    return ZkIntFp((int64_t)0, PUBLIC);
  }

  // less
  ZkIntFp operator<(const ZkIntFp& rhs) const { return zk_fp_less(*this, rhs, zk_party_id); }

  // less-equal
  ZkIntFp operator<=(const ZkIntFp& rhs) const { return zk_fp_less_equal(*this, rhs, zk_party_id); }

  // equal
  ZkIntFp operator==(const ZkIntFp& rhs) const { return zk_fp_equal(*this, rhs, zk_party_id); }

  // greater-equal
  ZkIntFp operator>=(const ZkIntFp& rhs) const {
    return zk_fp_greater_equal(*this, rhs, zk_party_id);
  }

  //greater
  ZkIntFp operator>(const ZkIntFp& rhs) const { return zk_fp_greater(*this, rhs, zk_party_id); }

  // negate
  ZkIntFp operator-() const { return zk_fp_neg(*this, zk_party_id); }

  friend ostream& operator<<(ostream& os, const ZkIntFp& obj) {
    char buf0[8] = {0};
    char buf1[8] = {0};
    unsigned char* p0 = (unsigned char*)&(obj.value); //mac
    unsigned char* p1 = ((unsigned char*)&(obj.value)) + 8; //value
    string s0, s1;
    for (int i = 0; i < 8; i++) {
      sprintf(buf0, "%02x", p0[i] & 0xFF);
      s0.append(buf0);
      sprintf(buf1, "%02x", p1[i] & 0xFF);
      s1.append(buf1);
    }

    os << s1 + "-" + s0; //value-mac
    return os;
  }

  static inline ZkIntFp zk_fp_sub(const ZkIntFp& lhs, const ZkIntFp& rhs, int party) {
    ZkIntFp res = lhs;
    if (ALICE == party) {
      __uint128_t val = mod((res.value >> 64) + (pr - (rhs.value >> 64)), pr);
      __uint128_t mac =
        mod((res.value & 0xFFFFFFFFFFFFFFFFULL) + (pr - (rhs.value & 0xFFFFFFFFFFFFFFFFULL)), pr);
      res.value = (val << 64) | mac;
    } else {
      res.value = mod(res.value + (pr - rhs.value), pr);
    }

    return res;
  }

  static inline void zk_fp_sub(
    const ZkIntFp* lhs,
    const ZkIntFp* rhs,
    int size,
    vector<ZkIntFp>& res,
    int party) {
    if (res.size() != size)
      res.resize(size);
    for (int i = 0; i < size; ++i) {
      res[i] = lhs[i];
      if (ALICE == party) {
        __uint128_t val = mod((res[i].value >> 64) + (pr - (rhs[i].value >> 64)), pr);
        __uint128_t mac = mod(
          (res[i].value & 0xFFFFFFFFFFFFFFFFULL) + (pr - (rhs[i].value & 0xFFFFFFFFFFFFFFFFULL)),
          pr);
        res[i].value = (val << 64) | mac;
      } else {
        res[i].value = mod(res[i].value + (pr - rhs[i].value), pr);
      }
    }
  }

  static inline ZkIntFp zk_fp_neg(const ZkIntFp& lhs, int party) {
    // printf("----  zk_fp_neg ... \n");
    ZkIntFp res = lhs;
    if (ALICE == party) {
      __uint128_t val = mod(pr - (lhs.value >> 64), pr);
      __uint128_t mac = mod(pr - (lhs.value & 0xFFFFFFFFFFFFFFFFULL), pr);
      res.value = (val << 64) | mac;
    } else {
      res.value = mod(pr - lhs.value, pr);
    }

    return res;
  }

  static inline void zk_fp_neg_this(ZkIntFp& lhs, int party) {
    // printf("----  zk_fp_neg this ... \n");
    if (ALICE == party) {
      __uint128_t val = mod(pr - (lhs.value >> 64), pr);
      __uint128_t mac = mod(pr - (lhs.value & 0xFFFFFFFFFFFFFFFFULL), pr);
      lhs.value = (val << 64) | mac;
    } else {
      lhs.value = mod(pr - lhs.value, pr);
    }
  }

  static inline void zk_fp_greater(
    const ZkIntFp* lhs,
    const ZkIntFp* rhs,
    int size,
    int party,
    vector<ZkIntFp>& out) {
    vector<ZkIntFp> delta;
    zk_fp_sub(lhs, rhs, size, delta, party);

    vector<Integer> bin(size, Integer(ZK_INT_LEN, 0, PUBLIC));
    sync_zk_bool<BoolIO<ZKNetIO>>();
    arith2bool<BoolIO<ZKNetIO>>(bin.data(), delta.data(), size);

    Integer zero(ZK_INT_LEN, 0, PUBLIC), one(ZK_INT_LEN, 1, PUBLIC);
    vector<Integer> greater_ret(size, Integer(ZK_INT_LEN, 0, PUBLIC));
    Integer pr_half(
      ZK_INT_LEN, (uint64_t)((pr >> 2) + 1), PUBLIC); // pr/2+1 is the biggest negative value
    for (size_t i = 0; i < size; ++i) {
      Integer equal_cmp = zero.select(bin[i].equal(zero), one);
      Integer less_equal =
        zero.select(bin[i].geq(pr_half), one) | equal_cmp; //less_equal = less | equal
      greater_ret[i] = one - less_equal; // greater = one - less_equal
      // cout << "greater[" << i << "]: " << greater_ret[i].reveal<uint64_t>(PUBLIC) << endl;
    }

    if (out.size() != size)
      out.resize(size);
    bool2arith<BoolIO<ZKNetIO>>(out.data(), greater_ret.data(), size);
    sync_zk_bool<BoolIO<ZKNetIO>>();
  }

  static inline ZkIntFp zk_fp_greater(const ZkIntFp& lhs, const ZkIntFp& rhs, int party) {
    vector<ZkIntFp> out(1);
    zk_fp_greater(&lhs, &rhs, 1, party, out);
    return out[0];
  }

  static inline void zk_fp_greater_equal(
    const ZkIntFp* lhs,
    const ZkIntFp* rhs,
    int size,
    int party,
    vector<ZkIntFp>& out) {
    vector<ZkIntFp> delta;
    zk_fp_sub(lhs, rhs, size, delta, party);

    vector<Integer> bin(size, Integer(ZK_INT_LEN, 0, PUBLIC));
    sync_zk_bool<BoolIO<ZKNetIO>>();
    arith2bool<BoolIO<ZKNetIO>>(bin.data(), delta.data(), size);

    Integer zero(ZK_INT_LEN, 0, PUBLIC), one(ZK_INT_LEN, 1, PUBLIC);
    vector<Integer> greater_equal_ret(size, Integer(ZK_INT_LEN, 0, PUBLIC));
    Integer pr_half(
      ZK_INT_LEN, (uint64_t)((pr >> 2) + 1), PUBLIC); // pr/2+1 is the biggest negative value
    for (size_t i = 0; i < size; ++i) {
      greater_equal_ret[i] =
        one - zero.select(bin[i].geq(pr_half), one); // greater_equal = 1 - less
      // cout << "greater_equal[" << i << "]: " << greater_equal_ret[i].reveal<uint64_t>(PUBLIC) << endl;
    }

    if (out.size() != size)
      out.resize(size);
    bool2arith<BoolIO<ZKNetIO>>(out.data(), greater_equal_ret.data(), size);
    sync_zk_bool<BoolIO<ZKNetIO>>();
  }

  static inline ZkIntFp zk_fp_greater_equal(const ZkIntFp& lhs, const ZkIntFp& rhs, int party) {
    vector<ZkIntFp> out(1);
    zk_fp_greater_equal(&lhs, &rhs, 1, party, out);
    return out[0];
  }

  static inline void zk_fp_equal(
    const ZkIntFp* lhs,
    const ZkIntFp* rhs,
    int size,
    int party,
    vector<ZkIntFp>& out) {
    vector<ZkIntFp> delta;
    zk_fp_sub(lhs, rhs, size, delta, party);

    vector<Integer> bin(size, Integer(ZK_INT_LEN, 0, PUBLIC));
    sync_zk_bool<BoolIO<ZKNetIO>>();
    arith2bool<BoolIO<ZKNetIO>>(bin.data(), delta.data(), size);

    Integer zero(ZK_INT_LEN, 0, PUBLIC), one(ZK_INT_LEN, 1, PUBLIC);
    vector<Integer> zk_equal(size, Integer(ZK_INT_LEN, 0, PUBLIC));
    Integer pr_half(
      ZK_INT_LEN, (uint64_t)((pr >> 2) + 1), PUBLIC); // pr/2+1 is the biggest negative value
    for (size_t i = 0; i < size; ++i) {
      zk_equal[i] = zero.select(bin[i].equal(zero), one);
      // cout << "equal[" << i << "]: " << zk_equal[i].reveal<uint64_t>(PUBLIC) << endl;
    }

    if (out.size() != size)
      out.resize(size);
    bool2arith<BoolIO<ZKNetIO>>(out.data(), zk_equal.data(), size);
    sync_zk_bool<BoolIO<ZKNetIO>>();
  }

  static inline ZkIntFp zk_fp_equal(const ZkIntFp& lhs, const ZkIntFp& rhs, int party) {
    vector<ZkIntFp> out(1);
    zk_fp_equal(&lhs, &rhs, 1, party, out);
    return out[0];
  }

  static inline void zk_fp_less(
    const ZkIntFp* lhs,
    const ZkIntFp* rhs,
    int size,
    int party,
    vector<ZkIntFp>& out) {
    vector<ZkIntFp> delta;
    zk_fp_sub(lhs, rhs, size, delta, party);

    vector<Integer> bin(size, Integer(ZK_INT_LEN, 0, PUBLIC));
    sync_zk_bool<BoolIO<ZKNetIO>>();
    arith2bool<BoolIO<ZKNetIO>>(bin.data(), delta.data(), size);

    Integer zero(ZK_INT_LEN, 0, PUBLIC), one(ZK_INT_LEN, 1, PUBLIC);
    vector<Integer> less_ret(size, Integer(ZK_INT_LEN, 0, PUBLIC));
    Integer pr_half(
      ZK_INT_LEN, (uint64_t)((pr >> 2) + 1), PUBLIC); // pr/2+1 is the biggest negative value
    for (size_t i = 0; i < size; ++i) {
      less_ret[i] = zero.select(bin[i].geq(pr_half), one);
      // cout << "less[" << i << "]: " << less_ret[i].reveal<uint64_t>(PUBLIC) << endl;
    }

    if (out.size() != size)
      out.resize(size);
    bool2arith<BoolIO<ZKNetIO>>(out.data(), less_ret.data(), size);
    sync_zk_bool<BoolIO<ZKNetIO>>();
  }

  static inline ZkIntFp zk_fp_less(const ZkIntFp& lhs, const ZkIntFp& rhs, int party) {
    vector<ZkIntFp> out(1);
    zk_fp_less(&lhs, &rhs, 1, party, out);
    return out[0];
  }

  static inline void zk_fp_less_equal(
    const ZkIntFp* lhs,
    const ZkIntFp* rhs,
    int size,
    int party,
    vector<ZkIntFp>& out) {
    vector<ZkIntFp> delta;
    zk_fp_sub(lhs, rhs, size, delta, party);

    vector<Integer> bin(size, Integer(ZK_INT_LEN, 0, PUBLIC));
    sync_zk_bool<BoolIO<ZKNetIO>>();
    arith2bool<BoolIO<ZKNetIO>>(bin.data(), delta.data(), size);

    Integer zero(ZK_INT_LEN, 0, PUBLIC), one(ZK_INT_LEN, 1, PUBLIC);
    vector<Integer> less_equal_ret(size, Integer(ZK_INT_LEN, 0, PUBLIC));
    Integer pr_half(
      ZK_INT_LEN, (uint64_t)((pr >> 2) + 1), PUBLIC); // pr/2+1 is the biggest negative value
    for (size_t i = 0; i < size; ++i) {
      Integer equal_cmp = zero.select(bin[i].equal(zero), one);
      less_equal_ret[i] =
        zero.select(bin[i].geq(pr_half), one) | equal_cmp; //less_equal = less | equal
      // cout << "less_equal[" << i << "]: " << less_equal_ret[i].reveal<uint64_t>(PUBLIC) << endl;
    }

    if (out.size() != size)
      out.resize(size);
    bool2arith<BoolIO<ZKNetIO>>(out.data(), less_equal_ret.data(), size);
    sync_zk_bool<BoolIO<ZKNetIO>>();
  }

  static inline ZkIntFp zk_fp_less_equal(const ZkIntFp& lhs, const ZkIntFp& rhs, int party) {
    vector<ZkIntFp> out(1);
    zk_fp_less_equal(&lhs, &rhs, 1, party, out);
    return out[0];
  }

}; //ZkIntFp

} // namespace zk
} // namespace rosetta
