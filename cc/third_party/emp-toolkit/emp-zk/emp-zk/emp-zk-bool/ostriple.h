#ifndef OS_TRIPLE_H__
#define OS_TRIPLE_H__

#include "emp-ot/emp-ot.h"
#include "emp-zk/emp-zk-bool/triple_auth.h"
#include "emp-zk/emp-zk-bool/bool_io.h"
#include "emp-zk/emp-zk-bool/cheat_record.h"

const static int ANDGATE_BUFFER_MEM_SZ= N_REG;
const static int CHECK_DIV_SZ = 8;
const static int ANDGATE_BUFFER_SZ = ((N_REG - K_REG - T_REG * BIN_SZ_REG - 128)/CHECK_DIV_SZ)*CHECK_DIV_SZ;
const static int INPUT_BUFFER_SZ = ANDGATE_BUFFER_SZ;

template<typename IO>
class OSTriple {
public:
	int party, threads;
	block delta;

	// managing buffers storing COTs
	int input_cnt = 0, andgate_cnt = 0, check_cnt = 0;
	block* auth_buffer_input = nullptr;
	block* auth_buffer_andgate = nullptr;
	block* andgate_left_buffer = nullptr;
	block* andgate_right_buffer = nullptr;

	GaloisFieldPacking pack;

	block choice[2], choice2[2];
	block minusone, one;
	IO *io;
	IO **ios;
	PRG prg;
	FerretCOT<IO> *ferret = nullptr;
	TripleAuth<IO> *auth_helper;
	ThreadPool *pool = nullptr;
	void * ferret_state = nullptr;
	
	OSTriple (int party, int threads, IO **ios, void * state = nullptr) {
		this->party = party;
		this->threads = threads;
		this->ferret_state = state;
		// initiate Iterative COT with regular noise and security against malicious adv
		if(ferret_state == nullptr)
			ferret = new FerretCOT<IO>(3-party, threads, ios, true);
		else {
			ferret = new FerretCOT<IO>(3-party, threads, ios, true, false);
			ferret->disassemble_state(ferret_state, 10400000);
		}
		this->delta = ferret->Delta;
		io = ios[0];
		this->ios = ios;
		pool = new ThreadPool(threads);

		auth_buffer_input = new block[ANDGATE_BUFFER_MEM_SZ];//INPUT_BUFFER_SZ];
		auth_buffer_andgate = new block[ANDGATE_BUFFER_MEM_SZ];
		andgate_left_buffer = new block[ANDGATE_BUFFER_SZ/CHECK_DIV_SZ];
		andgate_right_buffer = new block[ANDGATE_BUFFER_SZ/CHECK_DIV_SZ];

		ferret->rcot_inplace(auth_buffer_input, ANDGATE_BUFFER_MEM_SZ);
		ferret->rcot_inplace(auth_buffer_andgate, ANDGATE_BUFFER_MEM_SZ);

		choice[0] = choice2[0] = zero_block;
		choice[1] = this->delta;
		minusone = makeBlock(0xFFFFFFFFFFFFFFFFLL, 0xFFFFFFFFFFFFFFFELL);
		one = makeBlock(0x0L, 0x1L);
		choice2[1] = one;

		auth_helper = new TripleAuth<IO>(party, io);
		if(party == BOB) auth_helper->set_delta(this->delta);
	}	

	~OSTriple () {
		if(andgate_buf_not_empty()) {
			andgate_correctness_check_manage();
		}
		if(!auth_helper->finalize())
			CheatRecord::put("emp-zk-bool finalize");
		if(ferret_state != nullptr)
			ferret->assemble_state(ferret_state, 10400000);
		delete ferret;
		delete[] auth_buffer_input;
		delete[] auth_buffer_andgate;
		delete[] andgate_left_buffer;
		delete[] andgate_right_buffer;
		delete auth_helper;
		delete pool;
	}

	uint64_t communication() {
		uint64_t res = 0;
		for(int i = 0; i < threads; ++i)
			res += ios[i]->counter;
		return res; 
	}

