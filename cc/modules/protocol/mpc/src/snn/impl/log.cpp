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
#include "op_impl.h"

#include <cmath>
#include <unordered_map>
#include <string>
#include <vector>
using namespace std;

// clang-format off

/*
    Apprximating logarithm functionality with different polynomials
*/
namespace rosetta {
namespace mpc { 

// TODO: this is ugly now, we should make this as a singleton class and refactor this.
struct MPC_LOG_CONFIG {
public:
	unordered_map<string, vector< vector<mpc_t>>> FUNCS_MAP;
	static const std::vector<std::vector<mpc_t>>  FUNC_APPROX_LOG_OPTION_HD;
	static const std::vector<std::vector<mpc_t>>  FUNC_APPROX_LOG_OPTION_A;
	static const std::vector<std::vector<mpc_t>> FUNC_APPROX_LOG_OPTION_B_SEGMENTS;
	static const std::vector<std::vector<mpc_t>>  FUNC_APPROX_LOG_OPTION_B_1;
	static const std::vector<std::vector<mpc_t>>  FUNC_APPROX_LOG_OPTION_B_2;
	static const std::vector<std::vector<mpc_t>>  FUNC_APPROX_LOG_OPTION_B_3;	

	// this is customized for approximating 
	//static const std::vector<std::vector<mpc_t>>  FUNC_APPROX_LOG_CE;
};

///////*****************LOG INTPRETATION ****************
/// OPTION for general High Resolution%
/// this is for [0.5, 1]
const std::vector<std::vector<mpc_t>> MPC_LOG_CONFIG::FUNC_APPROX_LOG_OPTION_HD = \
				{{0, CoffUp(-2.6145621)},
				{1, CoffUp(6.92508636)},
				{2, CoffUp(-9.48726206)},
				{3, CoffUp(8.57287151)},
				{4, CoffUp(-4.31242379)},
				{5, CoffUp(0.91630145)}};

/// OPTION A: single polynomial
/// This apprximation is best for x \in [0.3, 1.8)
const std::vector<std::vector<mpc_t>> MPC_LOG_CONFIG::FUNC_APPROX_LOG_OPTION_A = \
				{{0, CoffUp(-3.35674972)},
				{1, CoffUp(12.79333646)},
				{2, CoffUp(-26.18955259)},
				{3, CoffUp(30.24596692)},
				{4, CoffUp(-17.30367641)},
				{5, CoffUp(3.82474222)}};

const std::vector<std::vector<mpc_t>> MPC_LOG_CONFIG::FUNC_APPROX_LOG_OPTION_B_SEGMENTS = \
				{{}};
/// OPTION B: 3-segement polynomial
// This approximation is best for x in [1.2, 10]
const std::vector<std::vector<mpc_t>> MPC_LOG_CONFIG::FUNC_APPROX_LOG_OPTION_B_3 = \
				{{0, CoffUp(-0.147409486)},
				{1, CoffUp(0.463403306)},
				{2, CoffUp(-0.022636005)}};

// This approximation is best for x in [0.05, 1.2]
const std::vector<std::vector<mpc_t>> MPC_LOG_CONFIG::FUNC_APPROX_LOG_OPTION_B_2 = \
				{{0, CoffUp(-3.0312942)},
				{1, CoffUp(8.253302766)},
				{2, CoffUp(-8.668159044)},
				{3, CoffUp(3.404663323)}};

// This approximation is best for x in [0.0001, 0.05]
const std::vector<std::vector<mpc_t>> MPC_LOG_CONFIG::FUNC_APPROX_LOG_OPTION_B_1 = \
			{{0, CoffUp(-6.805568387)},
			{1, CoffUp(284.0022382)},
			{2, CoffUp(-8360.491679)},
			{3, CoffUp(85873.96716)}};

struct LogFuncRegistrar {
    LogFuncRegistrar() {
		ConstPolynomial	log_default_appro_poly = ConstPolynomial(0, 0, 
												MPC_LOG_CONFIG::FUNC_APPROX_LOG_OPTION_HD);
		vector<ConstPolynomial>* log_default_vec = new vector<ConstPolynomial>({log_default_appro_poly});
		PolyConfFactory::func_register(string("LOG_HD"), log_default_vec);

		ConstPolynomial	log_v1_appro_poly = ConstPolynomial(0, 0, 
												MPC_LOG_CONFIG::FUNC_APPROX_LOG_OPTION_A);
		vector<ConstPolynomial>* log_v1_vec = new vector<ConstPolynomial>({log_v1_appro_poly});
		PolyConfFactory::func_register(string("LOG_V1"), log_v1_vec);
		// Note: Attention! though the the result may become worse when X < 0.0001.
		//		In Machine Laerning, It will be better tto recap it.
		ConstPolynomial	log_v2_appro_poly_1 = ConstPolynomial(FloatToMpcType(0.0001), 
															FloatToMpcType(0.05), 
												MPC_LOG_CONFIG::FUNC_APPROX_LOG_OPTION_B_1);

		ConstPolynomial	log_v2_appro_poly_2 = ConstPolynomial(FloatToMpcType(0.05), 
															FloatToMpcType(1.2), 
												MPC_LOG_CONFIG::FUNC_APPROX_LOG_OPTION_B_2);
		// Note: Attention! though the the result may become worse when X > 10.
		//		It will be better than ZERO. So we set it to be a VERY BIG NUMBER!
		ConstPolynomial	log_v2_appro_poly_3 = ConstPolynomial(FloatToMpcType(1.2), 
															FloatToMpcType(10000), // 2^40
												MPC_LOG_CONFIG::FUNC_APPROX_LOG_OPTION_B_3);
		vector<ConstPolynomial>* log_v2_vec = new vector<ConstPolynomial>({log_v2_appro_poly_1,
																		log_v2_appro_poly_2,
																		log_v2_appro_poly_3});
		PolyConfFactory::func_register(string("LOG_V2"), log_v2_vec);
    }
};

static LogFuncRegistrar log_func_registrar;

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
		for(int i = 0; i < LEN - FLOAT_PRECISION; ++i) {
			auto curr_x_minus_one = curr_x;
			vector<mpc_t> shared_cmp(vec_size, 0);
			
			subtractVectors<mpc_t>(curr_x, SHARED_ONE, curr_x_minus_one, vec_size);
			
			// funcRELUPrime3PC(curr_x_minus_one, shared_cmp, vec_size);
			GetMpcOpInner(ReluPrime)->Run3PC(curr_x_minus_one, shared_cmp, vec_size);

			// selection based on x - 1 > 0
			//mpc_select_1_of_2(SHARED_HALF, SHARED_ONE, shared_cmp,
			// 					shared_val_multiplier, vec_size);
			GetMpcOpInner(Select1Of2)->Run(SHARED_HALF, SHARED_ONE, shared_cmp,
			 					shared_val_multiplier, vec_size);
			//mpc_select_1_of_2(SHARED_ONE, SHARED_ZERO, shared_cmp,
			//					shared_power_add, vec_size);
			GetMpcOpInner(Select1Of2)->Run(SHARED_ONE, SHARED_ZERO, shared_cmp,
								shared_power_add, vec_size);
			vector<mpc_t> tmp_vec(vec_size, 0);
			//funcDotProductMPC(curr_x, shared_val_multiplier, tmp_vec, vec_size);
			GetMpcOpInner(DotProduct)->Run(curr_x, shared_val_multiplier, tmp_vec, vec_size);
			curr_x = tmp_vec;
			addVectors<mpc_t>(curr_power, shared_power_add, tmp_vec, vec_size);
			curr_power = tmp_vec;
		}
		// if(PRIMARY) {
		// 	GetMpcOpInner(Reconstruct2PC)->Run(curr_x, curr_x.size(), "curr_x");
		// 	GetMpcOpInner(Reconstruct2PC)->Run(curr_power, curr_power.size(), "curr power");
		// }
		// 1.2 scale up fractional part
		for(int i = 0; i < FLOAT_PRECISION; ++i) {
			auto curr_x_minus_half = curr_x;
			vector<mpc_t> shared_cmp(vec_size, 0);
			subtractVectors<mpc_t>(curr_x, SHARED_HALF, curr_x_minus_half, vec_size);
		
			//funcRELUPrime3PC(curr_x_minus_half, shared_cmp, vec_size);
			GetMpcOpInner(ReluPrime)->Run3PC(curr_x_minus_half, shared_cmp, vec_size);
			// selection based on x - 1 > 0
			//mpc_select_1_of_2(SHARED_ONE, SHARED_TWO, shared_cmp,
			// 					shared_val_multiplier, vec_size);
			GetMpcOpInner(Select1Of2)->Run(SHARED_ONE, SHARED_TWO, shared_cmp,
								shared_val_multiplier, vec_size);
			//mpc_select_1_of_2(SHARED_ZERO, SHARED_NEG_ONE, shared_cmp,
			//					shared_power_add, vec_size);
			GetMpcOpInner(Select1Of2)->Run(SHARED_ZERO, SHARED_NEG_ONE, shared_cmp,
								shared_power_add, vec_size);

			vector<mpc_t> tmp_vec(vec_size, 0);
			//funcDotProductMPC(curr_x, shared_val_multiplier, tmp_vec, vec_size);
			GetMpcOpInner(DotProduct)->Run(curr_x, shared_val_multiplier, tmp_vec, vec_size);
			curr_x = tmp_vec;
			addVectors<mpc_t>(curr_power, shared_power_add, tmp_vec, vec_size);
			curr_power = tmp_vec;
		}
		// if(PRIMARY) {
		// 	cout << "After fractional part:" << endl;
		// 	GetMpcOpInner(Reconstruct2PC)->Run(curr_x, curr_x.size(), "curr_x");
		// 	GetMpcOpInner(Reconstruct2PC)->Run(curr_power, curr_power.size(), "curr power");
		// }

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
		// for (auto i = 0; i< power_list.size(); i++) {
		// 	cout << MpcTypeToFloat(coff_list[i]) << "*X^" << power_list[i] << " + ";
		// } 

