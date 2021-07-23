#ifndef SPCOT_RECVER_H__
#define SPCOT_RECVER_H__
#include <iostream>
#include "emp-tool/emp-tool.h"
#include "emp-ot/emp-ot.h"
#include "emp-ot/ferret/twokeyprp.h"

using namespace emp;

template<typename IO>
class SPCOT_Recver {
public:
	block *ggm_tree, *m;
	bool *b;
	int choice_pos, depth, leave_n;
	IO *io;

	block secret_sum_f2;

	SPCOT_Recver(IO *io, int depth_in) {
		this->io = io;
		this->depth = depth_in;
		this->leave_n = 1<<(depth_in-1);
		m = new block[depth-1];
		b = new bool[depth-1];
	}

	~SPCOT_Recver(){
		delete[] m;
		delete[] b;
	}

	int get_index() {
		choice_pos = 0;
		for(int i = 0; i < depth-1; ++i) {
			choice_pos<<=1;
			if(!b[i])
				choice_pos +=1;
		}
		return choice_pos;
	}

	// receive the message and reconstruct the tree
	// j: position of the secret, begins from 0
	template<typename OT>
	void recv_f2k(OT * ot, IO * io2, int s) {
		ot->recv(m, b, depth-1, io2, s);
		io2->recv_data(&secret_sum_f2, sizeof(block));
	}

	// receive the message and reconstruct the tree
	// j: position of the secret, begins from 0
	void compute(block* ggm_tree_mem) {
		this->ggm_tree = ggm_tree_mem;
		ggm_tree_reconstruction(b, m);
		ggm_tree[choice_pos] = zero_block;
		block nodes_sum = zero_block;
		block one = makeBlock(0xFFFFFFFFFFFFFFFFLL,0xFFFFFFFFFFFFFFFELL);
		for(int i = 0; i < leave_n; ++i) {
			ggm_tree[i] = ggm_tree[i] & one;
			nodes_sum = nodes_sum ^ ggm_tree[i];
		}
		ggm_tree[choice_pos] = nodes_sum ^ secret_sum_f2;
	}

	void ggm_tree_reconstruction(bool *b, block *m) {
		int to_fill_idx = 0;
		TwoKeyPRP prp(zero_block, makeBlock(0, 1));
		for(int i = 1; i < depth; ++i) {
			to_fill_idx = to_fill_idx * 2;
			ggm_tree[to_fill_idx] = ggm_tree[to_fill_idx+1] = zero_block;
			if(b[i-1] == false) {
				layer_recover(i, 0, to_fill_idx, m[i-1], &prp);
				to_fill_idx += 1;
			} else layer_recover(i, 1, to_fill_idx+1, m[i-1], &prp);
		}
	}

	void layer_recover(int depth, int lr, int to_fill_idx, block sum, TwoKeyPRP *prp) {
		int layer_start = 0;
		int item_n = 1<<depth;
		block nodes_sum = zero_block;
		int lr_start = lr==0?layer_start:(layer_start+1);
		
		for(int i = lr_start; i < item_n; i+=2)
			nodes_sum = nodes_sum ^ ggm_tree[i];
		ggm_tree[to_fill_idx] = nodes_sum ^ sum;
		if(depth == this->depth-1) return;
		for(int i = item_n-2; i >= 0; i-=2)
			prp->node_expand_2to4(&ggm_tree[i*2], &ggm_tree[i]);
	}

	void consistency_check_msg_gen(block *chi_alpha, block *W) {
		// X
		block *chi = new block[leave_n];
		Hash hash;
		block digest[2];
		hash.hash_once(digest, &secret_sum_f2, sizeof(block));
		uni_hash_coeff_gen(chi, digest[0], leave_n);
		*chi_alpha = chi[choice_pos];
		vector_inn_prdt_sum_red(W, chi, ggm_tree, leave_n);
		delete[] chi;
	}
};
#endif
