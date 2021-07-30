#include "cc/modules/protocol/mpc/snn/include/snn_internal.h"
#include "cc/modules/common/include/utils/str_type_convert.h"
#include <thread>

using std::thread;

namespace rosetta {
namespace snn {

int SnnInternal::HLog(const vector<mpc_t>& shared_X, vector<mpc_t>& shared_Y) {
  assert(THREE_PC && "non-3PC running mode!!!");
  tlog_debug << "HLog ...";
  AUDIT("id:{}, P{} HLog, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(shared_X));
  size_t vec_size = shared_X.size();
  mpc_t LEN = 8 * sizeof(mpc_t);
  vector<mpc_t> SHARED_LN_2(vec_size, 0);
  const int float_precision = GetMpcContext()->FLOAT_PRECISION;
  if (partyNum == PARTY_A) {
    // math.log(2) = 0.693147181
    SHARED_LN_2 = vector<mpc_t>(vec_size, FloatToMpcType(0.693147181, float_precision));
  }
  vector<mpc_t> SHARED_HALF(vec_size, 0);
  if (partyNum == PARTY_A) {
    SHARED_HALF = vector<mpc_t>(vec_size, FloatToMpcType(0.5, float_precision));
    AUDIT("id:{}, P{} HLog, Shared_Half(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(SHARED_HALF));
  }
  vector<mpc_t> SHARED_ONE(vec_size, 0);
  if (partyNum == PARTY_A) {
    SHARED_ONE = vector<mpc_t>(vec_size, FloatToMpcType(1, float_precision));
    AUDIT("id:{}, P{} HLog, Shared_One(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(SHARED_ONE));
  }
  vector<mpc_t> SHARED_TWO(vec_size, 0);
  if (partyNum == PARTY_A) {
    SHARED_TWO = vector<mpc_t>(vec_size, FloatToMpcType(2, float_precision));
    AUDIT("id:{}, P{} HLog, Shared_Two(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(SHARED_TWO));
  }
  vector<mpc_t> SHARED_ZERO(vec_size, 0);
  vector<mpc_t> SHARED_NEG_ONE(vec_size, 0);
  if (partyNum == PARTY_A) {
    SHARED_NEG_ONE = vector<mpc_t>(vec_size, FloatToMpcType(-1, float_precision));
    AUDIT("id:{}, P{} HLog, Shared_Neg_One(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(SHARED_NEG_ONE));
  }

  vector<mpc_t> shared_val_multiplier(vec_size, 0);
  vector<mpc_t> shared_power_add(vec_size, 0);
  auto curr_x = shared_X;
  vector<mpc_t> curr_power(vec_size, 0);
  // step 1: express x as 2^{m} * {r/2} where r \in [1, 2)
  // 1.1 scale down integer part
  for (int i = 0; i < LEN - float_precision; ++i) {
    auto curr_x_minus_one = curr_x;
    vector<mpc_t> shared_cmp(vec_size, 0);

    subtractVectors<mpc_t>(curr_x, SHARED_ONE, curr_x_minus_one, vec_size);

    ReluPrime(curr_x_minus_one, shared_cmp);

    // selection based on x - 1 > 0
    Select1Of2(SHARED_HALF, SHARED_ONE, shared_cmp, shared_val_multiplier);
    Select1Of2(SHARED_ONE, SHARED_ZERO, shared_cmp, shared_power_add);
    vector<mpc_t> tmp_vec(vec_size, 0);
    DotProduct(curr_x, shared_val_multiplier, tmp_vec);
    curr_x = tmp_vec;
    addVectors<mpc_t>(curr_power, shared_power_add, tmp_vec, vec_size);
    curr_power = tmp_vec;
  }
  // 1.2 scale up fractional part
  for (int i = 0; i < float_precision; ++i) {
    auto curr_x_minus_half = curr_x;
    vector<mpc_t> shared_cmp(vec_size, 0);
    subtractVectors<mpc_t>(curr_x, SHARED_HALF, curr_x_minus_half, vec_size);

    ReluPrime(curr_x_minus_half, shared_cmp);
    // selection based on x - 1 > 0
    Select1Of2(SHARED_ONE, SHARED_TWO, shared_cmp, shared_val_multiplier);
    Select1Of2(SHARED_ZERO, SHARED_NEG_ONE, shared_cmp, shared_power_add);

    vector<mpc_t> tmp_vec(vec_size, 0);
    DotProduct(curr_x, shared_val_multiplier, tmp_vec);
    curr_x = tmp_vec;
    addVectors<mpc_t>(curr_power, shared_power_add, tmp_vec, vec_size);
    curr_power = tmp_vec;
  }
  AUDIT("id:{}, P{} HLog, step 1. power(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(curr_power));

  // step 2: computer basic ln(r/2) by polynomials interproplation
  vector<mpc_t> power_list;
  vector<mpc_t> coff_list;
  string my_func = "LOG_HD";
  vector<ConstPolynomial>* log_hd_p = NULL;
  if (!PolyConfFactory::get_func_polys(my_func, &log_hd_p)) {
    // TODO: throw exception
    cout << "ERROR! can not find polynomials for func " << my_func << endl;
    return -1;
  }
  log_hd_p->at(0).get_power_list(power_list);
  log_hd_p->at(0).get_coff_list(coff_list, float_precision);
  vector<mpc_t> shared_basic_val(vec_size, 0);
  vector<mpc_t> tmp_v(vec_size);
  UniPolynomial(curr_x, power_list, coff_list, tmp_v);
  for (int i = 0; i < vec_size; ++i) {
    shared_basic_val[i] = CoffDown(tmp_v[i]);
  }
  AUDIT("id:{}, P{} HLog, step 2. shared_basic_val(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(shared_basic_val));

  vector<mpc_t> shared_high_part(vec_size, 0);
  DotProduct(curr_power, SHARED_LN_2, shared_high_part);
  AUDIT("id:{}, P{} HLog, step 3. DotProduct, shared_high_part(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(shared_high_part));
  addVectors<mpc_t>(shared_high_part, shared_basic_val, shared_Y, vec_size);

  AUDIT("id:{}, P{} HLog, output Y(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(shared_Y));
  tlog_debug << "HLog ok.";
  return 0;
}

// Obsolete Log version, the origin one
int SnnInternal::LogV1(const vector<mpc_t>& shared_X, vector<mpc_t>& shared_Y) {
  tlog_debug << "LogV1 ...";
  AUDIT("id:{}, P{} LogV1, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(shared_X));
  for (size_t i = 0 ; i < shared_X.size(); ++i) {
    LogV1(shared_X[i], shared_Y[i]);
  }
  AUDIT("id:{}, P{} LogV1, output Y(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(shared_Y));
  tlog_debug << "LogV1 ok.";
  return 0;
}
int SnnInternal::LogV1(const mpc_t& shared_X, mpc_t& shared_Y) {
  AUDIT("id:{}, P{} LogV1, input X(mpc_t): {}", msg_id().get_hex(), context_->GetMyRole(), shared_X);
  shared_Y = 0;
  int float_precision = GetMpcContext()->FLOAT_PRECISION;
  vector<mpc_t> power_list;
  vector<mpc_t> coff_list;
  string my_func = "LOG_V1";
  vector<ConstPolynomial>* log_v1_p = NULL;
  if (!PolyConfFactory::get_func_polys(my_func, &log_v1_p)) {
    // TODO: throw exception
    cout << "ERROR! can not find polynomials for func " << my_func << endl;
    return -1;
  }
  log_v1_p->at(0).get_power_list(power_list);
  log_v1_p->at(0).get_coff_list(coff_list, float_precision);
  UniPolynomial(shared_X, power_list, coff_list, shared_Y);
  shared_Y = CoffDown(shared_Y);
  AUDIT("id:{}, P{} LogV1, output Y(mpc_t): {}", msg_id().get_hex(), context_->GetMyRole(), shared_Y);

  return 0;
}

int SnnInternal::Log(const vector<mpc_t>& shared_X, vector<mpc_t>& shared_Y) {
  tlog_debug << "Log v2 ...";
  AUDIT("id:{}, P{} Log, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(shared_X));
  int vec_size = shared_X.size();
  shared_Y.resize(vec_size);
  int float_precision = GetMpcContext()->FLOAT_PRECISION;

  string my_func = "LOG_V2";
  vector<ConstPolynomial>* log_v2_p = NULL;
  if (!PolyConfFactory::get_func_polys(my_func, &log_v2_p)) {
    // TODO: throw exception
    cout << "ERROR! can not find polynomials for func " << my_func << endl;
    return -1;
  }
  auto seg_size = log_v2_p->size();
  if (seg_size == 0) {
    // TODO: throw exception
    cout << "ERROR! empty polynomials in log_v2." << endl;
    return -1;
  }

  // actually we will use the [start, end)-style for each segment!
  vector<mpc_t> curr_power_list;
  vector<mpc_t> curr_coff_list;
  // some temporary variables
  vector<mpc_t> shared_cmp_init(vec_size, 0);
  vector<mpc_t> shared_cmp_end(vec_size, 0);
  vector<mpc_t> shared_init(vec_size, 0);
  vector<mpc_t> shared_end(vec_size, 0);
  vector<mpc_t> shared_res(vec_size, 0);
  for (int i = 0; i < seg_size; ++i) {
    ConstPolynomial curr_seg = log_v2_p->at(i);
    mpc_t seg_init = curr_seg.get_start(float_precision);
    mpc_t seg_end = curr_seg.get_end(float_precision);
    curr_seg.get_power_list(curr_power_list);
    curr_seg.get_coff_list(curr_coff_list, float_precision);
    // cout << "Seg " << i << ":[" << MpcTypeToFloat(seg_init, GetMpcContext()->FLOAT_PRECISION) <<
    //  			", " << MpcTypeToFloat(seg_end, GetMpcContext()->FLOAT_PRECISION) << "): ";
    // for(auto i = 0; i< curr_power_list.size(); i++) {
    //     cout << MpcTypeToFloat(curr_coff_list[i], GetMpcContext()->FLOAT_PRECISION) << "*X^" <<
    //  			to_readable_hex(curr_power_list[i], GetMpcContext()->FLOAT_PRECISION) << " + ";
    // }
    // S1: use ReLUPrime to get whether to use this segment[ multiplier is 0 or 1]
    /// 1.1 check start point
    // x >= start && (1 - (x >= end))
    shared_cmp_init = shared_X;
    shared_cmp_end = shared_X;
    for (int j = 0; j < vec_size; ++j) {
      if (partyNum == PARTY_A) {
        shared_cmp_init[j] = shared_X[j] - seg_init;
        shared_cmp_end[j] = shared_X[j] - seg_end;
      }
    }
    // packing for vectorization
    vector<mpc_t> shared_cmp = shared_cmp_init;
    shared_cmp.insert(shared_cmp.end(), shared_cmp_end.begin(), shared_cmp_end.end());
    vector<mpc_t> tmp_reluprime(2 * vec_size);
    ReluPrime(shared_cmp, tmp_reluprime);
    shared_init.assign(tmp_reluprime.begin(), tmp_reluprime.begin() + vec_size);
    shared_end.assign(tmp_reluprime.begin() + vec_size, tmp_reluprime.end());
    vector<mpc_t> CONST_ONE(vec_size, FloatToMpcType(1.0 / 2.0, GetMpcContext()->FLOAT_PRECISION));
    vector<mpc_t> shared_end2(vec_size, 0);
    if (PRIMARY) {
      subtractVectors(CONST_ONE, shared_end, shared_end2, vec_size);
    }
    DotProduct(shared_init, shared_end2, shared_res);

    vector<mpc_t> poly_res(vec_size, 0);
    // S2: compute the value in this segment
    UniPolynomial(shared_X, curr_power_list, curr_coff_list, poly_res);
    for (int i = 0; i < vec_size; ++i) {
      poly_res[i] = CoffDown(poly_res[i]);
    }
    // debug
    // if(PRIMARY) {
    //   	GetMpcOpInner(Reconstruct2PC)->Run(poly_res, poly_res.size(), "curr seg Poly value");
    // }
    vector<mpc_t> this_seg_res(vec_size, 0);
    DotProduct(shared_res, poly_res, this_seg_res);
    addVectors<mpc_t>(shared_Y, this_seg_res, shared_Y, vec_size);
  }
  AUDIT("id:{}, P{} Log, output Y(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(shared_Y));

  tlog_debug << "Log v2 ok.";
  return 0;
}

int SnnInternal::Log(const mpc_t& shared_X, mpc_t& shared_Y) {
  AUDIT("id:{}, P{} Log, input X(mpc_t): {}", msg_id().get_hex(), context_->GetMyRole(), shared_X);
  shared_Y = 0;
  string my_func = "LOG_V2";
  const int float_precision = GetMpcContext()->FLOAT_PRECISION;
  vector<ConstPolynomial>* log_v2_p = NULL;
  if (!PolyConfFactory::get_func_polys(my_func, &log_v2_p)) {
    // TODO: throw exception
    tlog_error << "ERROR! can not find polynomials for func " << my_func;
    return -1;
  }
  auto seg_size = log_v2_p->size();
  if (seg_size == 0) {
    // TODO: throw exception
    tlog_error << "ERROR! empty polynomials in log_v2." ;
    return -1;
  }
  // actually we will use the [start, end)-style for each segment!
  vector<mpc_t> curr_power_list;
  vector<mpc_t> curr_coff_list;
  mpc_t shared_seg_multiplier = 1;
  // some temporary variables
  vector<mpc_t> shared_cmp(2, 0);
  vector<mpc_t> shared_init(1, 0);
  vector<mpc_t> shared_end(1, 0);
  vector<mpc_t> shared_res(1, 0);
  for (int i = 0; i < seg_size; ++i) {
    ConstPolynomial curr_seg = log_v2_p->at(i);
    mpc_t seg_init = curr_seg.get_start(float_precision);
    mpc_t seg_end = curr_seg.get_end(float_precision);
    curr_seg.get_power_list(curr_power_list);
    curr_seg.get_coff_list(curr_coff_list, float_precision);
    /// S1: use ReLUPrime to get whether to use this segment[ multiplier is 0 or 1]
    /// 1.1 check start point
    // x >= start && (1 - (x >= end))
    shared_cmp[0] = shared_X;
    shared_cmp[1] = shared_X;
    if (partyNum == PARTY_A) {
      shared_cmp[0] = shared_X - seg_init;
      shared_cmp[1] = shared_X - seg_end;
    }
    vector<mpc_t> tmp_reluprime(2, 0);
    ReluPrime(shared_cmp, tmp_reluprime);
    if (partyNum == PARTY_A) {
      tmp_reluprime[1] = FloatToMpcType(1, float_precision) - tmp_reluprime[1];
    } else {
      tmp_reluprime[1] = -tmp_reluprime[1];
    }
    shared_init[0] = tmp_reluprime[0];
    shared_end[0] = tmp_reluprime[1];
    DotProduct(shared_init, shared_end, shared_res);
    // S2: compute the value in this segment
    mpc_t curr_shared_Y = 0;
    UniPolynomial(shared_X, curr_power_list, curr_coff_list, curr_shared_Y);
    shared_init[0] = CoffDown(curr_shared_Y);
    shared_end[0] = shared_res[0];
    DotProduct(shared_init, shared_end, shared_res);
    shared_Y += shared_res[0];
  }
  AUDIT("id:{}, P{} Log, output Y(mpc_t): {}", msg_id().get_hex(), context_->GetMyRole(), shared_Y);

  return 0;
}

int SnnInternal::Log1p(const mpc_t& a, mpc_t& b) {
  AUDIT("id:{}, P{} Log1p, input X(mpc_t): {}", msg_id().get_hex(), context_->GetMyRole(), a);
  mpc_t real_a = a;

  if (PRIMARY) {
    mpc_t local_one = FloatToMpcType(1.0, GetMpcContext()->FLOAT_PRECISION);
    if (partyNum == PARTY_A) {
      real_a = a + local_one;
      AUDIT("id:{}, P{} Log1p, compute X+1, a_plus_one(=X+1)(mpc_t): {}", msg_id().get_hex(), context_->GetMyRole(), real_a);
    }
  }
  Log(real_a, b);
  AUDIT("id:{}, P{} Log1p, output(mpc_t): {}", msg_id().get_hex(), context_->GetMyRole(), b);
  return 0;
}
int SnnInternal::Log1p(const vector<mpc_t>& a, vector<mpc_t>& b) {
  tlog_debug << "Log1p ...";
  AUDIT("id:{}, P{} Log1p, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(a));
  size_t size = a.size();
  vector<mpc_t> va(a.size(), 0);
  vector<mpc_t> a_plus_one(a.size(), 0);
  if (PRIMARY) {
    mpc_t local_one = FloatToMpcType(1.0, GetMpcContext()->FLOAT_PRECISION);
    if (partyNum == PARTY_A) {
      vector<mpc_t> v_one(a.size(), local_one);
      va.swap(v_one);
    }
    addVectors(a, va, a_plus_one, size);
    AUDIT("id:{}, P{} Log1p, compute X+1, a_plus_one(=X+1)(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(a_plus_one));
  }
  Log(a_plus_one, b);
  AUDIT("id:{}, P{} Log1p, output(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(b));

  tlog_debug << "Log1p ...";
  return 0;
}

} // namespace snn
} // namespace rosetta
