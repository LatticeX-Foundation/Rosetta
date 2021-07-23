#include "emp-zk/emp-zk.h"
#include <iostream>
#include "emp-tool/emp-tool.h"
#if defined(__linux__)
	#include <sys/time.h>
	#include <sys/resource.h>
#elif defined(__APPLE__)
	#include <unistd.h>
	#include <sys/resource.h>
	#include <mach/mach.h>
#endif

using namespace emp;
using namespace std;

int port, party;
const int threads = 1;

void test_sis_proof(BoolIO<NetIO> *ios[threads+1], int party, int n, int m) {

	srand(time(NULL));

	int mat_size = n * m;
	uint64_t *A, *s, *t;
	A = new uint64_t[mat_size];
	s = new uint64_t[m];
	t = new uint64_t[n+m];
	for(int i = 0; i < mat_size; ++i)
		A[i] = mod(rand());
	for(int i = 0; i < m; ++i)
		s[i] = rand()%2;
	for(int i = 0; i < (m+n); ++i) {
		t[i] = (uint64_t)0;
	}
	for(int i = 0; i < n; ++i) {
		for(int j = 0; j < m; ++j) {
			t[i] = add_mod(t[i], A[i*m+j]*s[j]);
		}
	}

	int repeat = 485;
	auto start = clock_start();
	setup_zk_arith<BoolIO<NetIO>>(ios, threads, party);

	// allocation
	IntFp *vec_s = new IntFp[m];
	IntFp *vec_t = new IntFp[n+m];
	IntFp *vec_r = new IntFp[m];
	uint64_t minusone = PR-1;
	int tt = 0;
	
	for(int round = 0; round < repeat; round++) {
		// init
		for(int i = 0; i < m; ++i)
			vec_s[i] = IntFp(s[i], ALICE);
		for(int i = 0; i < (n+m); ++i)
			vec_t[i] = IntFp((uint64_t)0, PUBLIC);

		// r[i] = s[i]^2
		for(int i = 0; i < m; ++i)
			vec_r[i] = vec_s[i] * vec_s[i];
		
		// y[i] = sum{A[j][i]*s[i]}
		for(int i = 0; i < n; ++i) {
			for(int j = 0; j < m; ++j) {
				IntFp tmp = vec_s[j] * A[i*m+j];
				vec_t[i] = vec_t[i] + tmp;
			}
		}

		// y[i+n] = r[i] - s[i]
		for(int i = 0; i < m; ++i) {
			vec_s[i] = vec_s[i] * minusone;
			vec_t[i+n] = vec_r[i] + vec_s[i];
		}
		tt += 1;
	}

	bool ret = batch_reveal_check(vec_t, t, n+m);
	finalize_zk_arith<BoolIO<NetIO>>();
	auto timeuse = time_from(start);
	cout << n << "\t" << m << "\t" << timeuse/tt << " us\t" << party << " " << ret << endl;
	std::cout << std::endl;

	delete[] A;
	delete[] s;
	delete[] t;
	delete[] vec_s;
	delete[] vec_t;
	delete[] vec_r;


#if defined(__linux__)
	struct rusage rusage;
	if (!getrusage(RUSAGE_SELF, &rusage))
		std::cout << "[Linux]Peak resident set size: " << (size_t)rusage.ru_maxrss << std::endl;
	else std::cout << "[Linux]Query RSS failed" << std::endl;
#elif defined(__APPLE__)
	struct mach_task_basic_info info;
	mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
	if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&info, &count) == KERN_SUCCESS)
		std::cout << "[Mac]Peak resident set size: " << (size_t)info.resident_size_max << std::endl;
	else std::cout << "[Mac]Query RSS failed" << std::endl;
#endif
}

int main(int argc, char** argv) {
	parse_party_and_port(argv, &party, &port);
	BoolIO<NetIO>* ios[threads+1];
	for(int i = 0; i < threads+1; ++i)
		ios[i] = new BoolIO<NetIO>(new NetIO(party == ALICE?nullptr:"127.0.0.1",port+i), party==ALICE);

	std::cout << std::endl << "------------ circuit zero-knowledge proof test ------------" << std::endl << std::endl;;

	test_sis_proof(ios, party, 8, 16);
//	test_sis_proof(ios, party, 1024, 4096);
//	test_sis_proof(ios, party, 256, 256*61);

	for(int i = 0; i < threads+1; ++i) {
		delete ios[i]->io;
		delete ios[i];
	}
	return 0;
}
