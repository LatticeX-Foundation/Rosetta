#include <iostream>
// #include "emp-wolverine-fp/emp-wolverine-fp.h"
#include "wvr_test.h"
#include "emp-tool/emp-tool.h"

using namespace emp;
using namespace std;

int port, party;
const int threads = 5;

using ZkVec=std::vector<IntFp>;


static int wolverine_matmul_fp(const ZkVec &a, const ZkVec &b, ZkVec &c, int party_id)
{
  cout << party_id << " : fixpoint matmul, (" << a.size() << " * " << b.size() << ")" << std::endl;
  if (a.size() == 0 || b.size() == 0)
    throw std::runtime_error("null input, wolverine_matmul_fp");

  // size multiplication
  // 10*size multiplications
  size_t size = a.size(); //
  c.resize(a.size());
  // prover owns zk-secret
  if (party_id == 1)
  {
    vector<uint64_t> a_b(size);
    for (size_t i = 0; i < size; i++)
    {
      a_b[i] = (a[i].value >> 64) * (b[i].value >> 64);
    }

    // Prover private_input C_prime
    for (size_t i = 0; i < a_b.size(); ++i)
    {
      c[i] = IntFp(a_b[i], ALICE);
    }
  } //Prover
  else
  {
    // Prover private_input C_prime, verifier receive
    for (size_t i = 0; i < size; ++i)
    {
      c[i] = IntFp((uint64_t)0, ALICE);
    }
  }

  ZkVec u(size);
  ZkVec x(2 * size);
  size_t index = 0;
  for (size_t i = 0; i < size; i++)
  {
    for (size_t j = 0; j < 2; j++)
      x[index++] = u[i] * a[i];
  }

  cout << "\n" << party_id << " wolverine_matmul_fp ok.\n";
  return 0;
}

void test_intfp_mul_check(ZK_NET_IO *ios[threads + 1], int party)
{
  //////////    test basic ops: multiplication
  cout << "start to setup....\n";
  auto start = clock_start();
  // setup_boolean_zk<NetIO>(ios, threads, party);
  // setup_fp_zk<NetIO>(ios, threads, party);
  setup_zk_bool<ZK_NET_IO>(ios, threads, party);
  setup_zk_arith<BoolIO<NetIO>>(ios, threads, party, true);
  
  cout << "setup use: " << time_from(start) << endl;

  start = clock_start();

  // private input 12 IntFp
  size_t size = 100;
  ZkVec a(size), b(size), c(size);
  for (size_t i = 0; i < size; ++i)
  {
    a[i] = IntFp(uint64_t(1), ALICE);
    b[i] = IntFp(uint64_t(1), ALICE);
  }

  // fixpoint mul, 12*10 mul
  wolverine_matmul_fp(a, b, c, party);

  // check
  vector<uint64_t> expect(size, 1);
  bool status = batch_reveal_check(c.data(), expect.data(), size);

  cout << "party-" << party << "\t, test_intfp_mul_check, cost time : " << time_from(start) << ", check: :  " << status << endl;
  // finalize_boolean_zk<NetIO>(party);
  
  finalize_zk_bool<BoolIO<NetIO>>();
	finalize_zk_arith<BoolIO<NetIO>>();
}

void test_inner_prdt(ZK_NET_IO *ios[threads + 1], int party, size_t size) {
  cout << "start to setup....\n";
  auto start = clock_start();
  // setup_boolean_zk<NetIO>(ios, threads, party);
  // setup_fp_zk<NetIO>(ios, threads, party);
  setup_zk_bool<ZK_NET_IO>(ios, threads, party);
  setup_zk_arith<BoolIO<NetIO>>(ios, threads, party, true);
  
  cout << "setup use: " << time_from(start) << endl;
  size = 1;
  start = clock_start();
  vector<IntFp> za(size, IntFp((uint64_t)1, ALICE));
  vector<IntFp> zb(size, IntFp((uint64_t)2, ALICE));

  fp_zkp_inner_prdt<BoolIO<NetIO>>(za.data(), zb.data(), (uint64_t)2*size, size);
  cout << "test_inner_prdt ok, cost(us): " << time_from(start) << endl;

  // finalize_fp_zk();
  // finalize_boolean_zk<NetIO>(party);
  finalize_zk_bool<BoolIO<NetIO>>();
	finalize_zk_arith<BoolIO<NetIO>>();
}

int main(int argc, char** argv) {
	parse_party_and_port(argv, &party, &port);
	ZK_NET_IO* ios[threads+1];
	for(int i = 0; i < threads+1; ++i)
		ios[i] = new ZK_NET_IO(new NetIO(party == ALICE ? nullptr : "127.0.0.1", port + i), party == ALICE);
	std::cout << std::endl << "------------ circuit zero-knowledge fixpoint test ------------" << std::endl << std::endl;;
  // test_intfp_mul_check(ios, party);
  // test_circuit_zk(ios, party, 0);
  test_inner_prdt(ios, party, 10);

	for(int i = 0; i < threads+1; ++i)
		delete ios[i];
	return 0;
}
