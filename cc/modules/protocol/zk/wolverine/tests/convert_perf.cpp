#include "wvr_test.h"
#include "cc/modules/common/include/utils/simple_timer.h"
#include "cc/modules/protocol/zk/wolverine/include/wvr_util.h"

using namespace std;
using namespace rosetta::zk;
using namespace emp;

void test_convert() {
  size_t size = 2560;
  vector<ZkIntFp> zks(size);
  vector<string> zks_str(size);
  for (size_t i = 0; i < size; ++i) {
    zks[i] = ZkIntFp(i+100, ALICE);
  }

  size_t test_count = 1;
  std::cout << "convert_mac_to_string ...\n" << endl;
  SimpleTimer timer;
  for (size_t i = 0; i < test_count; i++)
  {
    convert_mac_to_string(zks, zks_str);
  }
  std::cout << "convert_mac_to_string " << size << " element, cost: " << timer.ns_elapse() << " ns \n" << endl;

  std::cout << "---------------------  a2b ---------------------\n" << endl;
  test_count = 100;
  sync_zk_bool<BoolIO<NetIO>>();
  // a2b
  vector<Integer> bin(size);
  for (size_t i = 0; i < test_count; i++)
  {
    timer.start();
    arith2bool<BoolIO<NetIO>>(bin.data(), zks.data(), zks.size());
    std::cout << "a2b (" << size << ") cost: " << timer.elapse() << " seconds\n" << endl;
  }

  // b2a
  std::cout << "---------------------  b2a ---------------------\n";
  for (size_t i = 0; i < test_count; i++)
  {
    timer.start();
    bool2arith<BoolIO<NetIO>>(zks.data(), bin.data(), zks.size());
    std::cout << "b2a (" << size << ") cost: " << timer.elapse() << " seconds\n" << endl;
  }
  sync_zk_bool<BoolIO<NetIO>>();
}

void test_matmul_const()
{
  size_t test_count = 10;
  std::cout << "test_matmul_const ...\n" << endl;
  size_t size = 256;
  size_t m = 256, k = 256, n = 256;
  ZkMatrix za(m, k);
  za.fill(ZkIntFp(double(1.2), PUBLIC));
  
  U64Matrix plain_b(k, n);
  plain_b.fill(2.2);
  
  ZkMatrix zc(m, n);
  zc.fill(ZkIntFp((uint64_t)0, PUBLIC));
  SimpleTimer timer;
  for (size_t i = 0; i < test_count; i++)
  {
    zk_eigen_const_matmul(za, plain_b, zc);
  }
  log_error << "zk_eigen_const_matmul(" << m << "," << k << "," << n << ") " << test_count <<" times,  cost: " << timer.elapse() << " seconds\n" << endl;
}

void run(int partyid) {
  WVR_PROTOCOL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  {
    // test_convert();
    test_matmul_const();
  }
  
  WVR_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_ZK_TEST(run);

