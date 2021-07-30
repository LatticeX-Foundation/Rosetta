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

#include "cc/modules/protocol/mpc/snn/include/snn_tools.h"
#include "cc/modules/protocol/mpc/snn/include/snn_triple_generator.h"
// #include "cc/modules/protocol/mpc/snn/include/snn_opsets.h"

#include <thread>
using namespace std;

namespace rosetta {
namespace snn {
using rosetta::ProtocolContext;
msg_id_t SnnTripleGenerator::offline_mul_msg_id(string("offline_mul_msg_id"));

int SnnTripleGenerator::pre_gen() {
  log_debug << "begin SnnTripleGenerator::pre_gen()";
  // TODO: If I am not computation node, just return!

  if(!_is_init()) {
    log_debug << "Fail to pre_gen due to SnnTripleGenerator has not been inited correctly!";
    return 1;
  }
  
  tlog_debug << "DEBUG role:" << context_->GetMyRole();
  // aes_controller_->GetKeys().keys.print();

  
  extend_mul_cache(offline_mul_msg_id, max_batch_size);
  tlog_debug << "===========end DEBUG pre_gen";
  return 0;
}

int SnnTripleGenerator::get_mul_triple(const msg_id_t& op_msg_id, int size, vector<mpc_t>& triple_a, vector<mpc_t>& triple_b, vector<mpc_t>& triple_c){
  tlog_debug << "SnnTripleGenerator::get_mul_triple() :" << op_msg_id.str() << " size:" << size;
  if( size >= max_batch_size) {
    tlog_info << "Attention! Too much triples [" << size <<"] is needed in one use!, calling _direct_gen_mul!";
    _direct_gen_mul(op_msg_id, size, triple_a, triple_b, triple_c);
    return size;
  }
  std::unique_lock<std::mutex> cache_lock(mul_cache_lck);
  // std::lock_guard<std::mutex> lock(mul_cache_lck);
  // mul_cache_lck.lock();
  int cache_size = mul_triple_cache.size();
  
  tlog_debug << "curr cache size:" << mul_triple_cache.a.size();

  // cache has enough triples
  if(size < cache_size) {
    tlog_debug << "try to use the cache directly!";
    triple_a.clear();
    triple_b.clear();
    triple_c.clear();

    triple_a.insert(triple_a.end(), std::make_move_iterator(mul_triple_cache.a.begin()), 
                    std::make_move_iterator(mul_triple_cache.a.begin() + size));
    triple_b.insert(triple_b.end(), std::make_move_iterator(mul_triple_cache.b.begin()), 
                    std::make_move_iterator(mul_triple_cache.b.begin() + size));
    triple_c.insert(triple_c.end(), std::make_move_iterator(mul_triple_cache.c.begin()), 
                    std::make_move_iterator(mul_triple_cache.c.begin() + size));    
    
    mul_triple_cache.a.erase(mul_triple_cache.a.begin(), mul_triple_cache.a.begin() + size);
    mul_triple_cache.b.erase(mul_triple_cache.b.begin(), mul_triple_cache.b.begin() + size);
    mul_triple_cache.c.erase(mul_triple_cache.c.begin(), mul_triple_cache.c.begin() + size);

    cache_size = mul_triple_cache.size();
    tlog_debug << "curr cache size:" << cache_size;

    // refill cache is cache is too small
    if(cache_size - size < int(max_batch_size/3)) {
      cache_lock.unlock();
      cache_lock.release();
      tlog_debug << "refilling the cache(Todo: in another thread)!";
      int gen_size = max_batch_size;
      // thread extend_thread = thread(&SnnTripleGenerator::extend_mul_cache, this, offline_mul_msg_id, gen_size);
      // extend_thread.join();
      extend_mul_cache(offline_mul_msg_id, gen_size);
      // tlog_error << "DEBUG SJJJJ STUB 444";
    } else {
      // tlog_error << "DEBUG SJJJJ STUB 2";
      cache_lock.unlock();
      cache_lock.release();
      // tlog_error << "DEBUG SJJJJ STUB 3";
    }
  } else {
    tlog_debug << "Mul triple cache is too small, try to refil it!";
    cache_lock.unlock();
    cache_lock.release();
    extend_mul_cache(offline_mul_msg_id, max_batch_size);
    // try again.
    tlog_debug << "Mul triple cache should have been refilled!, and we try to fetch again.";
    get_mul_triple(op_msg_id, size, triple_a, triple_b, triple_c);
  }

  AUDIT("id:{}, P{} gen_mul_triple output triple_a(mpc_t){}", op_msg_id.get_hex(), context_->GetMyRole(), Vector<mpc_t>(triple_a));
  AUDIT("id:{}, P{} gen_mul_triple output triple_b(mpc_t){}", op_msg_id.get_hex(), context_->GetMyRole(), Vector<mpc_t>(triple_b));
  AUDIT("id:{}, P{} gen_mul_triple output triple_c(mpc_t){}", op_msg_id.get_hex(), context_->GetMyRole(), Vector<mpc_t>(triple_c));
  return size;
}

int SnnTripleGenerator::get_matmul_triple(const string& op_msg_id, int m, int k, int n, vector<mpc_t>& triple_a, vector<mpc_t>& triple_b, vector<mpc_t>& triple_c) {

  tlog_debug << "todo SnnTripleGenerator::get_matmul_triple() :" << op_msg_id << " size:" << m << ", " << n;
  return 0;
}

