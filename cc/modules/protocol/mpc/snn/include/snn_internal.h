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

#include <cassert>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <chrono>
#include <random>
#include <unordered_map>
using namespace std;

#include "cc/modules/common/include/utils/helper.h"
#include "cc/modules/common/include/utils/model_tool.h"
#include "cc/modules/common/include/utils/simple_timer.h"
#include "cc/modules/protocol/public/include/protocol_base.h"
#include "cc/modules/protocol/utility/include/prg.h"
#include "cc/modules/protocol/mpc/helix/include/helix_def.h"
#include "cc/modules/protocol/mpc/helix/include/helix_util.h"
// #include "cc/modules/protocol/mpc/snn/include/aesobjects.h"
#include "cc/modules/common/include/utils/str_type_convert.h"

#include "cc/modules/protocol/mpc/snn/include/snn_triple_generator.h"

namespace rosetta {
namespace snn {

class SnnInternal {
  SnnInternal& operator=(const SnnInternal&) = delete;

 protected:
  shared_ptr<NET_IO> io = nullptr;
  std::shared_ptr<RttPRG> gseed_ = nullptr;

  shared_ptr<SnnTripleGenerator> _triple_generator = nullptr;
  shared_ptr<SnnAesobjectsController> aes_controller_ = nullptr;

 protected:
  std::shared_ptr<AESObject> aes_randseed = nullptr;
  std::shared_ptr<AESObject> aes_common = nullptr;
  std::shared_ptr<AESObject> aes_indep = nullptr;
  std::shared_ptr<AESObject> aes_a_1 = nullptr;
  std::shared_ptr<AESObject> aes_a_2 = nullptr;
  std::shared_ptr<AESObject> aes_b_1 = nullptr;
  std::shared_ptr<AESObject> aes_b_2 = nullptr;
  std::shared_ptr<AESObject> aes_c_1 = nullptr;
  std::map<std::string, std::shared_ptr<AESObject>> aes_data; 

 protected:
  // statistics [will delete in near future]
  atomic<int64_t> bytes_sent_{0};
  atomic<int64_t> bytes_received_{0};
  atomic<int64_t> message_sent_{0};
  atomic<int64_t> message_received_{0};

  shared_ptr<ProtocolContext> context_ = nullptr;
  int partyNum;
  int num_of_parties;
  msg_id_t op_msg_id;

 public:
  SnnInternal() { }
  SnnInternal(const msg_id_t& msgid, shared_ptr<ProtocolContext> context, shared_ptr<RttPRG> prg_seed, shared_ptr<NET_IO> net_io) {
    op_msg_id = msgid;
    context_ = context; 
    partyNum = context_->GetMyRole();
    num_of_parties = context_->NODE_ROLE_MAPPING.size();
    gseed_ = prg_seed;
    io = net_io;
  }

  virtual ~SnnInternal() {}
  // virtual const msg_id_t& msg_id() const = 0;

  virtual void SetTripleGenerator(shared_ptr<SnnTripleGenerator> triple_generator) {
    log_debug << "calling SetTripleGenerator...";
    _triple_generator = triple_generator;
  }

  void SetAesController(shared_ptr<SnnAesobjectsController> aes_controller) {
    aes_controller_ = aes_controller;
    init_aes();
  }

  virtual void SetPrgSeed(shared_ptr<RttPRG> prg_seed) { gseed_ = prg_seed; }
  virtual shared_ptr<RttPRG> GetPrgSeed() { return gseed_; }

  virtual shared_ptr<ProtocolContext> GetMpcContext() { return context_;}
  virtual shared_ptr<NET_IO> GetIOChannel() { return io;}

  virtual int GetRoleId() { return context_->GetMyRole(); }
  const msg_id_t& msg_id() const { return op_msg_id; }

