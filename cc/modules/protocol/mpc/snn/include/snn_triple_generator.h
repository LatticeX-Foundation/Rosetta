
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
#include "cc/modules/protocol/mpc/comm/include/mpc_protocol.h"
#include "cc/modules/protocol/mpc/snn/include/aesobjects_controller.h"
#include "cc/modules/protocol/public/include/protocol_ops.h"

#include <unordered_map>
using namespace std;

#define ENABLE_OFFLINE_TRIPLES_MODE 0

#define SNN_OFFLINE_MUL_TRIPLE_NUM 120000
namespace rosetta {
namespace snn {

// Note: 
//  For mul-triples, this is a container for element-wise triple
//  For matmul-triples, this is a single flatten matrix a * b = c
struct ShareTriple {

vector<mpc_t> a;
vector<mpc_t> b;
vector<mpc_t> c;

// only for mul-triple
int size() {
  // assert(a.size() == b.size() && b.size() == c.size());
  // log_debug << "ShareTriple.size" << a.size() << b.size() << c.size();
  return a.size();
}

};

using rosetta::ProtocolContext;
// only for computation-node
class SnnTripleGenerator {
public:
  // For now, only basic MUL-triples are generated ahead
  SnnTripleGenerator(shared_ptr<NET_IO> io_channel, 
                    int cache_batch_size = SNN_OFFLINE_MUL_TRIPLE_NUM)
                    : net_io_(io_channel), 
                      max_batch_size(cache_batch_size) {
                        log_info << "calling SnnTripleGenerator:" << cache_batch_size;
                      }
  
  // offline triple generation
  int pre_gen();

  // todo: in this version, op_msg_id is ignored. So this does NOT support OP-parallel yet! 
  // the generated result has element-wise relationship: triple_a * triple_b = triple_c
  int get_mul_triple(const msg_id_t& op_msg_id, int size, vector<mpc_t>& triple_a, vector<mpc_t>& triple_b, vector<mpc_t>& triple_c);

  // get the matmul triple, corresponding to op_msg_id 
  // the generated result relationship: triple_a[m, k] * triple_b[k, n] = triple_c[m, n]
  int get_matmul_triple(const string& op_msg_id, int m, int k, int n, vector<mpc_t>& triple_a, vector<mpc_t>& triple_b, vector<mpc_t>& triple_c);

  // init context
  int init(shared_ptr<rosetta::ProtocolContext> context, shared_ptr<SnnAesobjectsController> aes_controller) {
    context_ = context;
    aes_controller_ = aes_controller;
    return 1;
  }

  // clang-format off
  int64_t bytes_sent() const noexcept { return bytes_sent_.load(); }
  int64_t bytes_received() const noexcept { return bytes_received_.load(); }
  int64_t message_sent() const noexcept { return message_sent_.load(); }
  int64_t message_received() const noexcept { return message_received_.load(); }
  // clang-format on

private:
  // check whether all related member variables are ready to use.
  bool _is_init() {
    if(net_io_ == nullptr || context_ == nullptr || aes_controller_ == nullptr){
      return false;
    }
    return true;
  }

  // online generate mul-triple and append it to mul_triple_cache 
  void extend_mul_cache(const msg_id_t&, int size);

  // not using the cache, but to directly generate these triples
  void _direct_gen_mul(const msg_id_t& msg_id, int big_size, vector<mpc_t>& triple_a, vector<mpc_t>& triple_b, vector<mpc_t>& triple_c);

  // online generate matmul-triple and append it to matmul_triple_cache
  void extend_matmul_cache(string msg_id, int size, int m, int k, int n);

  // temp solution, borrowed from populateRandomVector of OpBase_.
  template <typename T>
  void _polulate_random_vector(vector<T>& vec, size_t size, string r_type, string neg_type, const msg_id_t& msg_id);

  // todo: delete these, since these are also borrowed from OpBase_ directly. 
  void sendBuf(int player, const char* buf, int length, const msg_id_t& msg_id, int conn = 0);
  void receiveBuf(int player, char* buf, int length, const msg_id_t& msg_id, int conn = 0);

private:
  // for communication, this pointer should be the same as the one in SnnProtocol.
  shared_ptr<NET_IO> net_io_ = nullptr;
  // for getting some info like my roles.
  shared_ptr<rosetta::ProtocolContext> context_ = nullptr;
  // for getting AESObjects
  shared_ptr<SnnAesobjectsController> aes_controller_ = nullptr;

  int max_batch_size = 0;
  static msg_id_t offline_mul_msg_id;

  std::mutex mul_cache_lck;
  ShareTriple mul_triple_cache;

  std::mutex matmul_cache_lck;
  // msg_id_m_n, -->  m*n flatten matrix triples 
  unordered_map<string, vector<ShareTriple>> matmul_triple_cache;

  // statistics
  atomic<int64_t> bytes_sent_{0};
  atomic<int64_t> bytes_received_{0};
  atomic<int64_t> message_sent_{0};
  atomic<int64_t> message_received_{0};

};
}
}