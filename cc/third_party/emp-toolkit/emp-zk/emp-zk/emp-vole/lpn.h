#ifndef _LPN_FP_H__
#define _LPN_FP_H__

#include "emp-tool/emp-tool.h"
#include "emp-zk/emp-vole/utility.h"
namespace emp {
template<int d = 10>
class LpnFp { public:
	int party;
	int k, n;
	ThreadPool * pool;
	int threads;
	block seed;

	int round, leftover;

	__uint128_t *M;
	const __uint128_t *preM, *prex;
	__uint128_t *K;
	const __uint128_t *preK;

	uint32_t k_mask;

	LpnFp (int n, int k, ThreadPool * pool, int threads, block seed = zero_block) {
		this->k = k;
		this->n = n;
		this->pool = pool;
		this->threads = threads;
		this->seed = seed;
		
		round = d / 4;
		leftover = d % 4;

		this->k_mask = k_mask_gen(k);
	}

	uint32_t k_mask_gen(int kin) {
		int ksz = kin;
		int sz = 0;
		while(ksz > 1) {
			sz++;
			ksz = ksz>>1;
		}
		return (1<<sz)-1;
	}

	void add2(int idx1, int* idx2, uint64_t* mult) {
		__uint128_t res[2], valM[2];
		for(int j = 0; j < 5; ++j) {
			valM[0] = preM[idx2[2*j]];
			valM[1] = preM[idx2[2*j+1]];
			mult_mod_bch2((block*)res, (block*)valM, mult+2*j);
			M[idx1] = (__uint128_t)add_mod((block)M[idx1], (block)res[0]);
			M[idx1] = (__uint128_t)add_mod((block)M[idx1], (block)res[1]);
		}
	}

	void add1(int idx1, int* idx2, uint64_t* mult) {
		uint64_t res[2], valK[2];
		for(int j = 0; j < 5; ++j) {
			valK[0] = preK[idx2[2*j]];
			valK[1] = preK[idx2[2*j+1]];
			mult_mod_bch2(res, valK, mult+2*j);
			K[idx1] = (__uint128_t)add_mod(K[idx1], res[0]);
			K[idx1] = (__uint128_t)add_mod(K[idx1], res[1]);
		}
	}

	void __compute4(int i, PRP *prp, std::function<void(int, int*, uint64_t*)> add_func) {
		block tmp[30];
		for(int m = 0; m < 30; ++m)
			tmp[m] = makeBlock(i, m);
		prp->permute_block(tmp, 30);
		uint32_t* r = (uint32_t*)(tmp);
		uint64_t* mult = (uint64_t*)(tmp+10);
		for(int m = 0; m < 4; ++m) {
			int index[d];
			for (int j = 0; j < d; ++j) {
				index[j] = r[m*d+j]&k_mask;
				mult[m*d+j] = mod(mult[m*d+j]);
			}
			add_func(i+m, index, mult+m*d);
		}
	}

	void __compute1(int i, PRP *prp, std::function<void(int, int*, uint64_t*)> add_func) {
		block tmp[8];
		for(int m = 0; m < 8; ++m)
			tmp[m] = makeBlock(i, m);
		prp->permute_block(tmp, 8);
		uint32_t* r = (uint32_t*)(tmp);
		uint64_t* mult = (uint64_t*)(tmp+3);

		int index[d];
		for (int j = 0; j < d; ++j) {
			index[j] = r[j]&k_mask;
			mult[j] = mod(mult[j]);
		}
		add_func(i, index, mult);
	}

	void task(int start, int end) {
		PRP prp(seed);
		int j = start;
		if(party == 1) {
			std::function<void(int, int*, uint64_t*)> add_func1 = std::bind(&LpnFp::add1, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
			for(; j < end-4; j+=4)
				__compute4(j, &prp, add_func1);
			for(; j < end; ++j)
				__compute1(j, &prp, add_func1);
		} else {
			std::function<void(int, int*, uint64_t*)> add_func2 = std::bind(&LpnFp::add2, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
			for(; j < end-4; j+=4)
				__compute4(j, &prp, add_func2);
			for(; j < end; ++j)
				__compute1(j, &prp, add_func2);
		}
	}

	void compute() {
		vector<std::future<void>> fut;
		int width = n/(threads+1);
		for(int i = 0; i < threads; ++i) {
			int start = i * width;
			int end = min((i+1)* width, n);
			fut.push_back(pool->enqueue([this, start, end]() {
				task(start, end);
			}));
		}
		int start = threads * width;
		int end = min( (threads+1) * width, n);
		task(start, end);

		for (auto &f: fut) f.get();
	}

	void compute_send(__uint128_t *K, const __uint128_t *kkK) {
		this->party = ALICE;
		this->K = K;
		this->preK = kkK;
		compute();
	}

	void compute_recv(__uint128_t *M, const __uint128_t *kkM) {
		this->party = BOB;
		this->M = M;
		this->preM = kkM;
		compute();
	}
};
}
#endif
