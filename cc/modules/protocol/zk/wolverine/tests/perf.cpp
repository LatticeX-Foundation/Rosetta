#include "wvr_test.h"

void run(int partyid) {
  WVR_PROTOCOL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  // Also ref python/latticex/rosetta/secure/decorator/test_cases/conv2d_*.py

  msg_id_t msgid("performance ....");
  log_info << "msgid:" << msgid;

  {
    // init...
    vector<double> a, b;
    int64_t size = 1;
    random_vector(a, size, 1.0, 2.0);
    random_vector(b, size, 3.0, 4.0);
    vector<string> sa(a.size());
    vector<string> sb(b.size());
    vector<string> sc;
    WVR0.GetOps(msgid)->PrivateInput(ALICE - 1, a, sa);
    WVR0.GetOps(msgid)->PrivateInput(ALICE - 1, b, sb);
    WVR0.GetOps(msgid)->Mul(sa, sb, sc);
  }

  rosetta::PerfStats ps0, ps1, ps;

  // PrivateInput
  auto test_private_input = [&](int64_t NX, int64_t size) {
    msg_id_t msgid("performance PrivateInput....");
    vector<double> a;
    random_vector(a, size, -2.0, 2.0);
    vector<string> sa(a.size());

    ps0 = WVR0.GetPerfStats();
    for (int64_t i = 0; i < NX; i++) {
      WVR0.GetOps(msgid)->PrivateInput(ALICE - 1, a, sa);
    }
    ps = WVR0.GetPerfStats() - ps0;
    double avg = ps.s.elapse / NX;
    cout << "PERFSTATS NX:" << NX << " per PrivateInput(" << size << "):" << setprecision(8) << avg
         << endl;
  };
  test_private_input(30, 100);
  test_private_input(30, 1000);
  test_private_input(30, 10000);
  test_private_input(30, 100000);
  test_private_input(30, 1000000);
  test_private_input(30, 2000000);

  // Mul
  auto test_multiply = [&](int64_t NX, int64_t size) {
    msg_id_t msgid("performance Mul....");
    vector<double> a, b;
    random_vector(a, size, -2.0, 2.0);
    random_vector(b, size, -2.0, 2.0);
    vector<string> sa(a.size());
    vector<string> sb(b.size());
    vector<string> sc;
    WVR0.GetOps(msgid)->PrivateInput(ALICE - 1, a, sa);
    WVR0.GetOps(msgid)->PrivateInput(ALICE - 1, b, sb);

    ps0 = WVR0.GetPerfStats();
    for (int64_t i = 0; i < NX; i++) {
      WVR0.GetOps(msgid)->Mul(sa, sb, sc);
    }
    ps = WVR0.GetPerfStats() - ps0;
    double avg = ps.s.elapse / NX;
    cout << "PERFSTATS NX:" << NX << " per Mul(" << size << "):" << setprecision(8) << avg << endl;
  };
  test_multiply(30, 100);
  test_multiply(30, 1000);
  test_multiply(30, 10000);

  // Matmul
  auto test_matmul = [&](int64_t NX, int64_t m, int64_t k, int64_t n, bool rh_is_const = false) {
    msg_id_t msgid("performance Matmul....");
    vector<double> a, b;
    random_vector(a, m * k, -2.0, 2.0);
    random_vector(b, k * n, -2.0, 2.0);
    vector<string> sa(a.size());
    vector<string> sb(b.size());
    vector<string> sc;
    WVR0.GetOps(msgid)->PrivateInput(ALICE - 1, a, sa);
    if (rh_is_const) {
      for (int64_t i = 0; i < b.size(); i++) {
        sb[i] = to_string(b[i]);
      }
    }
    attr_type attr;
    attr["m"] = to_string(m);
    attr["k"] = to_string(k);
    attr["n"] = to_string(n);
    attr["rh_is_const"] = rh_is_const ? "1" : "0";

    ps0 = WVR0.GetPerfStats();
    for (int64_t i = 0; i < NX; i++) {
      WVR0.GetOps(msgid)->Matmul(sa, sb, sc, &attr);
    }
    ps = WVR0.GetPerfStats() - ps0;
    double avg = ps.s.elapse / NX;
    cout << "PERFSTATS NX:" << NX << " per Matmul(" << m << "x" << k << "," << k << "x" << n
         << "):" << setprecision(8) << avg << endl;
  };
  test_matmul(30, 10, 10, 10);
  test_matmul(30, 100, 10, 10);
  test_matmul(30, 10, 100, 10);
  test_matmul(30, 10, 10, 100);
  test_matmul(30, 64, 64, 256);
  test_matmul(20, 64, 1024, 256);
  test_matmul(10, 512, 512, 256);
  test_matmul(30, 10, 10, 10, true);
  test_matmul(30, 100, 10, 10, true);
  test_matmul(30, 10, 100, 10, true);
  test_matmul(30, 10, 10, 100, true);
  test_matmul(30, 64, 64, 256, true);
  test_matmul(20, 64, 1024, 256, true);
  test_matmul(10, 512, 512, 256, true);

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  WVR_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_ZK_TEST(run);