	/* ---------------------inputs----------------------*/
	/*
	 * authenticated bits for inputs of the prover
	 */
	void authenticated_bits_input(block *auth, const bool* in, int len) {
		if((input_cnt+len) > INPUT_BUFFER_SZ) {
			int left = INPUT_BUFFER_SZ - input_cnt;
			memcpy(auth, auth_buffer_input+input_cnt, left*sizeof(block));
			int round = (len - left) / INPUT_BUFFER_SZ;
			int finalr = (len - left) % INPUT_BUFFER_SZ;
			for(int i = 0; i < round; ++i) {
				ferret->rcot_inplace(auth_buffer_input, ANDGATE_BUFFER_MEM_SZ);
				memcpy(auth+left+i*INPUT_BUFFER_SZ, auth_buffer_input, INPUT_BUFFER_SZ*sizeof(block));
			}
			ferret->rcot_inplace(auth_buffer_input, ANDGATE_BUFFER_MEM_SZ);
			memcpy(auth+left+round*INPUT_BUFFER_SZ, auth_buffer_input, finalr*sizeof(block));
			input_cnt = finalr;
		} else {
			memcpy(auth, auth_buffer_input+input_cnt, len*sizeof(block));
			input_cnt += len;
		}

		if(party == ALICE) {
			for(int i = 0; i < len; ++i) {
				bool buff = getLSB(auth[i]) ^ in[i];
				set_value_in_block(auth[i], in[i]);
				io->send_bit(buff);
			}
		} else {
			for(int i = 0; i < len; ++i) {
				bool buff = io->recv_bit();
				auth[i] = auth[i] ^ choice[buff];
				set_zero_bit(auth[i]);
			}
		}
	}

	/*
	 * authenticated bits for computing AND gates
	 */
	block auth_compute_and(block a, block b) {
		block auth;
		if(check_cnt == ANDGATE_BUFFER_SZ/CHECK_DIV_SZ) {
			andgate_correctness_check_manage();
			check_cnt = 0;
			if (andgate_cnt == ANDGATE_BUFFER_SZ) {
				ferret->rcot_inplace(auth_buffer_andgate, ANDGATE_BUFFER_MEM_SZ);
				andgate_cnt = 0;
			}
		}
		auth = auth_buffer_andgate[andgate_cnt];
		andgate_left_buffer[check_cnt] = a;
		andgate_right_buffer[check_cnt] = b;

		if(party == ALICE) {
			bool s = getLSB(a) & getLSB(b);
			bool d = s ^ getLSB(auth);
			set_value_in_block(auth, s);
			io->send_bit(d);
		} else {
			bool d = io->recv_bit();
			auth = auth ^ choice[d];
			set_zero_bit(auth);
		}
		auth_buffer_andgate[andgate_cnt] = auth;
		andgate_cnt++;
		check_cnt++;
		return auth;
	}

	/* ---------------------check----------------------*/

	void andgate_correctness_check_manage() {
		io->flush();
		block seed = io->get_hash_block();
		vector<future<void>> fut;

		int share_seed_n = threads;
		block *share_seed = new block[share_seed_n];
		PRG(&seed).random_block(share_seed, share_seed_n);

		uint32_t task_base = check_cnt / threads;
		uint32_t leftover = task_base + (check_cnt % task_base);
		uint32_t start = 0;
		block *sum = new block[2*threads];
		for(int i = 0; i < threads - 1; ++i) {
			fut.push_back(pool->enqueue([this, sum, i, start, task_base, share_seed](){
				andgate_correctness_check(sum, i, start, task_base, share_seed[i]);
						}));
			start += task_base;
		}
		andgate_correctness_check(sum, threads - 1, start, leftover, share_seed[threads - 1]);

		for(auto &f : fut) f.get();

		if(party == ALICE) {
			block ope_data[128];
			ferret->rcot(ope_data, 128);
			uint64_t ch_bits[2];
			for(int i = 0; i < 2; ++i) {
				if(getLSB(ope_data[64*i+63])) ch_bits[i] = 1;
				else ch_bits[i] = 0;
				for(int j = 62; j >= 0; --j) {
					ch_bits[i] <<= 1;
					if(getLSB(ope_data[64*i+j]))
						ch_bits[i]++;
				}
			}
			block A_star[2];
			A_star[1] = makeBlock(ch_bits[1], ch_bits[0]);
			pack.packing(A_star, ope_data);
			for(int i = 0; i < threads; ++i) {
				A_star[0] = A_star[0] ^ sum[2*i];
				A_star[1] = A_star[1] ^ sum[2*i+1];
			}
			io->send_data(A_star, 2*sizeof(block));
		} else {
			block ope_data[128];
			ferret->rcot(ope_data, 128);
			block B_star;
			pack.packing(&B_star, ope_data);
			for(int i = 0; i < threads; ++i)
				B_star = B_star ^ sum[i];
			block A_star[2];
			io->recv_data(A_star, 2*sizeof(block));
			block W;
			gfmul(A_star[1], this->delta, &W);
			W = W ^ A_star[0];
			if(cmpBlock(&W, &B_star, 1) != 1)
				CheatRecord::put("emp_zk_bool AND batch check");
		}
		io->flush();
		delete[] share_seed;
		delete[] sum;
	}

