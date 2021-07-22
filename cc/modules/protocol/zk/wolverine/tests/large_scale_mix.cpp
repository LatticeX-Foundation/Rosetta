#include "emp-tool/emp-tool.h"
#include "wvr_test.h"
// #include "emp-wolverine-bool/emp-wolverine-bool.h"
// #include "emp-wolverine-fp/emp-wolverine-fp.h"
#include <iostream>
#include <stdexcept>

using namespace emp;
using namespace std;

int port, party;
const int threads = N_THREADS;

/* 
	GeorgeShi: We use this case to test the effect of SIZE parameter of arith2bool<BoolIO<NetIO>>.
	
	The main logic is just to square each element of a vector:
	Z[i]= X[i] * X[i];
*/
void test_mix_circuit(ZK_NET_IO *ios[threads], int party) {
	srand(time(NULL));
	// 100,001 is the precise threshold. It works fine when `sz` is less than 100,001. 
	int sz = 5*100 * 1000 + 1;
	std::cout << "###################### Test begin! Size is " << sz << std::endl;
	uint64_t *a = new uint64_t[sz];
	uint64_t *c = new uint64_t[sz];
	// To ease our debuging, we fix the values.
	for(int i = 0; i < sz; ++i) {
		a[i] = 3; // rand() % PR;
		c[i] = (a[i] * a[i]) % PR;
	}

  	setup_zk_bool<ZK_NET_IO>(ios, threads, party);
  	setup_zk_arith<BoolIO<NetIO>>(ios, threads, party, true);

	// 1, init and compute in ZK-style.
	vector<IntFp> x(sz);
	vector<IntFp> z(sz);
	vector<IntFp> z_prime(sz);
	vector<IntFp> arith_diff(sz);
	for(int i = 0; i < sz; ++i) {
		x[i] = IntFp(a[i], ALICE);
		z_prime[i] = IntFp(a[i] * a[i], ALICE);
		z[i] = x[i] * x[i];
		arith_diff[i] = z[i] + z_prime[i].negate();
	}
	std::cout << "debug stub 1: ZK Mul DONE!" << std::endl;
	// 2. check ZK mul result.
	batch_reveal_check(z.data(), c, sz);
	std::cout << "debug stub 2: batch_reveal_check DONE!" << std::endl;

	// 3. We convert the arithematic values to their boolean counterparts.
	Integer ZERO(62, 0, PUBLIC);
	vector<Integer> bool_diff(sz, ZERO);
	sync_zk_bool<BoolIO<NetIO>>();
	arith2bool<BoolIO<NetIO>>(bool_diff.data(), arith_diff.data(), sz);
	// sync_zk_bool<BoolIO<NetIO>>();
	std::cout << "debug stub 3: arith2bool<BoolIO<NetIO>> DONE!" << std::endl;
	
	Bit all_res(true);
	vector<Bit> curr_res(sz);
	for(int i = 0; i < sz; ++i) {
		curr_res[i] = bool_diff[i].equal(ZERO);
		all_res = all_res & curr_res[i];
	}
	bool plain_res = all_res.reveal<bool>(PUBLIC);
	sync_zk_bool<BoolIO<NetIO>>();
	if(!plain_res) {
		std::cout << "Check Fail! And the detail context is:" << std::endl;
		for(int i = 0; i < sz; ++i) {
			bool plain_curr_res = curr_res[i].reveal<bool>(PUBLIC);
			if(!plain_curr_res) {
				std::cout << i << "-th check fail:" << std::endl;
				std::cout << i << "-th plain arith   diff :" << arith_diff[i].reveal() << std::endl;
				std::cout << i << "-th plain boolean diff :" << bool_diff[i].reveal<uint64_t>(PUBLIC) << std::endl;
				throw std::runtime_error("CHECK ZK FAILED!");
			}
		}
	}

  	finalize_zk_bool<BoolIO<NetIO>>();
	finalize_zk_arith<BoolIO<NetIO>>();
	std::cout << "debug stub 4: test PASS! #############################" << std::endl;
	delete[] a;
	delete[] c;
}

int main(int argc, char** argv) {
	parse_party_and_port(argv, &party, &port);
	ZK_NET_IO* ios[threads+1];
	for(int i = 0; i < threads; ++i)
		ios[i] = new ZK_NET_IO(new NetIO(party == ALICE ? nullptr : "127.0.0.1", port + i), party == ALICE);

	std::cout << std::endl << "------------ circuit zero-knowledge proof test ------------" << std::endl << std::endl;;

	test_mix_circuit(ios, party);
	
    for(int i = 0; i < threads; ++i)
		delete ios[i];
	return 0;
}