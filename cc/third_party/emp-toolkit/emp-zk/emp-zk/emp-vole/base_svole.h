#ifndef BASE_VOLE_H__
#define BASE_VOLE_H__

#include "emp-tool/emp-tool.h"
#include "emp-zk/emp-vole/cope.h"

template<typename IO>
class Base_svole {
public:
	int party;
	int m;
	IO *io;
	Cope<IO> *cope;
	__uint128_t Delta;
	
	// SENDER
	Base_svole(int party, IO *io, __uint128_t Delta) {
		this->party = party;
		this->io = io;
		cope = new Cope<IO>(party, io, MERSENNE_PRIME_EXP);
		this->Delta = Delta;
		cope->initialize(Delta);
	}

	// RECEIVER
	Base_svole(int party, IO *io) {
		this->party = party;
		this->io = io;
		cope = new Cope<IO>(party, io, MERSENNE_PRIME_EXP);
		cope->initialize();
	}

	~Base_svole() {
		delete cope;
	}

	// sender
	void triple_gen_send(__uint128_t *share, int size) {
		cope->extend(share, size);
		__uint128_t b;
		cope->extend(&b, 1);
		sender_check(share, b, size);
	}
	
	// recver
	void triple_gen_recv(__uint128_t *share, int size) {
		PRG prg;
		uint64_t *x = new uint64_t[size+1];
		prg.random_data(x, (size+1)*sizeof(uint64_t));
		for(int i = 0; i < size+1; ++i) {
			x[i] = mod(x[i]);
		}
		cope->extend(share, x, size);
		__uint128_t c;
		cope->extend(&c, &x[size], 1);
		recver_check(share, x, c, x[size], size);	

		for(int i = 0; i < size; ++i)
			share[i] = (__uint128_t)makeBlock(x[i], share[i]);
		delete[] x;
	}
	
	// sender check
	void sender_check(__uint128_t *share, uint64_t b, int size) {
		PRG prg;
		uint64_t seed;
		prg.random_data(&seed, sizeof(uint64_t));
		seed = mod(seed);
		io->send_data(&seed, sizeof(uint64_t));
		uint64_t *chi = new uint64_t[size];
		uni_hash_coeff_gen(chi, seed, size);
		uint64_t y = vector_inn_prdt_sum_red(share, chi, size);
		y = add_mod(y, b);
		uint64_t xz[2];
		io->recv_data(xz, 2*sizeof(uint64_t));
		xz[1] = mult_mod(xz[1], Delta);
		y = add_mod(y, xz[1]);
		if(y != xz[0]) {
			std::cout << "base sVOLE check fails" << std::endl;
			abort();
		}
		delete[] chi;
	}

	// receiver check
	void recver_check(__uint128_t *share, uint64_t *x, uint64_t c, uint64_t a, int size) {
		uint64_t seed;
		io->recv_data(&seed, sizeof(uint64_t));
		uint64_t *chi = new uint64_t[size];
		uni_hash_coeff_gen(chi, seed, size);
		uint64_t xz[2];
		xz[0] = vector_inn_prdt_sum_red(share, chi, size);
		xz[1] = vector_inn_prdt_sum_red(x, chi, size);
		xz[0] = add_mod(xz[0], c);
		xz[1] = add_mod(xz[1], a);
		io->send_data(xz, 2*sizeof(uint64_t));
		delete[] chi;
	}
};

#endif
