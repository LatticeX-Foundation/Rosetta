#ifndef EDABITS_H__
#define EDABITS_H__

#include "emp-tool/emp-tool.h"
#include "emp-ot/emp-ot.h"
#include "emp-zk/emp-zk-bool/emp-zk-bool.h"
#include "emp-zk/emp-vole/emp-vole.h"
#include "emp-zk/edabit/auth_helper.h"

template<typename IO>
class EdaBits {
public:
	static EdaBits<IO> *conv;
	int party;
	IO **ios;

	block delta_f2;
	Integer *bool_candidate = nullptr;
	__uint128_t delta_fp;
	__uint128_t *arith_candidate = nullptr;
	
	VoleTriple<IO> *cot_fp = nullptr;
	
	DoubAuthHelper<IO> *auth_helper = nullptr;

	uint32_t np_pt, np_rg, np_sz;
	uint32_t rand_pt;

	uint32_t edabit_num, edabit_offset;

	const uint32_t N = 800000, B = 2, C = 2; // B=2, C=2, N>=741455
						 // B=3, C=3, N>=6251
	uint32_t ell;
	uint32_t Bm1, ell_faulty;

	Integer int_boo_pr, int_boo_zero, int_boo_pr_plus_two;

	EdaBits(int party, int threads, IO **ios, VoleTriple<IO> *cot_fp) {
		this->party = party;
		this->ios = ios;
		this->cot_fp = cot_fp;
		if(party == BOB) {
			this->delta_fp = cot_fp->delta();
		}

		this->np_sz = cot_fp->ot_limit;
		this->np_pt = 0;
		this->np_rg = 0;
		this->edabit_offset = 0;
		this->rand_pt = 0;
		this->edabit_num = 0;
		arith_candidate = new __uint128_t[cot_fp->n];
		cot_fp->extend_inplace(arith_candidate, cot_fp->n); 	// check the size

		this->ell = B * N + C; 		// batch size
		this->ell_faulty = ell - N;
		this->Bm1 = B - 1;
		bool_candidate = new Integer[ell];

		auth_helper = new DoubAuthHelper<IO>(party, ios[0]);

		int_boo_pr = Integer(62, PR, PUBLIC);
		int_boo_zero = Integer(62, 0, PUBLIC);
		int_boo_pr_plus_two = Integer(62, PR+2, PUBLIC); 	// TODO why????
	}

	~EdaBits() {
		if(!auth_helper->triple_equality_check())
			error("cut and choose fails");
		if(bool_candidate != nullptr) delete[] bool_candidate;
		if(arith_candidate != nullptr) delete[] arith_candidate;
		if(auth_helper != nullptr) delete auth_helper;
	}

	void install_boolean(block delta_f2) {
		this->delta_f2 = delta_f2;
		auth_helper->set_delta(delta_f2, delta_fp);
	}

	void edabits_gen_backend() {
		//auto start = clock_start();
		// If the buffer is used up, refill the Fp shares
		if(np_pt + ell > np_sz) {
			cot_fp->extend_inplace(arith_candidate, cot_fp->n);
			np_pt = 0;
		}
		np_rg = np_pt + ell;
		
		// Input \ell Fp shares into boolean circuits
		if(party == ALICE) { 
			for(uint32_t i = 0; i < ell; ++i)
				bool_candidate[i] = Integer(62, _mm_extract_epi64((block)arith_candidate[np_pt+i], 1), ALICE);
		} else {
			for(uint32_t i = 0; i < ell; ++i)
				bool_candidate[i] = Integer(62, 0, ALICE);
		}
		((ZKBoolCircExecPrv<IO>*)CircuitExecution::circ_exec)->ostriple->io->flush();

		// Generate a random point to do the permutation
		rand_pt = random_point(ell_faulty);
		
		// Open S_o TODO overflow
		if(party == ALICE)
			auth_helper->open_check_send(bool_candidate+N+rand_pt, arith_candidate+np_pt+N+rand_pt, C);
		else auth_helper->open_check_recv(bool_candidate+N+rand_pt, arith_candidate+np_pt+N+rand_pt, C);

		// bucketing
		uint32_t buc_start = fp_index(np_pt + N + rand_pt + C);
		uint32_t buc_start1 = f2_index(rand_pt + N + C);
		__uint128_t *fp_to_check = new __uint128_t[N];
		Integer *f2_to_check = new Integer[N];
		for(uint32_t j = 0; j < Bm1; ++j) { // TODO parameter
			uint32_t ifp0 = np_pt;
			uint32_t ifp1 = fp_index(buc_start+j);
			uint32_t if21 = f2_index(buc_start1+j);
			for(uint32_t i = 0; i < N; ++i) {
				fp_to_check[i] = intfp_add(arith_candidate[ifp0++], arith_candidate[ifp1]);
				ifp1 = fp_index(ifp1+Bm1);
				f2_to_check[i] = bool_candidate[i] + bool_candidate[if21];
				f2_to_check[i] = f2_to_check[i].select(f2_to_check[i].bits[61], f2_to_check[i]+int_boo_pr_plus_two);
				if21 = f2_index(if21+Bm1);
				// TODO boolean addition and selection costs a lot, and it should be subtraction
			}
			((ZKBoolCircExecPrv<IO>*)CircuitExecution::circ_exec)->ostriple->io->flush();
			if(party == ALICE) auth_helper->open_check_send(f2_to_check, fp_to_check, N);
			else auth_helper->open_check_recv(f2_to_check, fp_to_check, N);
		}
		if(!auth_helper->triple_equality_check())
			error("cut and choose fails");
		edabit_num = N;
		edabit_offset = 0;
		delete[] fp_to_check;
		delete[] f2_to_check;
		//std::cout << "edabits generation: " << time_from(start)/N << " us/edabits" << std::endl;
	}

