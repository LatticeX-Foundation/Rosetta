#include "helix__test.h"

void run(int partyid) {
  HELIX_PROTOCOL_INTERNAL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////

  {
    vector<mpc_t> y = {12, 53, 34, 35, 226, 21, 73, 92, 23, 34, 45, 56, 81, 32, 12};
    size_t size = y.size();
    vector<Share> Y;
    hi->Input(node_id_2, y, Y);

    vector<bit_t> X = {1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0};
    vector<BitShare> bitShareX(X.size());
    hi->Input(node_id_2, X, bitShareX);
    hi->RevealAndPrint(bitShareX, "bitShareX:");

    vector<Share> shareX(X.size());

    hi->beg_statistics();
    hi->BMA(bitShareX, Y, shareX);
    hi->end_statistics("RTT BMA(k=" + to_string(X.size()) + "):");

    hi->RevealAndPrint2(shareX, "shareX:");
  }

#if PERFORMANCE_TEST
  {
    size_t size = 13579;

    vector<double> y;
    random_vector(y, size);

    vector<Share> Y;
    hi->Input(node_id_2, y, Y);

    vector<bit_t> X;
    random_vector(X, size);
    vector<BitShare> bitShareX(X.size());
    hi->Input(node_id_2, X, bitShareX);

    vector<Share> shareX(X.size());

    hi->beg_statistics();
    hi->BMA(bitShareX, Y, shareX);
    hi->end_statistics("PERF-RTT BMA(k=" + to_string(X.size()) + "):");
  }
#endif

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  HELIX_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);