		// TODO: vectorization this func
		vector<mpc_t> shared_basic_val(vec_size, 0);
		mpc_t tmp_v = 0;
		for(int i = 0; i < vec_size; ++i) {
			auto& each_curr_x = curr_x[i]; 
			//mpc_uni_polinomial(each_curr_x, power_list, coff_list, tmp_v);
			GetMpcOpInner(Polynomial)->mpc_uni_polynomial(each_curr_x, power_list, coff_list, tmp_v);
			shared_basic_val[i] = CoffDown(tmp_v);
		}
		// if(PRIMARY) {
		// 	cout << "LOG basic val result:" << endl;
		// 	GetMpcOpInner(Reconstruct2PC)->Run(curr_x, curr_x.size(), "curr_x");
		// 	GetMpcOpInner(Reconstruct2PC)->Run(shared_basic_val, shared_basic_val.size(), "curr power");
		// }

		vector<mpc_t> shared_high_part(vec_size, 0);
		//funcDotProductMPC(curr_power, SHARED_LN_2, shared_high_part, vec_size);
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
	// for (auto i = 0; i< power_list.size(); i++) {
	// 	cout << MpcTypeToFloat(coff_list[i]) << "*X^" << power_list[i] << " + ";
	// } 
	//mpc_uni_polinomial(shared_X, power_list, coff_list, shared_Y);
	GetMpcOpInner(Polynomial)->mpc_uni_polynomial(shared_X, power_list, coff_list, shared_Y);
	shared_Y = CoffDown(shared_Y);
};

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
		// cout << "Seg " << i << ":[" << MpcTypeToFloat(seg_init) <<
		// 			", " << MpcTypeToFloat(seg_end) << "): ";
		// for(auto i = 0; i< curr_power_list.size(); i++) {
		// cout << MpcTypeToFloat(curr_coff_list[i]) << "*X^" << 
		// 			curr_power_list[i] << " + ";
		// }
		// cout << endl;
		// S1: use ReLUPrime to get whether to use this segemnt[ multiplier is 0 or 1]
		/// 1.1 check start point
		// x >= start && (1 - (x >= end))
		shared_cmp[0] = shared_X;
		shared_cmp[1] = shared_X;
		if(partyNum == PARTY_A) {
			shared_cmp[0] = shared_X - seg_init;
			shared_cmp[1] = shared_X - seg_end; 
		}
		vector<mpc_t> tmp_reluprime(2, 0);
		//funcRELUPrime3PC(shared_cmp, tmp_reluprime, 2);		
		GetMpcOpInner(ReluPrime)->Run3PC(shared_cmp, tmp_reluprime, 2);
		if(partyNum == PARTY_A) {
			tmp_reluprime[1] = FloatToMpcType(1) - tmp_reluprime[1];
		} else {
			tmp_reluprime[1] = -tmp_reluprime[1];
		}
		shared_init[0] = tmp_reluprime[0];
		shared_end[0] = tmp_reluprime[1];
		//funcDotProductMPC(shared_init, shared_end, shared_res, 1);
		GetMpcOpInner(DotProduct)->Run(shared_init, shared_end, shared_res, 1);
		// if(PRIMARY) {
		// 	//funcReconstruct2PC(tmp_reluprime, 1, "BOOL in this seg start");	
		// 	funcReconstruct2PC(shared_init, 1, "BOOL in this seg start");
		// 	funcReconstruct2PC(shared_end, 1, "BOOL in this seg end");
		// 	funcReconstruct2PC(shared_res, 1, "BOOL in this seg");
		// }
		// S2: compute the value in this segment
		mpc_t curr_shared_Y = 0;	
		//mpc_uni_polinomial(shared_X, curr_power_list, curr_coff_list, curr_shared_Y);
		GetMpcOpInner(Polynomial)->mpc_uni_polynomial(shared_X, curr_power_list, curr_coff_list, curr_shared_Y);
		shared_init[0] = CoffDown(curr_shared_Y);
		shared_end[0] = shared_res[0];
		//funcDotProductMPC(shared_init, shared_end, shared_res, 1);	
		GetMpcOpInner(DotProduct)->Run(shared_init, shared_end, shared_res, 1);	
		shared_Y += shared_res[0];
		// if(PRIMARY) {
		// 	//funcReconstruct2PC(tmp_reluprime, 1, "BOOL in this seg start");
		// 	vector<mpc_t> result(1, shared_Y);	
		// 	funcReconstruct2PC(result, 1, "Log result in this seg");
		// }
    }
}

}}

// clang-format on
