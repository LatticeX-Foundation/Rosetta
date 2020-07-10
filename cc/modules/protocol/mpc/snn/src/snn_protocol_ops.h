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
#include <unordered_map>
#include <vector>
#include "cc/modules/protocol/public/include/protocol_base.h"
#include "cc/modules/protocol/public/include/protocol_ops.h"
#include "cc/modules/io/include/ex.h"

#include "cc/modules/protocol/mpc/snn/include/snn_opsets.h"

namespace rosetta {
//namespace protocol {

// This is the interface that each specific cryptographic protocol should implement!
class SnnProtocolOps : public ProtocolOps {
 public:
  SnnProtocolOps(const string& msgid = "");

  //RttIO* GetNetIO();
  //void SetNetIO(RttIO* io);

  // template <typename T>
  int TfToSecure(const vector<string>& in, vector<string>& out, const attr_type* attr_info = nullptr);

  // decode the string in protocol-specific format to literal number
  // template <typename T>
  int SecureToTf(const vector<string>& in, vector<string>& out, const attr_type* attr_info = nullptr);

  int RandSeed(string op_seed, string& out_str);

  int PrivateInput(int party_id, const vector<double>& in_vec, vector<string>& out_str_vec);

  //////////////////////////////////    math ops   //////////////////////////////////
  int Add(
    const vector<string>& a, const vector<string>& b, vector<string>& output,
    const attr_type* attr_info = nullptr);

  int Sub(
    const vector<string>& a, const vector<string>& b, vector<string>& output,
    const attr_type* attr_info = nullptr);

  int Mul(
    const vector<string>& a, const vector<string>& b, vector<string>& output,
    const attr_type* attr_info = nullptr);

  int Div(
    const vector<string>& a, const vector<string>& b, vector<string>& output,
    const attr_type* attr_info = nullptr);

  int Truediv(
    const vector<string>& a, const vector<string>& b, vector<string>& output,
    const attr_type* attr_info = nullptr);

  int Floordiv(
    const vector<string>& a, const vector<string>& b, vector<string>& output,
    const attr_type* attr_info = nullptr);

  //// compare ops ////
  int Less(
    const vector<string>& a, const vector<string>& b, vector<string>& output,
    const attr_type* attr_info = nullptr);

  int LessEqual(
    const vector<string>& a, const vector<string>& b, vector<string>& output,
    const attr_type* attr_info = nullptr);

  int Equal(
    const vector<string>& a, const vector<string>& b, vector<string>& output,
    const attr_type* attr_info = nullptr);

  int NotEqual(
    const vector<string>& a, const vector<string>& b, vector<string>& output,
    const attr_type* attr_info = nullptr);

  int Greater(
    const vector<string>& a, const vector<string>& b, vector<string>& output,
    const attr_type* attr_info = nullptr);

  int GreaterEqual(
    const vector<string>& a, const vector<string>& b, vector<string>& output,
    const attr_type* attr_info = nullptr);
  //// compare ////

  int Pow(
    const vector<string>& a, const vector<string>& b, vector<string>& output,
    const attr_type* attr_info = nullptr);

  int Matmul(
    const vector<string>& a, const vector<string>& b, vector<string>& output,
    const attr_type* attr_info = nullptr);

  int Square(
    const vector<string>& a, vector<string>& output,
    const attr_type* attr_info = nullptr);

  int Negative(
    const vector<string>& a, vector<string>& output,
    const attr_type* attr_info = nullptr);

  int Abs(
    const vector<string>& a, vector<string>& output,
    const attr_type* attr_info = nullptr);

  int AbsPrime(
    const vector<string>& a, vector<string>& output,
    const attr_type* attr_info = nullptr);

  int Log(
    const vector<string>& a, vector<string>& output,
        const attr_type* attr_info = nullptr);

  int HLog(
    const vector<string>& a, vector<string>& output,
    const attr_type* attr_info = nullptr);
  
  int Log1p(
    const vector<string>& a, vector<string>& output,
    const attr_type* attr_info = nullptr);

  int Max(
    const vector<string>& a, vector<string>& output,
    const attr_type* attr_info = nullptr);

  int Min(
    const vector<string>& a, vector<string>& output,
    const attr_type* attr_info = nullptr);

  int Mean(
    const vector<string>& a, vector<string>& output,
    const attr_type* attr_info = nullptr);

  int Sum(
    const vector<string>& a, vector<string>& output,
    const attr_type* attr_info = nullptr);

  int AddN(
    const vector<string>& a, vector<string>& output,
    const attr_type* attr_info = nullptr);

  ////////////////////////////////// nn ops //////////////////////////////////
  int Relu(
    const vector<string>& a, vector<string>& output,
    const attr_type* attr_info = nullptr);
  
  int ReluPrime(
    const vector<string>& a, vector<string>& output,
    const attr_type* attr_info = nullptr);

  int Sigmoid(
    const vector<string>& a, vector<string>& output,
    const attr_type* attr_info = nullptr);

  int SigmoidCrossEntropy(
    const vector<string>& a, const vector<string>& b, vector<string>& output,
    const attr_type* attr_info = nullptr);

  int Reveal(
    const vector<string>& a, vector<string>& output,
    const attr_type* attr_info = nullptr);

  /**
    @desc: This is for Tensorflow's SaveV2 Op.

  */
  int ConditionalReveal(vector<string>& in_vec,
                        vector<string>& out_cipher_vec,
                        vector<double>& out_plain_vec);
public:
  shared_ptr<NET_IO> GetNetHandler() { return net_io_;}

public:
  shared_ptr<NET_IO> net_io_ = nullptr;

};

//} // namespace protocol
} // namespace rosetta