 public:
  void init_aes() {
    auto aesobjs = aes_controller_->Get(op_msg_id);
    aes_randseed = aesobjs->aes_randseed;
    aes_common = aesobjs->aes_common;
    aes_indep = aesobjs->aes_indep;
    aes_a_1 = aesobjs->aes_a_1;
    aes_a_2 = aesobjs->aes_a_2;
    aes_b_1 = aesobjs->aes_b_1;
    aes_b_2 = aesobjs->aes_b_2;
    aes_c_1 = aesobjs->aes_c_1;
    aes_data = aesobjs->aes_data;
  }
  
  // inner inline implementation of operations
  #include "cc/modules/protocol/mpc/snn/include/snn_internal_inner.hpp"

  mpc_t RandomSeed() { return aes_randseed->get64Bits(); }

  // [TODO] Synchronize all PRG keys among parties, use SyncAesKey
  // int SyncPRGKey();

  /*partyA send key to partyB as the public key of A and B
    eg:
    kab: A --> B
    kac: A --> C
    kbc: B --> C
  */
  int SyncAesKey(int partyA, int partyB, std::string& key_send, std::string& key_recv);
  int SyncAesKey(const string& node_id, int party_id, std::string& key_send, std::string& key_recv);

  // zero sharing with Pseudorandom Zero-Sharing
  int PRZS(int party0, int party1, vector<mpc_t>& shares);
  int PRZS(int party0, int party1, vector<double>& shares);
  int PRZS(int party0, int party1, mpc_t& shares);
  int PRZS(int party0, int party1, double& shares);
  int PRZS(const string& node_id, int party_id, vector<mpc_t>& shares);

  // Private input data, input private plain data and ouput sharing data
  int PrivateInput(const string& node_id, const vector<double>& v, vector<mpc_t>& shares);
  int PrivateInput(const string& node_id, const vector<mpc_t>& v, vector<mpc_t>& shares);
  int PrivateInput(const string& node_id, const vector<double>& v, vector<double>& shares);
  int PrivateInput(const string& node_id, double v, mpc_t& shares);
  int PrivateInput(const string& node_id, double v, double& shares);

  // Reveal secret sharing data to plain
  int Reconstruct2PC(const mpc_t& a, mpc_t& out, int recv_party) {
    vector<mpc_t> va = {a}, vo(1);
    Reconstruct2PC_ex(va, vo, recv_party);
    out = vo[0];
    return 0;
  }
  int Reconstruct2PC(const vector<mpc_t>& a, string str);
  int Reconstruct2PC(const vector<mpc_t>& a, vector<mpc_t>& out, int recv_party);
  int Reconstruct2PC(
    const vector<mpc_t>& a,
    vector<mpc_t>& out,
    const vector<string>& recv_parties);
  int Reconstruct2PC_ex(
    const vector<mpc_t>& a,
    vector<mpc_t>& out,
    int recv_party);
  int Reconstruct2PC_ex(
    const vector<mpc_t>& a,
    vector<mpc_t>& out,
    const string& recv_parties);
  int Reconstruct2PC_ex_mod_odd(
    const vector<mpc_t>& shared_v,
    vector<mpc_t>& plaintext_v,
    int recv_party);
  int Reconstruct2PC_ex_mod_odd_v2(
    const vector<mpc_t>& shared_v,
    vector<mpc_t>& plaintext_v,
    vector<string>& receivers);

  /**
   * @brief: the shared_v will be 'revealed' to plaintext_v held by rev_party 
   **/
  int Reconstruct2PC_general(
    const vector<mpc_t>& shared_v,
    vector<mpc_t>& plaintext_v,
    int recv_party);

  int ReconstructBit2PC(const vector<small_mpc_t>& a, size_t size, string str);

  // Broadcast data
  int Broadcast(const string& from_node, const string& msg, string& result);
  int Broadcast(const string& from_node, const char* msg, char* result, size_t size);
  

