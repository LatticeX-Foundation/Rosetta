#include <emp-zk/emp-zk.h>
#include <iostream>
#include "emp-tool/emp-tool.h"
using namespace emp;
using namespace std;

int port, party;
const int threads = 1;

void test_input_speed(BoolIO<NetIO> **ios, int party, int input_sz_log) {
	long long sz = input_sz_log;
	std::cout << "input size: " << sz << std::endl;
	srand(time(NULL));
	uint64_t *a = new uint64_t[sz];
	for(int i = 0; i < sz; ++i)
		a[i] = rand() % PR;

	setup_zk_arith<BoolIO<NetIO>>(ios, threads, party);

	IntFp *x = new IntFp[sz];

	/* normal input */
	auto start = clock_start();
	for(int i = 0; i < sz; ++i)
		x[i] = IntFp(a[i], ALICE);
	double tt = time_from(start);
	std::cout << "normal input average time: " << tt*1000/sz << " ns per element" << std::endl;

	/* batch input */
	start = clock_start();
	batch_feed(x, a, sz);
	tt = time_from(start);
	std::cout << "batch input average time: " << tt*1000/sz << " ns per element" << std::endl;

	finalize_zk_arith<BoolIO<NetIO>>();

	delete[] a;
	delete[] x;	
}

/*void test_circuit_zk(BoolIO<NetIO> *ios[threads+1], int party) {

	long long input_sz = 1048576;
	while(input_sz < 1000000000LL) {
		auto start = clock_start();
		setup_fp_zk<BoolIO<NetIO>, threads>(ios, party);
		IntFp *a = new IntFp[input_sz];
		for(int i = 0; i < input_sz; ++i)
			a[i] = IntFp((uint64_t)i, ALICE);
		cout << input_sz << "\t" << time_from(start)<< endl;
		cout << a[0].reveal(0) <<endl;
		delete[] a;
		input_sz = input_sz * 4;
	}

	long long unit = input_sz / 4;
	input_sz = unit * 2;
	while(input_sz < 1100000000LL) {
		auto start = clock_start();
		setup_fp_zk<BoolIO<NetIO>, threads>(ios, party);
		int round = input_sz / unit;
		IntFp **a = (IntFp**)malloc(round*sizeof(IntFp*));
		for(int i = 0; i < round; ++i) {
			a[i] = new IntFp[unit];
			for(int j = 0; j < unit; ++j)
				a[i][j] = IntFp((uint64_t)j, ALICE);
		}
		cout << input_sz << "\t" << time_from(start)<< endl;
		cout << a[0][0].reveal(0) <<endl;
		for(int i = 0; i < round; ++i)
			delete[] a[i];
		free(a);
		input_sz *= 2;
	}
}*/

int main(int argc, char** argv) {
	parse_party_and_port(argv, &party, &port);
	BoolIO<NetIO>* ios[threads+1];
	for(int i = 0; i < threads+1; ++i)
		ios[i] = new BoolIO<NetIO>(new NetIO(party == ALICE?nullptr:"127.0.0.1",port+i), party==ALICE);

	std::cout << std::endl << "------------ circuit zero-knowledge proof test ------------" << std::endl << std::endl;;

	int num = 0;
	if(argc < 3) {
		std::cout << "usage: [binary] PARTY PORT LOG(INPUT_SZ)" << std::endl;
		return -1;
	} else if (argc==3) {
		num = 20;
	} else {
		num = atoi(argv[3]);
	}


	test_input_speed(ios, party, num);

	for(int i = 0; i < threads+1; ++i) {
		delete ios[i]->io;
		delete ios[i];
	}
	return 0;
}
