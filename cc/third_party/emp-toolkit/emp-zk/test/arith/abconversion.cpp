#include "emp-tool/emp-tool.h"
#include "emp-zk/emp-zk.h"
#include <iostream>

using namespace emp;
using namespace std;

int port, party;
const int threads = 1;

void test_mix_circuit(BoolIO<NetIO> *ios[threads], int party, int sz) {
	srand(time(NULL));
	uint64_t *a = new uint64_t[sz];
	for(int i = 0; i < sz; ++i)
		a[i] = rand() % PR;

	setup_zk_bool<BoolIO<NetIO>>(ios, threads, party);
	setup_zk_arith<BoolIO<NetIO>>(ios, threads, party, true);

	IntFp *x = new IntFp[sz];
	batch_feed(x, a, sz);

	sync_zk_bool<BoolIO<NetIO>>();

	Integer *y = new Integer[sz];
	for(int i = 0; i < sz; ++i)
		y[i] = Integer(62, a[i], ALICE);

	Integer PR_bl = Integer(62, PR, PUBLIC);

	sync_zk_bool<BoolIO<NetIO>>();

	auto start = clock_start();
	for(int k = 0; k < 2; ++k) {
		for(int i = 0; i < 3; ++i) {
			for(int j = i; j < sz-3; j+=3) {
				y[j+2] = y[j+1] + y[j];
				y[j+2] = y[j+2].select(y[j+2].bits[61], y[j+2] - PR_bl);

				a[j+2] = a[j+1] + a[j];
				if(a[j+2] > PR) a[j+2] -= PR;
			}
			bool2arith<BoolIO<NetIO>>(x, y, sz);

			for(int j = i; j < sz-3; j+=3) {
				x[j] = x[j+1] + x[j+2];
				a[j] = a[j+1] + a[j+2];
				if(a[j] > PR) a[j] -= PR;
			}
			arith2bool<BoolIO<NetIO>>(y, x, sz);
		}
	}

	int incorrect_cnt = 0;
	for(int i = 0; i < sz; ++i) {
		Bit ret = y[i].equal(Integer(62, a[i], PUBLIC));
		if(ret.reveal<bool>(PUBLIC) != 1)
			incorrect_cnt++;
	}
	if(incorrect_cnt) std::cout << "incorrect boolean: " << incorrect_cnt << std::endl;
	std::cout << "end check boolean" << std::endl;

	batch_reveal_check(x, a, sz);
	std::cout << "end check arithmetic" << std::endl;

	finalize_zk_bool<BoolIO<NetIO>>();
	finalize_zk_arith<BoolIO<NetIO>>();

	double tt = time_from(start);
	std::cout << "conversion: " << tt/(2*3*2)/sz << std::endl;

	delete[] a;
	delete[] x;
	delete[] y;
}

int main(int argc, char** argv) {
	parse_party_and_port(argv, &party, &port);
	BoolIO<NetIO>* ios[threads];
	for(int i = 0; i < threads; ++i)
		ios[i] = new BoolIO<NetIO>(new NetIO(party == ALICE?nullptr:"127.0.0.1",port+i), party == ALICE);

	std::cout << std::endl << "------------ circuit zero-knowledge proof test ------------" << std::endl << std::endl;;

	int num = 0;
	if(argc < 3) {
		std::cout << "usage: bin/arith/abconversion PARTY PORT TEST_SIZE" << std::endl;
		return -1;
	} else if (argc == 3) {
		num = 10;
	} else {
		num = atoi(argv[3]);
	}

	test_mix_circuit(ios, party, num);

	for(int i = 0; i < threads; ++i) {
		delete ios[i]->io;
		delete ios[i];
	}
	return 0;
}