  ///////////  basic math functions ////////
  int Negate(const vector<mpc_t>& a, vector<mpc_t>& b);
  int Negative(vector<mpc_t>& a, vector<mpc_t>& b) { return Negate(a, b); }

  int DotProduct(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c);
  int DotProduct(const vector<double>& const_a, const vector<mpc_t>& b, vector<mpc_t>& c);
  int DotProduct(const vector<string>& const_a, const vector<mpc_t>& b, vector<mpc_t>& c);
  int DotProduct(const vector<mpc_t>& a, const vector<string>& const_b, vector<mpc_t>& c);

  int Mul(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c);
  int Mul(const vector<string>& const_a, const vector<mpc_t>& b, vector<mpc_t>& c);
  int Mul(const vector<mpc_t>& a, const vector<string>& const_b, vector<mpc_t>& c);
  int Mul(const vector<double>& const_a, const vector<mpc_t>& b, vector<mpc_t>& c);
  int Mul(const vector<mpc_t>& a, const vector<double>& const_b, vector<mpc_t>& c);

  int Square(const vector<mpc_t>& a, vector<mpc_t>& c);

  int Exp(const vector<mpc_t>& a, vector<mpc_t>& c);

  int Sqrt(const vector<mpc_t>& a, vector<mpc_t>& c);
  int Rsqrt(const vector<mpc_t>& a, vector<mpc_t>& c);

  int PowV1(const vector<mpc_t>& x, size_t n, vector<mpc_t>& y);
  int PowV1(const vector<mpc_t>& x, const string& n, vector<mpc_t>& y);
  int Pow(const vector<mpc_t>& x, vector<int64_t> n, vector<mpc_t>& y);
  int Pow(const vector<mpc_t>& x, const vector<string>& n, vector<mpc_t>& y);

  int Add(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c);
  int Add(const vector<double>& a, const vector<mpc_t>& b, vector<mpc_t>& c);
  int Add(const vector<mpc_t>& a, const vector<double>& b, vector<mpc_t>& c);
  int Add(const vector<string>& const_a, const vector<mpc_t>& b, vector<mpc_t>& c);
  int Add(const vector<mpc_t>& a, const vector<string>& const_b, vector<mpc_t>& c);

  int Sub(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c);
  int Sub(const vector<string>& const_a, const vector<mpc_t>& b, vector<mpc_t>& c);
  int Sub(const vector<mpc_t>& a, const vector<string>& const_b, vector<mpc_t>& c);

  /////  divisions  /////
  int Truedivision(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c);
  int Truedivision(const vector<string>& a, const vector<mpc_t>& b, vector<mpc_t>& c);
  int Truedivision(const vector<mpc_t>& a, const vector<string>& b, vector<mpc_t>& c);

  int DivisionV1(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& quotient);
  int DivisionV1(const vector<string>& a, const vector<mpc_t>& b, vector<mpc_t>& quotient);
  int DivisionV1(const vector<mpc_t>& a, const vector<string>& b, vector<mpc_t>& quotient);

  int Reciprocaldivision(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c);
  int Reciprocaldivision(const vector<string>& a, const vector<mpc_t>& b, vector<mpc_t>& c);
  int Reciprocaldivision(const vector<mpc_t>& a, const vector<string>& b, vector<mpc_t>& c);

  int Division(
    const vector<mpc_t>& shared_numerator_vec,
    const vector<mpc_t>& shared_denominator_vec,
    vector<mpc_t>& shared_quotient_vec,
    bool common_all_less = false);
  int Division(
    const vector<string>& shared_numerator_vec,
    const vector<mpc_t>& shared_denominator_vec,
    vector<mpc_t>& shared_quotient_vec,
    bool common_all_less = false);
  int Division(
    const vector<mpc_t>& shared_numerator_vec,
    const vector<string>& shared_denominator_vec,
    vector<mpc_t>& shared_quotient_vec,
    bool common_all_less = false);

