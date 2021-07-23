#ifndef MPCOT_REG_H__
#define MPCOT_REG_H__

#include <emp-tool/emp-tool.h>
#include <set>
#include "emp-ot/ferret/spcot_sender.h"
#include "emp-ot/ferret/spcot_recver.h"
#include "emp-ot/ferret/preot.h"

using namespace emp;
using std::future;

template<typename IO>
class MpcotReg {
public:
	int party, threads;
	int item_n, idx_max, m;
	int tree_height, leave_n;
	int tree_n;
	int consist_check_cot_num;
	bool is_malicious;

	PRG prg;
	IO *netio;
	IO **ios;
	block Delta_f2k;
	block *consist_check_chi_alpha = nullptr, *consist_check_VW = nullptr;
	ThreadPool *pool;
	
	std::vector<uint32_t> item_pos_recver;
	GaloisFieldPacking pack;

	MpcotReg(int party, int threads, int n, int t, int log_bin_sz, ThreadPool * pool, IO **ios) {
		this->party = party;
		this->threads = threads;
		netio = ios[0];
		this->ios = ios;
		consist_check_cot_num = 128;

		this->pool = pool;
		this->is_malicious = false;

		this->item_n = t;
		this->idx_max = n;
		this->tree_height = log_bin_sz+1;
		this->leave_n = 1<<(this->tree_height-1);
		this->tree_n = this->item_n;
	}

	void set_malicious() {
		this->is_malicious = true;
	}

	void sender_init(block delta) {
		Delta_f2k = delta;
	}

	void recver_init() {
		item_pos_recver.resize(this->item_n);
	}

	// MPFSS F_2k
	void mpcot(block * sparse_vector, OTPre<IO> * ot, block *pre_cot_data) {
		if(party == BOB) consist_check_chi_alpha = new block[item_n];
		consist_check_VW = new block[item_n];

		vector<SPCOT_Sender<IO>*> senders;
		vector<SPCOT_Recver<IO>*> recvers;

		if(party == ALICE) {
			mpcot_init_sender(senders, ot);
			exec_parallel_sender(senders, ot, sparse_vector);
		} else {
			mpcot_init_recver(recvers, ot);
			exec_parallel_recver(recvers, ot, sparse_vector);
		}

		if(is_malicious)
			consistency_check_f2k(pre_cot_data, tree_n);

		for (auto p : senders) delete p;
		for (auto p : recvers) delete p;

		if(party == BOB) delete[] consist_check_chi_alpha;
		delete[] consist_check_VW;
	}

	void mpcot_init_sender(vector<SPCOT_Sender<IO>*> &senders, OTPre<IO> *ot) {
		for(int i = 0; i < tree_n; ++i) {
			senders.push_back(new SPCOT_Sender<IO>(netio, tree_height));
			ot->choices_sender();
		}
		netio->flush();
		ot->reset();
	}

	void mpcot_init_recver(vector<SPCOT_Recver<IO>*> &recvers, OTPre<IO> *ot) {
		for(int i = 0; i < tree_n; ++i) {
			recvers.push_back(new SPCOT_Recver<IO>(netio, tree_height));
			ot->choices_recver(recvers[i]->b);
			item_pos_recver[i] = recvers[i]->get_index();
		}
		netio->flush();
		ot->reset();
	}

	void exec_parallel_sender(vector<SPCOT_Sender<IO>*> &senders,
			OTPre<IO> *ot, block* sparse_vector) {
		vector<future<void>> fut;		
		int width = tree_n / threads;
		int start = 0, end = width;
		for(int i = 0; i < threads - 1; ++i) {	
			fut.push_back(this->pool->enqueue([this, start, end, width, 
						senders, ot, sparse_vector](){
				for(int i = start; i < end; ++i)
					exec_f2k_sender(senders[i], ot, sparse_vector+i*leave_n, 
							ios[start/width], i);
			}));
			start = end;
			end += width;
		}
		end = tree_n;
		for(int i = start; i < end; ++i)
			exec_f2k_sender(senders[i], ot, sparse_vector+i*leave_n, 
					ios[threads - 1], i);
		for (auto & f : fut) f.get();
	}

