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
    For implemention of general MPC polynomials and approximating non-arithmatic
    functionalities. 
*/

#include "cc/modules/protocol/mpc/snn/src/impl/op_impl.h"

#include <unordered_map>
#include <string>
using namespace std;
using namespace rosetta;

namespace rosetta {
namespace snn {
void Polynomial::mpc_pow_const(const vector<mpc_t>& shared_X,
					mpc_t common_k,
					vector<mpc_t>& shared_Y,
					unordered_map<mpc_t, vector<mpc_t>>* curr_cache) {
	int vec_size = shared_X.size();
	shared_Y.resize(vec_size);
    // TODO[SJJ]: make this more elegant
	if(curr_cache == NULL) {
		cout << "ERROR! NULL pointer of cache parameter!" << endl;
	}
	
	if(curr_cache->find(common_k) != curr_cache->end()){
		shared_Y = (*curr_cache)[common_k];
		return;
	}
	vector<mpc_t> curr_Y(vec_size, FloatToMpcType(1));
	mpc_t curr_v = common_k;
	mpc_t curr_bit = 0;
	mpc_t curr_2_p = 1;
	// TODO: local constant ONE
	vector<mpc_t> tmp_y(vec_size, FloatToMpcType(1.0/2));
	vector<mpc_t> tmp_2_p = curr_Y;
	vector<mpc_t> tmp_new_y = curr_Y;
	while (curr_v != 0) {
		curr_bit = curr_v % 2;
		if(curr_2_p == 1) {
			if(curr_cache->find(curr_2_p) == curr_cache->end()) {
				(*curr_cache)[curr_2_p] = shared_X;				
			}
			tmp_2_p = shared_X;
		} else {
			vector<mpc_t> tmp_v;
			mpc_pow_const(shared_X, curr_2_p / 2, tmp_v, curr_cache);
			tmp_2_p = tmp_v;
			GetMpcOpInner(DotProduct)->Run(tmp_2_p, tmp_2_p, tmp_new_y, vec_size);
			(*curr_cache)[curr_2_p] = tmp_new_y;		
			tmp_2_p = tmp_new_y;
		}
		if (curr_bit) {
			GetMpcOpInner(DotProduct)->Run(tmp_2_p, tmp_y, tmp_new_y, vec_size);	
			tmp_y = tmp_new_y;
		}
   		curr_v = mpc_t(curr_v / 2);
    	curr_2_p = 2 * curr_2_p; 
    }
	shared_Y = tmp_y;
    return;
}


void Polynomial::mpc_pow_const(const mpc_t& shared_X,
					mpc_t common_k,
					mpc_t& shared_Y,
					unordered_map<mpc_t, mpc_t>* curr_cache) {
    // TODO[SJJ]: make this more elegant
	if(curr_cache == NULL) {
		cout << "ERROR! NULL pointer of cache parameter!" << endl;
	}
	
	if(curr_cache->find(common_k) != curr_cache->end()){
		shared_Y = (*curr_cache)[common_k];
		return;
	}
	mpc_t curr_Y = FloatToMpcType(1);
	mpc_t curr_v = common_k;
	mpc_t curr_bit = 0;
	mpc_t curr_2_p = 1;
	// TODO: remove 2
	vector<mpc_t> tmp_y(1, curr_Y/2);
	vector<mpc_t> tmp_2_p(1, curr_Y);
	vector<mpc_t> tmp_new_y(1, curr_Y);
	while (curr_v != 0) {
		curr_bit = curr_v % 2;
		if(curr_2_p == 1) {
			if(curr_cache->find(curr_2_p) == curr_cache->end()) {
				(*curr_cache)[curr_2_p] = shared_X;				
			}
			tmp_2_p[0] = shared_X;
		} else {
			mpc_t tmp_v;
			mpc_pow_const(shared_X, curr_2_p / 2, tmp_v, curr_cache);
			tmp_2_p[0] = tmp_v;
			GetMpcOpInner(DotProduct)->Run(tmp_2_p, tmp_2_p, tmp_new_y, 1);
			(*curr_cache)[curr_2_p] = tmp_new_y[0];		
			tmp_2_p = tmp_new_y;
		}
		if (curr_bit) {
			GetMpcOpInner(DotProduct)->Run(tmp_2_p, tmp_y, tmp_new_y, 1);	
			tmp_y = tmp_new_y;
		}
   		curr_v = mpc_t(curr_v / 2);
    	curr_2_p = 2 * curr_2_p; 
    }
	shared_Y = tmp_y[0];
    return;
}

void Polynomial::local_const_mul(const vector<mpc_t>& shared_X,
    mpc_t common_V,
    vector<mpc_t>& shared_Y){
    int vec_size = shared_X.size();
    shared_Y.resize(vec_size);
    if (PRIMARY) {
    	for(int i = 0; i < vec_size; ++i) {
          	shared_Y[i] = common_V * shared_X[i];
          	funcTruncateElem2PC(shared_Y[i], FLOAT_PRECISION_M, PARTY_A, PARTY_B); 
        }
    }
}

void Polynomial::mpc_uni_polynomial(const vector<mpc_t>& shared_X, 
					const vector<mpc_t>& common_power_list,
					const vector<mpc_t>& common_coff_list,
					vector<mpc_t>& shared_Y) {
    int vec_size = shared_X.size();
	shared_Y.resize(vec_size);
	// Step one:
	unordered_map<mpc_t, vector<mpc_t>> local_cache(common_power_list.size());
	local_cache[1] = shared_X;
	vector<mpc_t> local_value(vec_size, 0);
	for(auto i = 0; i < common_power_list.size(); ++i) {
		//cout << "power" << i << endl;
		vector<mpc_t> tmp_coff_vec(vec_size, common_coff_list[i]);
		vector<mpc_t> tmp_prod(vec_size);
		if (common_power_list[i] == 0) {
			if(partyNum == PARTY_A) {
				addVectors(local_value, tmp_coff_vec, local_value, vec_size);
			}
		} else if (common_power_list[i] == 1) {
			if (PRIMARY) {
				// local const multiply
				mpc_t coff_v = common_coff_list[i];
				local_const_mul(shared_X, coff_v, tmp_prod);
				addVectors(local_value, tmp_prod, local_value, vec_size);
			}
		} else {
			vector<mpc_t> term_v(vec_size);
			mpc_t curr_k = common_power_list[i];
		    mpc_pow_const(shared_X, curr_k, term_v, &local_cache);
			if (PRIMARY) { 
				// local const multiply
				mpc_t coff_v = common_coff_list[i];
				local_const_mul(term_v, common_coff_list[i], tmp_prod);
				addVectors(local_value, tmp_prod, local_value, vec_size);
			}
		}
	}
	shared_Y = local_value;
}

void Polynomial::mpc_uni_polynomial(const mpc_t& shared_X, 
					const vector<mpc_t>& common_power_list,
					const vector<mpc_t>& common_coff_list,
					mpc_t& shared_Y){
    // Step one: 
	unordered_map<mpc_t, mpc_t> local_cache(common_power_list.size());
	local_cache[1] = shared_X;
	mpc_t local_value = 0;
	for(auto i = 0; i < common_power_list.size(); ++i) {
		//cout << "power" << i << endl;
		vector<mpc_t> tmp_prod(1);
		if (common_power_list[i] == 0) {
			if(partyNum == PARTY_A) {
				local_value += common_coff_list[i];
			} 
		} else if (common_power_list[i] == 1) {
			if (PRIMARY) { 
				mpc_t coff_v = common_coff_list[i];
				// local const multiply
				mpc_t term_v = shared_X * coff_v;
				tmp_prod[0] = term_v;
				funcTruncate2PC(tmp_prod, FLOAT_PRECISION_M, 1, PARTY_A, PARTY_B);
				local_value += tmp_prod[0];
			} 
		} else {
			mpc_t term_v;
			mpc_t curr_k = common_power_list[i];
		    mpc_pow_const(shared_X, curr_k, term_v, &local_cache);
			if (PRIMARY) { 
				// local const multiply
				term_v = term_v * common_coff_list[i];
				tmp_prod[0] = term_v;
				funcTruncate2PC(tmp_prod, FLOAT_PRECISION_M, 1, PARTY_A, PARTY_B);
				local_value += tmp_prod[0]; 
			}
		}	
	}
	shared_Y = local_value;
}

} // namespace mpc
} // namespace rosetta