	__uint128_t bool2arith(Integer in) {
		uint32_t edab_f2, edab_fp;
		uint64_t diff;
		Integer diff_bool;

		next_edabits(edab_f2, edab_fp);
		diff_bool = in - bool_candidate[edab_f2];
		diff_bool = diff_bool.select(diff_bool.bits[61], diff_bool + int_boo_pr);

		if(party == ALICE) auth_helper->open_check_send(&diff, &diff_bool, 1);
		else auth_helper->open_check_recv(&diff, &diff_bool, 1);
	 	return intfp_add_const(arith_candidate[edab_fp], diff);
	}

	void bool2arith(__uint128_t *out, Integer *in, size_t len) {
		int counter = 0;
		int round1_num, round, leftover;
		if(len <= edabit_num) {
			round = 1;
			round1_num = len;
			leftover = 0;
		} else {
			round1_num = edabit_num;
			round = (len-edabit_num) / N + 2;
			leftover = (len-edabit_num) % N;
			if(leftover == 0) {round--; leftover = N;}
			if(!edabit_num) {round--; round1_num = N; if(round == 1) round1_num = leftover;}
		}
		for(int j = 0; j < round; ++j) {
			int num = N;
			if(j == round - 1) num = leftover;
			if(j == 0) num = round1_num;

			uint32_t edab_f2;
			uint32_t *edab_fp = new uint32_t[num];
			uint64_t *diff = new uint64_t[num];
			Integer *diff_bool = new Integer[num];
			for(int i = 0; i < num; ++i) {
				next_edabits(edab_f2, edab_fp[i]);
				diff_bool[i] = in[counter+i] - bool_candidate[edab_f2];
				diff_bool[i] = diff_bool[i].select(diff_bool[i].bits[61], diff_bool[i] + int_boo_pr);
			}
			if(party == ALICE) auth_helper->open_check_send(diff, diff_bool, num);
			else auth_helper->open_check_recv(diff, diff_bool, num);
			ios[0]->flush();
			for(int i = 0; i < num; ++i)
				out[counter+i] = intfp_add_const(arith_candidate[edab_fp[i]], diff[i]);
			delete[] edab_fp;
			delete[] diff;
			delete[] diff_bool;
			counter += num;
		}
	}

	Integer arith2bool(__uint128_t in) {
		uint32_t edab_fp, edab_f2;
		__uint128_t sum_fp;
		uint64_t sum;
		next_edabits(edab_f2, edab_fp);

		sum_fp = intfp_add(arith_candidate[edab_fp], in);
		if(party == ALICE) auth_helper->open_check_send(&sum, &sum_fp, 1);
		else auth_helper->open_check_recv(&sum, &sum_fp, 1);

		Integer sum_boo = Integer(62, sum, PUBLIC);
		sum_boo = sum_boo - bool_candidate[edab_f2];
		return sum_boo.select(sum_boo.bits[61], sum_boo + int_boo_pr);
	}

