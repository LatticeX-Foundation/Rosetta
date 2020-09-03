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
#include "cc/modules/protocol/mpc/snn/src/snn_protocol_ops.h"
#include "cc/modules/protocol/mpc/snn/include/snn_opsets.h"
#include "cc/modules/protocol/mpc/snn/include/opsets_nn.h"
#include "cc/modules/common/include/utils/logger.h"
#include "cc/modules/common/include/utils/str_type_convert.h"
#include "cc/modules/common/include/utils/perf_stats.h"
#include "cc/modules/protocol/mpc/comm/include/mpc_helper.h"
#include "cc/modules/common/include/utils/secure_encoder.h"
#include <string>

using StrVec = std::vector<std::string>;
using MpcVec = std::vector<mpc_t>;

namespace rosetta {
// step1 define the global(static) timer variables
DEFINE_GLOBAL_TIMER_COUNTER(convert_string_to_share_timer)
DEFINE_GLOBAL_TIMER_COUNTER(convert_share_to_string_timer)

// step2 define an atexit function(static)
DEFINE_AT_EXIT_FUNCTION_BEG()
DEFINE_AT_EXIT_FUNCTION_BODY(convert_string_to_share_timer)
DEFINE_AT_EXIT_FUNCTION_BODY(convert_share_to_string_timer)
DEFINE_AT_EXIT_FUNCTION_END()

// step3 use ELAPSED_STATISTIC_BEG/ELAPSED_STATISTIC_END in the program
} // namespace rosetta

