#include "cc/modules/protocol/mpc/snn/include/snn_internal.h"
#include "cc/modules/common/include/utils/str_type_convert.h"


namespace rosetta {
namespace snn {

int SnnInternal::Relu(const vector<mpc_t>& a, vector<mpc_t>& b) {
  assert(THREE_PC && "RELU called in non-3PC mode");
  tlog_debug << "Relu  ...";
  AUDIT("id:{}, P{} Relu, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(a));

  size_t size = a.size();
  vector<mpc_t> reluPrime(size);
  
  int ret = ReluPrime(a, reluPrime);
  if (ret != 0) {
    tlog_error << "internal relu_prime failed";
    return -1;
  }

  ret = SelectShares(a, reluPrime, b);
  if (ret != 0) {
    tlog_error << "internal select_share failed";
    return -1;
  }
  
  AUDIT("id:{}, P{} Relu, output Y(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(b));
  tlog_debug << "Relu ok.";
  return 0;
}

int SnnInternal::Relu(const vector<string>& as, vector<string>& bs) {
  vector<mpc_t> a, b;
  // rosetta::convert::from_binary_str(as, a);
  convert_string_to_mpctype(as, a);
  AUDIT("id:{}, P{} Relu, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(a));

  int ret = Relu(a, b);

  if (ret != 0) {
    tlog_error << "internal relu failed";
    return -1; 
  }
  AUDIT("id:{}, P{} Relu, output Y(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(b));
  convert_mpctype_to_string(b, bs);

  return 0;
}

int SnnInternal::ReluPrime(const vector<mpc_t>& a, vector<mpc_t>& b) {
  assert(THREE_PC && "RELUPrime called in non-3PC mode");
  tlog_debug << "ReluPrime ...";
  AUDIT("id:{}, P{} ReluPrime, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(a));

  size_t size = a.size();
  vector<mpc_t> twoA(size, 0);
  mpc_t j = 0;

  // x*2
  for (size_t i = 0; i < size; ++i)
    twoA[i] = (a[i] << 1);

  // get msb in Z_{L-1}
  ShareConvert(twoA);
  ComputeMSB(twoA, b);

  // share of one
  if (partyNum == PARTY_A)
    j = FloatToMpcType(1, GetMpcContext()->FLOAT_PRECISION);

  // convert msb to relu_prime
  if (PRIMARY) {
    for (size_t i = 0; i < size; ++i)
      b[i] = j - b[i];
  }

  AUDIT("id:{}, P{} ReluPrime, output Y(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(b));
  tlog_debug << "ReluPrime ok.";
  return 0;
}

int SnnInternal::SigmoidCrossEntropy(
  const vector<mpc_t>& shared_logits,
  const vector<mpc_t>& shared_labels,
  vector<mpc_t>& shared_result,
  size_t vec_size) {
  tlog_debug << "SigmoidCrossEntropy ...";
  AUDIT("id:{}, P{} SigmoidCrossEntropy, input shared_logits(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(shared_logits));
  AUDIT("id:{}, P{} SigmoidCrossEntropy, input shared_labels(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(shared_labels));
  vector<mpc_t> tmp_part(vec_size, 0);
  // max(logit, 0)
  Relu(shared_logits, tmp_part);
  vector<mpc_t> prod_part(vec_size, 0);
  // logit * label
  DotProduct(shared_logits, shared_labels, prod_part);
  AUDIT("id:{}, P{} SigmoidCrossEntropy compute: shared_logits*shared_labels, prod_part(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(prod_part));
  // max(logit, 0) - logit * label
  subtractVectors<mpc_t>(tmp_part, prod_part, shared_result, vec_size);
  AUDIT("id:{}, P{} SigmoidCrossEntropy compute: max(shared_logits, 0)-shared_logits*shared_labels, shared_result(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(shared_result));
  // if(PRIMARY) {
  //     GetMpcOpInner(Reconstruct2PC)->Run(shared_result, shared_result.size(), "curr_X");
  // }
  vector<mpc_t> log_part(vec_size, 0);
  // |X|
  Abs(shared_logits, tmp_part);
  AUDIT("id:{}, P{} SigmoidCrossEntropy compute: abs(shared_logits), abs_shared_logits(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(tmp_part));
  // log(1 + exp(-abs(x)))
  CELog(tmp_part, log_part, vec_size);
  AUDIT("id:{}, P{} SigmoidCrossEntropy compute: log(1+exp(-abs(shared_logits))), log_part(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(log_part));
  // if(PRIMARY) {
  //     GetMpcOpInner(Reconstruct2PC)->Run(log_part, log_part.size(), "log part curr_X");
  // }
  addVectors<mpc_t>(shared_result, log_part, shared_result, vec_size);

  AUDIT("id:{}, P{} SigmoidCrossEntropy, output Y{}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(shared_result));
  tlog_debug << "SigmoidCrossEntropy ok.";
  return 0;
}

int SnnInternal::SigmoidCrossEntropyBatch(
  const vector<mpc_t>& shared_logits,
  const vector<mpc_t>& shared_labels,
  vector<mpc_t>& shared_result,
  size_t vec_size) {
  // cout << "sigmoid_cross_entropy_batch !" << endl;
  ///********************1. prepare all data for batch-ReluPrime.
  tlog_debug << "SigmoidCrossEntropyBatch ...";
  AUDIT("id:{}, P{} SigmoidCrossEntropyBatch, input shared_logits(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(shared_logits));
  AUDIT("id:{}, P{} SigmoidCrossEntropyBatch, input shared_labels(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(shared_labels));
  vector<mpc_t> batch_comp_X;
  batch_comp_X.insert(batch_comp_X.end(), shared_logits.begin(), shared_logits.end());
  int float_precision = GetMpcContext()->FLOAT_PRECISION;

  string my_func = "LOG_CE";
  // we know that this functionality need single segment.
  vector<mpc_t> power_list;
  vector<mpc_t> coff_list;
  vector<ConstPolynomial>* log_ce_p = NULL;
  if (!PolyConfFactory::get_func_polys(my_func, &log_ce_p)) {
    // TODO: throw exception
    cout << "ERROR! can not find polynomials for func " << my_func << endl;
    return -1;
  }
  auto seg_size = log_ce_p->size();
  log_ce_p->at(0).get_power_list(power_list);
  log_ce_p->at(0).get_coff_list(coff_list, float_precision);
  auto end_v = log_ce_p->at(0).get_end(float_precision);
  vector<mpc_t> UPPER_V(vec_size, 0);
  if (partyNum == PARTY_A) {
    vector<mpc_t> tmp_vec(vec_size, end_v);
    UPPER_V.swap(tmp_vec);
  }

  vector<mpc_t> upper_bound_minus_logits(vec_size);
  vector<mpc_t> logits_plus_upper_bound(vec_size);

  subtractVectors<mpc_t>(UPPER_V, shared_logits, upper_bound_minus_logits, vec_size);
  addVectors<mpc_t>(shared_logits, UPPER_V, logits_plus_upper_bound, vec_size);

  batch_comp_X.insert(
    batch_comp_X.end(), upper_bound_minus_logits.begin(), upper_bound_minus_logits.end());

  batch_comp_X.insert(
    batch_comp_X.end(), logits_plus_upper_bound.begin(), logits_plus_upper_bound.end());

  ///********************2.  call the costly ReluPrime operation in one shot.
  vector<mpc_t> batch_cmp_res(batch_comp_X.size());
  ReluPrime(batch_comp_X, batch_cmp_res);
  batch_comp_X.clear();

  ///********************3.  unpack  results and get related results.
  vector<mpc_t> max_part_bit;
  max_part_bit.insert(max_part_bit.end(), batch_cmp_res.begin(), batch_cmp_res.begin() + vec_size);
  vector<mpc_t> upper_bound_minus_logits_bit;
  upper_bound_minus_logits_bit.insert(
    upper_bound_minus_logits_bit.end(), batch_cmp_res.begin() + vec_size,
    batch_cmp_res.begin() + 2 * vec_size);
  vector<mpc_t> logits_plus_upper_bound_bit;
  logits_plus_upper_bound_bit.insert(
    logits_plus_upper_bound_bit.end(), batch_cmp_res.begin() + 2 * vec_size, batch_cmp_res.end());
  batch_cmp_res.clear();

  // 3.1 get Logits Sign multiplier.
  vector<double> SHARED_TWO(vec_size, 2);
  vector<mpc_t> SHARED_NEG_ONE(vec_size, 0);
  if (partyNum == PARTY_A) {
    SHARED_NEG_ONE = vector<mpc_t>(vec_size, FloatToMpcType(-1, GetMpcContext()->FLOAT_PRECISION));
  }
  vector<mpc_t> shared_x_multiplier(vec_size, 0);
  vector<mpc_t> temp_mul_res(vec_size);
  // this is local
  DotProduct(SHARED_TWO, max_part_bit, temp_mul_res);
  addVectors<mpc_t>(SHARED_NEG_ONE, temp_mul_res, shared_x_multiplier, vec_size);
  temp_mul_res.clear();
  SHARED_TWO.clear();
  SHARED_NEG_ONE.clear();

  ///******************** 4. pack data for batch DotProduct[Mul]
  vector<mpc_t> batch_mul_op_left;
  vector<mpc_t> batch_mul_op_right;

  batch_mul_op_left.insert(batch_mul_op_left.end(), max_part_bit.begin(), max_part_bit.end());
  batch_mul_op_right.insert(batch_mul_op_right.end(), shared_logits.begin(), shared_logits.end());

  batch_mul_op_left.insert(
    batch_mul_op_left.end(), shared_x_multiplier.begin(), shared_x_multiplier.end());
  batch_mul_op_right.insert(batch_mul_op_right.end(), shared_logits.begin(), shared_logits.end());

  batch_mul_op_left.insert(batch_mul_op_left.end(), shared_labels.begin(), shared_labels.end());
  batch_mul_op_right.insert(batch_mul_op_right.end(), shared_logits.begin(), shared_logits.end());

  batch_mul_op_left.insert(
    batch_mul_op_left.end(), upper_bound_minus_logits_bit.begin(),
    upper_bound_minus_logits_bit.end());
  batch_mul_op_right.insert(
    batch_mul_op_right.end(), logits_plus_upper_bound_bit.begin(),
    logits_plus_upper_bound_bit.end());

  vector<mpc_t> batch_mul_op_res(batch_mul_op_right.size());
  DotProduct(batch_mul_op_left, batch_mul_op_right, batch_mul_op_res);
  batch_mul_op_left.clear();
  batch_mul_op_right.clear();

  ///******************** 5.  unpack MUL result.
  // max(logit, 0)
  vector<mpc_t> max_part;
  max_part.insert(max_part.end(), batch_mul_op_res.begin(), batch_mul_op_res.begin() + vec_size);
  // 5.1 abs(logit)
  vector<mpc_t> _abs;
  _abs.insert(
    _abs.end(), batch_mul_op_res.begin() + vec_size, batch_mul_op_res.begin() + 2 * vec_size);
  // logit * label
  vector<mpc_t> prod_part;
  prod_part.insert(
    prod_part.end(), batch_mul_op_res.begin() + 2 * vec_size,
    batch_mul_op_res.begin() + 3 * vec_size);
  // clipped_sign
  vector<mpc_t> no_need_clip_bit;
  no_need_clip_bit.insert(
    no_need_clip_bit.end(), batch_mul_op_res.begin() + 3 * vec_size, batch_mul_op_res.end());
  batch_mul_op_res.clear();

  ///******************** 6. get log(1 + exp(-X) part with polynomial.
  vector<mpc_t> log_part(vec_size);
  vector<mpc_t> LOWER_CLIP_CONST(vec_size);
  if (partyNum == PARTY_A) {
    LOWER_CLIP_CONST =
      vector<mpc_t>(vec_size, FloatToMpcType(0.0003, GetMpcContext()->FLOAT_PRECISION));
  }
  UniPolynomial(_abs, power_list, coff_list, log_part);
  Select1Of2(log_part, LOWER_CLIP_CONST, no_need_clip_bit, log_part);
  // 7. collect all parts
  // max(logit, 0) - logit * label
  subtractVectors<mpc_t>(max_part, prod_part, shared_result, vec_size);
  addVectors<mpc_t>(shared_result, log_part, shared_result, vec_size);

  AUDIT("id:{}, P{} SigmoidCrossEntropy, output(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(shared_result));
  tlog_debug << "SigmoidCrossEntropyBatch ok.";
  return 0;
}

/* 
    |x| means:
             1 * x if x >=  0
          (-1) * x if x < 0
*/
int SnnInternal::Abs(const vector<mpc_t>& shared_X, vector<mpc_t>& shared_Y) {
  tlog_debug << "Abs ...";
  AUDIT("id:{}, P{} Abs, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(shared_X));
  size_t vec_size = shared_X.size();
  vector<mpc_t> SHARED_ONE(vec_size, 0);
  if (partyNum == PARTY_A) {
    SHARED_ONE = vector<mpc_t>(vec_size, FloatToMpcType(1, GetMpcContext()->FLOAT_PRECISION));
  }
  vector<mpc_t> SHARED_NEG_ONE(vec_size, 0);
  if (partyNum == PARTY_A) {
    SHARED_NEG_ONE = vector<mpc_t>(vec_size, FloatToMpcType(-1, GetMpcContext()->FLOAT_PRECISION));
  }
  vector<mpc_t> shared_x_multiplier(vec_size, 0);
  vector<mpc_t> shared_cmp(vec_size, 0);
  ReluPrime(shared_X, shared_cmp);
  Select1Of2(SHARED_ONE, SHARED_NEG_ONE, shared_cmp, shared_x_multiplier);
  DotProduct(shared_x_multiplier, shared_X, shared_Y);

  AUDIT("id:{}, P{} Abs, output Y(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(shared_Y));
  tlog_debug << "Abs ok.";
  return 0;
}

/* 
  |x|' means:
            1  if x >=  0 [ in point 0 we set it to 1]
        (-1)  if x < 0
*/
int SnnInternal::AbsPrime(
  const vector<mpc_t>& shared_X,
  vector<mpc_t>& shared_Y) {
  tlog_debug << "AbsPrime ...";
  AUDIT("id:{}, P{} AbsPrime, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(shared_X));
  size_t vec_size = shared_X.size();
  vector<mpc_t> SHARED_ONE(vec_size, 0);
  if (partyNum == PARTY_A) {
    SHARED_ONE = vector<mpc_t>(vec_size, FloatToMpcType(1, GetMpcContext()->FLOAT_PRECISION));
  }
  vector<mpc_t> SHARED_NEG_ONE(vec_size, 0);
  if (partyNum == PARTY_A) {
    SHARED_NEG_ONE = vector<mpc_t>(vec_size, FloatToMpcType(-1, GetMpcContext()->FLOAT_PRECISION));
  }
  vector<mpc_t> shared_cmp(vec_size, 0);
  ReluPrime(shared_X, shared_cmp);
  Select1Of2(SHARED_ONE, SHARED_NEG_ONE, shared_cmp, shared_Y);

  AUDIT("id:{}, P{} AbsPrime, output Y(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(shared_Y));
  tlog_debug << "AbsPrime ok.";
  return 0;
}

void SnnInternal::CELog(
  const vector<mpc_t>& shared_X,
  vector<mpc_t>& shared_Y,
  size_t vec_size) {
  tlog_debug << "CELog ...";
  string my_func = "LOG_CE";
  int float_precision = GetMpcContext()->FLOAT_PRECISION;
  // we know that this functionality need single segment.
  vector<mpc_t> power_list;
  vector<mpc_t> coff_list;
  vector<ConstPolynomial>* log_ce_p = NULL;
  if (!PolyConfFactory::get_func_polys(my_func, &log_ce_p)) {
    // TODO: throw exception
    cout << "ERROR! can not find polynomials for func " << my_func << endl;
    return;
  }
  auto seg_size = log_ce_p->size();
  if (seg_size == 0) {
    // TODO: throw exception
    cout << "ERROR! empty polynomials in log_v2." << endl;
    return;
  }
  log_ce_p->at(0).get_power_list(power_list);
  log_ce_p->at(0).get_coff_list(coff_list, float_precision);
  auto end_v = log_ce_p->at(0).get_end(float_precision);

  // for (auto i = 0; i< power_list.size(); i++) {
  // 	cout << MpcTypeToFloat(coff_list[i], GetMpcContext()->FLOAT_PRECISION) << "*X^" << power_list[i] << " + ";
  // }

  if (THREE_PC) {
    vector<mpc_t> tmp_END(vec_size, 0);
    auto shared_cmp = shared_X;
    if (partyNum == PARTY_A) {
      vector<mpc_t> tmp_vec(vec_size, end_v);
      tmp_END.swap(tmp_vec);
      subtractVectors<mpc_t>(shared_X, tmp_END, shared_cmp, vec_size);
    }

    vector<mpc_t> cmp_res(vec_size, 0);
    ReluPrime(shared_cmp, cmp_res);
    vector<mpc_t> curr_X(vec_size, 0);
    Select1Of2(tmp_END, shared_X, cmp_res, curr_X);

    vector<mpc_t> LOWER_BOUND(vec_size, FloatToMpcType(0.00015, GetMpcContext()->FLOAT_PRECISION));
    vector<mpc_t> curr_res(vec_size, 0);
    vector<mpc_t> tmp_v(vec_size);
    tlog_debug << "DEBUG CE LOG, calling vectorization mpc_uni_polynomial" ;
    UniPolynomial(curr_X, power_list, coff_list, tmp_v);
    for (int i = 0; i < vec_size; ++i) {
      curr_res[i] = CoffDown(tmp_v[i]);
    }

    Select1Of2(LOWER_BOUND, curr_res, cmp_res, shared_Y);
  } else {
    notYet();
  }

  tlog_debug << "CELog ok.";
}

/**
* @brief approximate sigmoid implemented with 6-pieces function
* @param a input of mpc_t type in range (-inf,+inf)
* @return sigmoid of mpc_t input
* @note sigmoid(y) equals:
0, for y in range (-inf, -4];
0.0484792 * y + 0.1998976, for y in range (-4, -2]; 
0.1928931 * y + 0.4761351, for y in range (-2, 0];
0.1928931 * y + 0.5238649, for y in range (0, 2];
0.0484792 * y + 0.8001024, for y in range (2,4];
1, for y in range (4, +inf) 
*/
int SnnInternal::Sigmoid6PieceWise(const vector<mpc_t>& a, vector<mpc_t>& b) {
  size_t size = a.size();
  DEB("Sigmoid6PieceWise start");
  tlog_debug << "Sigmoid6PieceWise ...";
  const int float_precision = GetMpcContext()->FLOAT_PRECISION;
  if (THREE_PC) {
    int SEG = 5;
    // vector<mpc_t> y = a;
    //params(a,b): (0.0484792, 0.1998976),  (0.1928931, 0.4761351), (0.1928931, 0.5238649),  
    //(0.0484792,0.8001024)
    mpc_t a1 = FloatToMpcType(0.0484792, float_precision);
    mpc_t b1 = FloatToMpcType(0.1998976, float_precision);
    mpc_t a2 = FloatToMpcType(0.1928931, float_precision);
    mpc_t b2 = FloatToMpcType(0.4761351, float_precision);
    mpc_t a3 = FloatToMpcType(0.1928931, float_precision);
    mpc_t b3 = FloatToMpcType(0.5238649, float_precision);
    mpc_t a4 = FloatToMpcType(0.0484792, float_precision);
    mpc_t b4 = FloatToMpcType(0.8001024, float_precision);

    // vectorization-style to call the costly funcPrivateCompareMPCEx2(ReluPrime) only once.
    //[-4,4]: -4, -2, 0, 2, 4
    vector<mpc_t> batch_cmp_C;
    batch_cmp_C.insert(batch_cmp_C.end(), size, FloatToMpcType(-4, float_precision)/2);
    batch_cmp_C.insert(batch_cmp_C.end(), size, FloatToMpcType(-2, float_precision)/2);
    batch_cmp_C.insert(batch_cmp_C.end(), size, FloatToMpcType(0, float_precision)/2);
    batch_cmp_C.insert(batch_cmp_C.end(), size, FloatToMpcType(2, float_precision)/2);
    batch_cmp_C.insert(batch_cmp_C.end(), size, FloatToMpcType(4, float_precision)/2);
    
    const vector<mpc_t>& X = a;
    vector<mpc_t> batch_cmp_X;
    batch_cmp_X.insert(batch_cmp_X.end(), X.begin(), X.end());
    batch_cmp_X.insert(batch_cmp_X.end(), X.begin(), X.end());
    batch_cmp_X.insert(batch_cmp_X.end(), X.begin(), X.end());
    batch_cmp_X.insert(batch_cmp_X.end(), X.begin(), X.end());
    batch_cmp_X.insert(batch_cmp_X.end(), X.begin(), X.end());

    vector<mpc_t> batch_cmp_res(batch_cmp_X.size());
    GreaterEqual(batch_cmp_C, batch_cmp_X, batch_cmp_res);
    batch_cmp_X.clear();
    batch_cmp_C.clear();

    // vectorization for calling communication-costly DotProduct only once 
    vector<mpc_t> batch_dot_product;
    vector<mpc_t> linear_temp(size);

    if (PRIMARY)
      LinearMPC(X, 0 - a1, 0 - b1, linear_temp);
    batch_dot_product.insert(batch_dot_product.end(), linear_temp.begin(), linear_temp.end());

    if (PRIMARY)
      LinearMPC(X, a1 - a2, b1 - b2, linear_temp);
    batch_dot_product.insert(batch_dot_product.end(), linear_temp.begin(), linear_temp.end());

    if (PRIMARY)
      LinearMPC(X, a2 - a3, b2 - b3, linear_temp);
    batch_dot_product.insert(batch_dot_product.end(), linear_temp.begin(), linear_temp.end());

    if (PRIMARY)
      LinearMPC(X, a3 - a4, b3 - b4, linear_temp);
    batch_dot_product.insert(batch_dot_product.end(), linear_temp.begin(), linear_temp.end());

    if (PRIMARY)
      LinearMPC(X, a4 /*-0*/, b4 - FloatToMpcType(double(1.0), float_precision), linear_temp);
    batch_dot_product.insert(batch_dot_product.end(), linear_temp.begin(), linear_temp.end());
    
    vector<mpc_t> batch_dp_res(batch_dot_product.size());
    DotProduct(batch_cmp_res, batch_dot_product, batch_dp_res);
    batch_cmp_res.clear();
    batch_dot_product.clear();

    // unpack the vectorization result and sum up.
    auto& out = b;
    mpc_t lastOne = FloatToMpcType(1.0, float_precision) / 2; // add last 1
    out.resize(size);
    for (size_t pos = 0; pos < size; ++pos)
      out[pos] = lastOne;
    
    for(int i = 0; i < SEG; ++i) {
      auto iter_begin = batch_dp_res.begin() + i * size;
      for (size_t pos = 0; pos < size; ++pos)
        out[pos] = out[pos] + *(iter_begin+pos);
    }
  }

  DEB("Sigmoid5PieceWise start");
  tlog_debug << "Sigmoid6PieceWise ok.";
  return 0;
}

/**
** @brief Sigmoid approximate function 5-pieces-functions 
** @note sigmoid(y) =
  10^−4, y <= −5
  0.02776 * y + 0.145, where −5 < y <= −2.5
  0.17 * y + 0.5, where −2.5 < y <= 2.5
  0.02776 * y + 0.85498, where 2.5 < y <= 5
  1 − 10^−4, y > 5
*/
int SnnInternal::Sigmoid5PieceWise(const vector<mpc_t>& a, vector<mpc_t>& b) {
  assert(THREE_PC && "non-3pc setting!!!");
  tlog_debug << "Sigmoid5PieceWise ...";

  const int float_precision = GetMpcContext()->FLOAT_PRECISION;
  size_t size = a.size();

  if (THREE_PC) {
    int SEG = 4;
    mpc_t a1 = FloatToMpcType(0.02776, float_precision);
    mpc_t b1 = FloatToMpcType(0.145, float_precision);
    mpc_t a2 = FloatToMpcType(0.17, float_precision);
    mpc_t b2 = FloatToMpcType(0.5, float_precision);
    mpc_t a3 = FloatToMpcType(0.02776, float_precision);
    mpc_t b3 = FloatToMpcType(0.85498, float_precision);

    //[-5,5]: -5, -2.5, 2.5, 5
    vector<mpc_t> batch_cmp_C;
    batch_cmp_C.insert(batch_cmp_C.end(), size, FloatToMpcType(-5, float_precision)/2);
    batch_cmp_C.insert(batch_cmp_C.end(), size, FloatToMpcType(-2.5, float_precision)/2);
    batch_cmp_C.insert(batch_cmp_C.end(), size, FloatToMpcType(2.5, float_precision)/2);
    batch_cmp_C.insert(batch_cmp_C.end(), size, FloatToMpcType(5, float_precision)/2);
    
    const vector<mpc_t>& X = a;
    vector<mpc_t> batch_cmp_X;
    batch_cmp_X.insert(batch_cmp_X.end(), X.begin(), X.end());
    batch_cmp_X.insert(batch_cmp_X.end(), X.begin(), X.end());
    batch_cmp_X.insert(batch_cmp_X.end(), X.begin(), X.end());
    batch_cmp_X.insert(batch_cmp_X.end(), X.begin(), X.end());

    vector<mpc_t> batch_cmp_res(batch_cmp_X.size());
    GreaterEqual(batch_cmp_C, batch_cmp_X, batch_cmp_res);
    batch_cmp_X.clear();
    batch_cmp_C.clear();

    // vectorization for calling communication-costly DotProduct only once 
    vector<mpc_t> batch_dot_product;
    vector<mpc_t> linear_temp(size);

    if (PRIMARY)
      LinearMPC(X, 0 - a1, 0 - b1, linear_temp);
    batch_dot_product.insert(batch_dot_product.end(), linear_temp.begin(), linear_temp.end());

    if (PRIMARY)
      LinearMPC(X, a1 - a2, b1 - b2, linear_temp);
    batch_dot_product.insert(batch_dot_product.end(), linear_temp.begin(), linear_temp.end());

    if (PRIMARY)
      LinearMPC(X, a2 - a3, b2 - b3, linear_temp);
    batch_dot_product.insert(batch_dot_product.end(), linear_temp.begin(), linear_temp.end());

    if (PRIMARY)
      LinearMPC(X, a3 /*-0*/, b3 - FloatToMpcType(1.0, float_precision), linear_temp);
    batch_dot_product.insert(batch_dot_product.end(), linear_temp.begin(), linear_temp.end());
    
    vector<mpc_t> batch_dp_res(batch_dot_product.size());
    DotProduct(batch_cmp_res, batch_dot_product, batch_dp_res);
    batch_cmp_res.clear();
    batch_dot_product.clear();

    // unpack the vectorization result and sum up.
    auto& out = b;
    mpc_t lastOne = FloatToMpcType(1.0, float_precision) / 2; // add last 1
    out.resize(size);
    for (size_t pos = 0; pos < size; ++pos)
      out[pos] = lastOne;
    
    for(int i = 0; i < SEG; ++i) {
      auto iter_begin = batch_dp_res.begin() + i * size;
      for (size_t pos = 0; pos < size; ++pos)
        out[pos] = out[pos] + *(iter_begin+pos);
    }
  }
  
  DEB("Sigmoid5PieceWise end");
  tlog_debug << "Sigmoid5PieceWise right ok.";
  return 0;
}

/**
* @brief sigmoid approximate function Chebyshev polynomial
* @detail:
* sigmoid(x) = 0.5 + 0.2159198015 * x -0.0082176259 * x^3 + 0.0001825597 * x^5 - 0.0000018848 * x^7 + 0.0000000072 * x^9,  x in [-8,8] is preferable
*/
int SnnInternal::SigmoidChebyshevPolyMPC(const vector<mpc_t>& a, vector<mpc_t>& b) {
  assert(THREE_PC && "non-3pc setting!!!");
  tlog_debug << "SigmoidChebyshevPolyMPC ...";
  AUDIT("id:{}, P{} SigmoidChebyshevPolyMPC, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(a));
  const int float_precision = GetMpcContext()->FLOAT_PRECISION;
  size_t size = a.size();

  // y = 0.5 + 0.2159198015 * x -0.0082176259 * x^3 + 0.0001825597 * x^5 - 0.0000018848 * x^7 + 0.0000000072 * x^9
  mpc_t b0 = FloatToMpcType(0.5 / 2, float_precision);
  mpc_t a1 = FloatToMpcType(0.2159198015, float_precision);
  mpc_t a3 = FloatToMpcType(-0.0082176259, float_precision);
  mpc_t a5 = FloatToMpcType(0.0001825597, float_precision);
  mpc_t a7 = FloatToMpcType(-0.0000018848, float_precision);
  mpc_t a9 = FloatToMpcType(0.0000000072, float_precision);

  vector<mpc_t> x2(size), x3(size), x5(size), x7(size), x9(size);
  auto& x1 = a;
  // x2 = x^2
  Square(x1, x2);
  AUDIT("id:{}, P{} SigmoidChebyshevPolyMPC, compute x^2, X2(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(x2));

  // x3 = x^3
  DotProduct(x1, x2, x3);
  AUDIT("id:{}, P{} SigmoidChebyshevPolyMPC, compute x^3, X3(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(x3));

  // x5 = x^5, x5 = x2 * x3
  DotProduct(x2, x3, x5);
  AUDIT("id:{}, P{} SigmoidChebyshevPolyMPC, compute x^5, X5(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(x5));

  // x7 = x^7,  x7 = x2 * x5
  DotProduct(x2, x5, x7);
  AUDIT("id:{}, P{} SigmoidChebyshevPolyMPC, compute x^7, X7(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(x7));

  // x9 = x2 * x7
  DotProduct(x2, x7, x9);
  AUDIT("id:{}, P{} SigmoidChebyshevPolyMPC, compute x^9, X9(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(x9));

  // z = y9 + y7 + y5 + y3 + y1 + w0
  mpc_t c = 0;
  if (PRIMARY) {
    for (auto i = 0; i < size; ++i) {
      c = (x1[i] * a1) + (x3[i] * a3) + (x5[i] * a5) + (x7[i] * a7) + (x9[i] * a9);
      Truncate(c, float_precision, PARTY_A, PARTY_B, partyNum);
      b[i] = b0 + c;
    }
  } //if(PRIMARY)

  AUDIT("id:{}, P{} SigmoidChebyshevPolyMPC, output Y(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(b));
  tlog_debug << "SigmoidChebyshevPolyMPC ok.";
  return 0;
}

}//snn
}//rosetta
