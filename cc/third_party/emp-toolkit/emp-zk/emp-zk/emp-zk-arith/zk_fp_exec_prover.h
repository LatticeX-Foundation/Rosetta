#ifndef ZK_FP_EXECUTIION_PROVER_H__
#define ZK_FP_EXECUTIION_PROVER_H__
#include "emp-zk/emp-zk-arith/zk_fp_exec.h"

template<typename IO>
class ZKFpExecPrv : public ZKFpExec {
public:
	FpOSTriple<IO> *ostriple;
	IO* io = nullptr;

	ZKFpExecPrv(IO** ios, int threads) : ZKFpExec() {
		PRG prg(fix_key);
		prg.random_block((block*)&this->pub_mac, 1);
		this->pub_mac = mod(this->pub_mac & (__uint128_t)0xFFFFFFFFFFFFFFFFULL, pr);
		this->io = ios[0];
		this->ostriple = new FpOSTriple<IO>(ALICE, threads, ios);
	}

	~ZKFpExecPrv() {
		delete ostriple;
	}

	/* 
	 * Prover is the receiver in iterative COT
	 * interface: get 1 authenticated bit
	 * authenticated message, last bit as the value
	 * embeded in label
	 */
	void feed(__uint128_t& label, const uint64_t& val) {
		label = this->ostriple->authenticated_val_input(val);
	}

	void feed(__uint128_t *label, const uint64_t *val, int len) {
		this->ostriple->authenticated_val_input(label, val, len);
	}

	/*
	 * check correctness of triples using cut and choose and bucketing
	 * check correctness of the output
	 */
	void reveal(__uint128_t *mac, uint64_t *value, int len) {
		this->ostriple->reveal_send(mac, value, len);
	}

	void reveal_check(__uint128_t *mac, const uint64_t *value, int len) {
		this->ostriple->reveal_check_send(mac, value, len);
	}

	void reveal_check_zero(__uint128_t *mac, int len) {
		this->ostriple->reveal_check_zero(mac, len);
	}

	__uint128_t add_gate(const __uint128_t& a, const __uint128_t& b) {
		__uint128_t val = mod((a>>64)+(b>>64), pr);
		__uint128_t mac = mod((a&0xFFFFFFFFFFFFFFFFULL)+(b&0xFFFFFFFFFFFFFFFFULL), pr);
		return (val<<64)^mac;
	}

	__uint128_t mul_gate(const __uint128_t& a, const __uint128_t& b) {
		++this->gid;
		return ostriple->auth_compute_mul_send(a, b);
	}

	__uint128_t mul_const_gate(const __uint128_t& a, const uint64_t& b) {
		return ostriple->auth_mac_mul_const(a, (__uint128_t)b);
	}

	__uint128_t pub_label(const uint64_t& a) {
		return (__uint128_t)makeBlock(a, (uint64_t)this->pub_mac);
	}
};
#endif
