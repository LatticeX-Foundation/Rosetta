#include "wvr_test.h"
#include "cc/modules/common/include/utils/simple_timer.h"
#include "cc/modules/protocol/zk/wolverine/include/wvr_util.h"

using namespace rosetta::zk;

void run(int partyid) {
  WVR_PROTOCOL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  msg_id_t msgid("feed_test");
  {//test1,  simple private-input 1000*1000, expect values are 20
    log_info << "-----------   test1: private-input feed 1000*1000 ...";
    
    size_t size = 1000*1000;
    vector<double> ma(size, 2.129), mb(size, -19.023);
    
    vector<string> ma_str(size);
    SimpleTimer timer;
    WVR0.GetOps(msgid)->PrivateInput(0, ma, ma_str);
    log_error << "** test1: private input(1000*1000) cost: " << timer.ms_elapse() << " ms";
  }

  {//test2,  simple plain ZkIntFp batch 1000*1000, expect values are 20
    log_info << "-----------   test2: IntFp batch_feed 1000*1000 ...";
    
    size_t size = 1000*1000;
    vector<double> ma(size, 2.129), mb(size, -19.023);
    
    vector<uint64_t> a_fp(size);
    zk_encode(ma, a_fp);

    vector<ZkIntFp> zk_a(size);
    SimpleTimer timer;
    batch_feed(zk_a.data(), a_fp.data(), size);
    log_error << "** test2: batch_feed input(1000*1000) cost: " << timer.ms_elapse() << " ms";
  }

  {//test3,  simple plain ZkIntFp batch 1000*1000, expect values are 20
    log_info << "-----------   test3: IntFp feed 1000*1000 ...";
    
    size_t size = 1000*1000;
    vector<double> ma(size, 2.129), mb(size, -19.023);
    
    vector<uint64_t> a_fp(size);
    zk_encode(ma, a_fp);

    vector<ZkIntFp> zk_a(size);
    SimpleTimer timer;
    for (size_t i = 0; i < size; i++)
    {
      zk_a[i] = ZkIntFp(a_fp[i], ALICE);
    }
    log_error << "** test3: feed input(1000*1000) cost: " << timer.ms_elapse() << " ms";
  }

  {//test4,  simple private-input 1000*1000, expect values are 20
    log_info << "-----------   test4: private-input feed 1000*10000 ...";
    
    size_t size = 1000*10000;
    vector<double> ma(size, 2.129), mb(size, -19.023);
    
    vector<string> ma_str(size);
    SimpleTimer timer;
    WVR0.GetOps(msgid)->PrivateInput(0, ma, ma_str);
    log_error << "** test4: private input(1000*10000) cost: " << timer.ms_elapse() << " ms";
  }

  {//test5,  simple plain ZkIntFp batch 1000*10000, expect values are 20
    log_info << "-----------   test5: IntFp batch_feed 1000*10000 ...";
    
    size_t size = 1000*10000;
    vector<double> ma(size, 2.129), mb(size, -19.023);
    
    vector<uint64_t> a_fp(size);
    zk_encode(ma, a_fp);

    vector<ZkIntFp> zk_a(size);
    SimpleTimer timer;
    batch_feed(zk_a.data(), a_fp.data(), size);
    log_error << "** test5: batch_feed input(1000*10000) cost: " << timer.ms_elapse() << " ms";
  }

  {//test6,  simple plain ZkIntFp batch 1000*10000, expect values are 20
    log_info << "-----------   test6: IntFp feed 1000*10000 ...";
    
    size_t size = 1000*10000;
    vector<double> ma(size, 2.129), mb(size, -19.023);
    
    vector<uint64_t> a_fp(size);
    zk_encode(ma, a_fp);

    vector<ZkIntFp> zk_a(size);
    SimpleTimer timer;
    for (size_t i = 0; i < size; i++)
    {
      zk_a[i] = ZkIntFp(a_fp[i], ALICE);
    }
    
    log_error << "** test6: feed input(1000*10000) cost: " << timer.ms_elapse() << " ms";
  }

  {//test7,  simple private-input 10*10000, expect values are 20
    log_info << "-----------   test4: private-input feed 10*10000 ...";
    
    size_t size = 10*10000;
    vector<double> ma(size, 2.129), mb(size, -19.023);
    
    vector<string> ma_str(size);
    SimpleTimer timer;
    WVR0.GetOps(msgid)->PrivateInput(0, ma, ma_str);
    log_error << "** test7: private input(10*10000) cost: " << timer.ms_elapse() << " ms";
  }

  {//test5,  simple plain ZkIntFp batch 10*10000, expect values are 20
    log_info << "-----------   test8: IntFp batch_feed 10*10000 ...";
    
    size_t size = 10*10000;
    vector<double> ma(size, 2.129), mb(size, -19.023);
    
    vector<uint64_t> a_fp(size);
    zk_encode(ma, a_fp);

    vector<ZkIntFp> zk_a(size);
    SimpleTimer timer;
    batch_feed(zk_a.data(), a_fp.data(), size);
    log_error << "** test8: batch_feed input(10*10000) cost: " << timer.ms_elapse() << " ms";
  }

  {//test6,  simple plain ZkIntFp batch 10*10000, expect values are 20
    log_info << "-----------   test9: IntFp feed 10*10000 ...";
    
    size_t size = 10*10000;
    vector<double> ma(size, 2.129), mb(size, -19.023);
    
    vector<uint64_t> a_fp(size);
    zk_encode(ma, a_fp);

    vector<ZkIntFp> zk_a(size);
    SimpleTimer timer;
    for (size_t i = 0; i < size; i++)
    {
      zk_a[i] = ZkIntFp(a_fp[i], ALICE);
    }
    
    log_error << "** test9: feed input(10*10000) cost: " << timer.ms_elapse() << " ms";
  }

  WVR_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_ZK_TEST(run);