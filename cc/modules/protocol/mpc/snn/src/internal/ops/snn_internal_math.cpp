#include "cc/modules/protocol/mpc/snn/include/snn_internal.h"
#include "cc/modules/common/include/utils/str_type_convert.h"
#include <thread>

using std::thread;

namespace rosetta {
namespace snn {

// for continuous common_power_list with max order be N, round complexity is N.
void SnnInternal::UniPolynomial(
  const vector<mpc_t>& shared_X,
  const vector<mpc_t>& common_power_list,
  const vector<mpc_t>& common_coff_list,
  vector<mpc_t>& shared_Y) {
  tlog_debug << "UniPolynomial ...";
  AUDIT("id:{}, P{} UniPolynomial, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(shared_X));
  AUDIT("id:{}, P{} UniPolynomial, input power(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(common_power_list));
  AUDIT("id:{}, P{} UniPolynomial, input coff(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(common_coff_list));

  int vec_size = shared_X.size();
  shared_Y.resize(vec_size);
  // this cache is for reduce calling Mul operation.
  // It works for continuous common_power_list.
  unordered_map<int, vector<mpc_t>> X_pow_cache;
  X_pow_cache[1] = shared_X;

  // Step one:
  vector<mpc_t> local_value(vec_size, 0);
  for (auto i = 0; i < common_power_list.size(); ++i) {
    // cout << "power" << i << endl;
    vector<mpc_t> tmp_coff_vec(vec_size, common_coff_list[i]);
    vector<mpc_t> tmp_prod(vec_size);
    if (common_power_list[i] == 0) {
      if (partyNum == PARTY_A) {
        addVectors(local_value, tmp_coff_vec, local_value, vec_size);
      }
    } else if (common_power_list[i] == 1) {
      if (PRIMARY) {
        // local const multiply
        mpc_t coff_v = common_coff_list[i];
        PolynomialLocalConstMul(shared_X, coff_v, tmp_prod);
        addVectors(local_value, tmp_prod, local_value, vec_size);
      }
    } else {
      vector<mpc_t> term_v(vec_size);
      mpc_t curr_k = common_power_list[i];
      if (X_pow_cache.find(curr_k - 1) != X_pow_cache.end()) {
        DotProduct(shared_X, X_pow_cache[curr_k - 1], term_v);
        X_pow_cache[curr_k] = term_v;
      } else {
        PolynomialPowConst(shared_X, curr_k, term_v);
        X_pow_cache[curr_k] = term_v;
      }
      if (PRIMARY) {
        // local const multiply
        mpc_t coff_v = common_coff_list[i];
        PolynomialLocalConstMul(term_v, common_coff_list[i], tmp_prod);
        addVectors(local_value, tmp_prod, local_value, vec_size);
      }
    }
  }
  shared_Y = local_value;

  AUDIT("id:{}, P{} UniPolynomial, output(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(shared_Y));
  tlog_debug << "UniPolynomial ok.";
}

void SnnInternal::UniPolynomial(
  const mpc_t& shared_X,
  const vector<mpc_t>& common_power_list,
  const vector<mpc_t>& common_coff_list,
  mpc_t& shared_Y) {
  tlog_debug << "UniPolynomial single element ...";
  AUDIT("id:{}, P{} UniPolynomial single element, input X(mpc_t): {}", msg_id().get_hex(), context_->GetMyRole(), shared_Y);
  AUDIT("id:{}, P{} UniPolynomial single element, input power(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(common_power_list));
  AUDIT("id:{}, P{} UniPolynomial single element, input coff(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(common_coff_list));
  // Step one:
  mpc_t local_value = 0;
  const int float_precision = GetMpcContext()->FLOAT_PRECISION;
  for (auto i = 0; i < common_power_list.size(); ++i) {
    vector<mpc_t> tmp_prod(1);
    if (common_power_list[i] == 0) {
      if (partyNum == PARTY_A) {
        local_value += common_coff_list[i];
      }
    } else if (common_power_list[i] == 1) {
      if (PRIMARY) {
        mpc_t coff_v = common_coff_list[i];
        // local const multiply
        mpc_t term_v = shared_X * coff_v;
        tmp_prod[0] = term_v;
        Truncate(tmp_prod, float_precision, 1, PARTY_A, PARTY_B, partyNum);
        local_value += tmp_prod[0];
      }
    } else {
      mpc_t term_v;
      mpc_t curr_k = common_power_list[i];
      PolynomialPowConst(shared_X, curr_k, term_v);
      if (PRIMARY) {
        // local const multiply
        term_v = term_v * common_coff_list[i];
        tmp_prod[0] = term_v;
        Truncate(tmp_prod, float_precision, 1, PARTY_A, PARTY_B, partyNum);
        local_value += tmp_prod[0];
      }
    }
  }
  shared_Y = local_value;

  AUDIT("id:{}, P{} UniPolynomial single element, output(mpc_t): {}", msg_id().get_hex(), context_->GetMyRole(), shared_Y);
  tlog_debug << "UniPolynomial single element ok.";
}

void SnnInternal::PolynomialPowConst(
  const vector<mpc_t>& shared_X,
  mpc_t common_k,
  vector<mpc_t>& shared_Y) {
  tlog_debug << "UniPolynomialPowConst  ...";
  AUDIT("id:{}, P{} UniPolynomialPowConst, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(shared_Y));
  AUDIT("id:{}, P{} UniPolynomialPowConst, input K(mpc_t): {}", msg_id().get_hex(), context_->GetMyRole(), common_k);
  // cout << "DEBUG new mpc_pow_const" << endl;
  int vec_size = shared_X.size();
  shared_Y.resize(vec_size);
  // init curr_Y to '1'
  vector<mpc_t> curr_Y(vec_size, FloatToMpcType(1.0 / 2, GetMpcContext()->FLOAT_PRECISION));

  if (common_k == 0) {
    shared_Y = curr_Y;
    return;
  }
  if (common_k == 1) {
    shared_Y = shared_X;
    return;
  }

  int curr_k = common_k;
  // current value in this bit
  int curr_bit = 0;
  // 2's power value
  int curr_p = 1;

  vector<mpc_t> P = shared_X;
  vector<mpc_t> tmp_new_y = curr_Y;
  vector<mpc_t> tmp_new_P = P;
  bool least_bit_covered = false;
  while (curr_k != 0) {
    curr_bit = curr_k % 2;
    if (curr_p != 1) {
      DotProduct(P, P, tmp_new_P);
      P = tmp_new_P;
    }
    if (curr_bit) {
      // LSB bit, no need to use MPC to multiply const ONE.
      if (!least_bit_covered) {
        curr_Y = P;
        least_bit_covered = true;
      } else {
        DotProduct(P, curr_Y, tmp_new_y);
        curr_Y = tmp_new_y;
      }
    }
    curr_k = int(curr_k / 2);
    curr_p++;
  }
  shared_Y = curr_Y;

  AUDIT("id:{}, P{} UniPolynomialConst, output(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(shared_Y));
  tlog_debug << "UniPolynomialConst  ok.";
  return;
}

void SnnInternal::PolynomialPowConst(const mpc_t& shared_X, mpc_t common_k, mpc_t& shared_Y) {
  vector<mpc_t> vec_x(1, shared_X);
  vector<mpc_t> vec_y(1);
  PolynomialPowConst(vec_x, common_k, vec_y);
  shared_Y = vec_y[0];
}

void SnnInternal::PolynomialLocalConstMul(
  const vector<mpc_t>& shared_X,
  mpc_t common_V,
  vector<mpc_t>& shared_Y) {
  tlog_debug << "PolynomialLocalConstMul  ...";
  AUDIT("id:{}, P{} PolynomialLocalConstMul, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(shared_Y));
  AUDIT("id:{}, P{} PolynomialLocalConstMul, input V(mpc_t): {}", msg_id().get_hex(), context_->GetMyRole(), common_V);

  int vec_size = shared_X.size();
  shared_Y.resize(vec_size);
  const int float_precision = GetMpcContext()->FLOAT_PRECISION;
  if (PRIMARY) {
    for (int i = 0; i < vec_size; ++i) {
      shared_Y[i] = common_V * shared_X[i];
      Truncate(shared_Y[i], float_precision, PARTY_A, PARTY_B, partyNum);
    }
  }

  AUDIT("id:{}, P{} PolynomialLocalConstMul, output(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(shared_Y));
  tlog_debug << "PolynomialLocalConstMul  ok.";
}

int SnnInternal::PowV1(const vector<mpc_t>& x, size_t n, vector<mpc_t>& y) {
  size_t size = x.size();
  vector<int64_t> vn(size, (int64_t)n);
  return Pow(x, vn, y);//call LogV2
}

int SnnInternal::Pow(const vector<mpc_t>& x, vector<int64_t> n, vector<mpc_t>& y) {
  tlog_debug << "Pow  ...";
  AUDIT("id:{}, P{} Pow, input(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(x));
  size_t size = x.size();
  assert(x.size() == n.size());
  assert(x.size() == size);
  // auto poly = GetSnnOpInner(Polynomial);

  // Added in V0.2.1 to support vectorization
  bool is_common_k = true;
  for (int i = 1; i < size; ++i) {
    if (n[i] != n[i - 1]) {
      is_common_k = false;
      break;
    }
  }

  if (is_common_k) {
    // cout << "DEBUG; common k" << endl;
    PolynomialPowConst(x, n[0], y);
    return 0;
  }

  for (auto i = 0; i < size; i++) {
    PolynomialPowConst(x[i], n[i], y[i]);
  }

  AUDIT("id:{}, P{} Pow, output n(int64_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<int64_t>(n));
  AUDIT("id:{}, P{} Pow, output y(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(y));
  tlog_debug << "Pow  ok.";
  return 0;
}

//[kelvin] NOTE: string represents const
int SnnInternal::PowV1(const vector<mpc_t>& x, const string& n, vector<mpc_t>& y) {
  y.resize(x.size());
  int64_t ni = std::stoll(n);
  return PowV1(x, ni, y);
}

//[kelvin] NOTE: string represents const
int SnnInternal::Pow(
  const vector<mpc_t>& x,
  const vector<string>& n,
  vector<mpc_t>& y) {
  size_t size = x.size();
  y.resize(size);
  vector<int64_t> ni(size, 0);
  rosetta::convert::from_int_str(n, ni);
  return Pow(x, ni, y);
}

}
}
