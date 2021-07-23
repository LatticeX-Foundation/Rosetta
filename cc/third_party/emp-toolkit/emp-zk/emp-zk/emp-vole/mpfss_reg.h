#ifndef MPFSS_REG_FP_H__
#define MPFSS_REG_FP_H__

#include <emp-tool/emp-tool.h>
#include <set>
#include "emp-zk/emp-vole/utility.h"
#include "emp-zk/emp-vole/preot.h"

using namespace emp;

template<typename IO>
class MpfssRegFp {
public:
	int party;
	int threads;
	int item_n, idx_max, m;
	int tree_height, leave_n;
	int tree_n;
	bool is_malicious;

	PRG prg;
	IO *netio;
	IO **ios;
	__uint128_t secret_share_x;
	__uint128_t **ggm_tree;
	__uint128_t *check_chialpha_buf = nullptr, *check_VW_buf = nullptr;
	__uint128_t *triple_yz;
	ThreadPool *pool;
	std::vector<uint32_t> item_pos_recver;

	MpfssRegFp(int party, int threads, int n, int t, int log_bin_sz, ThreadPool * pool, IO** ios) {
		this->party = party;
		this->threads = threads;
		this->netio = ios[0];
		this->ios = ios;

		this->pool = pool;
		this->is_malicious = false;

		// make sure n = t * leave_n
		this->item_n = t;
		this->idx_max = n;
		this->tree_height = log_bin_sz+1;
		this->leave_n = 1<<(this->tree_height-1);
		this->tree_n = this->item_n;

		this->ggm_tree = (__uint128_t**)malloc(this->item_n*sizeof(__uint128_t*));

		if(party == BOB) check_chialpha_buf = new __uint128_t[item_n];
		check_VW_buf = new __uint128_t[item_n];
	}

	~MpfssRegFp() {
		free(ggm_tree);
		if(check_chialpha_buf != nullptr)
			delete[] check_chialpha_buf;
		delete[] check_VW_buf;
	}

	void set_malicious() {
		is_malicious = true;
	}

	void sender_init(__uint128_t delta) {
		secret_share_x = delta;
	}

	void recver_init() {
		item_pos_recver.resize(this->item_n);
	}

	void set_vec_x(__uint128_t *out, __uint128_t *in) {
		for(int i = 0; i < tree_n; ++i) {
			int pt = i*leave_n+(item_pos_recver[i]%leave_n);
			out[pt] = out[pt] ^ (__uint128_t)makeBlock(in[i], 0x0LL);
		}
	}

	void mpfss(OTPre<IO> *ot, __uint128_t *triple_yz, __uint128_t* sparse_vector) {
		this->triple_yz = triple_yz;
		mpfss(ot, sparse_vector);
	}

	void mpfss(OTPre<IO> *ot, __uint128_t * sparse_vector) {
		vector<SpfssSenderFp<IO>*> senders;
		vector<SpfssRecverFp<IO>*> recvers;
		vector<future<void>> fut;
		for(int i = 0; i < tree_n; ++i) {
			if(party == 1) {
				senders.push_back(new SpfssSenderFp<IO>(netio, tree_height));
				ot->choices_sender();
			} else {
				recvers.push_back(new SpfssRecverFp<IO>(netio, tree_height));
				ot->choices_recver(recvers[i]->b);
				item_pos_recver[i] = recvers[i]->get_index();
			}
		}
		netio->flush();
		ot->reset();

		uint32_t width = tree_n / threads;
		uint32_t start = 0, end = width;
		for(int i = 0; i < threads - 1; ++i) {
			fut.push_back(pool->enqueue([this, start, end, width, senders, recvers, ot, sparse_vector](){
				for (auto i = start; i < end; ++i) {
					if(party == ALICE) {
						ggm_tree[i] = sparse_vector+i*leave_n;
						senders[i]->compute(ggm_tree[i], secret_share_x, triple_yz[i]);
						senders[i]->template send<OTPre<IO>>(ot, ios[start/width], i);
						if(is_malicious) senders[i]->consistency_check_msg_gen(check_VW_buf[i], ios[start/width]);
						ios[start/width]->flush();
					} else {
						recvers[i]->template recv<OTPre<IO>>(ot, ios[start/width], i);
						ggm_tree[i] = sparse_vector+i*leave_n;
						recvers[i]->compute(ggm_tree[i], triple_yz[i]);
						if(is_malicious) recvers[i]->consistency_check_msg_gen(check_chialpha_buf[i], check_VW_buf[i], ios[start/width], triple_yz[i]);
						ios[start/width]->flush();
					}
				}
			}));
			start = end;
			end += width;
		}
		end = tree_n;
		for (auto i = start; i < end; ++i) {
			if(party == ALICE){
				ggm_tree[i] = sparse_vector+i*leave_n;
				senders[i]->compute(ggm_tree[i], secret_share_x, triple_yz[i]);
				senders[i]->template send<OTPre<IO>>(ot, ios[threads-1], i);
				if(is_malicious) senders[i]->consistency_check_msg_gen(check_VW_buf[i], ios[threads-1]);
				ios[threads-1]->flush();
			} else {
				recvers[i]->template recv<OTPre<IO>>(ot, ios[threads-1], i);
				ggm_tree[i] = sparse_vector+i*leave_n;
				recvers[i]->compute(ggm_tree[i], triple_yz[i]);
				if(is_malicious) recvers[i]->consistency_check_msg_gen(check_chialpha_buf[i], check_VW_buf[i], ios[threads-1], triple_yz[i]);
				ios[threads-1]->flush();
			}
		}
		for (auto & f : fut) f.get();

		if(is_malicious) {
			if(party == ALICE)
				consistency_batch_check(triple_yz[tree_n], tree_n);
			else consistency_batch_check(triple_yz, triple_yz[tree_n], tree_n);
		}

		for (auto p : senders) delete p;
		for (auto p : recvers) delete p;
	}

