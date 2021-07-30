#include "helix__test.h"

void run(int partyid) {
  HELIX_PROTOCOL_INTERNAL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////

  // MSB
  {
    vector<double> X = {-1, 3, 0.3, -2.1, 0, -0.001, 0.098};
    vector<Share> shareX(X.size());
    hi->Input(node_id_2, X, shareX);
    hi->RevealAndPrint(shareX, "shareX:");

    vector<BitShare> shareC;

    hi->beg_statistics();
    hi->MSB(shareX, shareC);
    hi->end_statistics("RTT MSB(k=" + to_string(X.size()) + "):");

    hi->RevealAndPrint(shareC, "shareC:");
  }

#if PERFORMANCE_TEST
  {
    size_t size = 13579;
    vector<double> X;
    random_vector(X, size);

    vector<Share> shareX;
    hi->Input(node_id_2, X, shareX);

    vector<BitShare> shareC;

    hi->beg_statistics();
    hi->MSB(shareX, shareC);
    hi->end_statistics("PERF-RTT MSB(k=" + to_string(X.size()) + "):");
  }
#endif

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  HELIX_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);