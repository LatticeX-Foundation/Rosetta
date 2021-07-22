#include "wvr_test.h"
#include "cc/modules/common/include/utils/simple_timer.h"

void run(int partyid) {
  WVR_PROTOCOL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  msg_id_t msgid("compare_ops_test");
  
  // {//test1,  simple one element compare
  //   log_info << "-----------   test1: compare one element ...";
  //   double pa = -2.88, pb = -3.98;
  //   double na = 2.88, nb = 3.98;
  //   double zero = 0;
  //   size_t size = 1;
  //   SimpleTimer timer;
  //   vector<double> pas(1, pa), pbs(1, pb);
  //   vector<double> nas(1, na), nbs(1, nb);
    
  //   vector<string> ma_str, mb_str;
  //   WVR0.GetOps(msgid)->PrivateInput(0, pas, ma_str);
  //   WVR0.GetOps(msgid)->PrivateInput(0, pbs, mb_str);
  //   log_info << "** private input cost: " << timer.ms_elapse() << " ms";

  //   vector<string> result;
  //   unordered_map<string,string> attrs;
  //   timer.start();

  //   vector<string> tmp;
  //   WVR0.GetOps(msgid)->GreaterEqual(ma_str, mb_str, tmp, &attrs);
  //   // WVR0.GetOps(msgid)->Relu(tmp, result, &attrs);//GreateEqual
  //   log_info << "** GreaterEqual cost: " << timer.ms_elapse() << " ms";

  //   vector<double> expect(size, 0.0);
  //   WVR0.GetOps(msgid)->Reveal(result, expect);
  //   log_info << "-----------   test1: integer (10,10)*(10,10)  check and ok. expect: 1.0, reveal: " << expect[0];
  // }

  {
    log_info << "-----------   test1: less_equal one element ...";
    double pa = -2.88, pb = -3.98;
    double na = 2.88, nb = 3.98;
    double zero = 0;
    size_t size = 1;
    SimpleTimer timer;
    vector<double> pas(1, pa), pbs(1, pb);
    vector<double> nas(1, na), nbs(1, nb);
    
    vector<string> ma_str, mb_str;
    vector<ZkIntFp> sa(1, ZkIntFp(pa, PUBLIC)), sb(1, ZkIntFp(pb, PUBLIC));
    log_info << "** private input cost: " << timer.ms_elapse() << " ms";


    vector<ZkIntFp> result(1);
    ZkIntFp::zk_fp_less_equal(sa.data(), sb.data(), sa.size(), partyid, result);
    log_info << "less_equal result:  " << result[0];//0
    // ZkIntFp::zk_fp_less(sa.data(), sb.data(), sa.size(), partyid, result);
    // log_info << "less result:  " << result[0];//0
    // ZkIntFp::zk_fp_equal(sa.data(), sb.data(), sa.size(), partyid, result);
    // log_info << "equal result:  " << result[0];//0
    // ZkIntFp::zk_fp_greater_equal(sa.data(), sb.data(), sa.size(), partyid, result);
    // log_info << "greater_equal result:  " << result[0];//1
    ZkIntFp::zk_fp_greater_equal(sa.data(), sb.data(), sa.size(), partyid, result);
    log_info << "greater_equal result:  " << result[0];//1
    
  }

  WVR_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_ZK_TEST(run);