	void exec_parallel_recver(vector<SPCOT_Recver<IO>*> &recvers,
			OTPre<IO> *ot, block* sparse_vector) {
		vector<future<void>> fut;		
		int width = tree_n / threads;
		int start = 0, end = width;
		for(int i = 0; i < threads - 1; ++i) {
			fut.push_back(this->pool->enqueue([this, start, end, width, 
						recvers, ot, sparse_vector](){
				for(int i = start; i < end; ++i)
					exec_f2k_recver(recvers[i], ot, sparse_vector+i*leave_n, 
							ios[start/width], i);
			}));
			start = end;
			end += width;
		}
		end = tree_n;
		for(int i = start; i < end; ++i)
			exec_f2k_recver(recvers[i], ot, sparse_vector+i*leave_n, 
					ios[threads - 1], i);
		for (auto & f : fut) f.get();
	}

	void exec_f2k_sender(SPCOT_Sender<IO> *sender, OTPre<IO> *ot, 
			block *ggm_tree_mem, IO *io, int i) {
		sender->compute(ggm_tree_mem, Delta_f2k);
		sender->template send_f2k<OTPre<IO>>(ot, io, i);
		io->flush();
		if(is_malicious)
			sender->consistency_check_msg_gen(consist_check_VW+i);
	}

	void exec_f2k_recver(SPCOT_Recver<IO> *recver, OTPre<IO> *ot,
			block *ggm_tree_mem, IO *io, int i) {
		recver->template recv_f2k<OTPre<IO>>(ot, io, i);
		recver->compute(ggm_tree_mem);
		if(is_malicious) 
			recver->consistency_check_msg_gen(consist_check_chi_alpha+i, consist_check_VW+i);
	}

	// f2k consistency check
	void consistency_check_f2k(block *pre_cot_data, int num) {
		if(this->party == ALICE) {
			block r1, r2;
			vector_self_xor(&r1, this->consist_check_VW, num);
			bool x_prime[128];
			this->netio->recv_data(x_prime, 128*sizeof(bool));
			for(int i = 0; i < 128; ++i) {
				if(x_prime[i])
					pre_cot_data[i] = pre_cot_data[i] ^ this->Delta_f2k;
			}
			pack.packing(&r2, pre_cot_data);
			r1 = r1 ^ r2;
			block dig[2];
			Hash hash;
			hash.hash_once(dig, &r1, sizeof(block));
			this->netio->send_data(dig, 2*sizeof(block));
			this->netio->flush();
		} else {
			block r1, r2, r3;
			vector_self_xor(&r1, this->consist_check_VW, num);
			vector_self_xor(&r2, this->consist_check_chi_alpha, num);
			uint64_t pos[2];
			pos[0] = _mm_extract_epi64(r2, 0);
			pos[1] = _mm_extract_epi64(r2, 1);
			bool pre_cot_bool[128];
			for(int i = 0; i < 2; ++i) {
				for(int j = 0; j < 64; ++j) {
					pre_cot_bool[i*64+j] = ((pos[i] & 1) == 1) ^ getLSB(pre_cot_data[i*64+j]);
					pos[i] >>= 1;
				}
			}
			this->netio->send_data(pre_cot_bool, 128*sizeof(bool));
			this->netio->flush();
			pack.packing(&r3, pre_cot_data);
			r1 = r1 ^ r3;
			block dig[2];
			Hash hash;
			hash.hash_once(dig, &r1, sizeof(block));
			block recv[2];
			this->netio->recv_data(recv, 2*sizeof(block));
			if(!cmpBlock(dig, recv, 2))
				std::cout << "SPCOT consistency check fails" << std::endl;
		}
	}
};
#endif