	void consistency_batch_check(__uint128_t y, int num) {
		uint64_t x_star;
		netio->recv_data(&x_star, sizeof(uint64_t));
		uint64_t tmp = mult_mod(secret_share_x, x_star);
		tmp = add_mod((uint64_t)y, tmp);
		uint64_t vb = pr - tmp;	// y_star

		for(int i = 0; i < num; ++i)
			vb = add_mod(vb, (uint64_t)check_VW_buf[i]);
		Hash hash;
		block h = hash.hash_for_block(&vb, sizeof(uint64_t));
		netio->send_data(&h, sizeof(block));
		netio->flush();
	}

	void consistency_batch_check(__uint128_t *delta2, __uint128_t z, int num) {
		uint64_t beta_mul_chialpha = (uint64_t)0;
		for(int i = 0; i < num; ++i) {
			uint64_t tmp = mult_mod(_mm_extract_epi64((block)delta2[i], 1), check_chialpha_buf[i]);
			beta_mul_chialpha = add_mod(beta_mul_chialpha, tmp);
		}
		uint64_t x_star = PR - beta_mul_chialpha;
		x_star = add_mod(_mm_extract_epi64((block)z, 1), x_star);
		netio->send_data(&x_star, sizeof(uint64_t));
		netio->flush();

		uint64_t va = PR - _mm_extract_epi64((block)z, 0);
		for(int i = 0; i < num; ++i)
			va = mod(va+check_VW_buf[i], pr);

		Hash hash;
		block h = hash.hash_for_block(&va, sizeof(uint64_t));
		block r;
		netio->recv_data(&r, sizeof(block));
		if(!cmpBlock(&r, &h, 1)) error("MPFSS batch check fails");
	}

	// debug
	void check_correctness(IO *io2, __uint128_t* vector, __uint128_t gamma, __uint128_t y) {
		io2->send_data(vector, leave_n*sizeof(__uint128_t));
		io2->send_data(&gamma, sizeof(__uint128_t));
		io2->send_data(&secret_share_x, sizeof(__uint128_t));

		io2->send_data(&y, sizeof(__uint128_t));
	}

	// debug
	void check_correctness(IO *io2, __uint128_t *vector, __uint128_t beta, __uint128_t delta2, int pos, __uint128_t x, __uint128_t z) {
		__uint128_t *sendervec = new __uint128_t[leave_n];
		__uint128_t gamma, delta, y;
		io2->recv_data(sendervec, leave_n*sizeof(__uint128_t));
		io2->recv_data(&gamma, sizeof(__uint128_t));
		io2->recv_data(&delta, sizeof(__uint128_t));
		__uint128_t delta3 = delta;
		io2->recv_data(&y, sizeof(__uint128_t));

		for(int i = 0; i < leave_n; ++i) {
			if(i == pos)
				continue;
			if(vector[i] != sendervec[i]) {
				std::cout << "wrong node at: "<<  i << " " << (uint64_t)vector[i] << " " << (uint64_t)sendervec[i] << std::endl;
				abort();
			}
		}

		delta = mod(delta*beta, pr);
		delta = mod(delta+sendervec[pos], pr);
		if(delta != vector[pos]) {
			std::cout << "wrong secret" << std::endl;
			abort();
		}
		else std::cout << "right vector" << std::endl;
		
		delta3 = mod(delta3*x, pr);
		delta3 = mod(delta3+y, pr);
		if(delta3 != z) {
			std::cout << "wrong triple" << std::endl;
			abort();
		} else std::cout << "right check triple" << std::endl;
	}


};
#endif