  int Floordivision(
    const vector<mpc_t>& ori_numerator_vec,
    const vector<mpc_t>& ori_denominator_vec,
    vector<mpc_t>& ori_quotient_vec,
    bool all_positive = false);
  int Floordivision(
    const vector<string>& ori_numerator_vec,
    const vector<mpc_t>& ori_denominator_vec,
    vector<mpc_t>& ori_quotient_vec,
    bool all_positive = false);
  int Floordivision(
    const vector<mpc_t>& ori_numerator_vec,
    const vector<string>& ori_denominator_vec,
    vector<mpc_t>& ori_quotient_vec,
    bool all_positive = false);
  
  int Fracdivision(
    const vector<mpc_t>& ori_numerator_vec,
    const vector<mpc_t>& ori_denominator_vec,
    vector<mpc_t>& ori_quotient_vec,
    bool all_positive = false);
  int Fracdivision(
    const vector<string>& ori_numerator_vec,
    const vector<mpc_t>& ori_denominator_vec,
    vector<mpc_t>& ori_quotient_vec,
    bool all_positive = false);
  int Fracdivision(
    const vector<mpc_t>& ori_numerator_vec,
    const vector<string>& ori_denominator_vec,
    vector<mpc_t>& ori_quotient_vec,
    bool all_positive = false);

  int ReciprocalDivfor2(
    const vector<mpc_t>& shared_numerator_vec, 
    const vector<mpc_t>& shared_denominator_vec, 
    vector<mpc_t>& shared_quotient_vec,
    bool all_less = false); 

  int MatMul(
    const vector<mpc_t>& a,
    const vector<mpc_t>& b,
    vector<mpc_t>& c,
    size_t rows,
    size_t common_dim,
    size_t columns,
    size_t transpose_a,
    size_t transpose_b);
  int MatMul(
    const vector<string>& as,
    const vector<string>& bs,
    vector<string>& cs,
    size_t rows,
    size_t common_dim,
    size_t columns,
    size_t transpose_a,
    size_t transpose_b);

  //////////    math non-linear   //////////
  /**
   * @brief: return |X|
  */
  int Abs(const vector<mpc_t>& shared_x, vector<mpc_t>& shared_y);
  /**
    @brief: 1 if X > 0, -1 if X < 0
  */
  int AbsPrime(const vector<mpc_t>& shared_x, vector<mpc_t>& sahred_y);
  /**
   * @brief get log(1 + exp(-x)) [x >= 0] with special polynommial
   */
  void CELog(const vector<mpc_t>& shared_x, vector<mpc_t>& shared_y, size_t vec_size);

  ///////////   High precision Log     ///////////
  int HLog(const vector<mpc_t>& shared_X, vector<mpc_t>& shared_Y);

  ///////////   Log version one     ///////////
  int LogV1(const vector<mpc_t>& shared_X, vector<mpc_t>& shared_Y);
  int LogV1(const mpc_t& shared_X, mpc_t& shared_Y);

  ///////////   Log version two     ///////////
  int Log(const vector<mpc_t>& shared_X, vector<mpc_t>& shared_Y);
  int Log(const mpc_t& shared_X, mpc_t& shared_Y);

  int Log1p(const mpc_t& a, mpc_t& b);
  int Log1p(const vector<mpc_t>& a, vector<mpc_t>& b);

  ////////////   Polynomial ///////////////
  /*
	@brief: Secret-shared version for calculating X^k, of which
			k is common known
	@param:
		[in] shared_X, the variable;
		[in] common_k, power value.
		[out] shared_Y, the resulting value.
		[in] curr_cache, the auxiliary cache,
			 containing some computed k --> Y, to accelerate.
  */
  void PolynomialPowConst(const mpc_t& shared_X, mpc_t common_k, mpc_t& shared_Y);
  void PolynomialPowConst(const vector<mpc_t>& shared_X, mpc_t common_k, vector<mpc_t>& shared_Y);
  void PolynomialLocalConstMul(
    const vector<mpc_t>& shared_X,
    mpc_t common_V,
    vector<mpc_t>& shared_Y);

