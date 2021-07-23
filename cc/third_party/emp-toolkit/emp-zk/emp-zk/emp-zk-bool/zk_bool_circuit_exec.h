#ifndef ZK_BOOL_CIRCUIT_EXE_H__
#define ZK_BOOL_CIRCUIT_EXE_H__
#include "emp-tool/emp-tool.h"
#include "emp-zk/emp-zk-bool/polynomial.h"
#include <iostream>

template<typename IO>
class ZKBoolCircExec:public CircuitExecution { public:
	int64_t gid = 0;
	OSTriple<IO> *ostriple;
	PolyProof<IO> *polyproof;
	block pub_label[2];
	uint64_t communication() {
		return ostriple->communication();
	}
	block and_gate(const block& a, const block& b) override {
		++gid;
		return ostriple->auth_compute_and(a, b);
	}
	block xor_gate(const block&a, const block& b) override {
		return a ^ b;
	}
	block not_gate(const block&a) override {
		return a ^ makeBlock(0, 1);
	}
	block public_label(bool b) override {
		return pub_label[b];
	}
	size_t num_and() override {
		return gid;
	}
	void sync() {
//		ostriple->bio->flush();
		for(int i = 0; i < ostriple->threads; ++i)
			ostriple->ios[i]->flush();
	}
};
#endif// ZK_BOOLEAN_GEN_H__
