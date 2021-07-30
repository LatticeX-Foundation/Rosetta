#include "helix__test.h"

void run(int partyid) {
  HELIX_PROTOCOL_INTERNAL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  {
    //vector<double> X = {-1.01, -2.0, 2.3, 2, 0.01, 0, -0.03, 0.031};
    //vector<double> X = {-2, -1, 0, 1, 2};
    vector<double> X = {-2, -1.1};
    vector<Share> shareX;
    hi->Input(node_id_2, X, shareX);

    vector<vector<Share>> shareY;
    size_t N = 6;

    hi->beg_statistics();
    hi->Pow(shareX, N, shareY);
    hi->end_statistics("RTT Pow(k=" + to_string(X.size()) + ",N=" + to_string(N) + "):");

    vector<Share> flattenShareY;
    flatten(shareY, flattenShareY);
    hi->RevealAndPrint(flattenShareY, "shareY:");
  }

#if PERFORMANCE_TEST
  {
    size_t size = 13579;

    vector<double> X;
    vector<Share> shareX;
    hi->Input(node_id_2, X, shareX);

    vector<vector<Share>> shareY;
    size_t N = 6;

    hi->beg_statistics();
    hi->Pow(shareX, N, shareY);
    hi->end_statistics("PERF-RTT Pow(k=" + to_string(X.size()) + ",N=" + to_string(N) + "):");
  }
  {
    size_t size = 1357;

    vector<double> X;
    vector<Share> shareX;
    hi->Input(node_id_2, X, shareX);

    vector<vector<Share>> shareY;
    size_t N = 11;

    hi->beg_statistics();
    hi->Pow(shareX, N, shareY);
    hi->end_statistics("PERF-RTT Pow(k=" + to_string(X.size()) + ",N=" + to_string(N) + "):");
  }
#endif

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  HELIX_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);