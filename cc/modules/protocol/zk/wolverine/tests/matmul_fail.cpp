#include <iostream>
// #include "emp-wolverine-fp/emp-wolverine-fp.h"
#include "wvr_test.h"

#include "emp-tool/emp-tool.h"

using namespace emp;
using namespace std;

int port, party;
const int threads = 5;

using ZkVec=std::vector<IntFp>;


static inline IntFp zk_fp_sub_debug(const IntFp &lhs, const IntFp &rhs, int party)
{
  IntFp res = lhs;
  if (ALICE == party)
  {
    __uint128_t val = mod((res.value >> 64) + (pr - (rhs.value >> 64)), pr);
    __uint128_t mac = mod((res.value & 0xFFFFFFFFFFFFFFFFULL) + (pr - (rhs.value & 0xFFFFFFFFFFFFFFFFULL)), pr);
    res.value = (val << 64) | mac;
  }
  else
  {
    res.value = mod(res.value + (pr - rhs.value), pr);
  }

  return res;
}

static inline int wolverine_matmul_plain(const uint64_t* a, const uint64_t* b, int N, int K, int M, uint64_t* c) {
  cout << "\n------------------------------ manual matmul...\n\n";
  for (size_t i = 0; i < N; i++)
  {
    for (size_t j = 0; j < M; j++)
    {
      //a(i*m)* [b(0)...b(mm)]
      uint64_t acc = 0;
      for (size_t l = 0; l < K; l++)
      {
        // IntFp tmp = (a[i*K+l] * b[l*M+j]);
        // cout << "temp: " << (uint64_t)(tmp.value >> 64) << ", \n";
        acc = acc + (a[i*K+l] * b[l*M+j]);
      }
      c[i*M+j] = acc;
      // cout << "output: " << c[i*M+j] << ", \n";
    }
    cout << "\n";
  }
  cout << "\n";

  return 0;
}

// matmul
static inline int wolverine_matmul_manual_debug(const IntFp* a, const IntFp* b, int N, int K, int M, IntFp* c) {
  cout << "\n------------------------------ manual matmul...\n\n";
  for (size_t i = 0; i < N; i++)
  {
    for (size_t j = 0; j < M; j++)
    {
      //a(i*m)* [b(0)...b(mm)]
      IntFp acc(0, PUBLIC);
      for (size_t l = 0; l < K; l++)
      {
        acc = acc + (a[i*K+l] * b[l*M+j]);
      }
      c[i*M+j] = acc;
      // cout << (uint64_t)(c[i*M+j].value >> 64) << ", \t";
    }
    // cout << "\n";
  }
  // cout << "\n";

  return 0;
}

static inline int wolverine_matmul_manual_debug(const ZkVec& a, const ZkVec& b, int N, int K, int M, ZkVec& c) {
  if (a.empty() || b.empty())
    throw std::runtime_error("null input, wolverine_matmul_dotproduct");
  
  if (!(a.size() == N*K && b.size() == K*M))
    throw std::runtime_error("wolverine_matmul_dotproduct, bad input `a.size() == n*k &&| b.size() == k*m` is false");

  c.resize(N*M, IntFp(0, PUBLIC));
  return wolverine_matmul_manual_debug(a.data(), b.data(), N, K, M, c.data());
}

