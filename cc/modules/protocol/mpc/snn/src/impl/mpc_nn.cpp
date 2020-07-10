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
        
        // if(PRIMARY) {
        //      GetMpcOpInner(Reconstruct2PC)->Run(shared_X, shared_X.size(), "original X");
		//      GetMpcOpInner(Reconstruct2PC)->Run(curr_X, curr_X.size(), "curr_X");
		// }
        vector<mpc_t> LOWER_BOUND(vec_size, FloatToMpcType(0.00015));
        vector<mpc_t> curr_res(vec_size, 0);
        mpc_t tmp_v = 0;
        // TODO: vectorization this func
        for(int i = 0; i < vec_size; ++i) {
            auto& each_x = curr_X[i];
            GetMpcOpInner(Polynomial)->mpc_uni_polynomial(each_x, power_list, coff_list, tmp_v);
            curr_res[i] = CoffDown(tmp_v);
        }

        GetMpcOpInner(Select1Of2)->Run(LOWER_BOUND, curr_res, cmp_res, shared_Y, vec_size);     
    } else {
        notYet();
    }
}

} // namespace mpc
} // namespace rosetta