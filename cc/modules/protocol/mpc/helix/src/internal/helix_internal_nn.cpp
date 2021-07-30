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

#include "cc/modules/protocol/mpc/helix/include/helix_internal.h"
#include "cc/modules/protocol/mpc/comm/include/mpc_common.h"

#include <iostream>
#include <vector>
#include <cassert>
#include <string>
#include <unordered_map>

using namespace std;
using namespace rosetta;

namespace rosetta {
namespace helix {
// do some inner Ops in batch-style to reduce communication cost [but bad for code readability compared to 'SigmoidCrossEntropy'] 
void HelixInternal::SigmoidCrossEntropy_batch(const vector<Share>& logits, const vector<Share>& labels, vector<Share>& Z) {
  // tlog_debug << "DEBUG HelixOpsImpl::SigmoidCrossEntropy_batch" ;
  AUDIT("id:{}, P{} SigmoidCrossEntropy_batch compute: Z=max(logit,0)-logit*label+log(1+exp(-abs(logits)), input logit(Share){}", msgid.get_hex(), player, Vector<Share>(logits));
  AUDIT("id:{}, P{} SigmoidCrossEntropy_batch compute: Z=max(logit,0)-logit*label+log(1+exp(-abs(logits)), input labels(Share){}", msgid.get_hex(), player, Vector<Share>(labels));
  int vec_size = logits.size();
  ///********************1. prepare all data for batch-MSB. 
  vector<Share> batch_comp_X;
  batch_comp_X.insert(batch_comp_X.end(), logits.begin(), logits.end());
  int float_precision = GetMpcContext()->FLOAT_PRECISION;

  vector<Share> log_part(vec_size);
  string my_func = "LOG_CE";
  vector<mpc_t> power_list;
	vector<mpc_t> coff_list;
  vector<ConstPolynomial>* log_ce_p = NULL;
  if (!PolyConfFactory::get_func_polys(my_func, &log_ce_p)) {
    // TODO: throw exception
    tlog_error << "ERROR! can not find polynomials for func " << my_func;
    //ERROR("ERROR! can not find polynomials for func {}", my_func);
    return;
  }
	// we know that this is a one-segment polynomial
  log_ce_p->at(0).get_power_list(power_list);
  log_ce_p->at(0).get_coff_list(coff_list, float_precision);	
  mpc_t end_v = log_ce_p->at(0).get_end(float_precision);
  vector<mpc_t> upper_bound_v(vec_size, end_v);
  vector<Share> UPPER_V(vec_size);
  ConstCommonInput(upper_bound_v, UPPER_V);

  vector<Share> upper_bound_minus_logits(vec_size);
  vector<Share> logits_plus_upper_bound(vec_size);

  Sub(UPPER_V, logits, upper_bound_minus_logits);
  Add(logits, UPPER_V, logits_plus_upper_bound);

  batch_comp_X.insert(batch_comp_X.end(),
                        upper_bound_minus_logits.begin(),
                        upper_bound_minus_logits.end());

  batch_comp_X.insert(batch_comp_X.end(),
                        logits_plus_upper_bound.begin(),
                        logits_plus_upper_bound.end());

  vector<BitShare> batch_comp_res(batch_comp_X.size());

  ///********************2.  call the costly MSB operation in one shot.
  MSB(batch_comp_X, batch_comp_res);
  batch_comp_X.clear();
    // 1 ^ bitX
  for (int i = 0; i < batch_comp_res.size(); i++) {
    batch_comp_res[i].s0.delta = 1 ^ batch_comp_res[i].s0.delta;
    batch_comp_res[i].s1.A0 = 1 ^ batch_comp_res[i].s1.A0;
  }
  ///********************3.  unpack MSB results and get related results.
  vector<Share> max_part(vec_size);
  vector<BitShare> logits_sign_bit;
  logits_sign_bit.insert(logits_sign_bit.end(),
                          batch_comp_res.begin(),
                          batch_comp_res.begin() + vec_size);
  vector<BitShare>  upper_bound_minus_logits_bit;
  upper_bound_minus_logits_bit.insert(upper_bound_minus_logits_bit.end(), 
                                      batch_comp_res.begin() + vec_size,
                                      batch_comp_res.begin() + 2* vec_size);
  vector<BitShare>  logits_plus_upper_bound_bit;
  logits_plus_upper_bound_bit.insert(logits_plus_upper_bound_bit.end(),
                                      batch_comp_res.begin() + 2* vec_size,
                                      batch_comp_res.end());
  batch_comp_res.clear();
  
  // 3.1 max(logits, 0) by Relu
  // ToDo: pack BMA with the following B2A etc to reduce communication round further.
  BMA(logits_sign_bit, logits, max_part); // BMA equals to B2A, Mul

  // 3.2 abs(logits) by DRelu and Select
  vector<Share> logits_sign_arith(vec_size);
  B2A(logits_sign_bit, logits_sign_arith);
  logits_sign_bit.clear();

  vector<double> DOUBLE_ONE(vec_size, 1.0);
  vector<double> DOUBLE_NEG_ONE(vec_size, -1.0);
  vector<Share> sign_multiplier(vec_size);
  // local
  Select1Of2(DOUBLE_ONE, DOUBLE_NEG_ONE, logits_sign_arith, sign_multiplier);
  logits_sign_arith.clear();
  
  // 3.3 compare abs(logits) > [upper bound value in the poly interval domain].
  // Note that |X|  C iff ((X > C) OR X < -C), i.e. ((C-X)>0) AND ((C+X)>0)
  vector<BitShare> no_need_clip_bit(vec_size);
  Mul(upper_bound_minus_logits_bit, logits_plus_upper_bound_bit, no_need_clip_bit);
  upper_bound_minus_logits_bit.clear();
  logits_plus_upper_bound_bit.clear();
  vector<Share> no_need_clip_arith(vec_size);
  B2A(no_need_clip_bit, no_need_clip_arith);
  no_need_clip_bit.clear();
  
  ///******************** 4. pack data for batch Mul
  vector<Share> batch_mul_op_left;
  vector<Share> batch_mul_op_right;

  batch_mul_op_left.insert(batch_mul_op_left.end(), sign_multiplier.begin(), sign_multiplier.end());
  batch_mul_op_right.insert(batch_mul_op_right.end(), logits.begin(), logits.end());

  batch_mul_op_left.insert(batch_mul_op_left.end(), labels.begin(), labels.end());
  batch_mul_op_right.insert(batch_mul_op_right.end(), logits.begin(), logits.end());

  ///******************** 5.  call MUL operation in one shot.
  vector<Share> batch_mul_op_res(batch_mul_op_right.size());
  Mul(batch_mul_op_left, batch_mul_op_right, batch_mul_op_res);
  batch_mul_op_left.clear();
  batch_mul_op_right.clear();

  ///******************** 5.  unpack MUL result.
  // 5.1 abs(logit)
  vector<Share> _abs;
  _abs.insert(_abs.end(), 
              batch_mul_op_res.begin(),
              batch_mul_op_res.begin() + vec_size);
  // 5.2 logit * label
  vector<Share> prod_part;
  prod_part.insert(prod_part.end(),
                  batch_mul_op_res.begin() + vec_size,
                  batch_mul_op_res.end());
  batch_mul_op_res.clear();

  vector<double> LOWER_CLIP_CONST(vec_size, 0.0003);
  vector<Share> LOWER_V(vec_size);
  ConstCommonInput(LOWER_CLIP_CONST, LOWER_V);
  vector<Share> caped_V(vec_size);

  // 6. log(1 + exp(-X) with polynomial
  vector<Share> basic_val(vec_size);
  UniPolynomial(_abs, power_list, coff_list, basic_val);
  Select1Of2(basic_val, LOWER_V, no_need_clip_arith, log_part);
  
  // 7. collect all parts
  Sub(max_part, prod_part, Z);
  Add(Z, log_part);

  AUDIT("id:{}, P{} SigmoidCrossEntropy_batch compute: Z=max(logit,0)-logit*label+log(1+exp(-abs(logits)), output Z(Share){}", msgid.get_hex(), player, Vector<Share>(Z));
}

/*
    max(logit, 0) - logit * label + log(1 + exp(-abs(logits)))
*/
void HelixInternal::SigmoidCrossEntropy(const vector<Share>& logits, const vector<Share>& labels, vector<Share>& Z) {
  AUDIT("id:{}, P{} SigmoidCrossEntropy compute: Z=max(logit,0)-logit*label+log(1+exp(-abs(logits)), input logit(Share){}", msgid.get_hex(), player, Vector<Share>(logits));
  AUDIT("id:{}, P{} SigmoidCrossEntropy compute: Z=max(logit,0)-logit*label+log(1+exp(-abs(logits)), input labels(Share){}", msgid.get_hex(), player, Vector<Share>(labels));
  int vec_size = logits.size();
  int float_precision = GetMpcContext()->FLOAT_PRECISION;
  // 1. max(logit, 0)
  vector<Share> max_part(vec_size);
  ReLU(logits, max_part);
  // 2. logit * label
  vector<Share> prod_part(vec_size);
  Mul(logits, labels, prod_part);
  // 3. abs(logit)
  vector<Share> _abs(vec_size);
  vector<double> DOUBLE_ONE(vec_size, 1.0);
  vector<double> DOUBLE_NEG_ONE(vec_size, -1.0);
  vector<Share> a_sign(vec_size);
  vector<Share> sign_multiplier(vec_size);
  DReLU(logits, a_sign);
  Select1Of2(DOUBLE_ONE, DOUBLE_NEG_ONE, a_sign, sign_multiplier);
  Mul(sign_multiplier, logits, _abs);
  // 4. log(1 + exp(-X) with polynomial
  vector<Share> log_part(vec_size);
  string my_func = "LOG_CE";
  vector<mpc_t> power_list;
	vector<mpc_t> coff_list;
  vector<ConstPolynomial>* log_ce_p = NULL;
  if (!PolyConfFactory::get_func_polys(my_func, &log_ce_p)) {
    // TODO: throw exception
    tlog_error << "ERROR! can not find polynomials for func " << my_func ;
    return;
  }
	// we know that this is a one-segment polynomial
  log_ce_p->at(0).get_power_list(power_list);
  log_ce_p->at(0).get_coff_list(coff_list, float_precision);
  // for (auto i = 0; i< power_list.size(); i++) {
	//  	cout << MpcTypeToFloat(CoffDown(coff_list[i])) << "*X^" << power_list[i] << " + ";
	// }
  // cout << endl;
  // caped input
  mpc_t end_v = log_ce_p->at(0).get_end(float_precision);
  vector<mpc_t> upper_bound_v(vec_size, end_v);
  vector<Share> UPPER_V(vec_size);
  // Input(0, upper_bound_v, UPPER_V);
  ConstCommonInput(upper_bound_v, UPPER_V);
  vector<double> LOWER_CLIP_CONST(vec_size, 0.0003);
  vector<Share> LOWER_V(vec_size);
  // Input(0, LOWER_CLIP_CONST, LOWER_V);
  ConstCommonInput(LOWER_CLIP_CONST, LOWER_V);
  vector<Share> caped_V(vec_size);
  vector<Share> x_minus_upper_bound(vec_size);
  Sub(_abs, UPPER_V, x_minus_upper_bound);
  vector<Share> cmp_res(vec_size);
  DReLU(x_minus_upper_bound, cmp_res);
  //vector<Share> cmp_res_scaled(vec_size);
  //Scale(cmp_res, cmp_res_scaled);
  Select1Of2(UPPER_V, _abs, cmp_res, caped_V);
  
  vector<Share> basic_val(vec_size);
  UniPolynomial(caped_V, power_list, coff_list, basic_val);
  Select1Of2(LOWER_V, basic_val, cmp_res, log_part);
  // 5. collect all parts
  Sub(max_part, prod_part, Z);
  Add(Z, log_part);

  AUDIT("id:{}, P{} SigmoidCrossEntropy compute: Z=max(logit,0)-logit*label+log(1+exp(-abs(logits)), output Z(Share){}", msgid.get_hex(), player, Vector<Share>(Z));
}

} // namespace helix
} // namespace rosetta
