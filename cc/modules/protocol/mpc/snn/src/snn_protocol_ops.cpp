#include "cc/modules/protocol/mpc/snn/src/snn_protocol_ops.h"
#include "cc/modules/protocol/mpc/snn/include/snn_opsets.h"
#include "cc/modules/protocol/mpc/snn/include/opsets_nn.h"
#include "cc/modules/common/include/utils/logger.h"
#include "cc/modules/common/include/utils/str_type_convert.h"
#include "cc/modules/protocol/mpc/comm/include/mpc_helper.h"
#include "cc/modules/common/include/utils/secure_encoder.h"
#include <string>

using rosetta::convert::SecureText;
using StrVec=std::vector<std::string>;
using MpcVec=std::vector<mpc_t>;

namespace rosetta {

static inline void convert_plain_double_to_mpctype(const vector<double>& a, vector<mpc_t>& b) {
  b.resize(a.size());
  for (int i = 0; i < a.size(); i++)
    b[i] = FloatToMpcType(a[i]/2);
}

static inline int decode_input_to_snn(const StrVec& a, MpcVec& sa) {
  if (a.empty())
  {
    log_error << "empty inputs !" << endl;
    return -1;
  }

  vector<string> content_a;
  int encode_type = rosetta::convert::encoder::decode(a, content_a);
  if (0 > encode_type) {
    log_error << "decode input failed ! first elem: " << a[0] << endl;
    return -1;
  }

  if (encode_type == (int)rosetta::convert::encoder::SECURE_STR)
    rosetta::convert::from_hex_str(content_a, sa);
  else
    convert_plain_double_to_mpctype(rosetta::convert::from_double_str(content_a), sa);

  return 0;
}

SnnProtocolOps::SnnProtocolOps(const string& msgid) : ProtocolOps(msgid) {
  _op_msg_id = msgid;
}


int SnnProtocolOps::TfToSecure(const vector<string>& in, vector<string>& out, const attr_type* attr_info/* = nullptr*/) {
  log_debug << "----> TfToSecure...";

  if (in.empty()) {
    log_error << "TfToSecure input is null !";
    return -1;
  }

  auto prot = SecureText::SNN;
 
 {// convension
    vector<mpc_t> mpcvalues(in.size());
    vector<double> dvalues(in.size());
    
    for (int i = 0; i < in.size(); ++i) {
      dvalues[i] = std::stod(in[i]);
    }

    convert_double_to_mpctype(dvalues, mpcvalues);
    if (0 != rosetta::convert::encoder::encode(mpcvalues, prot, out)) {
      log_error << "decode input string failed, check input !";
      return -1;
    }
  }

  return 0;
}

// decode the string in protocol-specific format to literal number
// template <typename T>
int SnnProtocolOps::SecureToTf(const vector<string>& in, vector<string>& out, const attr_type* attr_info/* = nullptr*/) {
  log_debug << "----> SecureToTf. from mpc_t to double hex string";
  vector<string> contents;
  if (0 != rosetta::convert::encoder::decode(in, out)) {
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

int SnnProtocolOps::PrivateInput(int party_id, const vector<double>& in_vec, vector<string>& out_str_vec) {
  log_debug << "----> PrivateInput(vector<double>).";
  vector<mpc_t> out_vec;
  std::make_shared<rosetta::snn::PrivateInput>(_op_msg_id, net_io_)->Run(party_id, in_vec, out_vec);
  
  //rosetta::convert::to_hex_str(out_vec, out_str_vec);
  rosetta::convert::encoder::encode(out_vec, SecureText::SNN, out_str_vec);
  return 0;
}

template <typename OpFunctor>
int snn_protocol_binary_ops_call(const char* name, const string& msg_id, shared_ptr<NET_IO> net_io, const StrVec& a, const StrVec& b, StrVec& c, const attr_type* attr_info) {
  log_debug << "----> " << name << " binary ops "; 
  bool rh_is_const = attr_info && attr_info->count("rh_is_const") > 0 && attr_info->at("rh_is_const") == "1";
  bool lh_is_const = attr_info && attr_info->count("lh_is_const") > 0 && attr_info->at("lh_is_const") == "1";
  vector<string> content_a, content_b;
  int lh_type = rosetta::convert::encoder::decode(a, content_a);
  int rh_type = rosetta::convert::encoder::decode(b, content_b);
  if (0 > lh_type) {
    log_error << "decode binary input0 failed !";
    return -1;
  }
  if (0 > rh_type) {
    log_error << "decode binary input1 failed !";
    return -1;
  }
  vector<mpc_t> out_vec(a.size());
  vector<mpc_t> private_a;
  vector<mpc_t> private_b;
  if (rh_is_const) {
    log_debug << name << ", rh_is_const = 1(True), encode type: " << lh_type;
    if (lh_type == (int)rosetta::convert::encoder::SECURE_STR)
      rosetta::convert::from_hex_str(content_a, private_a);
    else
      convert_plain_double_to_mpctype(rosetta::convert::from_double_str(content_a), private_a);
    
    std::make_shared<OpFunctor>(msg_id, net_io)->Run(private_a, b, out_vec, a.size());
  } else if (lh_is_const) {
    log_debug << name << ", lh_is_const = 1(True), encode type: " << rh_type;
    if (rh_type == (int)rosetta::convert::encoder::SECURE_STR)
      rosetta::convert::from_hex_str(content_b, private_b);
    else
      convert_plain_double_to_mpctype(rosetta::convert::from_double_str(content_b), private_b);
    
    std::make_shared<OpFunctor>(msg_id, net_io)->Run(a, private_b, out_vec, a.size());
  } else {
    log_debug << name << " input a, b are non-const, encode type: " << lh_type << ", " << rh_type;
    if (lh_type == static_cast<int>(rosetta::convert::encoder::SECURE_STR))
      rosetta::convert::from_hex_str(content_a, private_a);
    else
      convert_plain_double_to_mpctype(rosetta::convert::from_double_str(content_a), private_a);

    if (rh_type == (int)rosetta::convert::encoder::SECURE_STR)
      rosetta::convert::from_hex_str(content_b, private_b);
    else
      convert_plain_double_to_mpctype(rosetta::convert::from_double_str(content_b), private_b);

    std::make_shared<OpFunctor>(msg_id, net_io)->Run(private_a, private_b, out_vec, a.size());
  }

  rosetta::convert::encoder::encode(out_vec, SecureText::SNN, c);
  log_debug << name << " ok. <----";

  return 0;
}

#define SNN_PROTOCOL_BINARY_OPS_CALL(OpFunctor, name, a, b, c, attr)  snn_protocol_binary_ops_call<OpFunctor>(name, _op_msg_id, net_io_, a, b, c, attr)

int SnnProtocolOps::Add(
  const vector<string>& a,
  const vector<string>& b,
  vector<string>& output,
  const attr_type* attr_info) {
  SNN_PROTOCOL_BINARY_OPS_CALL(rosetta::snn::Add, "SnnAdd", a, b, output, attr_info);
  return 0;
}

int SnnProtocolOps::Sub(
  const vector<string>& a,
  const vector<string>& b,
  vector<string>& output,
  const attr_type* attr_info) {
  SNN_PROTOCOL_BINARY_OPS_CALL(rosetta::snn::Sub, "SnnSub", a, b, output, attr_info);
  return 0;
}
int SnnProtocolOps::Mul(
  const vector<string>& a,
  const vector<string>& b,
  vector<string>& output,
  const attr_type* attr_info) {
  SNN_PROTOCOL_BINARY_OPS_CALL(rosetta::snn::Mul, "SnnMul", a, b, output, attr_info);
  return 0;
}

int SnnProtocolOps::Div(
  const vector<string>& a,
  const vector<string>& b,
  vector<string>& output,
  const attr_type* attr_info) {
  SNN_PROTOCOL_BINARY_OPS_CALL(rosetta::snn::DivisionV2, "SnnDiv", a, b, output, attr_info);
  return 0;
}
int SnnProtocolOps::Truediv(
  const vector<string>& a,
  const vector<string>& b,
  vector<string>& output,
  const attr_type* attr_info) {
  SNN_PROTOCOL_BINARY_OPS_CALL(rosetta::snn::Truediv, "SnnTruediv", a, b, output, attr_info);
  return 0;
}

int SnnProtocolOps::Floordiv(
  const vector<string>& a,
  const vector<string>& b,
  vector<string>& output,
  const attr_type* attr_info) {
  SNN_PROTOCOL_BINARY_OPS_CALL(rosetta::snn::FloorDivision, "SnnFloordiv", a, b, output, attr_info);
  return 0;
}

//// compare ops ////
int SnnProtocolOps::Less(
  const vector<string>& a,
  const vector<string>& b,
  vector<string>& output,
  const attr_type* attr_info) {
  SNN_PROTOCOL_BINARY_OPS_CALL(rosetta::snn::Less, "SnnLess", a, b, output, attr_info);
  return 0;
}

int SnnProtocolOps::LessEqual(
  const vector<string>& a,
  const vector<string>& b,
  vector<string>& output,
  const attr_type* attr_info) {
  SNN_PROTOCOL_BINARY_OPS_CALL(rosetta::snn::LessEqual, "SnnLessEqual", a, b, output, attr_info);
  return 0;
}

int SnnProtocolOps::Equal(
  const vector<string>& a,
  const vector<string>& b,
  vector<string>& output,
  const attr_type* attr_info) {
  SNN_PROTOCOL_BINARY_OPS_CALL(rosetta::snn::Equal, "SnnEqual", a, b, output, attr_info);
  return 0;
}

int SnnProtocolOps::NotEqual(
  const vector<string>& a,
  const vector<string>& b,
  vector<string>& output,
  const attr_type* attr_info) {
  SNN_PROTOCOL_BINARY_OPS_CALL(rosetta::snn::NotEqual, "SnnNotEqual", a, b, output, attr_info);
  return 0;
}

int SnnProtocolOps::Greater(
  const vector<string>& a,
  const vector<string>& b,
  vector<string>& output,
  const attr_type* attr_info) {
  SNN_PROTOCOL_BINARY_OPS_CALL(rosetta::snn::Greater, "SnnGreater", a, b, output, attr_info);
  return 0;
}

int SnnProtocolOps::GreaterEqual(
  const vector<string>& a,
  const vector<string>& b,
  vector<string>& output,
  const attr_type* attr_info) {
  SNN_PROTOCOL_BINARY_OPS_CALL(rosetta::snn::GreaterEqual, "SnnGreaterEqual", a, b, output, attr_info);
  return 0;
}

int SnnProtocolOps::Pow(
  const vector<string>& a,
  const vector<string>& b,
  vector<string>& output,
  const attr_type* attr_info) {
  log_debug << "----> " << "SnnPow";
  
  vector<string> content_a;
  int lh_type = rosetta::convert::encoder::decode(a, content_a);
  if (0 > lh_type) {
    log_error << "decode input failed, first elem: " << a[0];
    return -1;
  }

  vector<mpc_t> out_vec(a.size());
  vector<mpc_t> private_a;

  if (lh_type == (int)rosetta::convert::encoder::SECURE_STR)
    rosetta::convert::from_hex_str(content_a, private_a);
  else
    convert_plain_double_to_mpctype(rosetta::convert::from_double_str(content_a), private_a);

  //hex-mpc_t -->  double --->
  // NOTICE: only support const b
  std::make_shared<rosetta::snn::Pow>(_op_msg_id, net_io_)->Run(private_a, b, out_vec, a.size());

  // decode to secure text
  rosetta::convert::encoder::encode(out_vec, SecureText::SNN, output);
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

  vector<string> content_a, content_b;
  int lh_type = rosetta::convert::encoder::decode(a, content_a);
  if (0 > lh_type) {
    log_error << "decode input0 failed !";
    return -1;
  }

  int rh_type = rosetta::convert::encoder::decode(b, content_b);
  if (0 > rh_type) {
    log_error << "decode input1 failed !";
    return -1;
  }
  vector<mpc_t> out_vec(m * n);
  vector<mpc_t> private_a(a.size()), private_b(b.size());
  if (lh_type == (int)rosetta::convert::encoder::SECURE_STR)
    rosetta::convert::from_hex_str(content_a, private_a);
  else
    convert_plain_double_to_mpctype(rosetta::convert::from_double_str(content_a), private_a);

  if (rh_type == (int)rosetta::convert::encoder::SECURE_STR)
    rosetta::convert::from_hex_str(content_b, private_b);
  else
    convert_plain_double_to_mpctype(rosetta::convert::from_double_str(content_b), private_b);
  
  std::make_shared<rosetta::snn::MatMul>(_op_msg_id, net_io_)->Run(private_a, private_b, out_vec, m, k, n, transpose_a, transpose_b);
  
  rosetta::convert::encoder::encode(out_vec, SecureText::SNN, output);
  log_debug << "SnnMatmul ok. <----";

  return 0;
}

template <typename OpFunctor>
int snn_protocol_unary_ops_call(const char* name, const string& msg_id, shared_ptr<NET_IO> net_io, const StrVec& a, StrVec& c, const attr_type* attr) {
  log_debug << "----> " << name << " uinary ops ";
  vector<string> content_a;
  int encode_type = rosetta::convert::encoder::decode(a, content_a);
  if (0 > encode_type) {
    log_error << "decode input failed ! first elem: " << a[0];
    return -1;
  }
  vector<mpc_t> out_vec(a.size());
  vector<mpc_t> private_a(a.size());

  if (encode_type == (int)rosetta::convert::encoder::SECURE_STR)
    rosetta::convert::from_hex_str(content_a, private_a);
  else
    convert_plain_double_to_mpctype(rosetta::convert::from_double_str(content_a), private_a);
  
  std::make_shared<OpFunctor>(msg_id, net_io)->Run(private_a, out_vec, a.size());
  rosetta::convert::encoder::encode(out_vec, SecureText::SNN, c);
  log_debug << name << " ok. <----";

  return 0;
}

#define SNN_PROTOCOL_UNARY_OPS_CALL(OpFunctor, name, a, c, attr) snn_protocol_unary_ops_call<OpFunctor>(name, _op_msg_id, net_io_, a, c, attr)

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
  vector<string> content_a;
  int lh_type = rosetta::convert::encoder::decode(a, content_a);
  if (0 > lh_type) {
    log_error << "decode input failed, first elem: " << a[0];
    return -1;
  }
  
  vector<mpc_t> sa, sb;
  if (lh_type == (int)rosetta::convert::encoder::SECURE_STR)
    rosetta::convert::from_hex_str(content_a, sa);
  else
    convert_plain_double_to_mpctype(rosetta::convert::from_double_str(content_a), sa);

  std::make_shared<rosetta::snn::SigmoidCrossEntropy>(_op_msg_id, net_io_)->ABS(sa, sb, a.size());
  rosetta::convert::encoder::encode(sb, SecureText::SNN, output);

  log_debug << "SnnAbs ok. <----";
  return 0;
}

int SnnProtocolOps::AbsPrime(
  const vector<string>& a,
  vector<string>& output,
  const attr_type* attr_info) {
  log_debug << "----> SnnAbsPrime";

  vector<string> content_a;
  int lh_type = rosetta::convert::encoder::decode(a, content_a);
  if (0 > lh_type) {
    log_error << "decode input failed, first elem: " << a[0];
    return -1;
  }
  
  vector<mpc_t> sa, sb;
  if (lh_type == (int)rosetta::convert::encoder::SECURE_STR)
    rosetta::convert::from_hex_str(content_a, sa);
  else
    convert_plain_double_to_mpctype(rosetta::convert::from_double_str(content_a), sa);

  std::make_shared<rosetta::snn::SigmoidCrossEntropy>(_op_msg_id, net_io_)->ABSPrime(sa, sb, a.size());
  rosetta::convert::encoder::encode(sb, SecureText::SNN, output);

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
  vector<string> content_a;
  int encode_type = rosetta::convert::encoder::decode(a, content_a);
  if (0 > encode_type) {
    log_error << "decode input failed ! first elem: " << a[0];
    return -1;
  }
  vector<mpc_t> out_vec(a.size());
  vector<mpc_t> private_a(a.size());

  if (encode_type == (int)rosetta::convert::encoder::SECURE_STR)
    rosetta::convert::from_hex_str(content_a, private_a);
  else
    convert_plain_double_to_mpctype(rosetta::convert::from_double_str(content_a), private_a);
  
  std::make_shared<rosetta::snn::Log>(_op_msg_id, net_io_)->RunHd(private_a, out_vec, a.size());
  rosetta::convert::encoder::encode(out_vec, SecureText::SNN, output);
  log_debug << "snnLogHD ok. <----";

  return 0;
}

int SnnProtocolOps::Log1p(
  const vector<string>& a,
  vector<string>& output,
  const attr_type* attr_info) {
  output.resize(a.size());
  log_debug << "----> SnnLog1p";
  vector<string> content_a;
  int encode_type = rosetta::convert::encoder::decode(a, content_a);
  if (0 > encode_type) {
    log_error << "decode input failed, first elem: " << a[0];
    return -1;
  }
  vector<mpc_t> out_vec(a.size());
  vector<mpc_t> private_a(a.size());

  if (encode_type == (int)rosetta::convert::encoder::SECURE_STR)
    rosetta::convert::from_hex_str(content_a, private_a);
  else
    convert_plain_double_to_mpctype(rosetta::convert::from_double_str(content_a), private_a);
  
  std::make_shared<rosetta::snn::Log>(_op_msg_id, net_io_)->Run1p(private_a, out_vec, a.size());

  rosetta::convert::encoder::encode(out_vec, SecureText::SNN, output);
  log_debug << "SnnLog1p ok. <----";

  return 0;
}

template <typename OpFunctor>
int snn_protocol_reduce_ops_call(const char* name, const string& msg_id, shared_ptr<NET_IO> net_io, const StrVec& a, StrVec& c, const attr_type* attr) {
  size_t rows = 0, cols = 0;
  if (attr->count("rows") > 0 && attr->count("cols") > 0) {
    rows = std::stoull(attr->at("rows"));
    cols = std::stoull(attr->at("cols"));
  } else {
    log_error << "rows, cols should be set for Max !";
    return -1;
  }
  log_debug << "----> " << name << ", reduce ops, rows: "<< rows << ", cols: " << cols;

  vector<string> content_a;
  int encode_type = rosetta::convert::encoder::decode(a, content_a);
  if (0 > encode_type) {
    log_error << "decode input failed, first elem: " << a[0];
    return -1;
  }
  vector<mpc_t> out_vec(rows);
  vector<mpc_t> private_a(a.size());

  if (encode_type == (int)rosetta::convert::encoder::SECURE_STR)
    rosetta::convert::from_hex_str(content_a, private_a);
  else
    convert_plain_double_to_mpctype(rosetta::convert::from_double_str(content_a), private_a);
  
  std::make_shared<OpFunctor>(msg_id, net_io)->Run(private_a, out_vec, rows, cols);
  rosetta::convert::encoder::encode(out_vec, SecureText::SNN, c);
  log_debug << name << " ok. <----";

  return 0;
}
 
#define SNN_PROTOCOL_REDUCE_OPS_CALL(OpFunctor, name, a, c, attr) snn_protocol_reduce_ops_call<OpFunctor>(name, _op_msg_id, net_io_, a, c, attr)

int SnnProtocolOps::Max(
  const vector<string>& a,
  vector<string>& output,
  const attr_type* attr_info) {
  return SNN_PROTOCOL_REDUCE_OPS_CALL(rosetta::snn::Max, "SnnMax", a, output, attr_info);
}

int SnnProtocolOps::Min(
  const vector<string>& a,
  vector<string>& output,
  const attr_type* attr_info) {
  return SNN_PROTOCOL_REDUCE_OPS_CALL(rosetta::snn::Min, "SnnMin", a, output, attr_info);
}

int SnnProtocolOps::Mean(
  const vector<string>& a,
  vector<string>& output,
  const attr_type* attr_info) {
    return SNN_PROTOCOL_REDUCE_OPS_CALL(rosetta::snn::Mean, "SnnMean", a, output, attr_info);
}

int SnnProtocolOps::Sum(
  const vector<string>& a,
  vector<string>& output,
  const attr_type* attr_info) {
  return SNN_PROTOCOL_REDUCE_OPS_CALL(rosetta::snn::Sum, "SnnSum", a, output, attr_info);
}

int SnnProtocolOps::AddN(
  const vector<string>& a,
  vector<string>& output,
  const attr_type* attr_info) {
  return SNN_PROTOCOL_REDUCE_OPS_CALL(rosetta::snn::AddN, "SnnAddN", a, output, attr_info);
}

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

  vector<string> content_a;
  int lh_encode_type = rosetta::convert::encoder::decode(a, content_a);
  if (0 > lh_encode_type) {
    log_error << "decode input0 failed !";
    return -1;
  }
  vector<string> content_b;
  int rh_encode_type = rosetta::convert::encoder::decode(b, content_b);
  if (0 > rh_encode_type) {
    log_error << "decode input1 failed !";
    return -1;
  }

  if (lh_encode_type == (int)rosetta::convert::encoder::SECURE_STR)
    rosetta::convert::from_hex_str(content_a, private_a);
  else
    convert_plain_double_to_mpctype(rosetta::convert::from_double_str(content_a), private_a);
  
  if (rh_encode_type == (int)rosetta::convert::encoder::SECURE_STR)
    rosetta::convert::from_hex_str(content_b, private_b);
  else
    convert_plain_double_to_mpctype(rosetta::convert::from_double_str(content_b), private_b);

  vector<mpc_t> out(a.size());
  std::make_shared<rosetta::snn::SigmoidCrossEntropy>(_op_msg_id, net_io_)->Run(private_a, private_b, out, a.size());
  rosetta::convert::encoder::encode(out, SecureText::SNN, output);

  log_debug << "SigmoidCrossEntropy ok. <----";
  return 0;
}

int SnnProtocolOps::Reveal(
  const vector<string>& a,
  vector<string>& output,
  const attr_type* attr_info) {
  log_debug << "----> Reveal";

  int party = attr_info ? std::stoi(attr_info->at("receive_party")) : 1;
  
  vector<string> content_a(a.size());
  int encode_type = rosetta::convert::encoder::decode(a, content_a);
  if (0 > encode_type) {
    log_error << "decode input failed, first elem: " << a[0];
    return -1;
  }
  vector<mpc_t> out_vec(a.size());
  vector<mpc_t> private_a(a.size());

  if (encode_type == (int)rosetta::convert::encoder::SECURE_STR)
    rosetta::convert::from_hex_str(content_a, private_a);
  else
    convert_plain_double_to_mpctype(rosetta::convert::from_double_str(content_a), private_a);

  std::make_shared<rosetta::snn::Reconstruct2PC>(_op_msg_id, net_io_)->RunEx(private_a, out_vec, party);
 
  // to double values
  vector<double> dvalues(out_vec.size());
  convert_mpctype_to_double(out_vec, dvalues);
  // to double literal strings
  for (int i = 0; i < dvalues.size(); ++i) {
    output[i] = std::to_string(dvalues[i]);
  }

  //reveal strings are readable, not decode now for snn
  //rosetta::convert::encoder::encode(out_vec, SecureText::SNN, output);

  log_debug << "Reveal ok. <----";
  return 0;
}

/**
    @desc: This is for Tensorflow's SaveV2 Op.
*/
int SnnProtocolOps::ConditionalReveal(vector<string>& in_vec,
                        vector<string>& out_cipher_vec,
                        vector<double>& out_plain_vec) {
  std::cout << "SnnProtocolOps::ConditionalReveal!" << std::endl;
  // not to reveal plaintext by default 
  int save_mode = 0;
  int vec_size = in_vec.size();
  if(op_config_map.find("save_mode") != op_config_map.end()) {
    cout << "DEBUG:" << op_config_map["save_mode"] << endl;
    save_mode = stoi(op_config_map["save_mode"]);
  }
  // case one: all ciphertext, just return
  if(0 == save_mode) {
    out_cipher_vec = in_vec;
    out_plain_vec.clear();
    return 0;
  }

  // case two: only plaintext for selected parties  
  vector<string> input_content(vec_size);
  int encode_type = rosetta::convert::encoder::decode(in_vec, input_content);
  if (0 > encode_type) {
    log_error << "decode input failed, first elem: " << in_vec[0];
    return -1;
  }
  vector<mpc_t> inner_out_vec(vec_size);
  vector<mpc_t> shared_input(vec_size);

  if (encode_type == (int)rosetta::convert::encoder::SECURE_STR)
    rosetta::convert::from_hex_str(input_content, shared_input);
  else {
    convert_plain_double_to_mpctype(rosetta::convert::from_double_str(input_content), shared_input);
  }

  if(save_mode & 1) {
    std::make_shared<rosetta::snn::Reconstruct2PC>(_op_msg_id, net_io_)->RunV2(shared_input, vec_size, inner_out_vec, PARTY_A);
  }
  if(save_mode & 2) {
    std::make_shared<rosetta::snn::Reconstruct2PC>(_op_msg_id, net_io_)->RunV2(shared_input, vec_size, inner_out_vec, PARTY_B);
  }
  if(save_mode & 4) {
    std::make_shared<rosetta::snn::Reconstruct2PC>(_op_msg_id, net_io_)->RunV2(shared_input, vec_size, inner_out_vec, PARTY_C);
  }

  // to double values
  vector<double> dvalues(vec_size);
  convert_mpctype_to_double(inner_out_vec, dvalues);
  // we will not use global variable partyNum in later version.
  int my_party_id = partyNum;
  if(((save_mode & 1) && my_party_id == 0) || 
     ((save_mode & 2) && my_party_id == 1) ||
     ((save_mode & 4) && my_party_id == 2)) {
    out_plain_vec.swap(dvalues);
  } else {
    out_plain_vec.clear();
  }
  out_cipher_vec.clear();

  return 0;

}

} // namespace rosetta
