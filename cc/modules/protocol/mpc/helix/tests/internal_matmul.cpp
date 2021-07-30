#include "helix__test.h"

void run(int partyid) {
  HELIX_PROTOCOL_INTERNAL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////

  {
    vector<double> X = {-1, 3, 0.3, -2.1};
    vector<Share> shareX(X.size());
    hi->Input(node_id_0, X, shareX);
    hi->RevealAndPrint(shareX, "shareX:");

    vector<double> Y = {-2.1, 0.098, -0.5, 2, -1.5, 1.2};
    vector<Share> shareY(Y.size());
    hi->Input(node_id_1, Y, shareY);
    hi->RevealAndPrint(shareY, "shareY:");

    // MatMul
    vector<Share> shareZ;
    int m = 2, n = 2, k = 3;

    hi->beg_statistics();
    hi->MatMul(shareX, shareY, shareZ, m, n, k, false, false);
    hi->end_statistics(
      "RTT MatMul-1(m=" + to_string(m) + ",n=" + to_string(n) + ",k=" + to_string(k) + "):");

    hi->RevealAndPrint(shareZ, "shareZ:");
  }

  {
    //int m = 10, n = 10, k = 3;
    //int m = 2, n = 2, k = 2;
    int m = 10, n = 3, k = 1;
    vector<double> X(m * n, 1.0), Y(n * k, 2.0);
    vector<Share> shareX(X.size()), shareY(Y.size()), shareZ(m * k);
    hi->Input(node_id_0, X, shareX);
    hi->Input(node_id_1, Y, shareY);

    hi->beg_statistics();
    hi->MatMul(shareX, shareY, shareZ, m, n, k, false, false);
    hi->end_statistics(
      "RTT MatMul-2(m=" + to_string(m) + ",n=" + to_string(n) + ",k=" + to_string(k) + "):");

    hi->RevealAndPrint(shareZ, "shareZ:");
  }

#if PERFORMANCE_TEST
  int m = 119, n = 123, k = 341;
  vector<double> X, Y;
  random_vector(X, m * n);
  random_vector(Y, n * k);
  vector<Share> shareX(X.size()), shareY(Y.size()), shareZ(m * k);
  hi->Input(node_id_0, X, shareX);
  hi->Input(node_id_1, Y, shareY);

  hi->beg_statistics();
  hi->MatMul(shareX, shareY, shareZ, m, n, k, false, false);
  hi->end_statistics(
    "PERF-RTT MatMul(m=" + to_string(m) + ",n=" + to_string(n) + ",k=" + to_string(k) + "):");
#endif

  if (false) {
    //int m = 119, n = 123, k = 341;
    int m = 800, n = 800, k = 800;
    vector<double> X, Y;
    random_vector(X, m * n);
    random_vector(Y, n * k);
    vector<Share> shareX(X.size()), shareY(Y.size()), shareZ(m * k);
    hi->Input(node_id_0, X, shareX);
    hi->Input(node_id_1, Y, shareY);

    int64_t all = 0;
    int NX = 100;

    {
      // SimpleTimer timer;
      for (int i = 0; i < 50; i++) {
        hi->MatMul(shareX, shareY, shareZ, m, n, k, false, false);
      }
      // cout << "performance matmul-avg:" << timer.us_elapse() / 50 << endl;
    }

    SimpleTimer timer;
    for (int i = 0; i < NX; i++) {
      hi->MatMul(shareX, shareY, shareZ, m, n, k, false, false);
    }
    cout << "performance matmul-avg:" << timer.us_elapse() / NX << endl;
  }

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  HELIX_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);
