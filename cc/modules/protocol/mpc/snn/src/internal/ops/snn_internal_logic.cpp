#include "cc/modules/protocol/mpc/snn/include/snn_internal.h"
#include "cc/modules/common/include/utils/str_type_convert.h"
#include <thread>

using std::thread;

namespace rosetta {
namespace snn {

// OR  (x or y)  = x + y - x*y
int SnnInternal::OR(
  const vector<mpc_t>& X,
  const vector<mpc_t>& Y,
  vector<mpc_t>& Z) {
  tlog_debug << "OR ...";
  AUDIT("id:{}, P{} OR compute: Z=X+Y-X*Y, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(X));
  AUDIT("id:{}, P{} OR compute: Z=X+Y-X*Y, input Y(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(Y));
  size_t size = X.size();
  vector<mpc_t> sum(size, 0);
  addVectors<mpc_t>(X, Y, sum, size);
  AUDIT("id:{}, P{} OR X+Y, sum(=X+Y)(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(sum));
  vector<mpc_t> prod(size, 0);
  DotProduct(X, Y, prod);
  AUDIT("id:{}, P{} OR X*Y, prod(=X*Y)(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(prod));
  subtractVectors(sum, prod, Z, size);
  AUDIT("id:{}, P{} OR compute: Z=X+Y-X*Y, output Z(=sum-prod=X+Y-X*Y)(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(Z));

  tlog_debug << "OR ok.";
  return 0;
}

int SnnInternal::OR(const vector<string>& X, const vector<mpc_t>& sY, vector<mpc_t>& Z) {
  return OR(sY, X, Z);
}

// OR  (x or y)  = x + y - x*y
int SnnInternal::OR(const vector<mpc_t>& X, const vector<string>& sY, vector<mpc_t>& Z) {
  tlog_debug << "OR rh_is_const ...";
  AUDIT("id:{}, P{} OR compute: Z=X+Y-X*Y, input X(std::string){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(X));
  AUDIT("id:{}, P{} OR compute: Z=X+Y-X*Y, input Y(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<std::string>(sY));
  size_t size = X.size();
  Z.resize(size);
  vector<double> dY(size, 0);
  vector<mpc_t> Y(size, 0);
  rosetta::convert::from_double_str(sY, dY);
  const int float_precision = GetMpcContext()->FLOAT_PRECISION;
  convert_double_to_mpctype(dY, Y, float_precision);

  ///////////////////////////////////
  vector<mpc_t> sum = X;
  if (partyNum == PARTY_A) {
    // X add Y in P0, X add 0 in P1/P2
    addVectors<mpc_t>(X, Y, sum, size);
    AUDIT("id:{}, P{} OR X+Y, sum(=X+Y)(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(sum));
  }

  vector<mpc_t> prod(size, 0);
  for (size_t i = 0; i < size; i++) {
    prod[i] = X[i] * Y[i];
  }
  AUDIT("id:{}, P{} OR X*Y, prod(=X*Y)(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(prod));

  if (PRIMARY)
    Truncate(prod, float_precision, size, PARTY_A, PARTY_B, partyNum);

  subtractVectors(sum, prod, Z, size);
  AUDIT("id:{}, P{} OR compute: Z=X+Y-X*Y, output Z(=sum-prod=X+Y-X*Y)(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(Z));

  tlog_debug << "OR rh_is_const ok.";
  return 0;
}

// XOR (x xor y) = x + y - 2*x*y
int SnnInternal::XOR(
  const vector<mpc_t>& X,
  const vector<mpc_t>& Y,
  vector<mpc_t>& Z) {
  tlog_debug << "XOR ...";
  AUDIT("id:{}, P{} XOR compute: Z=X+Y-2*X*Y, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(X));
  AUDIT("id:{}, P{} XOR compute: Z=X+Y-2*X*Y, input Y(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(Y));
  size_t size = X.size();
  Z.resize(size);

  vector<mpc_t> sum(size, 0);
  addVectors<mpc_t>(X, Y, sum, size);
  AUDIT("id:{}, P{} OR X+Y, sum(=X+Y)(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(sum));
  vector<mpc_t> prod(size, 0);
  DotProduct(X, Y, prod);
  AUDIT("id:{}, P{} OR X*Y, prod(=X*Y)(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(prod));

  if (PRIMARY) {
    for (int i = 0; i < size; ++i) {
      prod[i] = prod[i] << 1;
    }
    AUDIT("id:{}, P{} OR 2*X*Y, prod(=2*X*Y)(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(prod));
  }
  subtractVectors(sum, prod, Z, size);
  AUDIT("id:{}, P{} OR compute: Z=X+Y-2*X*Y, output Z(=sum-prod=X+Y-2*X*Y)(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(Z));

  tlog_debug << "XOR ok.";
  return 0;
}

int SnnInternal::XOR(const vector<string>& X, const vector<mpc_t>& sY, vector<mpc_t>& Z) {
  return XOR(sY, X, Z);
}

int SnnInternal::XOR(
  const vector<mpc_t>& X,
  const vector<string>& sY,
  vector<mpc_t>& Z) {
  tlog_debug << "XOR rh_is_const ...";
  AUDIT("id:{}, P{} XOR compute: Z=X+Y-2*X*Y, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(X));
  AUDIT("id:{}, P{} XOR compute: Z=X+Y-2*X*Y, input Y(std::string){}", msg_id().get_hex(), context_->GetMyRole(), Vector<std::string>(sY));
  size_t size = X.size();
  Z.resize(size);
  vector<double> dY(size, 0);
  vector<mpc_t> Y(size, 0);
  rosetta::convert::from_double_str(sY, dY);
  const int float_precision = GetMpcContext()->FLOAT_PRECISION;
  convert_double_to_mpctype(dY, Y, float_precision);

  ///////////////////////////////////
  vector<mpc_t> sum = X;
  if (partyNum == PARTY_A) {
    // X add Y in P0, X add 0 in P1/P2
    addVectors<mpc_t>(X, Y, sum, size);
    AUDIT("id:{}, P{} OR X+Y, sum(=X+Y)(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(sum));
  }

  vector<mpc_t> prod(size, 0);
  for (size_t i = 0; i < size; i++) {
    prod[i] = X[i] * Y[i];
  }
  AUDIT("id:{}, P{} OR X*Y, prod(=X*Y)(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(prod));

  if (PRIMARY)
    Truncate(prod, float_precision, size, PARTY_A, PARTY_B, partyNum);
  if (PRIMARY) {
    for (int i = 0; i < size; ++i) {
      prod[i] = prod[i] << 1;
    }
    AUDIT("id:{}, P{} OR 2*X*Y, prod(=2*X*Y)(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(prod));
  }
  subtractVectors(sum, prod, Z, size);
  AUDIT("id:{}, P{} OR compute: Z=X+Y-2*X*Y, output Z(=sum-prod=X+Y-2*X*Y)(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(Z));
  
  tlog_debug << "XOR rh_is_const ok.";
  return 0;
}

// NOT (not x)   = 1 - x
int SnnInternal::Not(const vector<mpc_t>& X, vector<mpc_t>& Z) {
  tlog_debug << "LogicalNot ...";
  size_t size = X.size();
  Z.resize(size);
  AUDIT("id:{}, P{} NOT compute: Z=1-X, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(X));
  vector<mpc_t> one(size, 1 << (GetMpcContext()->FLOAT_PRECISION - 1));
  Sub(one, X, Z);
  AUDIT("id:{}, P{} NOT compute: Z=1-X, output Z(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(Z));

  tlog_debug << "LogicalNot ok.";
  return 0;
}

// AND (x and y) = x*y
int SnnInternal::AND(
  const vector<mpc_t>& X,
  const vector<mpc_t>& Y,
  vector<mpc_t>& Z) {
  tlog_debug << "Logical AND ...";
  AUDIT("id:{}, P{} AND compute: Z=X*Y, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(X));
  AUDIT("id:{}, P{} AND compute: Z=X*Y, input Y(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(Y));
  Z.resize(X.size());
  DotProduct(X, Y, Z);
  AUDIT("id:{}, P{} AND compute: Z=X*Y, input Z=(X*Y)(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(Z));
  tlog_debug << "Logical AND.";
  return 0;
}

int SnnInternal::AND(const vector<string>& X, const vector<mpc_t>& sY, vector<mpc_t>& Z) {
  return AND(sY, X, Z);
}

int SnnInternal::AND(
  const vector<mpc_t>& X,
  const vector<string>& sY,
  vector<mpc_t>& Z) {
  AUDIT("id:{}, P{} AND compute: Z=X*Y, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(X));
  AUDIT("id:{}, P{} AND compute: Z=X*Y, input Y(std::string){}", msg_id().get_hex(), context_->GetMyRole(), Vector<std::string>(sY));
  tlog_debug << "Logical AND rh_is_const ...";
  size_t size = X.size();
  Z.resize(size);
  vector<double> dY(size, 0);
  vector<mpc_t> Y(size, 0);
  rosetta::convert::from_double_str(sY, dY);
  convert_double_to_mpctype(dY, Y, GetMpcContext()->FLOAT_PRECISION);

  for (size_t i = 0; i < size; i++) {
    Z[i] = X[i] * Y[i];
  }
  if (PRIMARY)
    Truncate(Z, GetMpcContext()->FLOAT_PRECISION, size, PARTY_A, PARTY_B, partyNum);
  
  AUDIT("id:{}, P{} AND compute: Z=X*Y, input Z=(X*Y)(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(Z));
  tlog_debug << "Logical AND rh_is_const ...";
  return 0;
}

}//snn
}//rosetta
