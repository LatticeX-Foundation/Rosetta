#ifndef ZK_VERIFIER_H__
#define ZK_VERIFIER_H__
#include <emp-tool/emp-tool.h>
#include <emp-ot/emp-ot.h>
#include "emp-zk/emp-zk-bool/ostriple.h"
#include "emp-zk/emp-zk-bool/zk_bool_circuit_exec.h"
#include "emp-zk/emp-zk-bool/polynomial.h"

template<typename IO>
class ZKBoolCircExecVer:public ZKBoolCircExec<IO> { public:
	block delta, zdelta;
	using ZKBoolCircExec<IO>::pub_label;
	ZKBoolCircExecVer() {
		PRG tmp;
		block a;
		tmp.random_block(&a, 1);
		set_delta(a);

		PRG prg(fix_key);
		prg.random_block(pub_label, 2);
		pub_label[0] = pub_label[0] & makeBlock(0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFEULL);
		pub_label[1] = pub_label[1] & makeBlock(0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFEULL);

	}
	void set_delta(const block &_delta) {
		this->delta = set_bit(_delta, 0);
		this->zdelta = (this->delta) ^ makeBlock(0x0LL, 0x1LL);
	}
	template<typename T>
	void set_ostriple(OSTriple<T> *ostriple) {
		this->ostriple = ostriple;
		this->delta = ostriple->delta;
		this->zdelta = (this->delta) ^ makeBlock(0x0LL, 0x1LL);
		pub_label[1] = pub_label[1] ^ zdelta;
	}
	block not_gate(const block&a) override {
		return a ^ zdelta;
	}
};


template<typename IO>
class ZKVerifier: public ProtocolExecution {
public:
	IO* io = nullptr;
	OSTriple<IO>* ostriple = nullptr;
	PolyProof<IO> *polyproof = nullptr;
	ZKBoolCircExecVer<IO> *eva = nullptr;
	ZKVerifier(IO **ios, int threads, ZKBoolCircExecVer<IO> *t, void * state): ProtocolExecution(BOB) {
		this->io = ios[0];
		ostriple = new OSTriple<IO>(BOB, threads, ios, state);
		polyproof = new PolyProof<IO>(BOB, ios[0], ostriple->ferret);
		polyproof->delta = ostriple->delta;
		t->template set_ostriple<IO>(ostriple);
		t->polyproof = this->polyproof;
		this->eva = t;
	}
	~ZKVerifier() {
		delete polyproof;
		delete ostriple;
	}

	/* 
	 * Verifier is the sender in iterative COT
	 * interface: get 1 authenticated bit
	 * authenticated message, KEY
	 */
	void feed(block * label, int party, const bool* b, int length) {
		if(party == ALICE)
			ostriple->authenticated_bits_input(label, b, length);
		else if (party == PUBLIC) {
			for(int i = 0; i < length; ++i)
				label[i] = eva->public_label(b[i]);
		}
	}

	/*
	 * check correctness of triples using cut and choose and bucketing
	 * check correctness of the output
	 */
	void reveal(bool * b, int party, const block * label, int length) {
		if (party == BOB or party == PUBLIC) {
			ostriple->verify_output(b, label, length);
		}
	}

};
#endif// ZK_VERIFIER_H__