	void andgate_correctness_check(block *ret, int thr_i, uint32_t start, uint32_t task_n, block chi_seed) {
		if(task_n == 0) return;
		block *left = andgate_left_buffer;
		block *right = andgate_right_buffer;
		block *gateout = auth_buffer_andgate + andgate_cnt - check_cnt;

		if(party == ALICE) {
			block ch_tmp[2];
			ch_tmp[0] = zero_block;
			for(uint32_t i = start; i < start + task_n; ++i) {
				block A0, A1;
				gfmul(left[i], right[i], &A0);
				ch_tmp[1] = right[i];
				A1 = ch_tmp[getLSB(left[i])];
				ch_tmp[1] = left[i];
				A1 = A1 ^ ch_tmp[getLSB(right[i])];
				A1 = A1 ^ gateout[i];
				left[i] = A0;
				right[i] = A1;
			}
		} else {
			for(uint32_t i = start; i < start + task_n; ++i) {
				block B;
				gfmul(left[i], right[i], &B);
				block tmp;
				gfmul(gateout[i], this->delta, &tmp);
				B = B ^ tmp;
				left[i] = B;
			}
		}

		block *chi = new block[task_n];
		uni_hash_coeff_gen(chi, chi_seed, task_n);
		if(party == ALICE) {
			vector_inn_prdt_sum_red(ret+2*thr_i, chi, left+start, task_n);
			vector_inn_prdt_sum_red(ret+2*thr_i+1, chi, right+start, task_n);
		} else vector_inn_prdt_sum_red(ret+thr_i, chi, left+start, task_n);

		delete[] chi;
	}

	/*
	 * verify the output
	 * open and check if the value equals 1
	 */
	void verify_output(bool *b, const block *output, int length) {
		for(int i = 0; i < length; ++i) {
			if(party == ALICE) {	
				b[i] = getLSB(output[i]);
				io->send_bit(b[i]);
			} else {
				b[i] = io->recv_bit();
			}
		}
		if(party == ALICE) {
			auth_helper->prv_check(b, output, length);
		} else auth_helper->ver_check(b, output, length);
	}


	/* ---------------------helper functions----------------------*/
	void set_zero_bit(block& b) {
		b = b & minusone;
	}

	void set_value_in_block(block &b, bool v) {
		b = b & minusone;
		b = b ^ choice2[v];
	}

	bool andgate_buf_not_empty() {
		if(andgate_cnt == 0) return false;
		else return true;
	}

	void sync() {
		io->flush();
		for(int i = 0; i < threads; ++i) {
			ios[i]->flush();
		}
	}

	/* ---------------------debug functions----------------------*/

	void check_auth_mac(block* auth, bool* in, int len, IO *tio) {
		if(party == ALICE) {
			tio->send_data(auth, len*sizeof(block));
			tio->send_data(in, len*sizeof(bool));
		} else {
			block* auth_recv = new block[len];
			tio->recv_data(auth_recv, len*sizeof(block));
			tio->recv_data(in, len*sizeof(bool));
			for(int i = 0; i < len; ++i) {
				if(in[i] != getLSB(auth_recv[i])) error("check1");
				set_zero_bit(auth[i]);
				block mac = auth[i] ^ choice[in[i]];
				if(!cmpBlock(&mac, &auth_recv[i], 1)) error ("check2");
			}
			delete[] auth_recv;
		}
	}
	void check_compute_and(block* a, block *b, block *c, int len, IO *tio) {
		if(party == ALICE) {
			tio->send_data(a, len*sizeof(block));
			tio->send_data(b, len*sizeof(block));
			tio->send_data(c, len*sizeof(block));
		} else {
			block* recv = new block[3*len];
			tio->recv_data(recv, len*sizeof(block));
			tio->recv_data(recv+len, len*sizeof(block));
			tio->recv_data(recv+2*len, len*sizeof(block));
			for(int i = 0; i < len; ++i) {
				bool ar = getLSB(recv[i]);
				bool br = getLSB(recv[len+i]);
				bool cr = getLSB(recv[2*len+i]);
				if(cr != (ar&br)) error("check3");
				block v[3];
				v[0] = a[i]; v[1] = b[i]; v[2] = c[i];
				set_zero_bit(v[0]);
				set_zero_bit(v[1]);
				set_zero_bit(v[2]);
				v[0] = v[0] ^ choice[ar];
				v[1] = v[1] ^ choice[br];
				v[2] = v[2] ^ choice[cr];
				if(!cmpBlock(v, recv+i, 1)) error("check4");
				if(!cmpBlock(v+1, recv+len+i, 1)) error("check5");
				if(!cmpBlock(v+2, recv+2*len+i, 1)) error("check6");
			}
			delete[] recv;
		}
	}

};
#endif