 void SnnTripleGenerator::_direct_gen_mul(const msg_id_t& msg_id, int big_size, vector<mpc_t>& triple_a, vector<mpc_t>& triple_b, vector<mpc_t>& triple_c) {
  int size = big_size;
  vector<mpc_t> A(size, 0), B(size, 0), C(size, 0);
  int partyNum = context_->GetMyRole();
  if (HELPER) {
      vector<mpc_t> A1(size, 0), A2(size, 0), B1(size, 0), B2(size, 0), C1(size, 0), C2(size, 0);

      _polulate_random_vector<mpc_t>(A1, size, "a_1", "POSITIVE", msg_id);
      AUDIT("id:{}, P{} direct_gen_mul polulate_random_vector A1(mpc_t){}", msg_id.get_hex(), context_->GetMyRole(), Vector<mpc_t>(A1));
      _polulate_random_vector<mpc_t>(A2, size, "a_2", "POSITIVE", msg_id);
      AUDIT("id:{}, P{} direct_gen_mul polulate_random_vector A2(mpc_t){}", msg_id.get_hex(), context_->GetMyRole(), Vector<mpc_t>(A2));
      _polulate_random_vector<mpc_t>(B1, size, "a_1", "POSITIVE", msg_id);
      AUDIT("id:{}, P{} direct_gen_mul polulate_random_vector B1(mpc_t){}", msg_id.get_hex(), context_->GetMyRole(), Vector<mpc_t>(B1));
      _polulate_random_vector<mpc_t>(B2, size, "a_2", "POSITIVE", msg_id);
      AUDIT("id:{}, P{} direct_gen_mul polulate_random_vector B2(mpc_t){}", msg_id.get_hex(), context_->GetMyRole(), Vector<mpc_t>(B2));
      _polulate_random_vector<mpc_t>(C1, size, "a_1", "POSITIVE", msg_id);
      AUDIT("id:{}, P{} direct_gen_mul polulate_random_vector C1(mpc_t){}", msg_id.get_hex(), context_->GetMyRole(), Vector<mpc_t>(C1));

      addVectors<mpc_t>(A1, A2, A, size);
      AUDIT("id:{}, P{} direct_gen_mul compute: A=A1+A2, A(=A1+A2)(mpc_t){}", msg_id.get_hex(), context_->GetMyRole(), Vector<mpc_t>(A));
      addVectors<mpc_t>(B1, B2, B, size);
      AUDIT("id:{}, P{} direct_gen_mul compute: B=B1+B2, B(=B1+B2)(mpc_t){}", msg_id.get_hex(), context_->GetMyRole(), Vector<mpc_t>(B));

      for (size_t i = 0; i < size; ++i) {
        C[i] = A[i] * B[i];
      }

      subtractVectors<mpc_t>(C, C1, C2, size);
      AUDIT("id:{}, P{} direct_gen_mul compute: C2=C-C1, C2(=C+C1)(mpc_t){}", msg_id.get_hex(), context_->GetMyRole(), Vector<mpc_t>(C2));
      // sendVector<mpc_t>(C2, PARTY_B, size);
      sendBuf(PARTY_B, (const char*)C2.data(), size * sizeof(mpc_t), msg_id, 0);
      AUDIT("id:{}, P{} direct_gen_mul SEND to P{}, C2{}", msg_id.get_hex(), context_->GetMyRole(), PARTY_B, Vector<mpc_t>(C2));
    }

    if (PRIMARY) {
      if (partyNum == PARTY_A) {
        _polulate_random_vector<mpc_t>(A, size, "a_1", "POSITIVE", msg_id);
        AUDIT("id:{}, P{} direct_gen_mul polulate_random_vector A1(mpc_t){}", msg_id.get_hex(), context_->GetMyRole(), Vector<mpc_t>(A));
        _polulate_random_vector<mpc_t>(B, size, "a_1", "POSITIVE", msg_id);
        AUDIT("id:{}, P{} direct_gen_mul polulate_random_vector B1(mpc_t){}", msg_id.get_hex(), context_->GetMyRole(), Vector<mpc_t>(B));
        _polulate_random_vector<mpc_t>(C, size, "a_1", "POSITIVE", msg_id);
        AUDIT("id:{}, P{} direct_gen_mul polulate_random_vector C1(mpc_t){}", msg_id.get_hex(), context_->GetMyRole(), Vector<mpc_t>(C));
      }

      if (partyNum == PARTY_B) {
        _polulate_random_vector<mpc_t>(A, size, "a_2", "POSITIVE", msg_id);
        AUDIT("id:{}, P{} direct_gen_mul polulate_random_vector A2(mpc_t){}", msg_id.get_hex(), context_->GetMyRole(), Vector<mpc_t>(A));
        _polulate_random_vector<mpc_t>(B, size, "a_2", "POSITIVE", msg_id);
        AUDIT("id:{}, P{} direct_gen_mul polulate_random_vector B2(mpc_t){}", msg_id.get_hex(), context_->GetMyRole(), Vector<mpc_t>(B));
        // receiveVector<mpc_t>(C, PARTY_C, size);
        receiveBuf(PARTY_C, (char*)C.data(), size * sizeof(mpc_t), msg_id, 0);
        AUDIT("id:{}, P{} direct_gen_mul RECV from P{}, C2{}", msg_id.get_hex(), context_->GetMyRole(), PARTY_C, Vector<mpc_t>(C));
        //cout << "partyNum " << partyNum << ",  C[0]:" << C[0] << endl;
      }
    }
    triple_a.swap(A);
    triple_b.swap(B);
    triple_c.swap(C);

    AUDIT("id:{}, P{} direct_gen_mul output triple_a(mpc_t){}", msg_id.get_hex(), context_->GetMyRole(), Vector<mpc_t>(triple_a));
    AUDIT("id:{}, P{} direct_gen_mul output triple_b(mpc_t){}", msg_id.get_hex(), context_->GetMyRole(), Vector<mpc_t>(triple_b));
    AUDIT("id:{}, P{} direct_gen_mul output triple_c(mpc_t){}", msg_id.get_hex(), context_->GetMyRole(), Vector<mpc_t>(triple_c));
 }

// online generate mul-triple and append it to mul_triple_cache 
void SnnTripleGenerator::extend_mul_cache(const msg_id_t& msg_id, int size) {
  // for now the msg_id is ignored.
  tlog_debug << "calling SnnTripleGenerator::extend_mul_cache with size:" << size;
  // todo: this lock seems too broad, narrow its usage with another thread
  std::lock_guard<std::mutex> lock(mul_cache_lck);
  int curr_size = mul_triple_cache.size();
  int gen_size = size;
  // max bound 
  if (curr_size + gen_size > max_batch_size) {
    gen_size = max_batch_size - curr_size;
  }

  size = gen_size;
  vector<mpc_t> A(size, 0), B(size, 0), C(size, 0);
  int partyNum = context_->GetMyRole();
  if (HELPER) {
      vector<mpc_t> A1(size, 0), A2(size, 0), B1(size, 0), B2(size, 0), C1(size, 0), C2(size, 0);

      _polulate_random_vector<mpc_t>(A1, size, "a_1", "POSITIVE", msg_id);
      AUDIT("id:{}, P{} extend_mul_cache polulate_random_vector A1(mpc_t){}", msg_id.get_hex(), context_->GetMyRole(), Vector<mpc_t>(A1));
      _polulate_random_vector<mpc_t>(A2, size, "a_2", "POSITIVE", msg_id);
      AUDIT("id:{}, P{} extend_mul_cache polulate_random_vector A2(mpc_t){}", msg_id.get_hex(), context_->GetMyRole(), Vector<mpc_t>(A2));
      _polulate_random_vector<mpc_t>(B1, size, "a_1", "POSITIVE", msg_id);
      AUDIT("id:{}, P{} extend_mul_cache polulate_random_vector B1(mpc_t){}", msg_id.get_hex(), context_->GetMyRole(), Vector<mpc_t>(B1));
      _polulate_random_vector<mpc_t>(B2, size, "a_2", "POSITIVE", msg_id);
      AUDIT("id:{}, P{} extend_mul_cache polulate_random_vector B2(mpc_t){}", msg_id.get_hex(), context_->GetMyRole(), Vector<mpc_t>(B2));
      _polulate_random_vector<mpc_t>(C1, size, "a_1", "POSITIVE", msg_id);
      AUDIT("id:{}, P{} extend_mul_cache polulate_random_vector C1(mpc_t){}", msg_id.get_hex(), context_->GetMyRole(), Vector<mpc_t>(C1));

      addVectors<mpc_t>(A1, A2, A, size);
      AUDIT("id:{}, P{} extend_mul_cache compute: A=A1+A2, A(=A1+A2)(mpc_t){}", msg_id.get_hex(), context_->GetMyRole(), Vector<mpc_t>(A));
      addVectors<mpc_t>(B1, B2, B, size);
      AUDIT("id:{}, P{} extend_mul_cache compute: B=B1+B2, B(=B1+B2)(mpc_t){}", msg_id.get_hex(), context_->GetMyRole(), Vector<mpc_t>(B));

      for (size_t i = 0; i < size; ++i) {
        C[i] = A[i] * B[i];
      }

      AUDIT("id:{}, P{} extend_mul_cache compute: C=A+B, C(=A+B)(mpc_t){}", msg_id.get_hex(), context_->GetMyRole(), Vector<mpc_t>(C));
      subtractVectors<mpc_t>(C, C1, C2, size);
      AUDIT("id:{}, P{} extend_mul_cache compute: C2=C-C1, C2(=C-C1)(mpc_t){}", msg_id.get_hex(), context_->GetMyRole(), Vector<mpc_t>(C2));
      // sendVector<mpc_t>(C2, PARTY_B, size);
      sendBuf(PARTY_B, (const char*)C2.data(), size * sizeof(mpc_t), msg_id, 0);
      AUDIT("id:{}, P{} extend_mul_cache SEND to P{}, C2(mpc_t){}", msg_id.get_hex(), context_->GetMyRole(), PARTY_B, Vector<mpc_t>(C2));
    }

    if (PRIMARY) {
      if (partyNum == PARTY_A) {
        _polulate_random_vector<mpc_t>(A, size, "a_1", "POSITIVE", msg_id);
        AUDIT("id:{}, P{} extend_mul_cache polulate_random_vector A1(mpc_t){}", msg_id.get_hex(), context_->GetMyRole(), Vector<mpc_t>(A));

        _polulate_random_vector<mpc_t>(B, size, "a_1", "POSITIVE", msg_id);
        AUDIT("id:{}, P{} extend_mul_cache polulate_random_vector B1(mpc_t){}", msg_id.get_hex(), context_->GetMyRole(), Vector<mpc_t>(B));

        _polulate_random_vector<mpc_t>(C, size, "a_1", "POSITIVE", msg_id);
        AUDIT("id:{}, P{} extend_mul_cache polulate_random_vector C1(mpc_t){}", msg_id.get_hex(), context_->GetMyRole(), Vector<mpc_t>(C));
      }

      if (partyNum == PARTY_B) {
        _polulate_random_vector<mpc_t>(A, size, "a_2", "POSITIVE", msg_id);
        AUDIT("id:{}, P{} extend_mul_cache polulate_random_vector A2(mpc_t){}", msg_id.get_hex(), context_->GetMyRole(), Vector<mpc_t>(A));
        _polulate_random_vector<mpc_t>(B, size, "a_2", "POSITIVE", msg_id);
        AUDIT("id:{}, P{} extend_mul_cache polulate_random_vector B2(mpc_t){}", msg_id.get_hex(), context_->GetMyRole(), Vector<mpc_t>(B));
        // receiveVector<mpc_t>(C, PARTY_C, size);
        receiveBuf(PARTY_C, (char*)C.data(), size * sizeof(mpc_t), msg_id, 0);
        AUDIT("id:{}, P{} extend_mul_cache RECV from P{}, C2(mpc_t){}", msg_id.get_hex(), context_->GetMyRole(), PARTY_C, Vector<mpc_t>(C));
        //cout << "partyNum " << partyNum << ",  C[0]:" << C[0] << endl;
      }
    }

    if(curr_size == 0) {
      tlog_debug << "curr size is 0, so swap with new size: " << gen_size;
      mul_triple_cache.a.swap(A);
      mul_triple_cache.b.swap(B);
      mul_triple_cache.c.swap(C);
      tlog_debug << "offline gen DONE!:" << mul_triple_cache.size();
    } else {
      tlog_debug << "curr size is " << curr_size << ", and append with size" << gen_size;
      mul_triple_cache.a.insert(mul_triple_cache.a.end(), std::make_move_iterator(A.begin()), 
                    std::make_move_iterator(A.end()));
      mul_triple_cache.b.insert(mul_triple_cache.b.end(), std::make_move_iterator(B.begin()), 
                    std::make_move_iterator(B.end()));
      mul_triple_cache.c.insert(mul_triple_cache.c.end(), std::make_move_iterator(C.begin()), 
                    std::make_move_iterator(C.end()));
    }

}

void SnnTripleGenerator::sendBuf(int player, const char* buf, int length, const msg_id_t& msg_id, int conn /* = 0*/) {
  message_sent_.fetch_add(1);
  bytes_sent_.fetch_add(length);

#if OPEN_MPCOP_DEBUG_AND_CHECK
  {
    string key(msg_id.str());
    unique_lock<mutex> lck(g_key_stat_mtx);
    if (g_key_stat.find(key) != g_key_stat.end()) {
      g_key_stat[key]->message_sent.fetch_add(1);
      g_key_stat[key]->bytes_sent.fetch_add(length);
    }
  }
#endif

#if USE_NETIO_WITH_MESSAGEID
  net_io_->send(player, buf, length, msg_id);
#else
  net_io_->send(player, buf, length, 0);
#endif
}

void SnnTripleGenerator::receiveBuf(int player, char* buf, int length, const msg_id_t& msg_id, int conn /* = 0*/) {
  message_received_.fetch_add(1);
  bytes_received_.fetch_add(length);

#if OPEN_MPCOP_DEBUG_AND_CHECK
  {
    string key(msg_id.str());
    unique_lock<mutex> lck(g_key_stat_mtx);
    if (g_key_stat.find(key) != g_key_stat.end()) {
      g_key_stat[key]->message_received.fetch_add(1);
      g_key_stat[key]->bytes_received.fetch_add(length);
    }
  }
#endif

#if USE_NETIO_WITH_MESSAGEID
  net_io_->recv(player, buf, length, msg_id);
#else
  net_io_->recv(player, buf, length, 0);
#endif
}

// online generate matmul-triple and append it to matmul_triple_cache
void SnnTripleGenerator::extend_matmul_cache(string msg_id, int size, int m, int k, int n) {

}

template <typename T>
void SnnTripleGenerator::_polulate_random_vector(vector<T>& vec, size_t size, string r_type, string neg_type, const msg_id_t& msg_id){
  tlog_debug << "SnnTripleGenerator _polulate_random_vector with msg_id:" << msg_id.str();
  int partyNum = context_->GetMyRole();

  auto aeskey_objects = *(aes_controller_->Get(msg_id));
  auto aes_common = aeskey_objects.aes_common;
  auto aes_indep = aeskey_objects.aes_indep;
  auto aes_a_1 = aeskey_objects.aes_a_1;
  auto aes_b_1 = aeskey_objects.aes_b_1;
  auto aes_c_1 = aeskey_objects.aes_c_1;
  auto aes_a_2 = aeskey_objects.aes_a_2;
  auto aes_b_2 = aeskey_objects.aes_b_2;

  // assert((r_type == "COMMON" or r_type == "INDEP") && "invalid randomness type for
  // populateRandomVector");
  assert(
    (neg_type == "NEGATIVE" or neg_type == "POSITIVE") &&
    "invalid negativeness type for populateRandomVector");
  // assert(sizeof(T) == sizeof(mpc_t) && "Probably only need 64-bit numbers");
  // assert(r_type == "COMMON" && "Only common randomness mode required currently");

  mpc_t sign = 1;
  if (r_type == "COMMON") {
    if (neg_type == "NEGATIVE") {
      if (partyNum == PARTY_B || partyNum == PARTY_D)
        sign = MINUS_ONE;

      if (sizeof(T) == sizeof(mpc_t)) {
        for (size_t i = 0; i < size; ++i)
          vec[i] = sign * aes_common->get64Bits();
      } else {
        for (size_t i = 0; i < size; ++i)
          vec[i] = sign * aes_common->get8Bits();
      }
    }

    if (neg_type == "POSITIVE") {
      if (sizeof(T) == sizeof(mpc_t)) {
        for (size_t i = 0; i < size; ++i)
          vec[i] = aes_common->get64Bits();
      } else {
        for (size_t i = 0; i < size; ++i)
          vec[i] = aes_common->get8Bits();
      }
    }
  }

  if (r_type == "INDEP") {
    if (neg_type == "NEGATIVE") {
      if (partyNum == PARTY_B || partyNum == PARTY_D)
        sign = MINUS_ONE;

      if (sizeof(T) == sizeof(mpc_t)) {
        for (size_t i = 0; i < size; ++i)
          vec[i] = sign * aes_indep->get64Bits();
      } else {
        for (size_t i = 0; i < size; ++i)
          vec[i] = sign * aes_indep->get8Bits();
      }
    }

    if (neg_type == "POSITIVE") {
      if (sizeof(T) == sizeof(mpc_t)) {
        for (size_t i = 0; i < size; ++i)
          vec[i] = aes_indep->get64Bits();
      } else {
        for (size_t i = 0; i < size; ++i)
          vec[i] = aes_indep->get8Bits();
      }
    }
  }

  if (r_type == "a_1") {
    assert((partyNum == PARTY_A || partyNum == PARTY_C) && "Only A and C can call for a_1");
    assert(neg_type == "POSITIVE" && "neg_type should be POSITIVE");
    // assert(sizeof(T) == sizeof(mpc_t) && "sizeof(T) == sizeof(mpc_t)");
    // for (size_t i = 0; i < size; ++i)
    //  vec[i] = aes_a_1->get64Bits();
    if (sizeof(T) == sizeof(mpc_t)) {
      for (size_t i = 0; i < size; ++i)
        vec[i] = aes_a_1->get64Bits();
    } else {
      for (size_t i = 0; i < size; ++i)
        vec[i] = aes_a_1->get8Bits();
    }
  }

  if (r_type == "b_1") {
    assert((partyNum == PARTY_A || partyNum == PARTY_C) && "Only A and C can call for b_1");
    assert(neg_type == "POSITIVE" && "neg_type should be POSITIVE");
    // assert(sizeof(T) == sizeof(mpc_t) && "sizeof(T) == sizeof(mpc_t)");
    // for (size_t i = 0; i < size; ++i)
    //   vec[i] = aes_b_1->get64Bits();
    if (sizeof(T) == sizeof(mpc_t)) {
      for (size_t i = 0; i < size; ++i)
        vec[i] = aes_b_1->get64Bits();
    } else {
      for (size_t i = 0; i < size; ++i)
        vec[i] = aes_b_1->get8Bits();
    }      
  }

  if (r_type == "c_1") {
    assert((partyNum == PARTY_A || partyNum == PARTY_C) && "Only A and C can call for c_1");
    assert(neg_type == "POSITIVE" && "neg_type should be POSITIVE");
    // assert(sizeof(T) == sizeof(mpc_t) && "sizeof(T) == sizeof(mpc_t)");
    // for (size_t i = 0; i < size; ++i)
    //   vec[i] = aes_c_1->get64Bits();
    if (sizeof(T) == sizeof(mpc_t)) {
      for (size_t i = 0; i < size; ++i)
        vec[i] = aes_c_1->get64Bits();
    } else {
      for (size_t i = 0; i < size; ++i)
        vec[i] = aes_c_1->get8Bits();
    }       
  }

  if (r_type == "a_2") {
    assert((partyNum == PARTY_B || partyNum == PARTY_C) && "Only B and C can call for a_2");
    assert(neg_type == "POSITIVE" && "neg_type should be POSITIVE");
    // assert(sizeof(T) == sizeof(mpc_t) && "sizeof(T) == sizeof(mpc_t)");
    // for (size_t i = 0; i < size; ++i)
    //   vec[i] = aes_a_2->get64Bits();
    if (sizeof(T) == sizeof(mpc_t)) {
      for (size_t i = 0; i < size; ++i)
        vec[i] = aes_a_2->get64Bits();
    } else {
      for (size_t i = 0; i < size; ++i)
        vec[i] = aes_a_2->get8Bits();
    }            
  }

  if (r_type == "b_2") {
    assert((partyNum == PARTY_B || partyNum == PARTY_C) && "Only B and C can call for b_2");
    assert(neg_type == "POSITIVE" && "neg_type should be POSITIVE");
    // assert(sizeof(T) == sizeof(mpc_t) && "sizeof(T) == sizeof(mpc_t)");
    // for (size_t i = 0; i < size; ++i)
    //   vec[i] = aes_b_2->get64Bits();
    if (sizeof(T) == sizeof(mpc_t)) {
      for (size_t i = 0; i < size; ++i)
        vec[i] = aes_b_2->get64Bits();
    } else {
      for (size_t i = 0; i < size; ++i)
        vec[i] = aes_b_2->get8Bits();
    }       
  }
}

}
}
