#include <iostream>
#include "wvr_test.h"
// #include "emp-wolverine-fp/emp-wolverine-fp.h"
#include "emp-tool/emp-tool.h"

using namespace emp;
using namespace std;

int port, party;
const int threads = 5;

using ZkVec=std::vector<IntFp>;

void test_bool(ZK_NET_IO *ios[threads + 1], int party)
{
  cout << "start to setup....\n";
  auto start = clock_start();
  // setup_boolean_zk<NetIO>(ios, threads, party);
  // setup_fp_zk<NetIO>(ios, threads, party);
  
  setup_zk_bool<ZK_NET_IO>(ios, threads, party);
  setup_zk_arith<BoolIO<NetIO>>(ios, threads, party, true);
  
  cout << "setup use: " << time_from(start) << endl;

  start = clock_start();

  // private input 1 IntFp, then multiply
  size_t size = 1;
  ZkVec a(size), b(size), c(size);
  for (size_t i = 0; i < size; ++i)
  {
    a[i] = IntFp(uint64_t(i+1), ALICE);
    b[i] = IntFp(uint64_t(i+1), ALICE);
    c[i] = b[i] * a[i];
  }

  // to bool
  std::cout << "party: " << party << ", arith2bool<BoolIO<NetIO>>..." << endl;
  vector<Integer> bc(size);
  sync_zk_bool<BoolIO<NetIO>>();
  arith2bool<BoolIO<NetIO>>(bc.data(), c.data(), size);
  // sync_zk_bool<BoolIO<NetIO>>();
  // bool reveal
  Integer five(62, 5, PUBLIC);
  Bit ret(false, PUBLIC);
  std::cout << "party: " << party << ", arith2bool<BoolIO<NetIO>> ok." << endl;
  for (size_t i = 0; i < size; i++)
  {
    ret = ret & bc[i].geq(five);// just for test, no meaning
    // std::cout << "party: " << party << ", reveal-"<< i << "  =>  " << ret.reveal<bool>(PUBLIC) << endl;
  }
  // sync_zk_bool<BoolIO<NetIO>>();
  std::cout << "party: " << party << ", ret reveal:  =>  " << ret.reveal<bool>(PUBLIC) << endl;

  // sync_zk_bool<BoolIO<NetIO>>();
  
  // batch check 
  vector<uint64_t> expect{1,4,9,16,25,   36,49,64,81,100};
  bool status = batch_reveal_check(c.data(), expect.data(), size);

  sync_zk_bool<BoolIO<NetIO>>();
  cout << "party-" << party << "\t, test_bool, cost time : " << time_from(start) << ", check: :  " << status << endl;
  // finalize_fp_zk();// when finalize zk, it gets "and gate check fails" error
  // finalize_boolean_zk<NetIO>(party);
  
  finalize_zk_bool<BoolIO<NetIO>>();
	finalize_zk_arith<BoolIO<NetIO>>();
}


int main(int argc, char** argv) {
	parse_party_and_port(argv, &party, &port);
	ZK_NET_IO* ios[threads+1];
	for(int i = 0; i < threads+1; ++i)
		//ios[i] = new NetIO(party == ALICE?nullptr:"127.0.0.1", port+i);
    ios[i] = new ZK_NET_IO(new NetIO(party == ALICE ? nullptr : "127.0.0.1", port + i), party == ALICE);
	std::cout << std::endl << "------------ zk bool test,  we will block in arith2bool<BoolIO<NetIO>>(), then \"AND gate check fails\" ------------" << std::endl << std::endl;;
  test_bool(ios, party);

	for(int i = 0; i < threads+1; ++i)
		delete ios[i];
	return 0;
}