namespace rosetta {
static inline void convert_plain_double_to_mpctype(const vector<double>& a, vector<mpc_t>& b) {
  b.resize(a.size());
  for (int i = 0; i < a.size(); i++)
    b[i] = FloatToMpcType(a[i] / 2);
}

static inline int snn_decode_(const StrVec& a, MpcVec& sa) {
  ELAPSED_STATISTIC_BEG(convert_string_to_share_timer);

  if (rosetta::convert::is_secure_text(a[0])) {
    sa.resize(a.size());
    for (auto i = 0; i < a.size(); ++i)
      memcpy((char*)&sa[i], a[i].data(), sizeof(mpc_t));

    ELAPSED_STATISTIC_END(convert_string_to_share_timer);
  } else {
    ELAPSED_STATISTIC_END(convert_string_to_share_timer);
    convert_plain_double_to_mpctype(rosetta::convert::from_double_str(a), sa);
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
#define snn_decode(a, sa)                                                                 \
  do {                                                                                    \
    if (0 != snn_decode_(a, sa)) {                                                        \
      log_error << "rosetta::convert::encoder::decode failed! In " << __FUNCTION__ << "#" \
                << __LINE__ << endl;                                                      \
      return -1;                                                                          \
    }                                                                                     \
  } while (0)
#define snn_encode(sa, a)                                                                 \
  do {                                                                                    \
    if (0 != snn_encode_(sa, a)) {                                                        \
      log_error << "rosetta::convert::encoder::encode failed! In " << __FUNCTION__ << "#" \
                << __LINE__ << endl;                                                      \
      return -1;                                                                          \
    }                                                                                     \
  } while (0)

static inline int decode_input_to_snn(const StrVec& a, MpcVec& sa) {
  if (a.empty()) {
    log_error << "empty inputs !" << endl;
    return -1;
  }

  snn_decode(a, sa);
  return 0;
}

SnnProtocolOps::SnnProtocolOps(const string& msgid) : ProtocolOps(msgid) { _op_msg_id = msgid; }

int SnnProtocolOps::TfToSecure(
  const vector<string>& in,
  vector<string>& out,
  const attr_type* attr_info /* = nullptr*/) {
  log_debug << "----> TfToSecure...";

  if (in.empty()) {
    log_error << "TfToSecure input is null !";
    return -1;
  }

  {
    // convension
    vector<mpc_t> mpcvalues(in.size());
    vector<double> dvalues(in.size());

    for (int i = 0; i < in.size(); ++i) {
      dvalues[i] = std::stod(in[i]);
    }

    convert_double_to_mpctype(dvalues, mpcvalues);
    snn_encode(mpcvalues, out);
  }

  return 0;
}

// decode the string in protocol-specific format to literal number
int SnnProtocolOps::SecureToTf(
  const vector<string>& in,
  vector<string>& out,
  const attr_type* attr_info /* = nullptr*/) {
  log_debug << "----> SecureToTf. from mpc_t to double hex string";
  vector<string> contents;
  if (0 != rosetta::convert::encoder::decode_secure(in, out)) {
    log_error << "decode error, input is valid !";
    return -1;
  }

  // to mpc_t values
  vector<mpc_t> mpcvalues(in.size());
  vector<double> dvalues(in.size());
  rosetta::convert::from_hex_str<mpc_t>(out, mpcvalues);

  // to double values
  convert_mpctype_to_double(mpcvalues, dvalues);

  // to hex string
  rosetta::convert::to_hex_str<double>(dvalues, out);
  return 0;
}

int SnnProtocolOps::RandSeed(std::string op_seed, string& out_str) {
  log_debug << "----> RandSeed.";
  mpc_t out;
  // GetSnnOpWithKey(RandomSeed, _op_msg_id)->Run(out);
  std::make_shared<rosetta::snn::RandomSeed>(_op_msg_id, net_io_)->Run(out);

  rosetta::convert::to_hex_str(out, out_str);
  return 0;
}

int SnnProtocolOps::PrivateInput(
  int party_id,
  const vector<double>& in_vec,
  vector<string>& out_str_vec) {
  log_debug << "----> PrivateInput(vector<double>).";
  vector<mpc_t> out_vec;
  std::make_shared<rosetta::snn::PrivateInput>(_op_msg_id, net_io_)->Run(party_id, in_vec, out_vec);

  snn_encode(out_vec, out_str_vec);
  return 0;
}

template <typename OpFunctor>
int snn_protocol_binary_ops_call(
  const char* name,
  const string& msg_id,
  shared_ptr<NET_IO> net_io,
  const StrVec& a,
  const StrVec& b,
  StrVec& c,
  const attr_type* attr_info) {
  log_debug << "----> " << name << " binary ops ";

  bool rh_is_const =
    attr_info && attr_info->count("rh_is_const") > 0 && attr_info->at("rh_is_const") == "1";
  bool lh_is_const =
    attr_info && attr_info->count("lh_is_const") > 0 && attr_info->at("lh_is_const") == "1";
  log_debug << name << ", lh_is_const = " << rh_is_const << ", rh_is_const = " << rh_is_const;

  vector<mpc_t> out_vec(a.size());
  vector<mpc_t> private_a;
  vector<mpc_t> private_b;

  if (rh_is_const) {
    snn_decode(a, private_a);
    std::make_shared<OpFunctor>(msg_id, net_io)->Run(private_a, b, out_vec, a.size());
  } else if (lh_is_const) {
    snn_decode(b, private_b);
    std::make_shared<OpFunctor>(msg_id, net_io)->Run(a, private_b, out_vec, a.size());
  } else {
    snn_decode(a, private_a);
    snn_decode(b, private_b);
    std::make_shared<OpFunctor>(msg_id, net_io)->Run(private_a, private_b, out_vec, a.size());
  }
  snn_encode(out_vec, c);

  log_debug << name << " ok. <----";
  return 0;
}

#define SNN_PROTOCOL_BINARY_OPS_CALL(op, OpFunctor)                                               \
  int SnnProtocolOps::op(                                                                         \
    const vector<string>& a, const vector<string>& b, vector<string>& c, const attr_type* attr) { \
    snn_protocol_binary_ops_call<OpFunctor>(#op, _op_msg_id, net_io_, a, b, c, attr);             \
    return 0;                                                                                     \
  }

SNN_PROTOCOL_BINARY_OPS_CALL(Add, rosetta::snn::Add)
SNN_PROTOCOL_BINARY_OPS_CALL(Sub, rosetta::snn::Sub)
SNN_PROTOCOL_BINARY_OPS_CALL(Mul, rosetta::snn::Mul)
SNN_PROTOCOL_BINARY_OPS_CALL(Div, rosetta::snn::DivisionV2)
SNN_PROTOCOL_BINARY_OPS_CALL(Truediv, rosetta::snn::Truediv)
SNN_PROTOCOL_BINARY_OPS_CALL(Floordiv, rosetta::snn::FloorDivision)

SNN_PROTOCOL_BINARY_OPS_CALL(Less, rosetta::snn::Less)
SNN_PROTOCOL_BINARY_OPS_CALL(LessEqual, rosetta::snn::LessEqual)
SNN_PROTOCOL_BINARY_OPS_CALL(Equal, rosetta::snn::Equal)
SNN_PROTOCOL_BINARY_OPS_CALL(NotEqual, rosetta::snn::NotEqual)
SNN_PROTOCOL_BINARY_OPS_CALL(Greater, rosetta::snn::Greater)
SNN_PROTOCOL_BINARY_OPS_CALL(GreaterEqual, rosetta::snn::GreaterEqual)

int SnnProtocolOps::Pow(
  const vector<string>& a,
  const vector<string>& b,
  vector<string>& output,
  const attr_type* attr_info) {
  log_debug << "----> SnnPow";

  vector<mpc_t> out_vec(a.size());
  vector<mpc_t> private_a;
  snn_decode(a, private_a);

  //! @attention: only support const b
  std::make_shared<rosetta::snn::Pow>(_op_msg_id, net_io_)->Run(private_a, b, out_vec, a.size());

  snn_encode(out_vec, output);
  log_debug << "SnnPow ok. <----";

  return 0;
}

int SnnProtocolOps::Matmul(
  const vector<string>& a,
  const vector<string>& b,
  vector<string>& output,
  const attr_type* attr_info) {
  log_debug << "----> "
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

  bool transpose_a = false, transpose_b = false;
  if (attr_info->count("transpose_a") > 0 && attr_info->at("transpose_a") == "1")
    transpose_a = true;
  if (attr_info->count("transpose_b") > 0 && attr_info->at("transpose_b") == "1")
    transpose_b = true;

  vector<mpc_t> out_vec(m * n);
  vector<mpc_t> private_a, private_b;
  snn_decode(a, private_a);
  snn_decode(b, private_b);

  std::make_shared<rosetta::snn::MatMul>(_op_msg_id, net_io_)
    ->Run(private_a, private_b, out_vec, m, k, n, transpose_a, transpose_b);

  snn_encode(out_vec, output);
  log_debug << "SnnMatmul ok. <----";

  return 0;
}

template <typename OpFunctor>
int snn_protocol_unary_ops_call(
  const char* name,
  const string& msg_id,
  shared_ptr<NET_IO> net_io,
  const StrVec& a,
  StrVec& c,
  const attr_type* attr) {
  log_debug << "----> " << name << " uinary ops ";

  vector<mpc_t> out_vec(a.size());
  vector<mpc_t> private_a(a.size());
  snn_decode(a, private_a);

  std::make_shared<OpFunctor>(msg_id, net_io)->Run(private_a, out_vec, a.size());
  snn_encode(out_vec, c);
  log_debug << name << " ok. <----";

  return 0;
}

#define SNN_PROTOCOL_UNARY_OPS_CALL(OpFunctor, name, a, c, attr) \
  snn_protocol_unary_ops_call<OpFunctor>(name, _op_msg_id, net_io_, a, c, attr)

int SnnProtocolOps::Square(
  const vector<string>& a,
  vector<string>& output,
  const attr_type* attr_info) {
  SNN_PROTOCOL_UNARY_OPS_CALL(rosetta::snn::Square, "SnnSquare", a, output, attr_info);
  return 0;
}

int SnnProtocolOps::Negative(
  const vector<string>& a,
  vector<string>& output,
  const attr_type* attr_info) {
  SNN_PROTOCOL_UNARY_OPS_CALL(rosetta::snn::Negative, "SnnNegative", a, output, attr_info);
  return 0;
}

int SnnProtocolOps::Abs(
  const vector<string>& a,
  vector<string>& output,
  const attr_type* attr_info) {
  log_debug << "----> SnnAbs";

  vector<mpc_t> sa, sb;
  snn_decode(a, sa);

  std::make_shared<rosetta::snn::SigmoidCrossEntropy>(_op_msg_id, net_io_)->ABS(sa, sb, a.size());
  snn_encode(sb, output);

  log_debug << "SnnAbs ok. <----";
  return 0;
}

int SnnProtocolOps::AbsPrime(
  const vector<string>& a,
  vector<string>& output,
  const attr_type* attr_info) {
  log_debug << "----> SnnAbsPrime";

  vector<mpc_t> sa, sb;
  snn_decode(a, sa);

  std::make_shared<rosetta::snn::SigmoidCrossEntropy>(_op_msg_id, net_io_)
    ->ABSPrime(sa, sb, a.size());
  snn_encode(sb, output);

  log_debug << "SnnAbsPrime ok. <----";
  return 0;
}

int SnnProtocolOps::Log(
  const vector<string>& a,
  vector<string>& output,
  const attr_type* attr_info) {
  log_debug << "----> SnnLog";
  output.resize(a.size());
  SNN_PROTOCOL_UNARY_OPS_CALL(rosetta::snn::Log, "SnnLog", a, output, attr_info);
  return 0;
}

int SnnProtocolOps::HLog(
  const vector<string>& a,
  vector<string>& output,
  const attr_type* attr_info) {
  output.resize(a.size());
  log_debug << "----> SnnHLog";

  vector<mpc_t> out_vec(a.size());
  vector<mpc_t> private_a(a.size());
  snn_decode(a, private_a);

  std::make_shared<rosetta::snn::Log>(_op_msg_id, net_io_)->RunHd(private_a, out_vec, a.size());
  snn_encode(out_vec, output);
  log_debug << "snnLogHD ok. <----";

  return 0;
}

int SnnProtocolOps::Log1p(
  const vector<string>& a,
  vector<string>& output,
  const attr_type* attr_info) {
  output.resize(a.size());
  log_debug << "----> SnnLog1p";

  vector<mpc_t> out_vec(a.size());
  vector<mpc_t> private_a(a.size());
  snn_decode(a, private_a);

  std::make_shared<rosetta::snn::Log>(_op_msg_id, net_io_)->Run1p(private_a, out_vec, a.size());

  snn_encode(out_vec, output);
  log_debug << "SnnLog1p ok. <----";

  return 0;
}

template <typename OpFunctor>
int snn_protocol_reduce_ops_call(
  const char* name,
  const string& msg_id,
  shared_ptr<NET_IO> net_io,
  const StrVec& a,
  StrVec& c,
  const attr_type* attr) {
  size_t rows = 0, cols = 0;
  if (attr->count("rows") > 0 && attr->count("cols") > 0) {
    rows = std::stoull(attr->at("rows"));
    cols = std::stoull(attr->at("cols"));
  } else {
    log_error << "rows, cols should be set for Max !";
    return -1;
  }
  log_debug << "----> " << name << ", reduce ops, rows: " << rows << ", cols: " << cols;

  vector<mpc_t> out_vec(rows);
  vector<mpc_t> private_a(a.size());
  snn_decode(a, private_a);

  std::make_shared<OpFunctor>(msg_id, net_io)->Run(private_a, out_vec, rows, cols);
  snn_encode(out_vec, c);
  log_debug << name << " ok. <----";

  return 0;
}

#define SNN_PROTOCOL_REDUCE_OPS_CALL(op, OpFunctor)                                           \
  int SnnProtocolOps::op(const vector<string>& a, vector<string>& c, const attr_type* attr) { \
    snn_protocol_reduce_ops_call<OpFunctor>(#op, _op_msg_id, net_io_, a, c, attr);            \
    return 0;                                                                                 \
  }

SNN_PROTOCOL_REDUCE_OPS_CALL(Max, rosetta::snn::Max)
SNN_PROTOCOL_REDUCE_OPS_CALL(Min, rosetta::snn::Min)
SNN_PROTOCOL_REDUCE_OPS_CALL(Mean, rosetta::snn::Mean)
SNN_PROTOCOL_REDUCE_OPS_CALL(Sum, rosetta::snn::Sum)
SNN_PROTOCOL_REDUCE_OPS_CALL(AddN, rosetta::snn::AddN)

////////////////////////////////// nn ops //////////////////////////////////
int SnnProtocolOps::Relu(
  const vector<string>& a,
  vector<string>& output,
  const attr_type* attr_info) {
  SNN_PROTOCOL_UNARY_OPS_CALL(rosetta::snn::Relu, "SnnRelu", a, output, attr_info);
  return 0;
}

int SnnProtocolOps::ReluPrime(
  const vector<string>& a,
  vector<string>& output,
  const attr_type* attr_info) {
  SNN_PROTOCOL_UNARY_OPS_CALL(rosetta::snn::ReluPrime, "SnnReluPrime", a, output, attr_info);
  return 0;
}

int SnnProtocolOps::Sigmoid(
  const vector<string>& a,
  vector<string>& output,
  const attr_type* attr_info) {
  SNN_PROTOCOL_UNARY_OPS_CALL(rosetta::snn::Sigmoid, "SnnSigmoid", a, output, attr_info);
  return 0;
}

int SnnProtocolOps::SigmoidCrossEntropy(
  const vector<string>& a,
  const vector<string>& b,
  vector<string>& output,
  const attr_type* attr_info) {
  log_debug << "----> SigmoidCrossEntropy";

  vector<mpc_t> private_a, private_b;
  snn_decode(a, private_a);
  snn_decode(b, private_b);

  vector<mpc_t> out(a.size());
  std::make_shared<rosetta::snn::SigmoidCrossEntropy>(_op_msg_id, net_io_)
    ->Run(private_a, private_b, out, a.size());
  snn_encode(out, output);

  log_debug << "SigmoidCrossEntropy ok. <----";
  return 0;
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
  log_debug << "----> Reveal\n" << endl;

  int party = attr_info ? std::stoi(attr_info->at("receive_party")) : 1;

  vector<mpc_t> out_vec(a.size());
  vector<mpc_t> private_a(a.size());
  snn_decode(a, private_a);

  log_debug << "----> Reveal, call Reconstruct2PC\n" << endl;
  std::make_shared<rosetta::snn::Reconstruct2PC>(_op_msg_id, net_io_)
    ->RunEx(private_a, out_vec, party);

  // to double values
  output.resize(out_vec.size());
  convert_mpctype_to_double(out_vec, output);

  log_debug << "Reveal ok. <----\n";
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
  int save_mode = 0;
  int vec_size = in_vec.size();
  if (op_config_map.find("save_mode") != op_config_map.end()) {
    cout << "DEBUG:" << op_config_map["save_mode"] << endl;
    save_mode = stoi(op_config_map["save_mode"]);
  }
  // case one: all ciphertext, just return
  if (0 == save_mode) {
    out_cipher_vec = in_vec;
    out_plain_vec.clear();
    return 0;
  }

  // case two: only plaintext for selected parties
  vector<mpc_t> inner_out_vec(vec_size);
  vector<mpc_t> shared_input(vec_size);
  snn_decode(in_vec, shared_input);

  if (save_mode & 1) {
    std::make_shared<rosetta::snn::Reconstruct2PC>(_op_msg_id, net_io_)
      ->RunV2(shared_input, vec_size, inner_out_vec, PARTY_A);
  }
  if (save_mode & 2) {
    std::make_shared<rosetta::snn::Reconstruct2PC>(_op_msg_id, net_io_)
      ->RunV2(shared_input, vec_size, inner_out_vec, PARTY_B);
  }
  if (save_mode & 4) {
    std::make_shared<rosetta::snn::Reconstruct2PC>(_op_msg_id, net_io_)
      ->RunV2(shared_input, vec_size, inner_out_vec, PARTY_C);
  }

  // to double values
  vector<double> dvalues(vec_size);
  convert_mpctype_to_double(inner_out_vec, dvalues);
  // we will not use global variable partyNum in later version.
  int my_party_id = partyNum;
  if (
    ((save_mode & 1) && my_party_id == 0) || ((save_mode & 2) && my_party_id == 1) ||
    ((save_mode & 4) && my_party_id == 2)) {
    out_plain_vec.swap(dvalues);
  } else {
    out_plain_vec.clear();
  }
  out_cipher_vec.clear();

  return 0;
}

} // namespace rosetta