  /*
	  @brief: secret-shared version for computing a univariate polynomial:
	  Y = C0 * X^P0 + C1 * X^P1 + ... + Cn * X^Pn
	  @param:
	  	[in] shared_X, the variable.
	  	[in] common_power_list, the {Pi} in the polynomial in order.
	  	[in] common_coff_list, the {Ci} in the polynomial, which must
	  			1-to-1 associate with {Pi}.
	  	[out] shared_Y, the result.
      @Note:
	  	for efficiency, it will be better to sort the list in ascending order!
  */
  void UniPolynomial(
    const mpc_t& shared_X,
    const vector<mpc_t>& common_power_list,
    const vector<mpc_t>& common_coff_list,
    mpc_t& shared_Y);

  void UniPolynomial(
    const vector<mpc_t>& shared_X,
    const vector<mpc_t>& common_power_list,
    const vector<mpc_t>& common_coff_list,
    vector<mpc_t>& shared_Y);

  //////////    select share  /////////////
  int Select1Of2(
    const vector<mpc_t> shared_x,
    const vector<mpc_t> shared_y,
    const vector<mpc_t> shared_bool,
    vector<mpc_t>& shared_result);
  //////////    xor bit in in \Z_L  /////////////
  int XorBit(
    const vector<mpc_t> shared_x,
    const vector<mpc_t> shared_y,
    vector<mpc_t>& shared_result);

  /*        nn operations        */
  ///////////     relu    /////////
  int Relu(const vector<mpc_t>& a, vector<mpc_t>& b);
  int Relu(const vector<string>& as, vector<string>& bs);

  /////////// relu-prime  /////////
  int ReluPrime(const vector<mpc_t>& a, vector<mpc_t>& b);

  /**
   * @brief sigmoid-with-entropy
   * @Note: Speedup in low-bandthwith network environment, with less communication round.
   */
  int SigmoidCrossEntropyBatch(
    const vector<mpc_t>& shared_logits,
    const vector<mpc_t>& shared_labels,
    vector<mpc_t>& shared_result,
    size_t vec_size);

  /**
   * @brief Regular SigmoidWithEntropy [Obsoleted]
   * @note Obsoleted, please use the faster one, sigmoid_cross_entropy_batch
    Just like the 'sigmoid_cross_entropy_with_logits' in python/ops/nn_impl.py of
    original Tensorflow source code, we implement formulation:
    max(logit, 0) - logit * label + log(1 + exp(-abs(x)))
  */
  int SigmoidCrossEntropy(
    const vector<mpc_t>& shared_logits,
    const vector<mpc_t>& shared_labels,
    vector<mpc_t>& shared_result,
    size_t vec_size);

  /**
   * @brief Sigmoid represented as 6 piecewise function
   */
  int Sigmoid(const vector<mpc_t>& a, vector<mpc_t>& b) { return Sigmoid5PieceWise(a, b); }
  int Sigmoid6PieceWise(const vector<mpc_t>& a, vector<mpc_t>& b);

  /**
   * @brief Sigmoid represented as 5 piecewise function
   */
  int Sigmoid5PieceWise(const vector<mpc_t>& a, vector<mpc_t>& b);
  /**
   * @brief Sigmoid represented as ChebyshevPoly polynomian
   */
  int SigmoidChebyshevPolyMPC(const vector<mpc_t>& a, vector<mpc_t>& b);

  /*       basic drelu-ops    */
  ///////////  private-compare /////////
  int PrivateCompare(
    const vector<small_mpc_t>& share_m,
    const vector<mpc_t>& r,
    const vector<small_mpc_t>& beta,
    vector<small_mpc_t>& betaPrime,
    size_t size,
    size_t dim);