static int wolverine_matmul_fp(const ZkVec &a, const ZkVec &b, ZkVec &c, int party_id, int N, int K, int M)
{
  cout << party_id << " : fixpoint matmul, (" << a.size() << " * " << b.size() << ")" << std::endl;
  if (a.size() == 0 || b.size() == 0)
    throw std::runtime_error("null input, wolverine_matmul_fp");

  // multiplication
  size_t size = N*K;
  ZkVec &C_prime = c;
  C_prime.resize(size);
  // prover owns zk-secret
  if (party_id == 1)
  {
    // plain matmul
    vector<uint64_t> ua(a.size()), ub(b.size()), uc(c.size());
    for (size_t i = 0; i < a.size(); i++)
    {
      ua[i] = uint64_t(a[i].value >> 64);
    }
    for (size_t i = 0; i < a.size(); i++)
    {
      ub[i] = uint64_t(b[i].value >> 64);
    }
    
    // cout << "to do matmul plain....." << endl;
    wolverine_matmul_plain(ua.data(), ub.data(), N, K, M, uc.data());

    // Prover private_input C_prime
    for (size_t i = 0; i < uc.size(); ++i)
    {
      C_prime[i] = IntFp(uc[i], ALICE);
      cout << "C_prime[" << i << "]:  " << (uint64_t)(C_prime[i].value>>64) << "\n";
    }
  } //Prover
  else
  {
    // Prover private_input C_prime, verifier receive
    for (size_t i = 0; i < size; ++i)
    {
      C_prime[i] = IntFp((uint64_t)0, ALICE);
    }
  }

  // generate seed
  emp::block seed_block;
  emp::PRG prg;
  if(party == ALICE) {
    emp::PRG seed_prg;
    prg.random_block(&seed_block);
      ((ZKFpExecPrv<BoolIO<NetIO>>*)(ZKFpExec::zk_exec))->io->send_data(&seed_block, sizeof(seed_block));
      sync_zk_bool<BoolIO<NetIO>>();
  } else {
      cout << "verifier recv seed_block...";
      sync_zk_bool<BoolIO<NetIO>>();
      ((ZKFpExecPrv<BoolIO<NetIO>>*)(ZKFpExec::zk_exec))->io->recv_data(&seed_block, sizeof(seed_block));
  }
  // cout << "prg.seed: " << get_hex_buffer(&seed_block, sizeof(seed_block));
  prg.reseed(&seed_block);

  // // generates u, v
  // // eg. A_prime(n,k), B_prime(k,m), u(1, n) and v(m,1), then x*y=t(1,1)
  ZkVec u(N, IntFp(0, PUBLIC)), v(M, IntFp(0, PUBLIC));

  // random u, v
  size_t u_size = std::ceil(N/2.0);// + N%2;
  size_t v_size = std::ceil(M/2.0);// + M%2;
  vector<emp::block> u_rds(u_size), v_rds(v_size);
  cout << "matmul_fp, step2.1. generate u, v randoms... u size: " << u_size  << ", v size: " << v_size;
  prg.random_block(u_rds.data(), u_rds.size());
  prg.random_block(v_rds.data(), v_rds.size());

  uint64_t* pu = (uint64_t*)u_rds.data();
  uint64_t* pv = (uint64_t*)v_rds.data();
  // public input u
  for (size_t i = 0; i < u_size; i++)
  {
    u[i] = IntFp(mod(*(pu+i)), PUBLIC);
  }
  // public input v
  for (size_t i = 0; i < v_size; i++)
  {
    v[i] = IntFp(mod(*(pv+i)), PUBLIC);
  }

  // x = u * a
  //u(1, n) and v(m,1), x(1,k),  y(k,1), t(1,1)
  ZkVec x(K);//x(1,k)
  wolverine_matmul_manual_debug(u.data(), a.data(), 1, N, K, x.data());
  sync_zk_bool<BoolIO<NetIO>>();

  // y = b * v
  ZkVec y(K);//y(k,1)
  wolverine_matmul_manual_debug(b.data(), v.data(), K, M, 1, y.data());
  sync_zk_bool<BoolIO<NetIO>>();

  // t = x * y
  ZkVec t(1);
  wolverine_matmul_manual_debug(x.data(), y.data(),  1,  K, 1, t.data());
  sync_zk_bool<BoolIO<NetIO>>();
  cout << "matmul_fp, step4. Verifier send random seed to prover, then use PRF generate random vectors u, v";

  // u*C_prime   (1,m)
  ZkVec left(1*M);
  wolverine_matmul_manual_debug(u.data(), C_prime.data(), 1, N, M, left.data());
  sync_zk_bool<BoolIO<NetIO>>();

  // u*C_prime*v  (1,1)
  ZkVec right(1*1);
  wolverine_matmul_manual_debug(left.data(), v.data(), 1, M, 1, right.data());
  sync_zk_bool<BoolIO<NetIO>>();

  IntFp result = zk_fp_sub_debug(t[0], right[0], party);

  uint64_t zero = 0;
  bool ret = batch_reveal_check(&result, &zero, 1);
  cout << "\n" << party_id << " wolverine_matmul_fp ok. result.value: " << (uint64_t)(result.value >> 64)  << ", ret: " << ret << endl;
  return 0;
}

void test_intfp_mul_check(ZK_NET_IO *ios[threads + 1], int party)
{
  cout << "start to setup....\n";
  auto start = clock_start();
  setup_zk_bool<ZK_NET_IO>(ios, threads, party);
  setup_zk_arith<BoolIO<NetIO>>(ios, threads, party, true);
  cout << "setup use: " << time_from(start) << endl;

  start = clock_start();

  // private input 12 IntFp
  size_t N = 10, K = 10, M = 10;
  size_t size = 100;
  ZkVec a(size), b(size), c(size);
  for (size_t i = 0; i < size; ++i)
  {
    a[i] = IntFp(uint64_t(2*(1<<16)), ALICE);
    b[i] = IntFp(uint64_t(4*(1<<16)), ALICE);
  }

  // fixpoint matmul
  wolverine_matmul_fp(a, b, c, party, N, K, M);

  // without truncation, expect 2* (2^32)
  vector<uint64_t> expect(size, 80ULL*(1ULL<<32));
  for (size_t i = 0; i < N; i++)
  {
    for (size_t j = 0; j < M; j++)
    {
      cout << (uint64_t)(c[i*M+j].value >> 64) << "\t";
    }
    cout << endl;
  }
  cout << endl;

  // batch check 
  bool status = batch_reveal_check(c.data(), expect.data(), size);

  cout << "party-" << party << "\t, test_intfp_mul_check, cost time : " << time_from(start) << ", check: :  " << status << endl;
	finalize_zk_bool<BoolIO<NetIO>>();
	finalize_zk_arith<BoolIO<NetIO>>();
}


int main(int argc, char** argv) {
	parse_party_and_port(argv, &party, &port);
	ZK_NET_IO* ios[threads+1];
	for(int i = 0; i < threads+1; ++i)
		//ios[i] = new NetIO(party == ALICE?nullptr:"127.0.0.1", port+i);
    ios[i] = new ZK_NET_IO(new NetIO(party == ALICE ? nullptr : "127.0.0.1", port + i), party == ALICE);

	std::cout << std::endl << "------------ circuit zero-knowledge fixpoint mul test ------------" << std::endl << std::endl;;
  test_intfp_mul_check(ios, party);
  // test_circuit_zk(ios, party, 0);

	for(int i = 0; i < threads+1; ++i)
		delete ios[i];
	return 0;
}
