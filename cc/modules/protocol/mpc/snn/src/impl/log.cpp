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

// clang-format off

/*
    Apprximating logarithm functionality with different polynomials
*/
namespace rosetta {
namespace snn { 

int Log::mpc_log_hd(const vector<mpc_t>& shared_X,
				vector<mpc_t>& shared_Y,
				size_t vec_size) {
	if(THREE_PC) {
		mpc_t LEN = 8 * sizeof(mpc_t);
		vector<mpc_t> SHARED_LN_2(vec_size, 0);
		if(partyNum == PARTY_A) {
			// math.log(2) = 0.693147181
			SHARED_LN_2 = vector<mpc_t>(vec_size, FloatToMpcType(0.693147181));
		}
		vector<mpc_t> SHARED_HALF(vec_size, 0);
		if(partyNum == PARTY_A) {
			SHARED_HALF = vector<mpc_t>(vec_size, FloatToMpcType(0.5));
		}
		vector<mpc_t> SHARED_ONE(vec_size, 0);
		if(partyNum == PARTY_A) {
			SHARED_ONE = vector<mpc_t>(vec_size, FloatToMpcType(1));
		}
		vector<mpc_t> SHARED_TWO(vec_size, 0);
		if(partyNum == PARTY_A) {
			SHARED_TWO = vector<mpc_t>(vec_size, FloatToMpcType(2));
		}
		vector<mpc_t> SHARED_ZERO(vec_size, 0);
		vector<mpc_t> SHARED_NEG_ONE(vec_size, 0);
		if(partyNum == PARTY_A) {
			SHARED_NEG_ONE = vector<mpc_t>(vec_size, FloatToMpcType(-1));
		}
		
		vector<mpc_t> shared_val_multiplier(vec_size, 0);
		vector<mpc_t> shared_power_add(vec_size, 0);
		auto curr_x = shared_X;
		vector<mpc_t> curr_power(vec_size, 0);
		// step 1: express x as 2^{m} * {r/2} where r \in [1, 2)
		// 1.1 scale down integer part
		for(int i = 0; i < LEN - FLOAT_PRECISION_M; ++i) {
			auto curr_x_minus_one = curr_x;
			vector<mpc_t> shared_cmp(vec_size, 0);
			
			subtractVectors<mpc_t>(curr_x, SHARED_ONE, curr_x_minus_one, vec_size);
			
			GetMpcOpInner(ReluPrime)->Run3PC(curr_x_minus_one, shared_cmp, vec_size);

			// selection based on x - 1 > 0
			GetMpcOpInner(Select1Of2)->Run(SHARED_HALF, SHARED_ONE, shared_cmp,
			 					shared_val_multiplier, vec_size);
			GetMpcOpInner(Select1Of2)->Run(SHARED_ONE, SHARED_ZERO, shared_cmp,
								shared_power_add, vec_size);
			vector<mpc_t> tmp_vec(vec_size, 0);
			GetMpcOpInner(DotProduct)->Run(curr_x, shared_val_multiplier, tmp_vec, vec_size);
			curr_x = tmp_vec;
			addVectors<mpc_t>(curr_power, shared_power_add, tmp_vec, vec_size);
			curr_power = tmp_vec;
		}
		// 1.2 scale up fractional part
		for(int i = 0; i < FLOAT_PRECISION_M; ++i) {
			auto curr_x_minus_half = curr_x;
			vector<mpc_t> shared_cmp(vec_size, 0);
			subtractVectors<mpc_t>(curr_x, SHARED_HALF, curr_x_minus_half, vec_size);
		
			GetMpcOpInner(ReluPrime)->Run3PC(curr_x_minus_half, shared_cmp, vec_size);
			// selection based on x - 1 > 0
			GetMpcOpInner(Select1Of2)->Run(SHARED_ONE, SHARED_TWO, shared_cmp,
								shared_val_multiplier, vec_size);
			GetMpcOpInner(Select1Of2)->Run(SHARED_ZERO, SHARED_NEG_ONE, shared_cmp,
								shared_power_add, vec_size);

			vector<mpc_t> tmp_vec(vec_size, 0);
			GetMpcOpInner(DotProduct)->Run(curr_x, shared_val_multiplier, tmp_vec, vec_size);
			curr_x = tmp_vec;
			addVectors<mpc_t>(curr_power, shared_power_add, tmp_vec, vec_size);
			curr_power = tmp_vec;
		}

		// step 2: computer basic ln(r/2) by polynomials interproplation
		vector<mpc_t> power_list;
		vector<mpc_t> coff_list;
    	string my_func = "LOG_HD";
		vector<ConstPolynomial>* log_hd_p = NULL;
    	if (!PolyConfFactory::get_func_polys(my_func, &log_hd_p)) {
    	    // TODO: throw exception
    	    cout << "ERROR! can not find polynomials for func " << my_func <<endl;
    	    return -1;
    	}
		log_hd_p->at(0).get_power_list(power_list);
		log_hd_p->at(0).get_coff_list(coff_list);
		vector<mpc_t> shared_basic_val(vec_size, 0);
		vector<mpc_t> tmp_v(vec_size);
		GetMpcOpInner(Polynomial)->mpc_uni_polynomial(curr_x, power_list, coff_list, tmp_v);
        for (int i = 0; i < vec_size; ++i) {
            shared_basic_val[i] = CoffDown(tmp_v[i]);
        }

		vector<mpc_t> shared_high_part(vec_size, 0);
		GetMpcOpInner(DotProduct)->Run(curr_power, SHARED_LN_2, shared_high_part, vec_size);
		addVectors<mpc_t>(shared_high_part, shared_basic_val, shared_Y, vec_size);
		return 0;
	}
	if(FOUR_PC) {
		cout << "ERROR! not support yet! " <<endl;
		// TODO throw exception
		return -1;
	}
	return 0;
}

// outdated
void Log::mpc_log_v1(const mpc_t& shared_X,
				mpc_t& shared_Y) {
	shared_Y = 0;   
	vector<mpc_t> power_list;
	vector<mpc_t> coff_list;
    string my_func = "LOG_V1";
	vector<ConstPolynomial>* log_v1_p = NULL;
    if (!PolyConfFactory::get_func_polys(my_func, &log_v1_p)) {
        // TODO: throw exception
        cout << "ERROR! can not find polynomials for func " << my_func <<endl;
        return;
    }
	log_v1_p->at(0).get_power_list(power_list);
	log_v1_p->at(0).get_coff_list(coff_list);	
	GetMpcOpInner(Polynomial)->mpc_uni_polynomial(shared_X, power_list, coff_list, shared_Y);
	shared_Y = CoffDown(shared_Y);
}

void Log::mpc_log_v2(const vector<mpc_t>& shared_X, 
					vector<mpc_t>& shared_Y) {
	int vec_size = shared_X.size();
	shared_Y.resize(vec_size);		
    string my_func = "LOG_V2";
	vector<ConstPolynomial>* log_v2_p = NULL;
    if (!PolyConfFactory::get_func_polys(my_func, &log_v2_p)) {
        // TODO: throw exception
        cout << "ERROR! can not find polynomials for func " << my_func <<endl;
        return;
    }
	auto seg_size = log_v2_p->size();
   	if (seg_size == 0) {
		// TODO: throw exception
		cout << "ERROR! empty polynomials in log_v2." << endl; 
		return;
   	}
	
   	// actually we will use the [start, end)-style for each segement!
   	vector<mpc_t> curr_power_list;
   	vector<mpc_t> curr_coff_list;
   	// some temprorary variables
	vector<mpc_t> shared_cmp_init(vec_size, 0);
	vector<mpc_t> shared_cmp_end(vec_size, 0);
   	vector<mpc_t> shared_init(vec_size, 0);
   	vector<mpc_t> shared_end(vec_size, 0);
   	vector<mpc_t> shared_res(vec_size, 0);
	for(int i = 0; i < seg_size; ++i) {
	   	ConstPolynomial curr_seg = log_v2_p->at(i);
	   	mpc_t seg_init = curr_seg.get_start();
	   	mpc_t seg_end = curr_seg.get_end();
		curr_seg.get_power_list(curr_power_list);
		curr_seg.get_coff_list(curr_coff_list);
		// cout << "Seg " << i << ":[" << MpcTypeToFloat(seg_init) <<
		//  			", " << MpcTypeToFloat(seg_end) << "): ";
		// for(auto i = 0; i< curr_power_list.size(); i++) {
		//     cout << MpcTypeToFloat(curr_coff_list[i]) << "*X^" << 
		//  			to_readable_hex(curr_power_list[i]) << " + ";
		// }
		// S1: use ReLUPrime to get whether to use this segemnt[ multiplier is 0 or 1]
		/// 1.1 check start point
		// x >= start && (1 - (x >= end))
		shared_cmp_init = shared_X;
		shared_cmp_end = shared_X;
		for(int j = 0; j < vec_size; ++j) {
			if(partyNum == PARTY_A) {
				shared_cmp_init[j] = shared_X[j] - seg_init;
				shared_cmp_end[j] = shared_X[j] - seg_end; 
			}
		}
		// packing for vectorization
		vector<mpc_t> shared_cmp = shared_cmp_init;
		shared_cmp.insert(shared_cmp.end(), shared_cmp_end.begin(), shared_cmp_end.end());
		vector<mpc_t> tmp_reluprime(2*vec_size);	
		GetMpcOpInner(ReluPrime)->Run3PC(shared_cmp, tmp_reluprime, 2*vec_size);
		shared_init.assign(tmp_reluprime.begin(), tmp_reluprime.begin() + vec_size);
		shared_end.assign(tmp_reluprime.begin() + vec_size, tmp_reluprime.end());
		vector<mpc_t> CONST_ONE(vec_size, FloatToMpcType(1.0/2.0));
		vector<mpc_t> shared_end2(vec_size, 0);
		if (PRIMARY) {
			subtractVectors(CONST_ONE, shared_end, shared_end2, vec_size);
		}
		GetMpcOpInner(DotProduct)->Run(shared_init, shared_end2, shared_res, vec_size);

		vector<mpc_t> poly_res(vec_size, 0);
		// S2: compute the value in this segment
		GetMpcOpInner(Polynomial)->mpc_uni_polynomial(shared_X, curr_power_list, curr_coff_list, poly_res);
		for (int i = 0; i < vec_size; ++i) {
            poly_res[i] = CoffDown(poly_res[i]);
        }
		// debug 
		// if(PRIMARY) {
        //   	GetMpcOpInner(Reconstruct2PC)->Run(poly_res, poly_res.size(), "curr seg Poly value");
    	// }
		vector<mpc_t> this_seg_res(vec_size, 0);
		GetMpcOpInner(DotProduct)->Run(shared_res, poly_res, this_seg_res, vec_size);	
		addVectors<mpc_t>(shared_Y, this_seg_res, shared_Y, vec_size);
    }
}


void Log::mpc_log_v2(const mpc_t& shared_X,
				mpc_t& shared_Y) {
	shared_Y = 0;				
    string my_func = "LOG_V2";
	vector<ConstPolynomial>* log_v2_p = NULL;
    if (!PolyConfFactory::get_func_polys(my_func, &log_v2_p)) {
        // TODO: throw exception
        cout << "ERROR! can not find polynomials for func " << my_func <<endl;
        return;
    }
	auto seg_size = log_v2_p->size();
   	if (seg_size == 0) {
		// TODO: throw exception
		cout << "ERROR! empty polynomials in log_v2." << endl; 
		return;
   	}
   	// actually we will use the [start, end)-style for each segement!
   	vector<mpc_t> curr_power_list;
   	vector<mpc_t> curr_coff_list;
   	mpc_t shared_seg_multiplier = 1;
   	// some temprorary variables
   	vector<mpc_t> shared_cmp(2, 0);
   	vector<mpc_t> shared_init(1, 0);
   	vector<mpc_t> shared_end(1, 0);
   	vector<mpc_t> shared_res(1, 0);
	for(int i = 0; i < seg_size; ++i) {
	   ConstPolynomial curr_seg = log_v2_p->at(i);
	   mpc_t seg_init = curr_seg.get_start();
	   mpc_t seg_end = curr_seg.get_end();
		curr_seg.get_power_list(curr_power_list);
		curr_seg.get_coff_list(curr_coff_list);
		/// S1: use ReLUPrime to get whether to use this segemnt[ multiplier is 0 or 1]
		/// 1.1 check start point
		// x >= start && (1 - (x >= end))
		shared_cmp[0] = shared_X;
		shared_cmp[1] = shared_X;
		if(partyNum == PARTY_A) {
			shared_cmp[0] = shared_X - seg_init;
			shared_cmp[1] = shared_X - seg_end; 
		}
		vector<mpc_t> tmp_reluprime(2, 0);	
		GetMpcOpInner(ReluPrime)->Run3PC(shared_cmp, tmp_reluprime, 2);
		if(partyNum == PARTY_A) {
			tmp_reluprime[1] = FloatToMpcType(1) - tmp_reluprime[1];
		} else {
			tmp_reluprime[1] = -tmp_reluprime[1];
		}
		shared_init[0] = tmp_reluprime[0];
		shared_end[0] = tmp_reluprime[1];
		GetMpcOpInner(DotProduct)->Run(shared_init, shared_end, shared_res, 1);
		// S2: compute the value in this segment
		mpc_t curr_shared_Y = 0;	
		GetMpcOpInner(Polynomial)->mpc_uni_polynomial(shared_X, curr_power_list, curr_coff_list, curr_shared_Y);
		shared_init[0] = CoffDown(curr_shared_Y);
		shared_end[0] = shared_res[0];
		GetMpcOpInner(DotProduct)->Run(shared_init, shared_end, shared_res, 1);	
		shared_Y += shared_res[0];
    }
}

} // namespace snn
} // namespace rosetta

// clang-format on
