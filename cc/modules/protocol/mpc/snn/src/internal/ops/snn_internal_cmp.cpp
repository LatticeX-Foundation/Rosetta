#include "cc/modules/protocol/mpc/snn/include/snn_internal.h"
#include "cc/modules/common/include/utils/str_type_convert.h"
#include <thread>

using std::thread;

namespace rosetta {
namespace snn {

/////////////////   greater_equal   /////////////////
int SnnInternal::GreaterEqual(const mpc_t& a, const mpc_t& b, mpc_t& c) {
  vector<mpc_t> va(1, a);
  vector<mpc_t> vb(1, b);
  vector<mpc_t> vc(1, 0);
  GreaterEqual(va, vb, vc);
  c = vc[0];
  return 0;
}

int SnnInternal::GreaterEqual(const vector<double>& a, const vector<mpc_t>& b, vector<mpc_t>& c) {
  vector<mpc_t> va(a.size(), 0);
  Const2Share(a, va);
  return GreaterEqual(va, b, c);
}

int SnnInternal::GreaterEqual(const vector<mpc_t>& a, const vector<double>& b, vector<mpc_t>& c) {
  vector<mpc_t> vb(b.size(), 0);
  Const2Share(b, vb);
  return GreaterEqual(a, vb, c);
}

int SnnInternal::GreaterEqual(
  const vector<string>& a,
  const vector<mpc_t>& b,
  vector<mpc_t>& c) {
  vector<mpc_t> va(a.size(), 0);
  Const2Share(a, va);
  return GreaterEqual(va, b, c);
}
int SnnInternal::GreaterEqual(
  const vector<mpc_t>& a,
  const vector<string>& b,
  vector<mpc_t>& c) {
  vector<mpc_t> vb(b.size(), 0);
  Const2Share(b, vb);
  return GreaterEqual(a, vb, c);
}

int SnnInternal::GreaterEqual(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c) {
  tlog_debug << "GreaterEqual ...";
  AUDIT("id:{}, P{} GreaterEqual, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(a));
  AUDIT("id:{}, P{} GreaterEqual, input Y(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(b));

  vector<mpc_t> delta(a.size(), 0);
  subtractVectors<mpc_t>(a, b, delta, a.size());
  AUDIT("id:{}, P{} GreaterEqual, compute delta=X-Y, delta(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(delta));

  c.resize(a.size());
  int ret = ReluPrime(delta, c);

  AUDIT("id:{}, P{} GreaterEqual, output Z(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(c));
  tlog_debug << "GreaterEqual ok.";
  return ret;
}

/////////////////   greater   /////////////////
int SnnInternal::Greater(const mpc_t& a, const mpc_t& b, mpc_t& c) {
  vector<mpc_t> va(1, a);
  vector<mpc_t> vb(1, b);
  vector<mpc_t> vc(1, 0);
  Greater(va, vb, vc);
  c = vc[0];
  return 0;
}

int SnnInternal::Greater(
  const vector<mpc_t>& a,
  const vector<mpc_t>& b,
  vector<mpc_t>& c) {
  tlog_debug << "Greater ...";
  AUDIT("id:{}, P{} Greater, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(a));
  AUDIT("id:{}, P{} Greater, input Y(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(b));
  //1. calculate LessEqual
  c.resize(a.size());
  vector<mpc_t> LE(a.size(), 0);
  vector<mpc_t> cmp(a.size(), 0);
  for (size_t i = 0; i < a.size(); ++i) {
    cmp[i] = b[i] - a[i];
  }
  int ret = ReluPrime(cmp, LE);

  //2. calculate result ==> Greater = 1 - LessEqual
  for (size_t i = 0; i < a.size(); ++i) {
    c[i] = FloatToMpcType(1, GetMpcContext()->FLOAT_PRECISION) * partyNum - LE[i];
  }

  AUDIT("id:{}, P{} Greater, output Z(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(c));
  tlog_debug << "Greater ok.";
  return ret;
}

int SnnInternal::Greater(const vector<double>& a, const vector<mpc_t>& b, vector<mpc_t>& c) {
  vector<mpc_t> va(a.size(), 0);
  Const2Share(a, va);
  return Greater(va, b, c);
}

int SnnInternal::Greater(const vector<mpc_t>& a, const vector<double>& b, vector<mpc_t>& c) {
  vector<mpc_t> vb(b.size(), 0);
  Const2Share(b, vb);
  return Greater(a, vb, c);
}

int SnnInternal::Greater(const vector<string>& a, const vector<mpc_t>& b, vector<mpc_t>& c) {
  vector<mpc_t> va(a.size(), 0);
  Const2Share(a, va);
  return Greater(va, b, c);
}
int SnnInternal::Greater(const vector<mpc_t>& a, const vector<string>& b, vector<mpc_t>& c) {
  vector<mpc_t> vb(b.size(), 0);
  Const2Share(b, vb);
  return Greater(a, vb, c);
}

/////////////////   Less OR Equal   /////////////////
int SnnInternal::LessEqual(const mpc_t& a, const mpc_t& b, mpc_t& c) {
  vector<mpc_t> va(1, a);
  vector<mpc_t> vb(1, b);
  vector<mpc_t> vc(1, 0);
  LessEqual(va, vb, vc);
  c = vc[0];
  return 0;
}

int SnnInternal::LessEqual(const vector<double>& a, const vector<mpc_t>& b, vector<mpc_t>& c) {
  vector<mpc_t> va(a.size(), 0);
  Const2Share(a, va);
  return LessEqual(va, b, c);
}

int SnnInternal::LessEqual(const vector<mpc_t>& a, const vector<double>& b, vector<mpc_t>& c) {
  vector<mpc_t> vb(b.size(), 0);
  Const2Share(b, vb);
  return LessEqual(a, vb, c);
}

int SnnInternal::LessEqual(const vector<string>& a, const vector<mpc_t>& b, vector<mpc_t>& c) {
  vector<mpc_t> va(a.size(), 0);
  Const2Share(a, va);
  return LessEqual(va, b, c);
}
int SnnInternal::LessEqual(const vector<mpc_t>& a, const vector<string>& b, vector<mpc_t>& c) {
  vector<mpc_t> vb(b.size(), 0);
  Const2Share(b, vb);
  return LessEqual(a, vb, c);
}

int SnnInternal::LessEqual(
  const vector<mpc_t>& a,
  const vector<mpc_t>& b,
  vector<mpc_t>& c) {
  tlog_debug << "LessEqual ...";
  AUDIT("id:{}, P{} LessEqual, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(a));
  AUDIT("id:{}, P{} LessEqual, input Y(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(b));

  size_t size = a.size();
  c.resize(size);
  vector<mpc_t> cmp(size, 0);
  for (size_t i = 0; i < size; ++i) {
    cmp[i] = b[i] - a[i];
  }

  int ret = ReluPrime(cmp, c);

  AUDIT("id:{}, P{} LessEqual, output Z(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(c));
  tlog_debug << "LessEqual ok.";
  return ret;
}

/////////////////   Less   /////////////////
int SnnInternal::Less(const mpc_t& a, const mpc_t& b, mpc_t& c) {
  vector<mpc_t> va(1, a);
  vector<mpc_t> vb(1, b);
  vector<mpc_t> vc(1, 0);
  Less(va, vb, vc);
  c = vc[0];
  return 0;
}

int SnnInternal::Less(const vector<double>& a, const vector<mpc_t>& b, vector<mpc_t>& c) {
  vector<mpc_t> va(a.size(), 0);
  Const2Share(a, va);
  return Less(va, b, c);
}

int SnnInternal::Less(const vector<mpc_t>& a, const vector<double>& b, vector<mpc_t>& c) {
  vector<mpc_t> vb(b.size(), 0);
  Const2Share(b, vb);
  return Less(a, vb, c);
}

int SnnInternal::Less(const vector<string>& a, const vector<mpc_t>& b, vector<mpc_t>& c) {
  vector<mpc_t> va(a.size(), 0);
  Const2Share(a, va);
  return Less(va, b, c);
}
int SnnInternal::Less(const vector<mpc_t>& a, const vector<string>& b, vector<mpc_t>& c) {
  vector<mpc_t> vb(b.size(), 0);
  Const2Share(b, vb);
  return Less(a, vb, c);
}

int SnnInternal::Less(
  const vector<mpc_t>& a,
  const vector<mpc_t>& b,
  vector<mpc_t>& c) {
  tlog_debug << "Less ...";
  AUDIT("id:{}, P{} Less, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(a));
  AUDIT("id:{}, P{} Less, input Y(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(b));

  c.resize(a.size());
  //1. calculate greaterEqual ==> A
  vector<mpc_t> A(a.size(), 0);
  vector<mpc_t> cmp(a.size(), 0);
  for (size_t i = 0; i < a.size(); ++i) {
    cmp[i] = a[i] - b[i];
  }

  int ret = ReluPrime(cmp, A);

  //2. calculate result ==> c = 1 - A
  for (size_t i = 0; i < a.size(); ++i) {
    c[i] = FloatToMpcType(1, GetMpcContext()->FLOAT_PRECISION) * partyNum - A[i];
  }
  
  AUDIT("id:{}, P{} Less, output Z(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(c));
  tlog_debug << "Less ok.";
  return ret;
}

/////////////    Equal  //////////////////////
int SnnInternal::Equal(const mpc_t& a, const mpc_t& b, mpc_t& c) {
  vector<mpc_t> va(1, a);
  vector<mpc_t> vb(1, b);
  vector<mpc_t> vc(1, 0);
  Equal(va, vb, vc);
  c = vc[0];
  return 0;
}

int SnnInternal::Equal(const vector<double>& a, const vector<mpc_t>& b, vector<mpc_t>& c) {
  vector<mpc_t> va(a.size(), 0);
  Const2Share(a, va);
  return Equal(va, b, c);
}

int SnnInternal::Equal(const vector<mpc_t>& a, const vector<double>& b, vector<mpc_t>& c) {
  vector<mpc_t> vb(b.size(), 0);
  Const2Share(b, vb);
  return Equal(a, vb, c);
}

int SnnInternal::Equal(const vector<string>& a, const vector<mpc_t>& b, vector<mpc_t>& c) {
  vector<mpc_t> va(a.size(), 0);
  Const2Share(a, va);
  return Equal(va, b, c);
}
int SnnInternal::Equal(const vector<mpc_t>& a, const vector<string>& b, vector<mpc_t>& c) {
  vector<mpc_t> vb(b.size(), 0);
  Const2Share(b, vb);
  return Equal(a, vb, c);
}

int SnnInternal::Equal(
  const vector<mpc_t>& a,
  const vector<mpc_t>& b,
  vector<mpc_t>& c) {
  AUDIT("id:{}, P{} Equal, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(a));
  AUDIT("id:{}, P{} Equal, input Y(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(b));
  return FastEqual(a, b, c);
}

int SnnInternal::EqualSlow(
  const vector<mpc_t>& a,
  const vector<string>& b,
  vector<mpc_t>& c) {
  c.resize(a.size());
  tlog_debug << "EqualSlow ...";
  //1. calculate greaterEqual ==> A
  vector<mpc_t> A(a.size(), 0);
  GreaterEqual(a, b, A);

  //2. calculate lessEqual ==> B
  vector<mpc_t> B(a.size(), 0);
  LessEqual(a, b, B);

  //3. result c = A + B - 1
  for (size_t i = 0; i < a.size(); ++i) {
    c[i] = A[i] + B[i] - partyNum * FloatToMpcType(1, GetMpcContext()->FLOAT_PRECISION);
  }

  tlog_debug << "EqualSlow ok.";
  return 0;
}

int SnnInternal::FastEqual(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c) {
  tlog_debug << "FastEqual ...";
  AUDIT("id:{}, P{} FastEqual, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(a));
  AUDIT("id:{}, P{} FastEqual, input Y(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(b));

  c.resize(a.size());
  size_t size = a.size();
  vector<mpc_t> z(size, 0);
  subtractVectors<mpc_t>(a, b, z, size);
  AUDIT("id:{}, P{} FastEqual, compute Z=X-Y, Z(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(z));

  vector<mpc_t> r_0(size, 0);
  vector<mpc_t> r_1(size, 0);
  vector<mpc_t> r(size, 0);
  if (HELPER) {
    // fixed for debuging
    // r_0 = vector<mpc_t>(size, 2 << FLOAT_PRECISION_M);
    // r_1 = vector<mpc_t>(size, 3 << FLOAT_PRECISION_M);
    // r = vector<mpc_t>(size, 5 << FLOAT_PRECISION_M);
    populateRandomVector<mpc_t>(r_0, size, "a_1", "POSITIVE");
    AUDIT("id:{}, P{} FastEqual, populateRandomVector r0(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(r_0));

    populateRandomVector<mpc_t>(r_1, size, "a_2", "POSITIVE");
    AUDIT("id:{}, P{} FastEqual, populateRandomVector r1(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(r_1));

    addVectors<mpc_t>(r_0, r_1, r, size);
    AUDIT("id:{}, P{} FastEqual, Ranom r(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(r));
  } else if (partyNum == PARTY_A) {
    // fixed for debuging
    // r_0 = vector<mpc_t>(size, 2 << FLOAT_PRECISION_M);
    populateRandomVector<mpc_t>(r_0, size, "a_1", "POSITIVE");
    AUDIT("id:{}, P{} FastEqual, populateRandomVector r0(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(r_0));
    r = r_0;
  } else if (partyNum == PARTY_B) {
    // fixed for debuging
    // r_1 = vector<mpc_t>(size, 3 << FLOAT_PRECISION_M);
    populateRandomVector<mpc_t>(r_1, size, "a_2", "POSITIVE");
    AUDIT("id:{}, P{} FastEqual, populateRandomVector r1(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(r_1));
    r = r_1;
  }

  // share r in binary-xor format
  vector<mpc_t> r_0_b(size, 0);
  vector<mpc_t> r_1_b(size, 0);

  // this is for interface compliance with communication.
  // Note that the all the inner vectors have the same size: sizeof(mpc_t)!
  int BIT_L = sizeof(mpc_t) * 8;
  vector<vector<small_mpc_t>> bit_share(size, vector<small_mpc_t>(BIT_L, 0));

  if (HELPER || partyNum == PARTY_A) {
    // fixed for debuging
    // r_0_b = vector<mpc_t>(size, 2 << FLOAT_PRECISION_M);
    populateRandomVector<mpc_t>(r_0_b, size, "a_1", "POSITIVE");
    AUDIT("id:{}, P{} FastEqual, populateRandomVector r_0_b(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(r_0_b));
  }
  if (HELPER) {
    for (size_t i = 0; i < size; ++i) {
      r_1_b[i] = r[i] ^ r_0_b[i];
    }
    sendVector<mpc_t>(r_1_b, PARTY_B, size);
    AUDIT("id:{}, P{} FastEqual SEND to P{}, r_1_b(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), PARTY_B, Vector<mpc_t>(r_1_b));
  }
  if (partyNum == PARTY_B) {
    receiveVector<mpc_t>(r_1_b, PARTY_C, size);
    AUDIT("id:{}, P{} FastEqual RECV from P{}, r_1_b(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), PARTY_C, Vector<mpc_t>(r_1_b));
  }

  // GetMpcOpInner(Reconstruct2PC)->Run(z, z.size(), "input z res");
  // reveal plaintext: Z + r
  addVectors<mpc_t>(z, r, z, size);
  AUDIT("id:{}, P{} FastEqual compute z=r+z, z(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(z));
  // GetMpcOpInner(Reconstruct2PC)->Run(r, r.size(), "mask r res");
  // GetMpcOpInner(Reconstruct2PC)->Run(z, z.size(), "masked Z res");
  vector<mpc_t> plain_z(size, 0);
  if (PRIMARY) {
    thread* exchange_threads = new thread[2];
    exchange_threads[0] =
      thread(&SnnInternal::sendVector<mpc_t>, this, ref(z), adversary(partyNum), size);
    AUDIT("id:{}, P{} FastEqual SEND to P{}, z(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), adversary(partyNum), Vector<mpc_t>(z));
    exchange_threads[1] =
      thread(&SnnInternal::receiveVector<mpc_t>, this, ref(plain_z), adversary(partyNum), size);
    AUDIT("id:{}, P{} FastEqual RECV from P{}, plain_z(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), adversary(partyNum), Vector<mpc_t>(plain_z));
    exchange_threads[0].join();
    exchange_threads[1].join();
    delete[] exchange_threads;

    addVectors<mpc_t>(z, plain_z, plain_z, size);
    AUDIT("id:{}, P{} FastEqual compute: plain_z=z+plain_z, plain_z(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(plain_z));
    
    // print_vec(plain_z, size, "plain masked Z");

    for (size_t i = 0; i < size; ++i) {
      // cout << "debug:" << i << "-th local bit-share: " << r_0_b[i] << " and " << plain_z[i];
      mpc_t tmp_v = ~(r_0_b[i] ^ plain_z[i]);
      for (size_t j = 0; j < BIT_L; ++j) {
        if (partyNum == PARTY_B) {
          bit_share[i][j] = (r_1_b[i] >> j) & 0x01;
          // cout << int(bit_share[i][j]);
        } else if (partyNum == PARTY_A) {
          bit_share[i][j] = (tmp_v >> j) & 0x01;
          // cout << int(bit_share[i][j]);
        }
      }
      // cout << endl;
    }
  }

  vector<small_mpc_t> res(size, 0);
  // // tmp debugging:
  // vector<vector<small_mpc_t>> tmp_share(1, vector<small_mpc_t>(64, 0));
  // vector<mpc_t> tmp_c(1, 0);
  // if (partyNum == PARTY_A) {
  //   tmp_share[0] = bit_share[5];
  // } else if (partyNum == PARTY_B) {
  //   tmp_share[0] = bit_share[5];
  // }
  // FanInBitAdd(tmp_share, res, 1);
  // B2A(res, tmp_c, 1);
  // GetMpcOpInner(Reconstruct2PC)->Run(tmp_c, tmp_c.size(), "Final Equal temp res");
  // tmp testing end!
  FanInBitAdd(bit_share, res);
  // GetMpcOpInner(ReconstructBit2PC)->Run(res, res.size(), "Debug BitAdd res");
  B2A(res, c);
  // GetMpcOpInner(Reconstruct2PC)->Run(c, c.size(), "Final Equal res");
  AUDIT("id:{}, P{} FastEqual, output(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(c));
  tlog_debug << "FastEqual ok.";
  return 0;
}

int SnnInternal::FanInBitAdd(const vector<vector<small_mpc_t>>& a, vector<small_mpc_t>& c) {
  // S0: check and init
  //    the inner vector must have the same size
  // normally, the LEN should be sizeof(mpc_t) * 8
  // cout << "FANInbitAdd size:" << a.size() << " of inner:" << a[0].size() << endl;
  tlog_debug << "FanInBitAdd ...";
  int LEN = a[0].size();
  size_t vec_size = a.size();
  c.resize(vec_size);

  for (int i = 0; i < vec_size; ++i) { assert(a[i].size() == LEN); }

  vector<vector<small_mpc_t>> all_bits = a;
  int Depth = int(log2(LEN));
  assert((1 << Depth) == LEN && "Only 2^i len is supported!");

  vector<small_mpc_t> flat_a_vec;
  vector<small_mpc_t> flat_b_vec;
  int curr_len = LEN;
  // This is just a tricky shortcut
  // this is binary-tree strategy to perform AND in parallel.
  for (int i = 0; i < Depth; ++i) {
    // cout << "depth:" << i << endl;
    flat_a_vec.clear();
    flat_b_vec.clear();
    // flat matrix to vector
    for (int k = 0; k < vec_size; ++k) {
      for (int j = 0; j < curr_len - 1; j = j + 2) {
        flat_a_vec.push_back(all_bits[k][j]);
        flat_b_vec.push_back(all_bits[k][j + 1]);
      }
    }
    AUDIT("id:{}, P{} FanInBitAdd flat_a_vec[{}](small_mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), i, Vector<small_mpc_t>(flat_a_vec));
    AUDIT("id:{}, P{} FanInBitAdd flat_b_vec[{}](small_mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), i, Vector<small_mpc_t>(flat_b_vec));

    vector<small_mpc_t> flat_c_vec(flat_a_vec.size(), 0);
    BitMul(flat_a_vec, flat_b_vec, flat_c_vec);
    AUDIT("id:{}, P{} FanInBitAdd compute: flat_c_vec[{}]=flat_a_vec[{}]*flat_b_vec[{}], flat_c_vec[{}](small_mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), i, i, i, i, Vector<small_mpc_t>(flat_c_vec));
    // for debuging
    // if(PRIMARY) {
    //   GetMpcOpInner(ReconstructBit2PC)->Run(flat_a_vec, flat_a_vec.size(), "Debug Bit A");
    //   GetMpcOpInner(ReconstructBit2PC)->Run(flat_b_vec, flat_b_vec.size(), "Debug Bit B");
    //   GetMpcOpInner(ReconstructBit2PC)->Run(flat_c_vec, flat_c_vec.size(), "Debug Bit C");
    // }

    // reconstruct matrix from vector
    curr_len = curr_len / 2;
    // cout << "new len:" << curr_len << endl;
    for (int k = 0; k < vec_size; ++k) {
      all_bits[k].clear();
      all_bits[k].insert(
        all_bits[k].begin(), flat_c_vec.begin() + k * curr_len,
        flat_c_vec.begin() + (k + 1) * curr_len);
    }
  }

  for (int i = 0; i < vec_size; ++i) {
    // make sure result is legal: all size should be 1!
    // cout << "check[" << i << "]:" << all_bits[i].size() << ":" << (all_bits[i][0] & 0x01) << std::endl;
    c[i] = all_bits[i][0] & 0x01;
  }

  AUDIT("id:{}, P{} FanInBitAdd output(small_mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<small_mpc_t>(c));
  tlog_debug << "FanInBitAdd ok.";
  return 0;
}

int SnnInternal::B2A(const vector<small_mpc_t>& bit_shares, vector<mpc_t>& arith_shares) {
  tlog_debug << "B2A ...";
  AUDIT("id:{}, P{} B2A, input(small_mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<small_mpc_t>(bit_shares));
  AUDIT("id:{}, P{} B2A output(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(arith_shares));

  size_t size = bit_shares.size();
  arith_shares.resize(size);
  vector<small_mpc_t> bit_m_0(size, 0);
  vector<small_mpc_t> bit_m_1(size, 0);
  vector<small_mpc_t> bit_m(size, 0);
  vector<mpc_t> m_0(size, 0);
  vector<mpc_t> m_1(size, 0);
  vector<mpc_t> m(size, 0);
  int float_precision = GetMpcContext()->FLOAT_PRECISION;
  if (HELPER) {
    populateRandomVector<small_mpc_t>(bit_m_0, size, "a_1", "POSITIVE");
    AUDIT("id:{}, P{} B2A, populateRandomVector bit_m_0(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<small_mpc_t>(bit_m_0));

    populateRandomVector<mpc_t>(m_0, size, "a_1", "POSITIVE");
    AUDIT("id:{}, P{} B2A, populateRandomVector m_0(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(m_0));

    populateRandomVector<small_mpc_t>(bit_m_1, size, "a_2", "POSITIVE");
    AUDIT("id:{}, P{} B2A, populateRandomVector bit_m_1(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<small_mpc_t>(bit_m_1));

    for (int i = 0; i < size; ++i) {
      bit_m[i] = (bit_m_0[i] ^ bit_m_1[i]) & 0x01;
      m_1[i] = (mpc_t)(bit_m[i] << float_precision) - m_0[i];
    }
    sendVector<mpc_t>(m_1, PARTY_B, size);
    AUDIT("id:{}, P{} B2A SEND to P{}, m_1(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), PARTY_B, Vector<mpc_t>(m_1));
  }
  if (partyNum == PARTY_A) {
    populateRandomVector<small_mpc_t>(bit_m_0, size, "a_1", "POSITIVE");
    AUDIT("id:{}, P{} B2A, populateRandomVector bit_m_0(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<small_mpc_t>(bit_m_0));

    populateRandomVector<mpc_t>(m_0, size, "a_1", "POSITIVE");
    AUDIT("id:{}, P{} B2A, populateRandomVector m_0(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(m_0));

    bit_m = bit_m_0;
    m = m_0;
  }
  if (partyNum == PARTY_B) {
    populateRandomVector<small_mpc_t>(bit_m_1, size, "a_2", "POSITIVE");
    AUDIT("id:{}, P{} B2A, populateRandomVector bit_m_1(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<small_mpc_t>(bit_m_1));

    bit_m = bit_m_1;
    receiveVector<mpc_t>(m_1, PARTY_C, size);
    AUDIT("id:{}, P{} B2A RECV from P{}, m_1(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), PARTY_C, Vector<mpc_t>(m_1));
    m = m_1;
  }

  vector<small_mpc_t> masked_bit_shares(size, 0);
  if (PRIMARY) {
    for (int i = 0; i < size; ++i) {
      masked_bit_shares[i] = bit_shares[i] ^ bit_m[i] & 0x01;
    }
    // reveal masked bit_shares
    vector<small_mpc_t> other_part(size, 0);
    vector<small_mpc_t> plain_bit(size, 0);
    thread* exchange_threads = new thread[2];
    exchange_threads[0] =
      thread(&SnnInternal::sendBitVector, this, ref(masked_bit_shares), adversary(partyNum), size);
    AUDIT("id:{}, P{} B2A SEND to P{}, masked_bit_shares(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), adversary(partyNum), Vector<small_mpc_t>(masked_bit_shares));
    exchange_threads[1] =
      thread(&SnnInternal::receiveBitVector, this, ref(other_part), adversary(partyNum), size);
    AUDIT("id:{}, P{} B2A RECV from P{}, other_part(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), adversary(partyNum), Vector<small_mpc_t>(other_part));

    exchange_threads[0].join();
    exchange_threads[1].join();
    delete[] exchange_threads;

    for (int i = 0; i < size; ++i) {
      plain_bit[i] = (masked_bit_shares[i] ^ other_part[i]) & 0x01;

      if (plain_bit[i] == 0) {
        arith_shares[i] = m[i];
      } else {
        if (partyNum == PARTY_A) {
          arith_shares[i] = (1 << float_precision) - m[i];
        }
        if (partyNum == PARTY_B) {
          arith_shares[i] = -m[i];
        }
      }
    }
  }

  AUDIT("id:{}, P{} B2A, output(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(arith_shares));
  tlog_debug << "B2A ok.";
  return 0;
}

/////////////    Not Equal  //////////////////////
int SnnInternal::NotEqual(const mpc_t& a, const mpc_t& b, mpc_t& c) {
  vector<mpc_t> va(1, a);
  vector<mpc_t> vb(1, b);
  vector<mpc_t> vc(1, 0);
  NotEqual(va, vb, vc);
  c = vc[0];
  return 0;
}

int SnnInternal::NotEqual(const vector<double>& a, const vector<mpc_t>& b, vector<mpc_t>& c) {
  vector<mpc_t> va(a.size(), 0);
  Const2Share(a, va);
  return NotEqual(va, b, c);
}

int SnnInternal::NotEqual(const vector<mpc_t>& a, const vector<double>& b, vector<mpc_t>& c) {
  vector<mpc_t> vb(b.size(), 0);
  Const2Share(b, vb);
  return NotEqual(a, vb, c);
}

int SnnInternal::NotEqual(const vector<string>& a, const vector<mpc_t>& b, vector<mpc_t>& c) {
  vector<mpc_t> va(a.size(), 0);
  Const2Share(a, va);
  return NotEqual(va, b, c);
}
int SnnInternal::NotEqual(const vector<mpc_t>& a, const vector<string>& b, vector<mpc_t>& c) {
  vector<mpc_t> vb(b.size(), 0);
  Const2Share(b, vb);
  return NotEqual(a, vb, c);
}

int SnnInternal::NotEqual(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c) {
  return FastNotEqual(a, b, c);
}

int SnnInternal::NotEqualSlow(
  const vector<mpc_t>& a,
  const vector<mpc_t>& b,
  vector<mpc_t>& c) {
  tlog_debug << "NotEqualSlow ...";
  AUDIT("id:{}, P{} NotEqualSlow, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(a));
  AUDIT("id:{}, P{} NotEqualSlow, input Y(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(b));
  size_t size = a.size();
  c.resize(size);
  //1. calculate greaterEqual ==> A
  vector<mpc_t> A(a.size(), 0);
  GreaterEqual(a, b, A);

  //2. calculate lessEqual ==> B
  vector<mpc_t> B(a.size(), 0);
  LessEqual(a, b, B);

  //3. result c = 2 - A + B
  for (size_t i = 0; i < a.size(); ++i) {
    c[i] = FloatToMpcType(1, GetMpcContext()->FLOAT_PRECISION) -
      (A[i] + B[i]); //2 - 1 == 1 (not equal) or 2 - 2 == 0 (equal)
  }

  AUDIT("id:{}, P{} NotEqualSlow, output(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(c));
  tlog_debug << "NotEqualSlow ok.";
  return 0;
}

int SnnInternal::FastNotEqual(
  const vector<mpc_t>& a,
  const vector<mpc_t>& b,
  vector<mpc_t>& c) {
  tlog_debug << "FastNotEqual ...";
  AUDIT("id:{}, P{} FastNotEqual, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(a));
  AUDIT("id:{}, P{} FastNotEqual, input Y(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(b));

  size_t size = a.size();
  c.resize(size);
  // Just (1 - Equal())
  vector<mpc_t> CONST_ONE(size, 0);
  if (partyNum == PARTY_A) {
    CONST_ONE = vector<mpc_t>(size, 1 << GetMpcContext()->FLOAT_PRECISION);
  }
  FastEqual(a, b, c);
  subtractVectors<mpc_t>(CONST_ONE, c, c, size);

  AUDIT("id:{}, P{} FastNotEqual, output Z(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(c));
  tlog_debug << "FastNotEqual ...";
  return 0;
}

}//snn
}//rosetta
