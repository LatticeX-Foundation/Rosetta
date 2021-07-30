#include "helix__test.h"

void run(int partyid) {
  HELIX_PROTOCOL_INTERNAL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////

  {
    vector<bit_t> X = {1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1};
    size_t size = X.size();
    vector<BitShare> bitShareX(X.size());
    hi->Input(node_id_2, X, bitShareX);
    hi->RevealAndPrint(bitShareX, "bitShareX:");

    vector<Share> shareX(X.size());

    hi->beg_statistics();
    hi->B2A(bitShareX, shareX);
    hi->end_statistics("RTT B2A(k=" + to_string(X.size()) + "):");

    hi->RevealAndPrint2(shareX, "shareX:");
  }

#if PERFORMANCE_TEST
  {
    size_t size = 1234567;

    vector<bit_t> X;
    random_vector(X, size);
    vector<BitShare> bitShareX(X.size());
    hi->Input(node_id_2, X, bitShareX);

    vector<Share> shareX(X.size());

    hi->beg_statistics();
    hi->B2A(bitShareX, shareX);
    hi->end_statistics("PERF-RTT B2A(k=" + to_string(X.size()) + "):");
  }
#endif

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  HELIX_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);