	void arith2bool(Integer *out, __uint128_t *in, size_t len) {
		int counter = 0;
		int round1_num, round, leftover;
		if(len <= edabit_num) {
			round = 1;
			round1_num = len;
			leftover = 0;
		} else {
			round1_num = edabit_num;
			round = (len-edabit_num) / N + 2;
			leftover = (len-edabit_num) % N;
			if(leftover == 0) {round--; leftover = N;}
			if(!edabit_num) {round--; round1_num = N; if(round == 1) round1_num = leftover;}
		}

		for(int j = 0; j < round; ++j) {
			int num = N;
			if(j == round - 1) num = leftover;
			if(j == 0) num = round1_num;

			uint32_t edab_fp;
			uint32_t *edab_f2 = new uint32_t[num];
			__uint128_t *sum_fp = new __uint128_t[num];
			uint64_t *sum = new uint64_t[num];
			for(int i = 0; i < num; ++i) {
				next_edabits(edab_f2[i], edab_fp);
				sum_fp[i] = intfp_add(arith_candidate[edab_fp], in[counter+i]);
			}
			if(party == ALICE) auth_helper->open_check_send(sum, sum_fp, num);
			else auth_helper->open_check_recv(sum, sum_fp, num);
			ios[0]->flush();
			for(int i = 0; i < num; ++i) {
				Integer sum_boo = Integer(62, sum[i], PUBLIC);
				sum_boo = sum_boo - bool_candidate[edab_f2[i]];
				out[counter+i] = sum_boo.select(sum_boo.bits[61], sum_boo + int_boo_pr);
			}
			delete[] edab_f2;
			delete[] sum_fp;
			delete[] sum;
			counter += num;
		}
	}

	uint32_t random_point(uint32_t range) {
		uint32_t rand_pt = 0;
		if(party == ALICE) {
			ios[0]->recv_data(&rand_pt, sizeof(uint32_t));
		} else {
			PRG prg;
			prg.random_data(&rand_pt, sizeof(uint32_t));
			rand_pt = rand_pt % range;
			ios[0]->send_data(&rand_pt, sizeof(uint32_t));
			ios[0]->flush();
		}
		return rand_pt;
	}

	uint32_t fp_index(uint32_t offset) {
		if(offset >= np_rg) offset -= (ell_faulty);
		return offset;
	}

	uint32_t f2_index(uint32_t offset) {
		if(offset >= ell) offset -= (ell_faulty);
		return offset;
	}

	void next_edabits(uint32_t &f2_indexin, uint32_t &fp_index) {
		if(edabit_num == 0) {
			np_pt = np_rg;
			edabits_gen_backend();
		}
		f2_indexin = edabit_offset;
		fp_index = np_pt + edabit_offset;
		edabit_offset++;
		edabit_num--;
	}

	__uint128_t intfp_add_const(__uint128_t a, uint64_t b) {
		if(party == ALICE) {
			uint64_t high = _mm_extract_epi64((block)a, 1);
			uint64_t low = _mm_extract_epi64((block)a, 0);
			high = add_mod(high, b);
			return (__uint128_t)makeBlock(high, low);
		} else {
			uint64_t low = _mm_extract_epi64((block)a, 0);
			b = mult_mod(b, (uint64_t)delta_fp);
			b = PR - b;
			return (__uint128_t)makeBlock(0x0LL, add_mod(low, b));
		}
	}

	__uint128_t intfp_add(__uint128_t a, __uint128_t b) {
		if(party == ALICE) {
			uint64_t high = add_mod(_mm_extract_epi64((block)a, 1), _mm_extract_epi64((block)b, 1));
			uint64_t low = add_mod(_mm_extract_epi64((block)a, 0), _mm_extract_epi64((block)b, 0));
			return (__uint128_t)makeBlock(high, low);
		} else {
			return (__uint128_t)makeBlock(0x0LL, add_mod(_mm_extract_epi64((block)a, 0), _mm_extract_epi64((block)b, 0)));
		}
	}

	// DEBUG
	void sender_check_edabits(uint32_t edab_f2, uint32_t edab_fp) {
		if(party == ALICE) {
			uint64_t a = sender_check_int_value(bool_candidate[edab_f2]);
			uint64_t b = sender_check_int_value(arith_candidate[edab_fp]);
			if(a != b) error("edabit error!");
		}
	}

	bool sender_check_conversion(Integer in2, __uint128_t inp) {
		if(party == ALICE) {
			uint64_t a = sender_check_int_value(in2);
			assert(a < PR);
			uint64_t b = sender_check_int_value(inp);
			assert(b < PR);
			if(a != b) {
				return false;
			}
		}
		return true;
	}

	uint64_t sender_check_int_value(Integer in) {
		std::bitset<64> val = 0;
		int bit_len = in.size();
		for(int i = 0; i < bit_len; ++i)
			val.set(i, getLSB(in.bits[i].bit));
		return val.to_ullong();
	}

	uint64_t sender_check_int_value(__uint128_t in) {
		return _mm_extract_epi64((block)in, 1);
	}
};
template<typename IO>
EdaBits<IO>* EdaBits<IO>::conv = nullptr;
#endif