  ///////////  share-convert  //////////
  int ShareConvert(vector<mpc_t>& a);

  ///////////  MSB(most significant bit)  /////////
  int ComputeMSB(const vector<mpc_t>& a, vector<mpc_t>& b);

  /////////  select one out two shares /////////
  int SelectShares(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c);

  /////////  compare operations  //////////
  int GreaterEqual(const mpc_t& a, const mpc_t& b, mpc_t& c);
  int GreaterEqual(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c);
  int GreaterEqual(const vector<double>& a, const vector<mpc_t>& b, vector<mpc_t>& c);
  int GreaterEqual(const vector<mpc_t>& a, const vector<double>& b, vector<mpc_t>& c);
  int GreaterEqual(const vector<string>& a, const vector<mpc_t>& b, vector<mpc_t>& c);
  int GreaterEqual(const vector<mpc_t>& a, const vector<string>& b, vector<mpc_t>& c);

  int Greater(const mpc_t& a, const mpc_t& b, mpc_t& c);
  int Greater(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c);
  int Greater(const vector<double>& a, const vector<mpc_t>& b, vector<mpc_t>& c);
  int Greater(const vector<mpc_t>& a, const vector<double>& b, vector<mpc_t>& c);
  int Greater(const vector<string>& a, const vector<mpc_t>& b, vector<mpc_t>& c);
  int Greater(const vector<mpc_t>& a, const vector<string>& b, vector<mpc_t>& c);

  int LessEqual(const mpc_t& a, const mpc_t& b, mpc_t& c);
  int LessEqual(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c);
  int LessEqual(const vector<double>& a, const vector<mpc_t>& b, vector<mpc_t>& c);
  int LessEqual(const vector<mpc_t>& a, const vector<double>& b, vector<mpc_t>& c);
  int LessEqual(const vector<string>& a, const vector<mpc_t>& b, vector<mpc_t>& c);
  int LessEqual(const vector<mpc_t>& a, const vector<string>& b, vector<mpc_t>& c);

  int Less(const mpc_t& a, const mpc_t& b, mpc_t& c);
  int Less(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c);
  int Less(const vector<double>& a, const vector<mpc_t>& b, vector<mpc_t>& c);
  int Less(const vector<mpc_t>& a, const vector<double>& b, vector<mpc_t>& c);
  int Less(const vector<string>& a, const vector<mpc_t>& b, vector<mpc_t>& c);
  int Less(const vector<mpc_t>& a, const vector<string>& b, vector<mpc_t>& c);

  int Equal(const mpc_t& a, const mpc_t& b, mpc_t& c);
  int Equal(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c);
  int Equal(const vector<double>& a, const vector<mpc_t>& b, vector<mpc_t>& c);
  int Equal(const vector<mpc_t>& a, const vector<double>& b, vector<mpc_t>& c);
  int Equal(const vector<string>& a, const vector<mpc_t>& b, vector<mpc_t>& c);
  int Equal(const vector<mpc_t>& a, const vector<string>& b, vector<mpc_t>& c);
  /* deprecated equal implementation */
  int EqualSlow(const vector<mpc_t>& a, const vector<string>& b, vector<mpc_t>& c);

  /* 
  * fast equal with computation and communication efficient
  */
  int FastEqual(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c);
  int FanInBitAdd(const vector<vector<small_mpc_t>>& a, vector<small_mpc_t>& c);
  int B2A(const vector<small_mpc_t>& bit_shares, vector<mpc_t>& arith_shares);
  int BitMul(const vector<small_mpc_t>& a, const vector<small_mpc_t>& b, vector<small_mpc_t>& c);

