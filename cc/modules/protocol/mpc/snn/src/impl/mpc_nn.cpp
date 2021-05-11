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
/*
    Apprximating logarithm functionality with different polynomials
*/
#include "cc/modules/protocol/mpc/snn/src/impl/op_impl.h"

#include <cmath>
#include <unordered_map>
#include <string>
#include <vector>
using namespace std;
using namespace rosetta;

namespace rosetta {
namespace snn {
// Specificated version for reducing round complexity
int SigmoidCrossEntropy::sigmoid_cross_entropy_batch(const vector<mpc_t>& shared_logits,
                                            const vector<mpc_t>& shared_labels,
                                            vector<mpc_t>& shared_result,
                                            size_t vec_size) {
    // cout << "sigmoid_cross_entropy_batch !" << endl;
    ///********************1. prepare all data for batch-ReluPrime. 
    vector<mpc_t> batch_comp_X;
    batch_comp_X.insert(batch_comp_X.end(), shared_logits.begin(), shared_logits.end());
    
    string my_func = "LOG_CE";
    // we know that this functionality need single segment.
    vector<mpc_t> power_list;
	vector<mpc_t> coff_list;
    vector<ConstPolynomial> * log_ce_p = NULL;
    if(!PolyConfFactory::get_func_polys(my_func, &log_ce_p)) {
        // TODO: throw exception
        cout << "ERROR! can not find polynomials for func " << my_func <<endl;
        return -1;
    }
	auto seg_size = log_ce_p->size();
   	log_ce_p->at(0).get_power_list(power_list);
	log_ce_p->at(0).get_coff_list(coff_list);
    auto end_v = log_ce_p->at(0).get_end();
    vector<mpc_t> UPPER_V(vec_size, 0);
    if(partyNum == PARTY_A) {
        vector<mpc_t> tmp_vec(vec_size, end_v);
        UPPER_V.swap(tmp_vec);
    }

    vector<mpc_t> upper_bound_minus_logits(vec_size);
    vector<mpc_t> logits_plus_upper_bound(vec_size);

    subtractVectors<mpc_t>(UPPER_V, shared_logits, upper_bound_minus_logits, vec_size);
    addVectors<mpc_t>(shared_logits, UPPER_V, logits_plus_upper_bound, vec_size);
    
    batch_comp_X.insert(batch_comp_X.end(),
                        upper_bound_minus_logits.begin(),
                        upper_bound_minus_logits.end());

    batch_comp_X.insert(batch_comp_X.end(),
                        logits_plus_upper_bound.begin(),
                        logits_plus_upper_bound.end());
    
    ///********************2.  call the costly ReluPrime operation in one shot.
    vector<mpc_t> batch_cmp_res(batch_comp_X.size());
    GetMpcOpInner(ReluPrime)->Run(batch_comp_X, batch_cmp_res, batch_comp_X.size());
    batch_comp_X.clear();

    ///********************3.  unpack  results and get related results.
    vector<mpc_t> max_part_bit;
    max_part_bit.insert(max_part_bit.end(), batch_cmp_res.begin(), batch_cmp_res.begin() +vec_size);
    vector<mpc_t> upper_bound_minus_logits_bit;
    upper_bound_minus_logits_bit.insert(upper_bound_minus_logits_bit.end(), 
                                      batch_cmp_res.begin() + vec_size,
                                      batch_cmp_res.begin() + 2* vec_size);
    vector<mpc_t>  logits_plus_upper_bound_bit;
    logits_plus_upper_bound_bit.insert(logits_plus_upper_bound_bit.end(),
                                      batch_cmp_res.begin() + 2* vec_size,
                                      batch_cmp_res.end());
    batch_cmp_res.clear();
    
    // 3.1 get Logits Sign multiplier.
    vector<double> SHARED_TWO(vec_size, 2);
    vector<mpc_t> SHARED_NEG_ONE(vec_size, 0);
    if (partyNum == PARTY_A) {
        SHARED_NEG_ONE = vector<mpc_t>(vec_size, FloatToMpcType(-1));
    }
    vector<mpc_t> shared_x_multiplier(vec_size, 0);
    vector<mpc_t> temp_mul_res(vec_size);
    // this is local
    GetMpcOpInner(DotProduct)->Run(SHARED_TWO, max_part_bit, temp_mul_res, vec_size);
    addVectors<mpc_t>(SHARED_NEG_ONE, temp_mul_res, shared_x_multiplier, vec_size);
    temp_mul_res.clear();
    SHARED_TWO.clear();
    SHARED_NEG_ONE.clear();

    ///******************** 4. pack data for batch DotProduct[Mul]
    vector<mpc_t> batch_mul_op_left;
    vector<mpc_t> batch_mul_op_right;

    batch_mul_op_left.insert(batch_mul_op_left.end(), max_part_bit.begin(), max_part_bit.end());
    batch_mul_op_right.insert(batch_mul_op_right.end(), shared_logits.begin(), shared_logits.end());

    batch_mul_op_left.insert(batch_mul_op_left.end(), shared_x_multiplier.begin(), shared_x_multiplier.end());
    batch_mul_op_right.insert(batch_mul_op_right.end(), shared_logits.begin(), shared_logits.end());

    batch_mul_op_left.insert(batch_mul_op_left.end(), shared_labels.begin(), shared_labels.end());
    batch_mul_op_right.insert(batch_mul_op_right.end(), shared_logits.begin(), shared_logits.end());

    batch_mul_op_left.insert(batch_mul_op_left.end(), 
                            upper_bound_minus_logits_bit.begin(), upper_bound_minus_logits_bit.end());
    batch_mul_op_right.insert(batch_mul_op_right.end(), 
                            logits_plus_upper_bound_bit.begin(), logits_plus_upper_bound_bit.end());
    
    vector<mpc_t>  batch_mul_op_res(batch_mul_op_right.size());
    GetMpcOpInner(DotProduct)->Run(batch_mul_op_left, 
                                    batch_mul_op_right,
                                    batch_mul_op_res, batch_mul_op_left.size());
    batch_mul_op_left.clear();
    batch_mul_op_right.clear();

    ///******************** 5.  unpack MUL result.
    // max(logit, 0)
    vector<mpc_t> max_part;
    max_part.insert(max_part.end(), batch_mul_op_res.begin(), batch_mul_op_res.begin() + vec_size);
    // 5.1 abs(logit)
    vector<mpc_t> _abs;
    _abs.insert(_abs.end(), 
              batch_mul_op_res.begin() + vec_size,
              batch_mul_op_res.begin() + 2 * vec_size);
    // logit * label
    vector<mpc_t> prod_part;
    prod_part.insert(prod_part.end(), 
                    batch_mul_op_res.begin() + 2 * vec_size, 
                    batch_mul_op_res.begin() + 3 * vec_size);
    // clipped_sign
    vector<mpc_t> no_need_clip_bit;
    no_need_clip_bit.insert(no_need_clip_bit.end(), 
                    batch_mul_op_res.begin() + 3 * vec_size, 
                    batch_mul_op_res.end());
    batch_mul_op_res.clear();
    
    ///******************** 6. get log(1 + exp(-X) part with polynomial.
    vector<mpc_t> log_part(vec_size);
    vector<mpc_t> LOWER_CLIP_CONST(vec_size);
    if (partyNum == PARTY_A) {
        LOWER_CLIP_CONST = vector<mpc_t>(vec_size, FloatToMpcType(0.0003));
    }
    GetMpcOpInner(Polynomial)->mpc_uni_polynomial(_abs, power_list, coff_list, log_part);
    GetMpcOpInner(Select1Of2)->Run(log_part, LOWER_CLIP_CONST, no_need_clip_bit, log_part, vec_size);   
    // 7. collect all parts
    // max(logit, 0) - logit * label
    subtractVectors<mpc_t>(max_part, prod_part, shared_result, vec_size);
    addVectors<mpc_t>(shared_result, log_part, shared_result, vec_size);
    return 0;
}

/*
    max(logit, 0) - logit * label + log(1 + exp(-abs(x)))
*/
int SigmoidCrossEntropy::sigmoid_cross_entropy(const vector<mpc_t>& shared_logits,
                                            const vector<mpc_t>& shared_labels,
                                            vector<mpc_t>& shared_result,
                                            size_t vec_size) {
    vector<mpc_t> tmp_part(vec_size, 0);
    // max(logit, 0)
    GetMpcOpInner(Relu)->Run(shared_logits, tmp_part, vec_size);
    vector<mpc_t> prod_part(vec_size, 0);
    // logit * label
    GetMpcOpInner(DotProduct)->Run(shared_logits, shared_labels, prod_part, vec_size);
    // max(logit, 0) - logit * label
    subtractVectors<mpc_t>(tmp_part, prod_part, shared_result, vec_size);
    // if(PRIMARY) {
    //     GetMpcOpInner(Reconstruct2PC)->Run(shared_result, shared_result.size(), "curr_X");
    // }
    vector<mpc_t> log_part(vec_size, 0);
    // |X|
    ABS(shared_logits, tmp_part, vec_size);
    // log(1 + exp(-abs(x))) 
    CELog(tmp_part, log_part, vec_size);
    // if(PRIMARY) {
    //     GetMpcOpInner(Reconstruct2PC)->Run(log_part, log_part.size(), "log part curr_X");
    // }    
    addVectors<mpc_t>(shared_result, log_part, shared_result, vec_size);
    
    return 0;
}

/* 
    |x| means:
             1 * x if x >=  0
          (-1) * x if x < 0
*/
void SigmoidCrossEntropy::ABS(const vector<mpc_t>& shared_X,
               vector<mpc_t>& shared_Y,
               size_t vec_size) {
    vector<mpc_t> SHARED_ONE(vec_size, 0);
    if (partyNum == PARTY_A) {
        SHARED_ONE = vector<mpc_t>(vec_size, FloatToMpcType(1));
    }
    vector<mpc_t> SHARED_NEG_ONE(vec_size, 0);
    if (partyNum == PARTY_A) {
        SHARED_NEG_ONE = vector<mpc_t>(vec_size, FloatToMpcType(-1));
    }
    vector<mpc_t> shared_x_multiplier(vec_size, 0);
	vector<mpc_t> shared_cmp(vec_size, 0);
    GetMpcOpInner(ReluPrime)->Run3PC(shared_X, shared_cmp, vec_size);
    GetMpcOpInner(Select1Of2)->Run(SHARED_ONE,
                                    SHARED_NEG_ONE, shared_cmp, shared_x_multiplier, vec_size);
    GetMpcOpInner(DotProduct)->Run(shared_x_multiplier, shared_X, shared_Y, vec_size);
}
/* 
    |x|' means:
             1  if x >=  0 [ in point 0 we set it to 1]
          (-1)  if x < 0

*/
void SigmoidCrossEntropy::ABSPrime(const vector<mpc_t>& shared_X,
               vector<mpc_t>& shared_Y,
               size_t vec_size) {
    vector<mpc_t> SHARED_ONE(vec_size, 0);
    if (partyNum == PARTY_A) {
        SHARED_ONE = vector<mpc_t>(vec_size, FloatToMpcType(1));
    }
    vector<mpc_t> SHARED_NEG_ONE(vec_size, 0);
    if (partyNum == PARTY_A) {
        SHARED_NEG_ONE = vector<mpc_t>(vec_size, FloatToMpcType(-1));
    }
	vector<mpc_t> shared_cmp(vec_size, 0);
    GetMpcOpInner(ReluPrime)->Run3PC(shared_X, shared_cmp, vec_size);
    GetMpcOpInner(Select1Of2)->Run(SHARED_ONE,
                                    SHARED_NEG_ONE, shared_cmp, shared_Y, vec_size);
}

void SigmoidCrossEntropy::CELog(const vector<mpc_t>& shared_X,
               vector<mpc_t>& shared_Y,
               size_t vec_size) {
    string my_func = "LOG_CE";
    // we know that this functionality need single segment.
    vector<mpc_t> power_list;
	vector<mpc_t> coff_list;
    vector<ConstPolynomial> * log_ce_p = NULL;
    if(!PolyConfFactory::get_func_polys(my_func, &log_ce_p)) {
        // TODO: throw exception
        cout << "ERROR! can not find polynomials for func " << my_func <<endl;
        return;
    }
	auto seg_size = log_ce_p->size();
   if(seg_size == 0) {
		// TODO: throw exception
		cout << "ERROR! empty polynomials in log_v2." << endl; 
		return;
   }
   	log_ce_p->at(0).get_power_list(power_list);
	log_ce_p->at(0).get_coff_list(coff_list);
    auto end_v = log_ce_p->at(0).get_end();

    // for (auto i = 0; i< power_list.size(); i++) {
	// 	cout << MpcTypeToFloat(coff_list[i]) << "*X^" << power_list[i] << " + ";
	// }
    
    if(THREE_PC) {
        vector<mpc_t> tmp_END(vec_size, 0);
        auto shared_cmp = shared_X;
        if(partyNum == PARTY_A) {
            vector<mpc_t> tmp_vec(vec_size, end_v);
            tmp_END.swap(tmp_vec);
            subtractVectors<mpc_t>(shared_X, tmp_END, shared_cmp, vec_size);
        }

        vector<mpc_t> cmp_res(vec_size, 0);
        GetMpcOpInner(ReluPrime)->Run3PC(shared_cmp, cmp_res, vec_size);
        vector<mpc_t> curr_X(vec_size, 0);
        GetMpcOpInner(Select1Of2)->Run(tmp_END, shared_X, cmp_res,
								curr_X, vec_size);
        
        vector<mpc_t> LOWER_BOUND(vec_size, FloatToMpcType(0.00015));
        vector<mpc_t> curr_res(vec_size, 0);
        vector<mpc_t> tmp_v(vec_size);
        log_debug << "DEBUG CE LOG, calling vectorization mpc_uni_polynomial" << endl;
        GetMpcOpInner(Polynomial)->mpc_uni_polynomial(curr_X, power_list, coff_list, tmp_v);
        for (int i = 0; i < vec_size; ++i) {
            curr_res[i] = CoffDown(tmp_v[i]);
        }

        GetMpcOpInner(Select1Of2)->Run(LOWER_BOUND, curr_res, cmp_res, shared_Y, vec_size);     
    } else {
        notYet();
    }
}

} // namespace mpc
} // namespace rosetta