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
#include "cc/modules/protocol/mpc/snn/include/snn_ops.h"
#include "cc/modules/protocol/utility/include/prg.h"
#include "cc/modules/protocol/mpc/snn/include/snn_internal.h"
#include "cc/modules/common/include/utils/rtt_logger.h"
#include "cc/modules/common/include/utils/str_type_convert.h"
#include "cc/modules/common/include/utils/perf_stats.h"
#include "cc/modules/protocol/mpc/comm/include/mpc_helper.h"
#include "cc/modules/common/include/utils/secure_encoder.h"
#include <string>


using StrVec = std::vector<std::string>;
using MpcVec = std::vector<mpc_t>;

#define GET_NAME(N) #N

#define GET_ATTR_TAG(attr_info_ptr, tag) \
  attr_info_ptr && attr_info_ptr->count(tag) > 0 && attr_info_ptr->at(tag) == "1"

namespace rosetta {
namespace snn {

/************  static utils functions     *****/
static inline void convert_plain_double_to_mpctype(
  const vector<double>& a,
  vector<mpc_t>& b,
  int float_precision,
  bool disable_sharing = false) {
  b.resize(a.size());
  for (int i = 0; i < a.size(); i++)
    b[i] = disable_sharing ? FloatToMpcType(a[i], float_precision)
                           : FloatToMpcType(a[i] / 2, float_precision);
}

static inline int snn_decode_(const StrVec& a, MpcVec& sa, int float_precision) {
  ELAPSED_STATISTIC_BEG(convert_string_to_share_timer);

  if (rosetta::convert::is_secure_text(a[0])) {
    sa.resize(a.size());
    for (auto i = 0; i < a.size(); ++i) {
      memcpy((char*)&sa[i], a[i].data(), sizeof(mpc_t));
    }

    ELAPSED_STATISTIC_END(convert_string_to_share_timer);
  } else if (rosetta::convert::is_binary_double(a[0])) {
    ELAPSED_STATISTIC_END(convert_string_to_share_timer);
    vector<double> sd(a.size());
    for (int i = 0; i < a.size(); i++) {
      memcpy(&sd[i], a[i].data(), sizeof(double));
      sd[i] /= 2;
    }
    convert_double_to_mpctype(sd, sa, float_precision);
  } else {
    ELAPSED_STATISTIC_END(convert_string_to_share_timer);
    convert_plain_double_to_mpctype(rosetta::convert::from_double_str(a), sa, float_precision);
  }

  return 0;
}

static inline int snn_encode_(const MpcVec& sa, StrVec& a) {
  ELAPSED_STATISTIC_BEG(convert_share_to_string_timer);
  if (0 != rosetta::convert::encoder::encode_to_secure(sa, a)) {
    log_error << "encode_to_secure failed!";
    return -1;
  }
  ELAPSED_STATISTIC_END(convert_share_to_string_timer);
  return 0;
}

//! maybe more actions here
#define snn_decode(a, sa, precision)                                                      \
  do {                                                                                    \
    if (0 != snn_decode_(a, sa, precision)) {                                             \
      log_error << "rosetta::convert::encoder::decode failed! In " << __FUNCTION__ << "#" \
                << __LINE__ ;                                                      \
      return -1;                                                                          \
    }                                                                                     \
  } while (0)

#define snn_encode(sa, a)                                                                 \
  do {                                                                                    \
    if (0 != snn_encode_(sa, a)) {                                                        \
      log_error << "rosetta::convert::encoder::encode failed! In " << __FUNCTION__ << "#" \
                << __LINE__ ;                                                      \
      return -1;                                                                          \
    }                                                                                     \
  } while (0)

static inline int decode_input_to_snn(const StrVec& a, MpcVec& sa, int float_precision) {
  if (a.empty()) {
    log_error << "empty inputs !" ;
    return -1;
  }

  snn_decode(a, sa, float_precision);
  return 0;
}


/*** Binary OP(s) * Add / Sub / Mul / Div / Compare(s) */
#define SNN_PROTOCOL_BINARY_OP_(op)                                                               \
  int SnnProtocolOps::op(                                                                     \
    const vector<string>& a, const vector<string>& b, vector<string>& c, const attr_type* attr) { \
    bool rh_is_const = GET_ATTR_TAG(attr, "rh_is_const");                                         \
    bool lh_is_const = GET_ATTR_TAG(attr, "lh_is_const");                                         \
    assert(!(lh_is_const && rh_is_const));                                                        \
    int float_precision = context_->FLOAT_PRECISION;                                       \
    vector<mpc_t> shareA, shareB, shareC(a.size());                                               \
    int ret = -1;                                                                                 \
    if (lh_is_const) {                                                                            \
      snn_decode(b, shareB, float_precision);                                                     \
      ret = internal_->op(a, shareB, shareC);                                                     \
    } else if (rh_is_const) {                                                                     \
      snn_decode(a, shareA, float_precision);                                                     \
      ret = internal_->op(shareA, b, shareC);                                                     \
    } else {                                                                                      \
      snn_decode(a, shareA, float_precision);                                                     \
      snn_decode(b, shareB, float_precision);                                                     \
      ret = internal_->op(shareA, shareB, shareC);                                                \
    }                                                                                             \
    snn_encode(shareC, c);                                                                        \
    return 0;                                                                                     \
  }

/**
 * Binary OP(s)
 * Add/Sub/Mul/Div/Compare(s)
 */
#define SNN_PROTOCOL_BINARY_DIV_OP_(op)                                                           \
  int SnnProtocolOps::op(                                                                     \
    const vector<string>& a, const vector<string>& b, vector<string>& c, const attr_type* attr) { \
    bool rh_is_const = GET_ATTR_TAG(attr, "rh_is_const");                                         \
    bool lh_is_const = GET_ATTR_TAG(attr, "lh_is_const");                                         \
    assert(!(lh_is_const && rh_is_const));                                                        \
    int float_precision = context_->FLOAT_PRECISION;                                       \
    vector<mpc_t> shareA, shareB, shareC(a.size(), 0);                                                         \
    int ret = -1;                                                                                 \
    if (lh_is_const) {                                                                            \
      snn_decode(b, shareB, float_precision);                                                     \
      ret = internal_->op##ision(a, shareB, shareC);                                              \
    } else if (rh_is_const) {                                                                     \
      snn_decode(a, shareA, float_precision);                                                     \
      ret = internal_->op##ision(shareA, b, shareC);                                              \
    } else {                                                                                      \
      snn_decode(a, shareA, float_precision);                                                     \
      snn_decode(b, shareB, float_precision);                                                     \
      ret = internal_->op##ision(shareA, shareB, shareC);                                         \
    }                                                                                             \
    snn_encode(shareC, c);                                                                        \
    return ret;                                                                                   \
  }

#define SNN_PROTOCOL_BINARY_OP(op) SNN_PROTOCOL_BINARY_OP_(op)
#define SNN_PROTOCOL_BINARY_DIV_OP(op) SNN_PROTOCOL_BINARY_DIV_OP_(op)

/**
 * Unary OP(s)
 * Pow/Square/log...
 */
#define SNN_PROTOCOLL_UNARY_OP_(op)                                                               \
  int SnnProtocolOps::op(const vector<string>& a, vector<string>& c, const attr_type* attr) { \
    tlog_debug << "----> "                                                                         \
              << "Snn" << GET_NAME(op) << " unary ops ";                                          \
    int float_precision = context_->FLOAT_PRECISION;                                       \
    c.resize(a.size());                                                                           \
    vector<mpc_t> shareA(a.size(), 0), shareC(a.size(), 0);                                                                 \
    snn_decode(a, shareA, float_precision);                                                       \
    int ret = internal_->op(shareA, shareC);                                                      \
    snn_encode(shareC, c);                                                                        \
    tlog_debug << "Snn " << GET_NAME(op) << " ok. <----";                                          \
    return ret;                                                                                   \
  }

#define SNN_PROTOCOLL_UNARY_OP(op) SNN_PROTOCOLL_UNARY_OP_(op)

/**
 * Unary OP(s)
 * Pow/Square/log...
 */
#define SNN_PROTOCOLL_REDUCE_OP_(op)                                                              \
  int SnnProtocolOps::op(const vector<string>& a, vector<string>& c, const attr_type* attr) { \
    tlog_debug << "----> "                                                                         \
              << "Snn" << GET_NAME(op) << " unary ops ";                                          \
    size_t rows = 0, cols = 0;                                                                    \
    assert(attr && attr->count("rows") > 0 && attr->count("cols") > 0);                           \
    rows = std::stoull(attr->at("rows"));                                                         \
    cols = std::stoull(attr->at("cols"));                                                         \
    int float_precision = context_->FLOAT_PRECISION;                                       \
    vector<mpc_t> shareA(a.size(), 0), shareC(rows, 0);                                           \
    snn_decode(a, shareA, float_precision);                                                       \
    int ret = internal_->op(shareA, shareC, rows, cols);                                          \
    snn_encode(shareC, c);                                                                        \
    tlog_debug << "Snn" << GET_NAME(op) << " ok. <----";                                           \
    return ret;                                                                                   \
  }

#define SNN_PROTOCOLL_REDUCE_OP(op) SNN_PROTOCOLL_REDUCE_OP_(op)

// step1 define the global(static) timer variables
DEFINE_GLOBAL_TIMER_COUNTER(convert_string_to_share_timer)
DEFINE_GLOBAL_TIMER_COUNTER(convert_share_to_string_timer)

// step2 define an atexit function(static)
DEFINE_AT_EXIT_FUNCTION_BEG()
DEFINE_AT_EXIT_FUNCTION_BODY(convert_string_to_share_timer)
DEFINE_AT_EXIT_FUNCTION_BODY(convert_share_to_string_timer)
DEFINE_AT_EXIT_FUNCTION_END()
// step3 use ELAPSED_STATISTIC_BEG/ELAPSED_STATISTIC_END in the program

SnnProtocolOps::SnnProtocolOps(
  const msg_id_t& msgid,
  shared_ptr<ProtocolContext> context,
  shared_ptr<RttPRG> prg_seed,
  shared_ptr<NET_IO> io_channel)
    : ProtocolOps(msgid, context),
    gseed_(prg_seed), net_io_(io_channel) {
  internal_ = make_shared<rosetta::snn::SnnInternal>(msgid, context, gseed_, io_channel);

  internal_->SetTripleGenerator(_triple_generator);
}

int SnnProtocolOps::TfToSecure(
  const vector<string>& in,
  vector<string>& out,
  const attr_type* attr_info /* = nullptr*/) {
  tlog_debug << "----> TfToSecure...";

  if (in.empty()) {
    tlog_error << "TfToSecure input is null !";
    return -1;
  }

  {
    // convension
    vector<mpc_t> mpcvalues(in.size());
    vector<double> dvalues(in.size());

    for (int i = 0; i < in.size(); ++i) {
      dvalues[i] = std::stod(in[i]);
    }

    convert_double_to_mpctype(dvalues, mpcvalues, context_->FLOAT_PRECISION);
    snn_encode(mpcvalues, out);
  }

  return 0;
}

// decode the string in protocol-specific format to literal number
int SnnProtocolOps::SecureToTf(
  const vector<string>& in,
  vector<string>& out,
  const attr_type* attr_info /* = nullptr*/) {
  tlog_debug << "----> SecureToTf. from mpc_t to double hex string";
  vector<string> contents;
  if (0 != rosetta::convert::encoder::decode_secure(in, out)) {
    tlog_error << "decode error, input is valid !";
    return -1;
  }

  // to mpc_t values
  vector<mpc_t> mpcvalues(in.size());
  vector<double> dvalues(in.size());
  rosetta::convert::from_binary_str<mpc_t>(out, mpcvalues);

  // to double values
  convert_mpctype_to_double(mpcvalues, dvalues, context_->FLOAT_PRECISION);

  // to hex string
  rosetta::convert::to_binary_str<double>(dvalues, out);
  return 0;
}

int SnnProtocolOps::RandSeed(std::string op_seed, string& out_str) {
  tlog_debug << "----> RandSeed.";
  mpc_t out = internal_->RandomSeed();
  rosetta::convert::to_binary_str(out, out_str);
  return 0;
}

int SnnProtocolOps::PrivateInput(
  const string& node_id,
  const vector<double>& in_vec,
  vector<string>& out_str_vec) {
  int party_id = net_io_->GetPartyId(node_id);
  if (party_id == PARTY_A || party_id == PARTY_B) {
    vector<mpc_t> out_vec(in_vec.size(), 0);
    if (party_id == context_->GetMyRole()) {
      convert_double_to_mpctype(in_vec, out_vec, context_->FLOAT_PRECISION);
    }
    snn_encode(out_vec, out_str_vec);
    return 0;
  }

  vector<mpc_t> out_vec(in_vec.size());
  internal_->PrivateInput(node_id, in_vec, out_vec);

  snn_encode(out_vec, out_str_vec);
  return 0;
}

int SnnProtocolOps::PublicInput(
    const string& node_id,
    const vector<double>& in_x,
    vector<string>& out_x) {
  convert_double_to_literal_str(in_x, out_x, context_->FLOAT_PRECISION);
  return 0;
}

int SnnProtocolOps::Broadcast(const string& from_node, const string& msg, string& result) {
  tlog_debug << "----> Broadcast(msg).";
  internal_->Broadcast(from_node, msg, result);
  return 0;
}
int SnnProtocolOps::Broadcast(
  const string& from_node,
  const char* msg,
  char* result,
  size_t size) {
  internal_->Broadcast(from_node, msg, result, size);
  return 0;
}

SNN_PROTOCOL_BINARY_OP(Add)
SNN_PROTOCOL_BINARY_OP(Sub)
SNN_PROTOCOL_BINARY_OP(Mul)
SNN_PROTOCOL_BINARY_DIV_OP(Div)
SNN_PROTOCOL_BINARY_DIV_OP(Reciprocaldiv)
SNN_PROTOCOL_BINARY_DIV_OP(Truediv)
SNN_PROTOCOL_BINARY_DIV_OP(Floordiv)
SNN_PROTOCOL_BINARY_OP(Less)
SNN_PROTOCOL_BINARY_OP(LessEqual)
SNN_PROTOCOL_BINARY_OP(Greater)
SNN_PROTOCOL_BINARY_OP(GreaterEqual)
SNN_PROTOCOL_BINARY_OP(Equal)
SNN_PROTOCOL_BINARY_OP(NotEqual)

int SnnProtocolOps::Pow(
  const vector<string>& a,
  const vector<string>& b,
  vector<string>& output,
  const attr_type* attr_info) {
  tlog_debug << "----> SnnPow";

  vector<mpc_t> out_vec(a.size());
  vector<mpc_t> private_a;
  snn_decode(a, private_a, context_->FLOAT_PRECISION);

  //! @attention: only support const b
  int ret = internal_->Pow(private_a, b, out_vec);

  snn_encode(out_vec, output);
  tlog_debug << "SnnPow ok. <----";

  return ret;
}

int SnnProtocolOps::Matmul(
  const vector<string>& a,
  const vector<string>& b,
  vector<string>& output,
  const attr_type* attr_info) {
  tlog_debug << "----> "
            << "SnnMatmul";
  int m = 0, k = 0, n = 0;
  if (attr_info->count("m") > 0 && attr_info->count("n") > 0 && attr_info->count("k") > 0) {
    m = std::stoi(attr_info->at("m"));
    k = std::stoi(attr_info->at("k"));
    n = std::stoi(attr_info->at("n"));
  } else {
    log_error << "please fill m, k, n for SnnMatmul(x, y, m, n, k, transpose_a, transpose_b) ";
    return -1;
  }

  bool transpose_a = false;
  bool transpose_b = false;
  if (attr_info->count("transpose_a") > 0 && attr_info->at("transpose_a") == "1")
    transpose_a = true;
  if (attr_info->count("transpose_b") > 0 && attr_info->at("transpose_b") == "1")
    transpose_b = true;

  vector<mpc_t> out_vec(m * n);
  vector<mpc_t> private_a, private_b;
  snn_decode(a, private_a, context_->FLOAT_PRECISION);
  snn_decode(b, private_b, context_->FLOAT_PRECISION);

  // auto ops = std::make_shared<rosetta::snn::MatMul>(_op_msg_id, net_io_, GetMpcContext());
  // ops->Run(private_a, private_b, out_vec, m, k, n, transpose_a, transpose_b);
  internal_->MatMul(private_a, private_b, out_vec, m, k, n, transpose_a, transpose_b);

  snn_encode(out_vec, output);
  tlog_debug << "SnnMatmul ok. <----";

  return 0;
}

SNN_PROTOCOLL_UNARY_OP(Square)
SNN_PROTOCOLL_UNARY_OP(Negative)
SNN_PROTOCOLL_UNARY_OP(Abs)
SNN_PROTOCOLL_UNARY_OP(Exp)
SNN_PROTOCOLL_UNARY_OP(Sqrt)
SNN_PROTOCOLL_UNARY_OP(Rsqrt)
SNN_PROTOCOLL_UNARY_OP(AbsPrime)
SNN_PROTOCOLL_UNARY_OP(Log)
SNN_PROTOCOLL_UNARY_OP(HLog)
SNN_PROTOCOLL_UNARY_OP(Log1p)
SNN_PROTOCOLL_UNARY_OP(Relu)
SNN_PROTOCOLL_UNARY_OP(ReluPrime)
SNN_PROTOCOLL_UNARY_OP(Sigmoid)

SNN_PROTOCOLL_REDUCE_OP(Max)
SNN_PROTOCOLL_REDUCE_OP(Min)
SNN_PROTOCOLL_REDUCE_OP(Mean)
SNN_PROTOCOLL_REDUCE_OP(Sum)
SNN_PROTOCOLL_REDUCE_OP(AddN)

int SnnProtocolOps::SigmoidCrossEntropy(
  const vector<string>& a,
  const vector<string>& b,
  vector<string>& output,
  const attr_type* attr_info) {
  tlog_debug << "----> SigmoidCrossEntropy";

  vector<mpc_t> private_a, private_b;

  snn_decode(a, private_a, context_->FLOAT_PRECISION);
  snn_decode(b, private_b, context_->FLOAT_PRECISION);

  vector<mpc_t> out(a.size());
  // auto ops =
  //   std::make_shared<rosetta::snn::SigmoidCrossEntropy>(_op_msg_id, net_io_, GetMpcContext());
  // ops->Run(private_a, private_b, out, a.size());
  int ret = internal_->SigmoidCrossEntropy(private_a, private_b, out, a.size());

  snn_encode(out, output);

  tlog_debug << "SigmoidCrossEntropy ok. <----";
  return ret;
}

int SnnProtocolOps::Reveal(
  const vector<string>& a,
  vector<string>& output,
  const attr_type* attr_info) {
  vector<double> dvalues;
  Reveal(a, dvalues, attr_info);
  output.resize(dvalues.size());
  for (int i = 0; i < dvalues.size(); ++i) {
    output[i] = std::to_string(dvalues[i]);
  }
  return 0;
}

int SnnProtocolOps::Reveal(
  const vector<string>& a,
  vector<double>& output,
  const attr_type* attr_info) {
  tlog_debug << "----> Reveal" ;

  string parties = attr_info ? attr_info->at("receive_parties") : "";

  vector<mpc_t> out_vec(a.size());
  vector<mpc_t> private_a(a.size());

  snn_decode(a, private_a, context_->FLOAT_PRECISION);

  vector<string> reveal_nodes = decode_reveal_nodes(parties, net_io_->GetParty2Node(), net_io_->GetResultNodes());
  string reveal_parties_str = "[";
  for (auto i = 0; i < reveal_nodes.size(); ++i) {
    reveal_parties_str.append(reveal_nodes[0]);
    if (i != reveal_nodes.size()-1)
      reveal_parties_str.append(", ");
  }
  reveal_parties_str.append("]");
  
  tlog_debug << "----> Reveal, receive_parties: " << reveal_parties_str << " ...";
  internal_->Reconstruct2PC_ex(private_a, out_vec, parties);

  // to double values
  output.resize(out_vec.size());
  convert_mpctype_to_double(out_vec, output, context_->FLOAT_PRECISION);

  tlog_debug << "Reveal ok. <----\n";
  return 0;
}

/**
 * Logical ops
 */
#define SNN_PROTOCOL_LOGICAL_OPS_CALL SNN_PROTOCOL_BINARY_OP

SNN_PROTOCOL_LOGICAL_OPS_CALL(AND)
SNN_PROTOCOL_LOGICAL_OPS_CALL(OR)
SNN_PROTOCOL_LOGICAL_OPS_CALL(XOR)

int SnnProtocolOps::NOT(
  const vector<string>& a,
  vector<string>& output,
  const attr_type* attr_info) {
  size_t size = a.size();
  vector<mpc_t> private_a(a.size());
  vector<mpc_t> out_vec(a.size());
  snn_decode(a, private_a, context_->FLOAT_PRECISION);

  internal_->Not(private_a, out_vec);
  snn_encode(out_vec, output);

  return 0;
}

/**
    @desc: This is for Tensorflow's SaveV2 Op.
*/
int SnnProtocolOps::ConditionalReveal(
  vector<string>& in_vec,
  vector<string>& out_cipher_vec,
  vector<double>& out_plain_vec) {
  std::cout << "SnnProtocolOps::ConditionalReveal!" << std::endl;
  // not to reveal plaintext by default
  const vector<string>& save_mode = context_->SAVER_MODE;
  int vec_size = in_vec.size();
  // case one: all ciphertext, just return
  if (save_mode.empty()) {
    out_cipher_vec = in_vec;
    out_plain_vec.clear();
    return 0;
  }

  // case two: only plaintext for selected parties
  vector<mpc_t> inner_out_vec(vec_size);
  vector<mpc_t> shared_input(vec_size);

  snn_decode(in_vec, shared_input, context_->FLOAT_PRECISION);
  // snn_decode(in_vec, shared_input);

  internal_->Reconstruct2PC(shared_input, inner_out_vec, save_mode);

  // to double values
  vector<double> dvalues(vec_size);
  convert_mpctype_to_double(inner_out_vec, dvalues, context_->FLOAT_PRECISION);
  // we will not use global variable partyNum in later version.
  string current_node_id = net_io_->GetCurrentNodeId();
  if (std::find(save_mode.begin(), save_mode.end(), current_node_id) != save_mode.end()) {
    out_plain_vec.swap(dvalues);
  } else {
    out_plain_vec.resize(in_vec.size());
  }
  out_cipher_vec.clear();

  return 0;
}

} // snn
} // namespace rosetta