  int NotEqual(const mpc_t& a, const mpc_t& b, mpc_t& c);
  int NotEqual(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c);
  int NotEqual(const vector<double>& a, const vector<mpc_t>& b, vector<mpc_t>& c);
  int NotEqual(const vector<mpc_t>& a, const vector<double>& b, vector<mpc_t>& c);
  int NotEqual(const vector<string>& a, const vector<mpc_t>& b, vector<mpc_t>& c);
  int NotEqual(const vector<mpc_t>& a, const vector<string>& b, vector<mpc_t>& c);
  int FastNotEqual(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c);
  int NotEqualSlow(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c);

  /////////  reduce operations  //////////
  int Max(
    const vector<mpc_t>& a,
    vector<mpc_t>& maxv,
    vector<mpc_t>& maxIndex,
    size_t rows,
    size_t cols);
  int Max(
    const vector<mpc_t>& a,
    vector<mpc_t>& maxv,
    size_t rows,
    size_t cols);
  int MaxIndex(vector<mpc_t>& a, const vector<mpc_t>& maxIndex, size_t rows, size_t cols);

  int Min(
    const vector<mpc_t>& a,
    vector<mpc_t>& minv,
    vector<mpc_t>& minIndex,
    size_t rows,
    size_t columns);
  int Min(
    const vector<mpc_t>& a,
    vector<mpc_t>& minv,
    size_t rows,
    size_t columns);
  int MinIndex(vector<mpc_t>& a, const vector<mpc_t>& minIndex, size_t rows, size_t cols);

  int Mean(const vector<mpc_t>& a, vector<mpc_t>& b, size_t rows, size_t cols);

  int AddN(const vector<mpc_t>& a, vector<mpc_t>& b, size_t rows, size_t cols);

  int Sum(const vector<mpc_t>& a, vector<mpc_t>& b, size_t rows, size_t cols);

  ///////// misc operations  //////////
  /////////   convert const value to sharing value   /////////
  void Const2Share(const vector<string>& const_a, vector<mpc_t>& a) {
    a.resize(const_a.size(), 0);
    int float_precision = GetMpcContext()->FLOAT_PRECISION;
    if (partyNum == PARTY_A) {
      vector<double> da(a.size());
      rosetta::convert::from_double_str(const_a, da);
      convert_double_to_mpctype(da, a, float_precision);
    }
  }

  void Const2Share(const vector<double>& const_a, vector<mpc_t>& a) {
    a.resize(const_a.size(), 0);
    int float_precision = GetMpcContext()->FLOAT_PRECISION;
    if (partyNum == PARTY_A) {
      convert_double_to_mpctype(const_a, a, float_precision);
    }
  }

  /**
   * unary linear: f(x[i]) = a * x[i] + b
   */
  int LinearMPC(const vector<mpc_t>& x, double a, double b, vector<mpc_t>& out);
  int LinearMPC(const vector<mpc_t>& x, mpc_t a, mpc_t b, vector<mpc_t>& out);


  /////////////////////  Logical operations /////////////////////
  int OR(const vector<mpc_t>& X, const vector<mpc_t>& Y, vector<mpc_t>& Z);
  int OR(const vector<string>& X, const vector<mpc_t>& Y, vector<mpc_t>& Z);
  int OR(const vector<mpc_t>& X, const vector<string>& Y, vector<mpc_t>& Z);

  int XOR(const vector<mpc_t>& X, const vector<mpc_t>& Y, vector<mpc_t>& Z);
  int XOR(const vector<string>& X, const vector<mpc_t>& Y, vector<mpc_t>& Z);
  int XOR(const vector<mpc_t>& X, const vector<string>& Y, vector<mpc_t>& Z);

  int AND(const vector<mpc_t>& X, const vector<mpc_t>& Y, vector<mpc_t>& Z);
  int AND(const vector<string>& X, const vector<mpc_t>& Y, vector<mpc_t>& Z);
  int AND(const vector<mpc_t>& X, const vector<string>& Y, vector<mpc_t>& Z);

  int Not(const vector<mpc_t>& X, vector<mpc_t>& Y);
};

}//snn
}//rosetta
