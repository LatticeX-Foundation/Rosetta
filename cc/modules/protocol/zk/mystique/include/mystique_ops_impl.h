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
#include <string>
#include <vector>
#include <unordered_map>

#include "cc/modules/protocol/public/include/protocol_base.h"
#include "cc/modules/protocol/public/include/protocol_ops.h"

#include "cc/modules/protocol/zk/mystique/include/wvr_util.h"

namespace rosetta {
class MystiqueOpsImpl : public ProtocolOps {
 public:
  MystiqueOpsImpl(const msg_id_t& msg_id, shared_ptr<ProtocolContext> context)
      : ProtocolOps(msg_id, context) {}
  // MystiqueOpsImpl(const msg_id_t& msg_id) : ProtocolOps(msg_id) {}
  ~MystiqueOpsImpl() = default;

  virtual int PrivateInput(const string& node_id, const vector<double>& in_x, vector<string>& out_x);
  virtual int PrivateInput(int party_id, const vector<double>& in_x, vector<string>& out_x);
  virtual int PublicInput(const string& node_id, const vector<double>& in_x, vector<string>& out_x);
  virtual int PublicInput(int party_id, const vector<double>& in_x, vector<string>& out_x);

  int PublicInput(const string& node_id, const vector<double>& in_x, vector<ZkIntFp>& out_x);
  int PublicInput(int party_id, const vector<double>& in_x, vector<ZkIntFp>& out_x);
  
  int Broadcast(const string& from_node, const char* msg, char* result, size_t size);
  int Broadcast(int from_party, const char* msg, char* result, size_t size);

  int Broadcast(const string& from_node, const string& msg, string& result);
  int Broadcast(int from_party, const string& msg, string& result);
  

  int Add(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr);

  int Sub(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr);

  int Mul(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr);

  int Matmul(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr);

  int Square(const vector<string>& a, vector<string>& output, const attr_type* attr_info = nullptr);

  int Negative(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr);

  int Mean(const vector<string>& a, vector<string>& output, const attr_type* attr_info = nullptr);

  int Max(const vector<string>& a, vector<string>& output, const attr_type* attr_info = nullptr);

  // get \sqrt{a}
  virtual int Sqrt(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr);

  // get 1/\sqrt{a}
  virtual int Rsqrt(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr);

  int Invert(const vector<string>& a, vector<string>& output, const attr_type* attr_info = nullptr)
    override;

  int Exp(const vector<string>& a, vector<string>& output, const attr_type* attr_info = nullptr)
    override;

  int Relu(const vector<string>& a, vector<string>& output, const attr_type* attr_info = nullptr);

  int Div(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr);

  int Truediv(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr) override;

  int Reveal(const vector<string>& a, vector<string>& output, const attr_type* attr_info = nullptr);

  int Reveal(const vector<string>& a, vector<double>& output, const attr_type* attr_info = nullptr);

  int Softmax(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr);

  int Sigmoid(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr);
  // ********* internal!

private:
  int Exp_Approx(const vector<string>& a, vector<string>& output, const attr_type* attr_info = nullptr);
  int Exp_Exact(const vector<string>& a, vector<string>& output, const attr_type* attr_info = nullptr);

 protected:
  int _Inner_Invert(const vector<ZkIntFp>& a, vector<ZkIntFp>& output);
  int _Inner_Div(const vector<ZkIntFp>& a, const vector<ZkIntFp>& b, vector<ZkIntFp>& output);
  int _Inner_Sqrt(const vector<ZkIntFp>& a, vector<ZkIntFp>& output);

  /** Note: 
   * extra_scale_multiplier, how many extra scales should be truncated by Float.
   * rhs_broadcast: whether the `b` should be broadcasted to align size with a.
   */ 
  template<typename T>
  int _Inner_Mul(const vector<ZkIntFp>& a, const vector<T>& b, vector<ZkIntFp>& output, int extra_scale_multiplier = 1, bool rhs_broadcast = false);
  int _Inner_Mul(const vector<uint64_t>& a, const vector<uint64_t>& b, vector<uint64_t>& output, int a_scale = 1, int b_scale = 1);

 public:
  shared_ptr<NET_IO> io = nullptr;
  int port, party;

  // TODO: find a right place to hold these.
  ZK_NET_IO* zk_ios[THREAD_NUM + 1];
};

} // namespace rosetta
