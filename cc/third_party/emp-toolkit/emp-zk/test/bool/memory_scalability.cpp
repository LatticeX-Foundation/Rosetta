#include <emp-zk/emp-zk.h>
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
const string circuit_file_location = macro_xstr(EMP_CIRCUIT_PATH) + string("bristol_format/");

void merkle_tree(Bit *dig, int node_index, int current_level, int depth, bool* witness, BristolFormat *cf) {
	Bit msg[768];
	if(current_level < (depth-1)) {
		merkle_tree(msg, 2*node_index, current_level+1, depth, witness, cf);
		merkle_tree(msg+256, 2*node_index+1, current_level+1, depth, witness, cf);
		for(int i = 512; i < 768; ++i)
			msg[i] = Bit(true, PUBLIC);
	} else if((current_level+1) == depth) {
		for(int i = 0; i < 512; ++i)
			msg[i] = Bit(witness+node_index*512+i, ALICE);
	}
	cf->compute((block*)dig, (block*)msg, nullptr);
}

void test_merkle_tree_dfs(BoolIO<NetIO> *ios[threads], int party, int depth) {
	std::cout << "merkle tree of depth: " << depth << std::endl;
	string file = circuit_file_location+"sha-256.txt";
	BristolFormat cf(file.c_str());

	int width = 1 << (depth-1);
	std::cout << "number of nodes: " << (2*width-1) << std::endl;

	auto start = clock_start();
	setup_zk_bool<BoolIO<NetIO>>(ios, threads, party);

	bool *witness = new bool[width];
	PRG prg;
	prg.random_bool(witness, width);
	Bit dig_cipher_bit[512];

	merkle_tree(dig_cipher_bit, 0, 0, depth, witness, &cf);
	Bit res = Bit(true, PUBLIC);
	bool ret = res.reveal<bool>(PUBLIC);
	finalize_zk_bool<BoolIO<NetIO>>();
	std::cout << depth << " " << time_from(start)<<" us "<<party<< " " << ret << "\n";

	delete[] witness;

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

void test_merkle_tree(BoolIO<NetIO> *ios[threads], int party, int depth) {
	std::cout << "merkle tree of depth: " << depth << std::endl;
	string file = circuit_file_location+"sha-256.txt";
	int input_n, output_n;
	BristolFormat cf(file.c_str());
	input_n = cf.n1;
	output_n = cf.n3;

	int width = 1 << (depth-1);
	std::cout << "number of nodes: " << (2*width-1) << std::endl;

	auto start = clock_start();
	setup_zk_bool<BoolIO<NetIO>>(ios, threads, party);

	Bit *msg_cipher_bit = new Bit[input_n];
	Bit *dig_cipher_bit = new Bit[output_n*(2*width-1)];

	for(int i = 0; i < input_n; ++i) 
		msg_cipher_bit[i] = Bit(true, ALICE);

	// leaves
	Bit *cipher_ptr = dig_cipher_bit;
	Bit *plain_ptr = cipher_ptr;
	for(int i = 0; i < width; ++i) {
		cf.compute((block*)cipher_ptr, (block*)msg_cipher_bit, nullptr);

		for(int j = 0; j < output_n; ++j)
			msg_cipher_bit[j] = cipher_ptr[j];

		cipher_ptr += output_n;
	}
	width = width>>1;

	// inner nodes
	for(int j = 0; j < depth-1; ++j) {
		for(int i = 0; i < width; ++i) {
			cf.compute((block*)cipher_ptr, (block*)plain_ptr, nullptr);
			cipher_ptr += output_n;
			plain_ptr += output_n*2;
		}
		width = width>>1;
	}

	Bit res = Bit(false, PUBLIC);
	bool ret = res.reveal<bool>(PUBLIC);
	bool cheated = finalize_zk_bool<BoolIO<NetIO>>();
	if(cheated) error("cheated\n");
	std::cout << depth << " " << time_from(start)<<" ms "<<party<< " " << ret << "\n";

	delete[] msg_cipher_bit;
	delete[] dig_cipher_bit;

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


	int depth = 0;
	if(argc < 3) {
		std::cout << "usage: bin/bool/memory_scalability PARTY PORT DEPTH_OF_MERKLE_TREE" << std::endl;
		return -1;
	} else if (argc==3) {
		depth = 5;
	} else {
		depth = atoi(argv[3]);
	}


	test_merkle_tree_dfs(ios, party, depth);
	//test_merkle_tree(ios, party, depth);

	for(int i = 0; i < threads; ++i) {
		delete ios[i]->io;
		delete ios[i];
	}
	return 0;
}
