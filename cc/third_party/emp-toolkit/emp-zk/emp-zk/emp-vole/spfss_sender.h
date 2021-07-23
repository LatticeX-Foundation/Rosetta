#ifndef SPFSS_SENDER_FP_H__
#define SPFSS_SENDER_FP_H__
#include <iostream>
#include <emp-tool/emp-tool.h>
#include <emp-ot/emp-ot.h>
#include "emp-zk/emp-vole/utility.h"

using namespace emp;

template<typename IO>
class SpfssSenderFp { public:
	block seed;
	block *ggm_tree, *m;
	__uint128_t delta;
	uint64_t secret_sum;
	IO *io;
	int depth;
	int leave_n;
	PRG prg;

	SpfssSenderFp(IO *io, int depth_in) {
		initialization(io, depth_in);
		prg.random_block(&seed, 1);
	}

	void initialization(IO *io, int depth_in) {
		this->io = io;
		this->depth = depth_in;
		this->leave_n = 1<<(this->depth-1);
		m = new block[(depth-1)*2];
	}

	~SpfssSenderFp() {
		delete[] m;
	}

	// send the nodes by oblivious transfer
	void compute(__uint128_t* ggm_tree_mem, __uint128_t secret, __uint128_t gamma) {
		this->delta = secret;
		ggm_tree_gen(m, m+depth-1, ggm_tree_mem, secret, gamma);
	}

	// send the nodes by oblivious transfer
	template<typename OT>
	void send(OT * ot, IO * io2, int s) {
		ot->send(m, &m[depth-1], depth-1, io2, s);
		io2->send_data(&secret_sum, sizeof(uint64_t));
		io2->flush();
	}

	// generate GGM tree from the top
	void ggm_tree_gen(block *ot_msg_0, block *ot_msg_1, __uint128_t* ggm_tree_mem, 
			__uint128_t secret, __uint128_t gamma) {
		this->ggm_tree = (block*)ggm_tree_mem;
		TwoKeyPRP *prp = new TwoKeyPRP(zero_block, makeBlock(0, 1));
		prp->node_expand_1to2(ggm_tree, seed);
		ot_msg_0[0] = ggm_tree[0];
		ot_msg_1[0] = ggm_tree[1];
		for(int h = 1; h < depth-1; ++h) {
			ot_msg_0[h] = ot_msg_1[h] = zero_block;
			int sz = 1<<h;
			for(int i = sz-2; i >=0; i-=2) {
				prp->node_expand_2to4(&ggm_tree[i*2], &ggm_tree[i]);
				ot_msg_0[h] = ot_msg_0[h] ^ ggm_tree[i*2];
				ot_msg_0[h] = ot_msg_0[h] ^ ggm_tree[i*2+2];
				ot_msg_1[h] = ot_msg_1[h] ^ ggm_tree[i*2+1];
				ot_msg_1[h] = ot_msg_1[h] ^ ggm_tree[i*2+3];
			}
		}
		delete prp;
		secret_sum = (uint64_t)0;
		for(int i = 0; i < leave_n; ++i) {
			extract_fp(ggm_tree_mem[i]);
			secret_sum = add_mod(secret_sum, (uint64_t)ggm_tree_mem[i]);
		}
		secret_sum = PR - secret_sum;
		secret_sum = add_mod((uint64_t)gamma, secret_sum);
	}

	// consistency check: Protocol PI_spsVOLE
	void consistency_check(IO *io2, __uint128_t y) {
		__uint128_t *chi = new __uint128_t[leave_n];
		Hash hash;
		__uint128_t digest = mod(_mm_extract_epi64(hash.hash_for_block(&secret_sum, sizeof(uint64_t)), 0));
		uni_hash_coeff_gen(chi, digest, leave_n);

		// receive the x_star
		// Y = y_star = y + x_star * delta
		__uint128_t y_star, x_star;
		io2->recv_data(&x_star, sizeof(__uint128_t));
		__uint128_t tmp = mod(x_star * delta, pr);
		tmp = pr - tmp;
		y_star = mod(y + tmp, pr);

		// V = \sum{chi_i*v_i} - Y
		__uint128_t V = vector_inn_prdt_sum_red(chi, (__uint128_t*)ggm_tree, leave_n);

		y_star = pr - y_star;
		V = mod(V + y_star, pr);

		io2->send_data(&V, sizeof(__uint128_t));
		io2->flush();

		delete[] chi;
	}

	// consistency check: Protocol PI_spsVOLE
	void consistency_check_msg_gen(__uint128_t& V, IO *io2) {
		block seed;
		io2->recv_data(&seed, sizeof(block));
		__uint128_t *chi = new __uint128_t[leave_n];
		Hash hash;
		__uint128_t digest = mod(_mm_extract_epi64(hash.hash_for_block(&seed, sizeof(block)), 0));
		uni_hash_coeff_gen(chi, digest, leave_n);

		// V = \sum{chi_i*v_i}
		V = vector_inn_prdt_sum_red(chi, (__uint128_t*)ggm_tree, leave_n);

		delete[] chi;
	}

};

#endif
