#include "emp-zk/emp-vole/cope.h"
#include "emp-tool/emp-tool.h"

using namespace emp;
using namespace std;

int party, port;

void test_cope(NetIO *io, int party) {
	Cope<NetIO> cope(party, io, MERSENNE_PRIME_EXP);

	PRG prg;
	__uint128_t Delta;
	prg.random_data(&Delta, sizeof(__uint128_t));
	Delta = Delta & ((__uint128_t)0xFFFFFFFFFFFFFFFFLL);
	Delta = mod(Delta, pr);

	int test_n = 1024*128;
	__uint128_t mac[test_n];

	// test batch
	if(party == ALICE) {
		cope.initialize(Delta);
		cope.extend(mac, test_n);
		cope.check_triple((uint64_t*)&Delta, mac, test_n);
	} else {
		cope.initialize();
		uint64_t *u = new uint64_t[test_n];
		prg.random_block((block*)u, test_n/2);
		for(int i = 0; i < test_n; ++i) {
			u[i] = mod(u[i]);
		}

		auto start = clock_start();
		cope.extend(mac, u, test_n);
		double ttt = time_from(start);
		std::cout << "batch triple generation: " << ttt << " ms" << std::endl;
		cope.check_triple(u, mac, test_n);
		delete[] u;
	}
}

int main(int argc, char** argv) {
	parse_party_and_port(argv, &party, &port);
	NetIO *io = new NetIO(party == ALICE?nullptr:"127.0.0.1",port);

	std::cout << std::endl << "------------ COPE ------------" << std::endl << std::endl;;

	test_cope(io, party);

	delete io;
	return 0;
}
