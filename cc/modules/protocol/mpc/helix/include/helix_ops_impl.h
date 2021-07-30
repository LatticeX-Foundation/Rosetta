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
#include "cc/modules/protocol/mpc/helix/include/helix_internal.h"
#include "cc/modules/protocol/mpc/helix/include/helix_def.h"

namespace rosetta {
namespace helix {
class HelixInternal;
}
} // namespace rosetta
using namespace rosetta::helix;

namespace rosetta {
class HelixOpsImpl : public ProtocolOps {
 public:
  shared_ptr<NET_IO> io = nullptr;
  shared_ptr<HelixInternal> hi = nullptr;
  HelixOpsImpl(const msg_id_t& msg_id, shared_ptr<ProtocolContext> context) : ProtocolOps(msg_id, context) {}
  ~HelixOpsImpl() = default;

  int TfToSecure(
    const vector<string>& in,
    vector<string>& out,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  int SecureToTf(
    const vector<string>& in,
    vector<string>& out,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  int RandSeed(std::string op_seed, string& out_str);
  uint64_t RandSeed();
  uint64_t RandSeed(vector<uint64_t>& seed);

  virtual int PrivateInput(const string& node_id, const vector<double>& in_x, vector<string>& out_x);
  int PublicInput(const string& node_id, const vector<double>& in_x, vector<string>& out_x);

  virtual int Broadcast(const string& node_id, const string& msg, string& result);
  virtual int Broadcast(const string& node_id, const char* msg, char* result, size_t size);

  //////////////////////////////////    math ops   //////////////////////////////////
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
  // Note that if 'b' is zero in plaintext, the result is undefined.
  int Div(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr);
  // the same as 'Div'
  int Truediv(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr);
  int Floordiv(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr);

  //// compare ops ////
  int Less(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr);
  int LessEqual(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr);
  int Equal(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr);
  int NotEqual(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr);
  int Greater(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr);
  int GreaterEqual(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr);
  //// compare ////

  int Pow(
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

  int Reciprocaldiv(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr);
  int Exp(const vector<string>& a, vector<string>& output, const attr_type* attr_info = nullptr);
  
  int Rsqrt(const vector<string>& a, vector<string>& output, const attr_type* attr_info = nullptr);

  int Sqrt(const vector<string>& a, vector<string>& output, const attr_type* attr_info = nullptr);

  int Negative(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr);

  int Log(const vector<string>& a, vector<string>& output, const attr_type* attr_info = nullptr);
  int HLog(const vector<string>& a, vector<string>& output, const attr_type* attr_info = nullptr);
  int Log1p(const vector<string>& a, vector<string>& output, const attr_type* attr_info = nullptr);

  int Max(const vector<string>& a, vector<string>& output, const attr_type* attr_info = nullptr);
  int Min(const vector<string>& a, vector<string>& output, const attr_type* attr_info = nullptr);
  int Mean(const vector<string>& a, vector<string>& output, const attr_type* attr_info = nullptr);
  int Sum(const vector<string>& a, vector<string>& output, const attr_type* attr_info = nullptr);
  int Sum(const vector<string>& a, string& output, const attr_type* attr_info = nullptr);
  int AddN(const vector<string>& a, vector<string>& output, const attr_type* attr_info = nullptr);

  ////////////////////////////////// nn ops //////////////////////////////////
  int Relu(const vector<string>& a, vector<string>& output, const attr_type* attr_info = nullptr);
  int ReluPrime(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) override;

  int Abs(const vector<string>& a, vector<string>& output, const attr_type* attr_info = nullptr);
  int AbsPrime(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr);

  int Sigmoid(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr);
  int SigmoidCrossEntropy(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr);

  ////////////////////////////////// training ops //////////////////////////////////
  int Reveal(const vector<string>& a, vector<string>& output, const attr_type* attr_info = nullptr);
  int Reveal(const vector<string>& a, vector<double>& output, const attr_type* attr_info = nullptr);

  int ConditionalReveal(
    vector<string>& in_vec,
    vector<string>& out_cipher_vec,
    vector<double>& out_plain_vec);

  ////////////////////////////////// logical ops //////////////////////////////////
  int AND(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr);
  int OR(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr);
  int XOR(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr);
  int NOT(const vector<string>& a, vector<string>& output, const attr_type* attr_info = nullptr);
};

} // namespace rosetta
