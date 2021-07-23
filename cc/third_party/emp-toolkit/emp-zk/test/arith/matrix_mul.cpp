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

void test_circuit_zk(BoolIO<NetIO> *ios[threads], int party, int matrix_sz) {
	long long test_n = matrix_sz * matrix_sz;

	uint64_t *ar, *br, *cr;
	ar = new uint64_t[test_n];
	br = new uint64_t[test_n];
	cr = new uint64_t[test_n];
	for(int i = 0; i < test_n; ++i) {
		ar[i] = i;
		br[i] = (test_n-i);
		cr[i] = 0;
	}
	for(int i = 0; i < matrix_sz; ++i) {
		for(int j = 0; j < matrix_sz; ++j) {
			for(int k = 0; k < matrix_sz; ++k) {
				uint64_t tmp = (ar[i*matrix_sz+j] * br[j*matrix_sz+k])%pr;
				cr[i*matrix_sz+k] = (cr[i*matrix_sz+k] + tmp)%pr;
			}
		}
	}

	auto start = clock_start();

	setup_zk_arith<BoolIO<NetIO>>(ios, threads, party);

	IntFp *mat_a = new IntFp[test_n];
	IntFp *mat_b = new IntFp[test_n];
	IntFp *mat_c = new IntFp[test_n];
	
	for(int i = 0; i < test_n; ++i) {
		mat_a[i] = IntFp((uint64_t)i, ALICE);
		mat_b[i] = IntFp((uint64_t)(test_n-i), ALICE);
		mat_c[i] = IntFp((uint64_t)0, PUBLIC);
	}

	for(int i = 0; i < matrix_sz; ++i) {
		for(int j = 0; j < matrix_sz; ++j) {
			for(int k = 0; k < matrix_sz; ++k) {
				IntFp tmp = mat_a[i*matrix_sz+j] * mat_b[j*matrix_sz+k];
				mat_c[i*matrix_sz+k] = mat_c[i*matrix_sz+k] + tmp;
			}
		}
	}

	batch_reveal_check(mat_c, cr, test_n);
	auto timeuse = time_from(start);
	finalize_zk_arith<BoolIO<NetIO>>();
	cout << matrix_sz << "\t" << timeuse << " us\t" << party << " " << endl;
	std::cout << std::endl;

	delete[] ar;
	delete[] br;
	delete[] cr;
	delete[] mat_a;
	delete[] mat_b;
	delete[] mat_c;


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
	BoolIO<NetIO>* ios[threads];
	for(int i = 0; i < threads; ++i)
		ios[i] = new BoolIO<NetIO>(new NetIO(party == ALICE?nullptr:"127.0.0.1",port+i), party==ALICE);

	std::cout << std::endl << "------------ circuit zero-knowledge proof test ------------" << std::endl << std::endl;;

	int num = 0;
	if(argc < 3) {
		std::cout << "usage: bin/arith/matrix_mul_arith PARTY PORT DIMENSION" << std::endl;
		return -1;
	} else if (argc == 3) {
		num = 10;
	} else {
		num = atoi(argv[3]);
	}
	

	test_circuit_zk(ios, party, num);

	for(int i = 0; i < threads; ++i) {
		delete ios[i]->io;
		delete ios[i];
	}
	return 0;
}
