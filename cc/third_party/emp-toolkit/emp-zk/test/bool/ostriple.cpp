#include "emp-tool/emp-tool.h"
#include <emp-zk/emp-zk.h>
#include <iostream>
using namespace emp;
using namespace std;

int port, party;
const int threads = 1;

void test_auth_bit_input(OSTriple<BoolIO<NetIO>> *os, BoolIO<NetIO> *io) {
	PRG prg;
	int len = 1024;
	block *auth = new block[len];
	bool *in = new bool[len];
	if(party == ALICE) {
		PRG prg;
		prg.random_bool(in, len);
		os->authenticated_bits_input(auth, in, len);
		os->check_auth_mac(auth, in, len, io);
	} else {
		os->authenticated_bits_input(auth, in, len);
		os->check_auth_mac(auth, in, len, io);
	}
	delete[] auth;
	delete[] in;
	io->flush();
}

void test_compute_and_gate_check(OSTriple<BoolIO<NetIO>> *os, BoolIO<NetIO> *io) {
	PRG prg;
	int len = 1024;
	block *a = new block[3*len];
	bool *ain = new bool[3*len];
	if(party == ALICE) {
		prg.random_bool(ain, 2*len);
	}
	os->authenticated_bits_input(a, ain, 2*len);
	os->check_auth_mac(a, ain, 2*len, io);
	std::cout << "generate triple inputs" << std::endl;
	for(int i = 0; i < len; ++i) {
		a[2*len+i] = os->auth_compute_and(a[i], a[len+i]);
		ain[2*len+i] = getLSB(a[2*len+i]);
	}
	std::cout << "compute AND" << std::endl;
	os->check_auth_mac(a+2*len, ain+2*len, len, io);

	os->check_compute_and(a, a+len, a+2*len, len, io);
	std::cout << "check for computing AND gate\n";

	std::cout << "number of triples computed in buffer: " << os->andgate_cnt << "\n";

	delete[] a;
	delete[] ain;
	io->flush();
}
void test_ostriple(BoolIO<NetIO> *ios[threads+1], int party) {
	auto t1 = clock_start();
	OSTriple<BoolIO<NetIO>> os(party, threads, ios);
	cout <<party<<"\tconstructor\t"<< time_from(t1)<<" us"<<endl;

	test_auth_bit_input(&os, ios[0]);
	std::cout << "check for authenticated bit input\n";

	test_compute_and_gate_check(&os, ios[0]);
}

int main(int argc, char** argv) {
	parse_party_and_port(argv, &party, &port);
	BoolIO<NetIO>* ios[threads];
	for(int i = 0; i < threads; ++i)
		ios[i] = new BoolIO<NetIO>(new NetIO(party == ALICE?nullptr:"127.0.0.1",port+i), party == ALICE);

	std::cout << std::endl << "------------ triple generation test ------------" << std::endl << std::endl;;

	test_ostriple(ios, party);
	for(int i = 0; i < threads; ++i) {
		delete ios[i]->io;
		delete ios[i];
	}
	return 0;
}
