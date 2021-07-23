#ifndef COPE_H__
#define COPE_H__

#include "emp-tool/emp-tool.h"
#include "emp-ot/emp-ot.h"
#include "emp-zk/emp-vole/utility.h"

template<typename IO>
class Cope {
public:
	int party;
	int m;
	IO *io;
	block *K = nullptr;
	__uint128_t delta;
	PRG *G0 = nullptr, *G1 = nullptr;
	bool *delta_bool = nullptr;
	__uint128_t mask;

	Cope(int party, IO *io, int m) {
		this->party = party;
		this->m = m;
		this->io = io;
		mask = (__uint128_t)0xFFFFFFFFFFFFFFFFLL;
	}

	~Cope() {
		if(G0 != nullptr) delete[] G0;
		if(G1 != nullptr) delete[] G1;
		if(delta_bool != nullptr) delete[] delta_bool;
	}

	// sender
	void initialize(__uint128_t delta) {
		this->delta = delta;
		delta_bool = new bool[m];
		delta64_to_bool(delta_bool, delta);

		K = new block[m];
		OTCO<IO> otco(io);
		otco.recv(K, delta_bool, m);

		G0 = new PRG[m];
		for(int i = 0; i < m; ++i)
			G0[i].reseed(K+i);

		delete[] K;
	}

	// recver
	void initialize() {
		K = new block[2*m];
		PRG prg;
		prg.random_block(K, 2*m);
		OTCO<IO> otco(io);
		otco.send(K, K+m, m);

		G0 = new PRG[m];
		G1 = new PRG[m];
		for(int i = 0; i < m; ++i) {
			G0[i].reseed(K+i);
			G1[i].reseed(K+m+i);
		}

		delete[] K;
	}

	// sender
	__uint128_t extend() {
		__uint128_t *w = new __uint128_t[m];
		__uint128_t *v = new __uint128_t[m];
		for(int i = 0; i < m; ++i) {
			G0[i].random_block((block*)(&w[i]), 1);
			extract_fp(w[i]);
		}

		io->recv_data(v, m*sizeof(__uint128_t));
		__uint128_t ch[2];
		ch[0] = (__uint128_t)0;
		for(int i = 0; i < m; ++i) {
			ch[1] = v[i];
			v[i] = mod(w[i]+ch[delta_bool[i]], pr);
		}

		return prm2pr(v);
	}

	// sender batch
	void extend(__uint128_t *ret, int size) {
		uint64_t *w = new uint64_t[m*size];
		uint64_t *v = new uint64_t[m*size];
		for(int i = 0; i < m; ++i) {
			G0[i].random_data(&w[i*size], size*sizeof(uint64_t));
			for(int j = 0; j < size; ++j) {
				w[i*size+j] = mod(w[i*size+j]);
			}
		}

		uint64_t ch[2];
		ch[0] = (uint64_t)0;
		for(int i = 0; i < m; ++i) {
			for(int j = 0; j < size; ++j) {
				io->recv_data(&v[i*size+j], sizeof(uint64_t));
				ch[1] = v[i*size+j];
				v[i*size+j] = add_mod(w[i*size+j], ch[delta_bool[i]]);
			}
		}

		prm2pr(ret, v, size);

		delete[] w;
		delete[] v;
	}

	// recver
	__uint128_t extend(__uint128_t u) {
		__uint128_t *w0 = new __uint128_t[m];
		__uint128_t *w1 = new __uint128_t[m];
		__uint128_t *tau = new __uint128_t[m];
		for(int i = 0; i < m; ++i) {
			G0[i].random_block((block*)(&w0[i]), 1);
			G1[i].random_block((block*)(&w1[i]), 1);
			extract_fp(w0[i]);
			extract_fp(w1[i]);
			w1[i] = mod(w1[i]+u, pr);
			w1[i] = pr - w1[i];
			tau[i] = mod(w0[i]+w1[i], pr);
		}

		io->send_data(tau, m*sizeof(__uint128_t));
		io->flush();
		
		return prm2pr(w0);
	}
	
	// recver batch
	void extend(__uint128_t *ret, uint64_t *u, int size) {
		uint64_t *w0 = new uint64_t[m*size];
		uint64_t *w1 = new uint64_t[m*size];
		for(int i = 0; i < m; ++i) {
			G0[i].random_data(&w0[i*size], size*sizeof(uint64_t));
			G1[i].random_data(&w1[i*size], size*sizeof(uint64_t));
			for(int j = 0; j < size; ++j) {
				w0[i*size+j] = mod(w0[i*size+j]);
				w1[i*size+j] = mod(w1[i*size+j]);
				
				w1[i*size+j] = add_mod(w1[i*size+j], u[j]);
				w1[i*size+j] = PR - w1[i*size+j];
				uint64_t tau = add_mod(w0[i*size+j], w1[i*size+j]);
				io->send_data(&tau, sizeof(uint64_t));
	//			io->flush();
			}
		}

		prm2pr(ret, w0, size);

		delete[] w0;
		delete[] w1;
	}

	void delta64_to_bool(bool *bdata, __uint128_t u128) {
		uint64_t *ptr = (uint64_t*)(&u128);
		uint64_t in = ptr[0];
		for(int i = 0; i < m; ++i) {
			bdata[i] = ((in & 0x1LL) == 1);
			in >>= 1;
		}
	}

	__uint128_t prm2pr(__uint128_t *a) {
		__uint128_t ret = (__uint128_t)0;
		__uint128_t tmp;
		for(int i = 0; i < m; ++i) {
			tmp = mod(a[i]<<i, pr);
			ret = mod(ret+tmp, pr);
		}
		return ret;
	}
	
	void prm2pr(__uint128_t *ret, __uint128_t *a, int size) {
		memset(ret, 0, size*sizeof(__uint128_t));
		__uint128_t tmp;
		for(int i = 0; i < m; ++i) {
			for(int j = 0; j < size; ++j) {
				tmp = mod(a[i*size+j]<<i, pr);
				ret[j] = mod(ret[j] + tmp, pr);
			}
		}
	}

	void prm2pr(__uint128_t *ret, uint64_t *a, int size) {
		memset(ret, 0, size*sizeof(__uint128_t));
		__uint128_t tmp;
		for(int i = 0; i < m; ++i) {
			for(int j = 0; j < size; ++j) {
				tmp = (__uint128_t)a[i*size+j];
				tmp = mod(tmp<<i, pr);
				ret[j] = (__uint128_t)add_mod(ret[j], tmp);
			}
		}
	}

	// debug function
	void check_triple(uint64_t *a, __uint128_t *b, int sz) {
		if(party == ALICE) {
			io->send_data(a, sizeof(uint64_t));
			io->send_data(b, sz*sizeof(__uint128_t));
		} else {
			uint64_t delta;
			__uint128_t *c = new __uint128_t[sz];
			io->recv_data(&delta, sizeof(uint64_t));
			io->recv_data(c, sz*sizeof(__uint128_t));
			for(int i = 0; i < sz; ++i) {
				__uint128_t tmp = mod((__uint128_t)a[i]*delta, pr);
				tmp = mod(tmp + c[i], pr);
				if(tmp != b[i]) {
					std::cout << "wrong triple" << i<<std::endl;
					abort();
				}
			}
		}
		std::cout << "pass check" << std::endl;
	}
};

#endif
