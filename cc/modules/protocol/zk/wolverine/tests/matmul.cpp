#include "wvr_test.h"
#include "cc/modules/common/include/utils/simple_timer.h"
#include "cc/modules/protocol/zk/wolverine/include/zk_int_fp_eigen.h"

void run(int partyid) {
  WVR_PROTOCOL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  msg_id_t msgid("matmul_ops_test");

  {//test1,  simple matmul (10,10)*(10,10), expect values are 20
    log_info << "-----------   test1: integer matmul(10,10)*(10,10) ...";
    SimpleTimer timer;
    size_t k = 10, m = 10, n = 10;
    size_t size = m*n;
    vector<double> ma(m*k, 2), mb(k*n, 1);//mc(size, 20)
    
    vector<string> ma_str, mb_str;
    WVR0.GetOps(msgid)->PrivateInput(0, ma, ma_str);
    WVR0.GetOps(msgid)->PrivateInput(0, mb, mb_str);
    log_error << "** private input cost: " << timer.ms_elapse() << " ms";

    vector<string> result;
    unordered_map<string,string> attrs;
    attrs.insert(std::pair<string, string>("m", std::to_string(m)));
    attrs.insert(std::pair<string, string>("k", std::to_string(k)));
    attrs.insert(std::pair<string, string>("n", std::to_string(n)));
    timer.start();
    WVR0.GetOps(msgid)->Matmul(ma_str, mb_str, result, &attrs);
    log_error << "** Matmul cost: " << timer.ms_elapse() << " ms";
    // print_vec(result, 10, "[Matmul] cipher result:");
    vector<double> expect(size);// expect 20
    WVR0.GetOps(msgid)->Reveal(result, expect);
    print_vec(expect, 10, "matmul reveal: ");
    
    log_info << "-----------   test1: integer (10,10)*(10,10)  check and ok. expect: 20, reveal: " << expect[0];

    // ////////////////////
    // ZkMatrix za(size, size);
    // za.fill(ZkIntFp((uint64_t)1, ALICE));
    // ZkMatrix zb(size, size);
    // zb.fill(ZkIntFp((uint64_t)2, ALICE));
    // ZkMatrix zc;
    // zk_eigen_matmul_with_inner_prdt_opt(za, zb, zc);
    // cout << "INNER-Product end!!\n\n";
  }

  {//test1-const,  simple matmul_const (10,10)*(10,10), expect values are 20
    log_info << "-----------   test1: integer matmul_const (10,10)*(10,10) ...";
    SimpleTimer timer;
    size_t k = 10, m = 10, n = 10;
    size_t size = m*n;
    double db(1);
    string db_str(sizeof(db), 0);
    memcpy((char*)db_str.data(), &db, sizeof(db));
    vector<double> ma(m*k, 2), mb(k*n, 1);//mc(size, 20)
    
    vector<string> ma_str, mb_str;
    WVR0.GetOps(msgid)->PrivateInput(0, ma, ma_str);
    // WVR0.GetOps(msgid)->PrivateInput(0, mb, mb_str);
    vector<string> mb_str_const(mb.size(), db_str);//std::to_string(1));
    log_error << "** private input cost: " << timer.ms_elapse() << " ms";

    vector<string> result;
    unordered_map<string,string> attrs;
    attrs.insert(std::pair<string, string>("m", std::to_string(m)));
    attrs.insert(std::pair<string, string>("k", std::to_string(k)));
    attrs.insert(std::pair<string, string>("n", std::to_string(n)));
    attrs["rh_is_const"] = "1";
    timer.start();
    WVR0.GetOps(msgid)->Matmul(ma_str, mb_str_const, result, &attrs);
    log_error << "** Matmul cost: " << timer.ms_elapse() << " ms";

    vector<double> expect(size);// expect 20
    WVR0.GetOps(msgid)->Reveal(result, expect);
    log_info << "-----------   test1: integer matmul_const (10,10)*(10,10)  check and ok. expect: 20, reveal: " << expect[0];
  }

  {//test2,  simple matmul (7,7)*(7,7), expect values are 20
    log_info << "-----------   test2: integer matmul(7,7)*(7,7) ...";
    size_t k = 7, m = 7, n = 7;
    size_t size = m*n;
    vector<double> ma(m*k, 2), mb(k*n, 1);//mc(size, 14)
    
    vector<string> ma_str, mb_str;
    SimpleTimer timer;
    WVR0.GetOps(msgid)->PrivateInput(0, ma, ma_str);
    WVR0.GetOps(msgid)->PrivateInput(0, mb, mb_str);
    log_error << "** private input cost: " << timer.ms_elapse() << " ms";

    vector<string> result;
    unordered_map<string,string> attrs;
    attrs.insert(std::pair<string, string>("m", std::to_string(m)));
    attrs.insert(std::pair<string, string>("k", std::to_string(k)));
    attrs.insert(std::pair<string, string>("n", std::to_string(n)));
    timer.start();
    WVR0.GetOps(msgid)->Matmul(ma_str, mb_str, result, &attrs);
    log_error << "** Matmul cost: " << timer.ms_elapse() << " ms";

    vector<double> expect(size);
    WVR0.GetOps(msgid)->Reveal(result, expect);// expect 14
    log_info << "-----------   test2: integer matmul(7,7)*(7,7)  check and ok. expect: 14, reveal: " << expect[0];
  }

  {//test3,  simple matmul(3,5)*(5,1), expect values are 20
    log_info << "-----------   test3: integer matmul(3,5)*(5,1) ...";
    size_t m = 3, k = 5, n = 1;
    size_t size = m*n;
    vector<double> ma(m*k, 2), mb(k*n, 1);//mc(size, 10)
    
    vector<string> ma_str, mb_str;
    SimpleTimer timer;
    WVR0.GetOps(msgid)->PrivateInput(0, ma, ma_str);
    WVR0.GetOps(msgid)->PrivateInput(0, mb, mb_str);
    log_error << "** private input cost: " << timer.ms_elapse() << " ms";

    vector<string> result;
    unordered_map<string,string> attrs;
    attrs.insert(std::pair<string, string>("m", std::to_string(m)));
    attrs.insert(std::pair<string, string>("k", std::to_string(k)));
    attrs.insert(std::pair<string, string>("n", std::to_string(n)));
    timer.start();
    WVR0.GetOps(msgid)->Matmul(ma_str, mb_str, result, &attrs);
    log_error << "** Matmul cost: " << timer.ms_elapse() << " ms";

    vector<double> expect(size);
    WVR0.GetOps(msgid)->Reveal(result, expect);// expect 10
    log_info << "-----------   test3: integer matmul(3,5)*(5,1)  check and ok. expect: 10, reveal: " << expect[0];
  }

  // {//test4,  large scale matmul(196,49)*(49,64), expect values are 20
  //   log_info << "-----------   test4: large scale integer matmul(196,49)*(49,64) ...";
  //   size_t m = 196, k = 49, n = 64;
  //   size_t size = m*n;
  //   vector<double> ma(m*k, 4.24), mb(k*n, 2.434);//expect (m*n, 505.68784);
    
  //   vector<string> ma_str, mb_str;
  //   SimpleTimer timer;
  //   WVR0.GetOps(msgid)->PrivateInput(0, ma, ma_str);
  //   WVR0.GetOps(msgid)->PrivateInput(0, mb, mb_str);
  //   log_error << "** private input cost: " << timer.ms_elapse() << " ms";

  //   vector<string> result;
  //   unordered_map<string,string> attrs;
  //   attrs.insert(std::pair<string, string>("m", std::to_string(m)));
  //   attrs.insert(std::pair<string, string>("k", std::to_string(k)));
  //   attrs.insert(std::pair<string, string>("n", std::to_string(n)));
  //   timer.start();
  //   WVR0.GetOps(msgid)->Matmul(ma_str, mb_str, result, &attrs);
  //   log_error << "** Matmul cost: " << timer.ms_elapse() << " ms";

  //   vector<double> expect(size);
  //   WVR0.GetOps(msgid)->Reveal(result, expect);// expect 505.68784
  //   log_info << "-----------   test4: large scale integer matmul(196,49)*(49,64)  check and ok. expect: 505.68784, reveal: " << expect[0];
  // }

  // {//test5,  large scale matmul(255,49)*(49,63), expect values are 20
  //   log_info << "-----------   test5: large scale integer matmul(255,49)*(49,63) ...";
  //   size_t m = 255, k = 49, n = 63;
  //   size_t size = m*n;
  //   vector<double> ma(m*k, 4.24), mb(k*n, 2.434);//expect (m*n, 505.68784);
    
  //   vector<string> ma_str, mb_str;
  //   SimpleTimer timer;
  //   WVR0.GetOps(msgid)->PrivateInput(0, ma, ma_str);
  //   WVR0.GetOps(msgid)->PrivateInput(0, mb, mb_str);
  //   log_error << "** private input cost: " << timer.ms_elapse() << " ms";

  //   vector<string> result;
  //   unordered_map<string,string> attrs;
  //   attrs.insert(std::pair<string, string>("m", std::to_string(m)));
  //   attrs.insert(std::pair<string, string>("k", std::to_string(k)));
  //   attrs.insert(std::pair<string, string>("n", std::to_string(n)));
  //   timer.start();
  //   WVR0.GetOps(msgid)->Matmul(ma_str, mb_str, result, &attrs);
  //   log_error << "** Matmul cost: " << timer.ms_elapse() << " ms";

  //   vector<double> expect(size);
  //   WVR0.GetOps(msgid)->Reveal(result, expect);// expect 505.68784
  //   log_info << "-----------   test5: large scale integer matmul(255,49)*(49,63)  check and ok. expect: 505.68784, reveal: " << expect[0];
  // }

  // {//test6,  large scale matmul(255,49)*(49,63), expect values are 20
  //   log_info << "-----------   test6: large scale negative integer matmul(255,49)*(49,63) ...";
  //   size_t m = 255, k = 49, n = 63;
  //   size_t size = m*n;
  //   vector<double> ma(m*k, -4.24), mb(k*n, -2.434);//expect (m*n, 505.68784);
    
  //   vector<string> ma_str, mb_str;
  //   SimpleTimer timer;
  //   WVR0.GetOps(msgid)->PrivateInput(0, ma, ma_str);
  //   WVR0.GetOps(msgid)->PrivateInput(0, mb, mb_str);
  //   log_error << "** private input cost: " << timer.ms_elapse() << " ms";

  //   vector<string> result;
  //   unordered_map<string,string> attrs;
  //   attrs.insert(std::pair<string, string>("m", std::to_string(m)));
  //   attrs.insert(std::pair<string, string>("k", std::to_string(k)));
  //   attrs.insert(std::pair<string, string>("n", std::to_string(n)));
  //   timer.start();
  //   WVR0.GetOps(msgid)->Matmul(ma_str, mb_str, result, &attrs);
  //   log_error << "** Matmul cost: " << timer.ms_elapse() << " ms";

  //   vector<double> expect(size);
  //   WVR0.GetOps(msgid)->Reveal(result, expect);// expect 505.68784
  //   log_info << "-----------   test6: large scale negative integer matmul(255,49)*(49,63)  check and ok. expect: 505.68784, reveal: " << expect[0];
  // }

  // {//test7,  large scale matmul(255,49)*(49,63), expect values are 20
  //   log_info << "-----------   test7: large scale negative & positive integer matmul(255,49)*(49,63) ...";
  //   size_t m = 255, k = 49, n = 63;
  //   size_t size = m*n;
  //   vector<double> ma(m*k, -4.24), mb(k*n, 2.434);//expect (m*n, -505.68784);
    
  //   vector<string> ma_str, mb_str;
  //   SimpleTimer timer;
  //   WVR0.GetOps(msgid)->PrivateInput(0, ma, ma_str);
  //   WVR0.GetOps(msgid)->PrivateInput(0, mb, mb_str);
  //   log_error << "** private input cost: " << timer.ms_elapse() << " ms";

  //   vector<string> result;
  //   unordered_map<string,string> attrs;
  //   attrs.insert(std::pair<string, string>("m", std::to_string(m)));
  //   attrs.insert(std::pair<string, string>("k", std::to_string(k)));
  //   attrs.insert(std::pair<string, string>("n", std::to_string(n)));
  //   timer.start();
  //   WVR0.GetOps(msgid)->Matmul(ma_str, mb_str, result, &attrs);
  //   log_error << "** Matmul cost: " << timer.ms_elapse() << " ms";

  //   vector<double> expect(size);
  //   WVR0.GetOps(msgid)->Reveal(result, expect);// expect 505.68784
  //   log_info << "-----------   test7: large scale negative integer matmul(255,49)*(49,63)  check and ok. expect: -505.68784, reveal: " << expect[0];
  // }

  // {//test7,  large scale matmul(255,49)*(49,63), expect values are 20
  //   log_info << "-----------   test7: large scale big positive integer matmul(255,49)*(49,63) ...";
  //   size_t m = 255, k = 49, n = 63;
  //   size_t size = m*n;
  //   vector<double> ma(m*k, 14.24), mb(k*n, 12.434);//expect (m*n, 8675.94784);
    
  //   vector<string> ma_str, mb_str;
  //   SimpleTimer timer;
  //   WVR0.GetOps(msgid)->PrivateInput(0, ma, ma_str);
  //   WVR0.GetOps(msgid)->PrivateInput(0, mb, mb_str);
  //   log_error << "** private input cost: " << timer.ms_elapse() << " ms";

  //   vector<string> result;
  //   unordered_map<string,string> attrs;
  //   attrs.insert(std::pair<string, string>("m", std::to_string(m)));
  //   attrs.insert(std::pair<string, string>("k", std::to_string(k)));
  //   attrs.insert(std::pair<string, string>("n", std::to_string(n)));
  //   timer.start();
  //   WVR0.GetOps(msgid)->Matmul(ma_str, mb_str, result, &attrs);
  //   log_error << "** Matmul cost: " << timer.ms_elapse() << " ms";

  //   vector<double> expect(size);
  //   WVR0.GetOps(msgid)->Reveal(result, expect);// expect 8675.94784
  //   log_info << "-----------   test7: large scale big positive integer matmul(255,49)*(49,63)  check and ok. expect: 8675.94784, reveal: " << expect[0];
  // }

  // {//test8,  large scale matmul(4096,1024)*(1024,1024), expect values are 20
  //   log_info << "-----------   test8: very large scale positive integer matmul(4096,1024)*(1024,1024) ...";
  //   size_t m = 4096, k = 1024, n = 1024;
  //   size_t size = m*n;
  //   vector<double> ma(m*k, 4.24), mb(k*n, 2.434);//expect (m*n, 10567.84384);
    
  //   vector<string> ma_str, mb_str;
  //   SimpleTimer timer;
  //   WVR0.GetOps(msgid)->PrivateInput(0, ma, ma_str);
  //   WVR0.GetOps(msgid)->PrivateInput(0, mb, mb_str);
  //   log_error << "** private input cost: " << timer.ms_elapse() << " ms";

  //   vector<string> result;
  //   unordered_map<string,string> attrs;
  //   attrs.insert(std::pair<string, string>("m", std::to_string(m)));
  //   attrs.insert(std::pair<string, string>("k", std::to_string(k)));
  //   attrs.insert(std::pair<string, string>("n", std::to_string(n)));
  //   timer.start();
  //   WVR0.GetOps(msgid)->Matmul(ma_str, mb_str, result, &attrs);
  //   log_error << "** Matmul cost: " << timer.ms_elapse() << " ms";

  //   vector<double> expect(size);
  //   WVR0.GetOps(msgid)->Reveal(result, expect);// expect 10567.84384
  //   log_info << "-----------   test8: large scale positive integer matmul(4096,1024)*(1024,1024)  check and ok. expect: 10567.84384, reveal: " << expect[0];
  // }

  // {//test9,  large scale matmul(4096,1024)*(1024,1024), expect values are 20
  //   log_info << "-----------   test9: very large scale big positive integer matmul(4096,1024)*(1024,1024) ...";
  //   size_t m = 4096, k = 1024, n = 1024;
  //   size_t size = m*n;
  //   vector<double> ma(m*k, 14.24), mb(k*n, 12.434);//expect (m*n, 181309.60384);
    
  //   vector<string> ma_str, mb_str;
  //   SimpleTimer timer;
  //   WVR0.GetOps(msgid)->PrivateInput(0, ma, ma_str);
  //   WVR0.GetOps(msgid)->PrivateInput(0, mb, mb_str);
  //   log_error << "** private input cost: " << timer.ms_elapse() << " ms";

  //   vector<string> result;
  //   unordered_map<string,string> attrs;
  //   attrs.insert(std::pair<string, string>("m", std::to_string(m)));
  //   attrs.insert(std::pair<string, string>("k", std::to_string(k)));
  //   attrs.insert(std::pair<string, string>("n", std::to_string(n)));
  //   timer.start();
  //   WVR0.GetOps(msgid)->Matmul(ma_str, mb_str, result, &attrs);
  //   log_error << "** Matmul cost: " << timer.ms_elapse() << " ms";

  //   vector<double> expect(size);
  //   WVR0.GetOps(msgid)->Reveal(result, expect);// expect 181309.60384
  //   log_info << "-----------   test9: large scale big positive integer matmul(4096,1024)*(1024,1024)  check and ok. expect: 181309.60384, reveal: " << expect[0];
  // }

  // {//test10,  small matmul(10,10)*(10,10), matmul(zk, const),  expect values are 103.2016
  //   log_info << "-----------   test10: small positive integer matmul(10,10)*(10,10) ...";
  //   size_t m = 10, k = 10, n = 10;
  //   size_t size = m*n;
  //   vector<double> ma(m*k, 4), mb(k*n, 2);//mc(size, 20)
    
  //   vector<string> ma_str;
  //   double db(2);
  //   string db_str(sizeof(db), 0);
  //   memcpy((char*)db_str.data(), &db, sizeof(db));
  //   vector<string> mb_str(mb.size(), db_str);
  //   SimpleTimer timer;
  //   WVR0.GetOps(msgid)->PrivateInput(0, ma, ma_str);
  //   log_error << "** private input cost: " << timer.ms_elapse() << " ms";

  //   vector<string> result;
  //   unordered_map<string,string> attrs;
  //   attrs.insert(std::pair<string, string>("m", std::to_string(m)));
  //   attrs.insert(std::pair<string, string>("k", std::to_string(k)));
  //   attrs.insert(std::pair<string, string>("n", std::to_string(n)));
  //   attrs.insert(std::pair<string, string>("rh_is_const", std::to_string(1)));
  //   timer.start();
  //   WVR0.GetOps(msgid)->Matmul(ma_str, mb_str, result, &attrs);
  //   log_error << "** Matmul cost: " << timer.ms_elapse() << " ms";

  //   vector<double> expect(size);
  //   WVR0.GetOps(msgid)->Reveal(result, expect);// expect 20
  //   log_info << "-----------   test10: small positive integer matmul(10,10)*(10,10)  check and ok. expect: 80, reveal: " << expect[0];
  // }

  // {//test11,  large scale matmul(255,49)*(49,63), expect values are 20
  //   log_info << "-----------   test11: large scale integer const matmul(255,49)*(49,63) ...";
  //   size_t m = 255, k = 49, n = 63;
  //   size_t size = m*n;
  //   vector<double> ma(m*k, 4.24), mb(k*n, 2.434);//expect (m*n, 505.68784);
    
  //   vector<string> ma_str;
  //   double db(2.434);
  //   string db_str(sizeof(db), 0);
  //   memcpy((char*)db_str.data(), &db, sizeof(db));
  //   vector<string> mb_str(mb.size(), db_str);
  //   // vector<string> mb_str(mb.size(), "2.434");
  //   SimpleTimer timer;
  //   WVR0.GetOps(msgid)->PrivateInput(0, ma, ma_str);
  //   log_error << "** private input cost: " << timer.ms_elapse() << " ms";

  //   vector<string> result;
  //   unordered_map<string,string> attrs;
  //   attrs.insert(std::pair<string, string>("m", std::to_string(m)));
  //   attrs.insert(std::pair<string, string>("k", std::to_string(k)));
  //   attrs.insert(std::pair<string, string>("n", std::to_string(n)));
  //   attrs.insert(std::pair<string, string>("rh_is_const", std::to_string(1)));
  //   timer.start();
  //   WVR0.GetOps(msgid)->Matmul(ma_str, mb_str, result, &attrs);
  //   log_error << "** Matmul cost: " << timer.ms_elapse() << " ms";

  //   vector<double> revealed(size);
  //   WVR0.GetOps(msgid)->Reveal(result, revealed);// expect 505.68784
  //   print_vec(revealed, 20, "Matmul revealed result:");
  //   log_info << "-----------   test11: large scale integer const  =matmul(255,49)*(49,63)  check and ok. expect: 505.68784, reveal: " << revealed[0];
  // }

  // {
  //   log_info << "-----------   test11: large scale integer const matmul(255,49)*(49,63) ...";
  //   size_t m = 255, k = 49, n = 63;
  //   size_t size = m*n;
  //   vector<double> ma(m*k, 4.24), mb(k*n, 2.434);//expect (m*n, 505.68784);
    
  //   vector<string> ma_str, mb_str;
  //   SimpleTimer timer;
  //   WVR0.GetOps(msgid)->PrivateInput(0, ma, ma_str);
  //   WVR0.GetOps(msgid)->PrivateInput(0, mb, mb_str);
  //   log_error << "** private input cost: " << timer.ms_elapse() << " ms";

  //   vector<string> result;
  //   unordered_map<string,string> attrs;
  //   attrs.insert(std::pair<string, string>("m", std::to_string(m)));
  //   attrs.insert(std::pair<string, string>("k", std::to_string(k)));
  //   attrs.insert(std::pair<string, string>("n", std::to_string(n)));
    
  //   timer.start();
  //   int test_count = 100;
  //   for (size_t i = 0; i < test_count; ++i)
  //     WVR0.GetOps(msgid)->Matmul(ma_str, mb_str, result, &attrs);
  //   log_error << "** Matmul(" << m << "," << k << "," << n << ") " << test_count <<" times,  cost: " << timer.ms_elapse() << " ms";

  //   vector<double> revealed(size);
  //   WVR0.GetOps(msgid)->Reveal(result, revealed);// expect 505.68784
  //   print_vec(revealed, 20, "Matmul revealed result:");
  //   log_info << "-----------   test11: large scale integer const  =matmul(255,49)*(49,63)  check and ok. expect: 505.68784, reveal: " << revealed[0];
  // }

  WVR_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_ZK_TEST(run);