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
#include <cmath>

namespace rosetta {
namespace snn {

// clang-format off
static int validate_input(const vector<mpc_t> &a, const vector<mpc_t> &b, const vector<mpc_t>& c, size_t size)
{
  if (a.size() == 0)
  {
    cout << "-----  error, input with bad size ------" << endl;
    return -1;
  }

  if (a.size() != b.size() || a.size() != c.size())
  {
    cout << "-----  error, input with size not match ------" << endl;
    return -1;
  }

  if (a.size() != size)
  {
    cout << "-----  error, input size not match with specified size------" << endl;
    return -1;
  }

  return 0;
}

// retired! NOT use this any more! Use FastEqual.
int Equal::funcEqual(const vector<mpc_t> &a, const vector<mpc_t> &b, vector<mpc_t> &c, size_t size)
{
  if (validate_input(a, b, c, size) != 0)
  {
    cout << "invald inputs:  " << __FILE__ << ":" << __LINE__ << "  " << __FUNCTION__ << endl;
    cout.flush();
    throw;
  }
  
  // compare all the elements
  //sub then compare with zero
  //1. calculate greaterEqual ==> A
  vector<mpc_t> A(a.size(), 0);
  ////GreaterEqual(a, b, A, a.size());
  GetMpcOpInner(GreaterEqual)->Run(a, b, A, a.size());

  //2. calculate lessEqual ==> B
  vector<mpc_t> B(a.size(), 0);
  ////LessEqual(a, b, B, a.size());
  GetMpcOpInner(LessEqual)->Run(a, b, B, a.size());

  //3. result c = A + B - 1
  for (size_t i = 0; i < a.size(); ++i)
  {
    c[i] = A[i] + B[i] - FloatToMpcType(1)/2;
  }

  //cout << ":) mpc equal ok." << endl;
  return 0;
}

int Equal::funcFastEqual(const vector<mpc_t> &a, const vector<mpc_t> &b, vector<mpc_t> &c, size_t size)
{
  if (validate_input(a, b, c, size) != 0)
  {
    log_error << "invald inputs:  " << __FILE__ << ":" << __LINE__ << "  " << __FUNCTION__ << endl;
    throw;
  }
  vector<mpc_t> z(size, 0);
  subtractVectors<mpc_t>(a, b, z, size);

  vector<mpc_t> r_0(size, 0);
  vector<mpc_t> r_1(size, 0);
  vector<mpc_t> r(size, 0);
  if (HELPER) {
    // fixed for debuging
    // r_0 = vector<mpc_t>(size, 2 << FLOAT_PRECISION_M);
    // r_1 = vector<mpc_t>(size, 3 << FLOAT_PRECISION_M);
    // r = vector<mpc_t>(size, 5 << FLOAT_PRECISION_M);
    populateRandomVector<mpc_t>(r_0, size, "a_1", "POSITIVE");
    populateRandomVector<mpc_t>(r_1, size, "a_2", "POSITIVE");
    addVectors<mpc_t>(r_0, r_1, r, size);
  } else if (partyNum == PARTY_A) {
    // fixed for debuging
    // r_0 = vector<mpc_t>(size, 2 << FLOAT_PRECISION_M);
    populateRandomVector<mpc_t>(r_0, size, "a_1", "POSITIVE");
    r = r_0;
  } else if (partyNum == PARTY_B) {
    // fixed for debuging
    // r_1 = vector<mpc_t>(size, 3 << FLOAT_PRECISION_M);
    populateRandomVector<mpc_t>(r_1, size, "a_2", "POSITIVE");
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
  }
  if(HELPER) {
    for(size_t i = 0; i < size; ++i) {
      r_1_b[i] = r[i] ^ r_0_b[i];
    }
    sendVector<mpc_t>(r_1_b, PARTY_B, size);
  }
  if(partyNum == PARTY_B) {
    receiveVector<mpc_t>(r_1_b, PARTY_C, size);
  }

  // GetMpcOpInner(Reconstruct2PC)->Run(z, z.size(), "input z res");
  // reveal plaintext: Z + r
  addVectors<mpc_t>(z, r, z, size);
  // GetMpcOpInner(Reconstruct2PC)->Run(r, r.size(), "mask r res");
  // GetMpcOpInner(Reconstruct2PC)->Run(z, z.size(), "masked Z res");
  vector<mpc_t> plain_z(size, 0);
  if(PRIMARY) {
    thread* exchange_threads = new thread[2];
    exchange_threads[0] = thread(&OpBase_::sendVector<mpc_t>, 
                            this, ref(z), adversary(partyNum), size);
    exchange_threads[1] = thread(&OpBase_::receiveVector<mpc_t>, 
                            this, ref(plain_z), adversary(partyNum), size);
    exchange_threads[0].join();
    exchange_threads[1].join();
    delete[] exchange_threads;

    addVectors<mpc_t>(z, plain_z, plain_z, size);
    // print_vec(plain_z, size, "plain masked Z");

    for (size_t i = 0; i < size; ++i) {
      // cout << "debug:" << i << "-th local bit-share: " << r_0_b[i] << " and " << plain_z[i];
      mpc_t tmp_v = ~ (r_0_b[i] ^ plain_z[i]);
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
  FanInBitAdd(bit_share, res, size);
  // GetMpcOpInner(ReconstructBit2PC)->Run(res, res.size(), "Debug BitAdd res");
  B2A(res, c, size);
  // GetMpcOpInner(Reconstruct2PC)->Run(c, c.size(), "Final Equal res");
  return 0;
}

int Equal::FanInBitAdd(const vector<vector<small_mpc_t>>& a, vector<small_mpc_t>& c, size_t vec_size) {
  // S0: check and init
  //    the inner vector must have the same size 
  // normally, the LEN should be sizeof(mpc_t) * 8
  // cout << "FANInbitAdd size:" << a.size() << " of inner:" << a[0].size() << endl;
  int LEN = a[0].size();
  for (int i = 0; i < vec_size; ++i) {
    assert(a[i].size() == LEN);
  }

  vector<vector<small_mpc_t>> all_bits = a;
  int Depth = int(log2(LEN));
  assert((1 << Depth) == LEN && "Only 2^i len is supported!");

  vector<small_mpc_t> flat_a_vec;
  vector<small_mpc_t> flat_b_vec;
  int curr_len = LEN;
  // This is just a tricky shortcut
  auto ptr = std::make_shared<rosetta::snn::DotProduct>(msg_id(), io);
  // this is binary-tree strategy to perform AND in parallel.
  for(int i = 0; i < Depth; ++i) {
    // cout << "depth:" << i << endl;
    flat_a_vec.clear();
    flat_b_vec.clear();
    // flat matrix to vector
    for (int k = 0; k < vec_size; ++k) {
      for (int j = 0; j < curr_len -1; j=j+2) {
        flat_a_vec.push_back(all_bits[k][j]);
        flat_b_vec.push_back(all_bits[k][j+1]);
      }
    }
    vector<small_mpc_t> flat_c_vec(flat_a_vec.size(),0);
    ptr->BitMul(flat_a_vec, flat_b_vec, flat_c_vec, flat_a_vec.size());
    // for debuging
    // if(PRIMARY) {
    //   GetMpcOpInner(ReconstructBit2PC)->Run(flat_a_vec, flat_a_vec.size(), "Debug Bit A");
    //   GetMpcOpInner(ReconstructBit2PC)->Run(flat_b_vec, flat_b_vec.size(), "Debug Bit B");
    //   GetMpcOpInner(ReconstructBit2PC)->Run(flat_c_vec, flat_c_vec.size(), "Debug Bit C");
    // }

    // reconstruct matrix from vector 
    curr_len = curr_len / 2;
    // cout << "new len:" << curr_len << endl;
    for(int k = 0; k < vec_size; ++k) {
      all_bits[k].clear();
      all_bits[k].insert(all_bits[k].begin(), flat_c_vec.begin() + k * curr_len, flat_c_vec.begin() + (k+1) * curr_len);
    }
  }

  for(int i = 0; i < vec_size; ++i) {
    // make sure result is legal: all size should be 1! 
    // cout << "check[" << i << "]:" << all_bits[i].size() << ":" << (all_bits[i][0] & 0x01) << std::endl; 
    c[i] = all_bits[i][0] & 0x01;
  }
  return 0;
}

int Equal::B2A(const vector<small_mpc_t>& bit_shares, vector<mpc_t>& arith_shares, size_t size) {
  vector<small_mpc_t> bit_m_0(size, 0);
  vector<small_mpc_t> bit_m_1(size, 0);
  vector<small_mpc_t> bit_m(size, 0);
  vector<mpc_t> m_0(size, 0);
  vector<mpc_t> m_1(size, 0);
  vector<mpc_t> m(size, 0);

  if (HELPER) {
    populateRandomVector<small_mpc_t>(bit_m_0, size, "a_1", "POSITIVE");
    populateRandomVector<mpc_t>(m_0, size, "a_1", "POSITIVE");
    populateRandomVector<small_mpc_t>(bit_m_1, size, "a_2", "POSITIVE");
    for (int i = 0; i < size; ++i) {
      bit_m[i] = (bit_m_0[i] ^ bit_m_1[i]) & 0x01;
      m_1[i] = (mpc_t)(bit_m[i] << FLOAT_PRECISION_M) - m_0[i];
    }
    sendVector<mpc_t>(m_1, PARTY_B, size);
  }
  if (partyNum == PARTY_A) {
    populateRandomVector<small_mpc_t>(bit_m_0, size, "a_1", "POSITIVE");
    populateRandomVector<mpc_t>(m_0, size, "a_1", "POSITIVE");
    bit_m = bit_m_0;
    m = m_0;
  } 
  if (partyNum == PARTY_B) {
    populateRandomVector<small_mpc_t>(bit_m_1, size, "a_2", "POSITIVE");
    bit_m = bit_m_1;
    receiveVector<mpc_t>(m_1, PARTY_C, size);
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
      exchange_threads[0] = thread(
        &OpBase_::sendBitVector, this, ref(masked_bit_shares), adversary(partyNum), size);
      exchange_threads[1] = thread(
        &OpBase_::receiveBitVector, this, ref(other_part), adversary(partyNum), size);
    exchange_threads[0].join();
    exchange_threads[1].join();
    delete[] exchange_threads;
    
    for (int i = 0; i < size; ++i) {
      plain_bit[i] = (masked_bit_shares[i] ^ other_part[i]) & 0x01;
      
      if (plain_bit[i] == 0) {
        arith_shares[i] = m[i];
      } else {
        if (partyNum == PARTY_A) {
          arith_shares[i] = (1 << FLOAT_PRECISION_M) - m[i];
        }
        if (partyNum == PARTY_B) {
          arith_shares[i] = -m[i];
        }
      }
    }
  }
  return 0;
}

int NotEqual::funcNotEqual(const vector<mpc_t> &a, const vector<mpc_t> &b, vector<mpc_t> &c, size_t size)
{
  if (validate_input(a, b, c, size) != 0)
  {
    cout << "invald inputs:  " << __FILE__ << ":" << __LINE__ << "  " << __FUNCTION__ << endl;
    cout.flush();
    throw;
  }

  // compare all the elements
  //sub then compare with zero
  //1. calculate greaterEqual ==> A
  vector<mpc_t> A(a.size(), 0);
  ////GreaterEqual(a, b, A, a.size());
  GetMpcOpInner(GreaterEqual)->Run(a, b, A, a.size());

  //2. calculate lessEqual ==> B
  vector<mpc_t> B(a.size(), 0);
  ////LessEqual(a, b, B, a.size());
  GetMpcOpInner(LessEqual)->Run(a, b, B, a.size());

  //3. result c = A + B - 1
  for (size_t i = 0; i < a.size(); ++i)
  {
    c[i] = FloatToMpcType(1) - (A[i] + B[i]);//2 - 1 == 1 (not equal) or 2 - 2 == 0 (equal)
  }

  //cout << ":) mpc not equal ok." << endl;
  return 0;
}

int NotEqual::funcFastNotEqual(const vector<mpc_t> &a, const vector<mpc_t> &b, vector<mpc_t> &c, size_t size)
{
  // Just (1 - Equal())
  vector<mpc_t> CONST_ONE(size, 0);
  if(partyNum == PARTY_A) {
    CONST_ONE = vector<mpc_t>(size, 1 << FLOAT_PRECISION_M);
  }
  GetMpcOpInner(Equal)->Run(a, b, c, size);
  subtractVectors<mpc_t>(CONST_ONE, c, c, size);
  return 0;
}

int Greater::funcGreater(const vector<mpc_t> &a, const vector<mpc_t> &b, vector<mpc_t> &c, size_t size)
{
  if (validate_input(a, b, c, size) != 0)
  {
    cout << "invald inputs:  " << __FILE__ << ":" << __LINE__ << "  " << __FUNCTION__ << endl;
    cout.flush();
    throw;
  }

  // compare all the elements
  //sub then compare with zero
  // //1. calculate greaterEqual ==> A
  // vector<mpc_t> A(a.size(), 0);
  // GreaterEqual(a, b, A, a.size());

  //2. calculate lessEqual ==> B
  vector<mpc_t> B(a.size(), 0);
  //sub then compare with zero
  vector<mpc_t> cmp(a.size(), 0);
  for (size_t i = 0; i < a.size(); ++i)
  {
    cmp[i] = b[i] - a[i];
  }

  ////funcRELUPrime3PC(cmp, B, size);
  GetMpcOpInner(ReluPrime)->Run3PC(cmp, B, size);

  // //3. calculate equal ==>  E = A + B - 1
  // vector<mpc_t> E(a.size(), 0);
  // for (size_t i = 0; i < a.size(); ++i)
  // {
  //     E[i] = A[i] + B[i] - 1;
  // }

  //4. calculate result ==> c = A - E = A - (A + B - 1) = 1 - B
  for (size_t i = 0; i < a.size(); ++i)
  {
    c[i] = FloatToMpcType(1)/2 - B[i];
  }

  //cout << ":) mpc Greater ok." << endl;
  return 0;
}

int Less::funcLess(const vector<mpc_t> &a, const vector<mpc_t> &b, vector<mpc_t> &c, size_t size)
{
  if (validate_input(a, b, c, size) != 0)
  {
    cout << "invald inputs:  " << __FILE__ << ":" << __LINE__ << "  " << __FUNCTION__ << endl;
    cout.flush();
    throw;
  }

  // compare all the elements
  //sub then compare with zero
  //1. calculate greaterEqual ==> A
  vector<mpc_t> A(a.size(), 0);
  vector<mpc_t> cmp(a.size(), 0);
  for (size_t i = 0; i < a.size(); ++i)
  {
    cmp[i] = a[i] - b[i];
  }

  ////funcRELUPrime3PC(cmp, A, size);
  GetMpcOpInner(ReluPrime)->Run3PC(cmp, A, size);

  //2. calculate lessEqual ==> B

  // //3. calculate equal ==>  E = A + B - 1
  // vector<mpc_t> E(a.size(), 0);
  // for (size_t i = 0; i < a.size(); ++i)
  // {
  //     E[i] = A[i] + B[i] - 1;
  // }

  //4. calculate result ==> c = B - E = B - (A + B - 1) = 1 - A
  for (size_t i = 0; i < a.size(); ++i)
  {
    c[i] = FloatToMpcType(1)/2 - A[i];
  }

  //cout << ":) mpc Less ok." << endl;
  return 0;
}

int GreaterEqual::funcGreaterEqual(const vector<mpc_t> &a, const vector<mpc_t> &b, vector<mpc_t> &c, size_t size)
{
  if (validate_input(a, b, c, size) != 0)
  {
    cout << "invald inputs:  " << __FILE__ << ":" << __LINE__ << "  " << __FUNCTION__ << endl;
    cout.flush();
    throw;
  }

  // compare all the elements
  //sub then compare with zero
  vector<mpc_t> cmp(a.size(), 0);
  for (size_t i = 0; i < a.size(); ++i)
  {
    cmp[i] = a[i] - b[i];
  }

  ////funcRELUPrime3PC(cmp, c, size);
  GetMpcOpInner(ReluPrime)->Run3PC(cmp, c, size);

  //cout << "------> :) => mpc GreaterEqual ok. <= <------" << endl;
  return 0;
}


int LessEqual::funcLessEqual(const vector<mpc_t> &a, const vector<mpc_t> &b, vector<mpc_t> &c, size_t size)
{
  if (validate_input(a, b, c, size) != 0)
  {
    cout << "invald inputs:  " << __FILE__ << ":" << __LINE__ << "  " << __FUNCTION__ << endl;
    cout.flush();
    throw;
  }

    // compare all the elements
    //sub then compare with zero
    vector<mpc_t> cmp(a.size(), 0);
    for (size_t i = 0; i < a.size(); ++i)
    {
        cmp[i] = b[i] - a[i];
    }

    ////funcRELUPrime3PC(cmp, c, size);
    GetMpcOpInner(ReluPrime)->Run3PC(cmp, c, size);

    //cout << "------> :) => mpc LessEqual ok. <= <------" << endl;
  return 0;
}
// clang-format on

} // namespace mpc
} // namespace rosetta
