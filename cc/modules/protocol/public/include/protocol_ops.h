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

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <memory>
#include <vector>
#include <string>
#include <iostream>
#include "cc/modules/protocol/mpc/comm/include/mpc_defines.h"
#include "cc/modules/common/include/utils/msg_id.h"

namespace rosetta {

using std::cout;
using std::endl;
using std::shared_ptr;
using std::string;
using std::unordered_map;
using std::vector;
using attr_type = unordered_map<string, string>;

#define THROW_NOT_IMPL      \
  throw std::runtime_error( \
    string("please implements the interface " + string(__FUNCTION__) + string(" in subclass")))
#define THROW_NOT_IMPL_FN(name) \
  throw std::runtime_error(string("please implements '") + name + "' in subclass")

// This is the interface that each specific cryptographic protocol should implement!
class ProtocolOps {
 public:
  /**
   * message id for parallel communication
   */
  msg_id_t _op_msg_id;

  /** 
   * For now, some tools are passed frm ProtocolBase to its ProtocolOps in 'GetOps' directly.
   * We will refine this in later version.
   */
  // the same as config_map of its ProtocolBase class.
  unordered_map<string, string> op_config_map;
  //shared_ptr<ProtocolBase> _base_ptr;

 public:
  ProtocolOps(const msg_id_t& msg_id) : _op_msg_id(msg_id){};
  //ProtocolOps(shared_ptr<ProtocolBase> base_ptr): _base_ptr(base_ptr){};

  virtual ~ProtocolOps() = default;

  /**
   * @desc: encode the literal number to a protocol-specific format 
   *         wrapped as string.
   * @param:
   *     in, the vector of literal number
   *     out, the vector of the encoded strings
   * @return:
   *     0 if success, errcode otherwise
   */
  virtual int TfToSecure(
    const vector<string>& in,
    vector<string>& out,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  //! decode the string in protocol-specific format to literal number
  virtual int SecureToTf(
    const vector<string>& in,
    vector<string>& out,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int RandSeed(std::string op_seed, string& out_str) { THROW_NOT_IMPL; }
  virtual uint64_t RandSeed() { return 0x123456; }
  virtual uint64_t RandSeed(vector<uint64_t>& seed) { return 0x123456; }

  virtual int PrivateInput(int party_id, const vector<double>& in_x, vector<string>& out_x) {
    THROW_NOT_IMPL;
  }
  virtual int PublicInput(int party_id, const vector<double>& in_x, vector<string>& out_x) {
    THROW_NOT_IMPL;
  }

  virtual int Broadcast(int from_party, const string& msg, string& result) { THROW_NOT_IMPL; }

  // result must be allocate outside!
  virtual int Broadcast(int from_party, const char* msg, char* result, size_t size) { THROW_NOT_IMPL; }
  // virtual ssize_t Send(int party, const char* data, size_t size, std::string msgid) { THROW_NOT_IMPL; }
  // virtual ssize_t Recv(int party, char* data, size_t size, std::string msgid) { THROW_NOT_IMPL; }

  /**
   * @desc: This is for Tensorflow's SaveV2 Op.
   */
  virtual int ConditionalReveal(
    vector<string>& in_vec,
    vector<string>& out_cipher_vec,
    vector<double>& out_plain_vec) {
    std::cout << "calling ProtocolOps::ConditionalReveal!" << std::endl;
    if (op_config_map.find("save_mode") != op_config_map.end()) {
      cout << "DEBUG:" << op_config_map["save_mode"] << endl;
    }
    return -1;
  }

  //////////////////////////////////    math ops   //////////////////////////////////
  virtual int Add(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr);

  virtual int Sub(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int Mul(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int Div(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int Truediv(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int Floordiv(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  //// compare ops ////
  virtual int Less(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int LessEqual(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int Equal(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int NotEqual(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int Greater(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int GreaterEqual(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }
  //// compare ////

  virtual int Pow(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int Matmul(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }
  virtual int Rsqrt(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }
  
  virtual int Sqrt(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int Exp(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }


  virtual int Square(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int Negative(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int Abs(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  /**
   * @desc: get the derivative of |X| function by using MSB.
   * @note: you may override this in your derived class to have a customized algorithm
   */
  virtual int AbsPrime(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    cout << "TODO: base AbsPrime called" << endl;
    return 0;
  }

  virtual int Log(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int Log1p(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  /**
   * @desc: optional, for high-precision logarithm. 
   *    The default implementation in base class will use the Log.
   */
  virtual int HLog(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    return Log(a, output, attr_info);
  }

  virtual int Max(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int Min(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int Mean(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int Sum(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int Sum(const vector<string>& a, string& output, const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int AddN(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  ////////////////////////////////// nn ops //////////////////////////////////
  virtual int Relu(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int ReluPrime(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    cout << "TODO: base ReluPrime!" << endl;
    return 0;
  }

  virtual int Sigmoid(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int SigmoidCrossEntropy(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int BiasAdd(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  // get \sqrt{a}
  virtual int Sqrt(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  // get 1/\sqrt{a}
  virtual int Rsqrt(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int Invert(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int Exp(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  ////////////////////////////////// training ops //////////////////////////////////
  virtual int Reveal(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int Reveal(
    const vector<string>& a,
    vector<double>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }
  ////////////////////////////////// logical ops //////////////////////////////////
  virtual int AND(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }
  virtual int OR(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }
  virtual int XOR(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }
  virtual int NOT(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int Softmax(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }
};

} // namespace rosetta
