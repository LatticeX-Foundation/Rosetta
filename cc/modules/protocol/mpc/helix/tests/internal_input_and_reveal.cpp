#include "helix__test.h"

void run(int partyid) {
  HELIX_PROTOCOL_INTERNAL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////

  vector<double> X = {-1, 3, 0.3, -2.1, 0, -0.001, 0.098, 1};
  vector<Share> shareX(X.size());
  {
    hi->beg_statistics();
    hi->Input(node_id_0, X, shareX);
    hi->end_statistics("RTT Input-0(k=" + to_string(X.size()) + "):");

    hi->RevealAndPrint(shareX, "Input-0:");
  }
  {
    hi->beg_statistics();
    hi->Input(node_id_1, X, shareX);
    hi->end_statistics("RTT Input-1(k=" + to_string(X.size()) + "):");

    hi->RevealAndPrint(shareX, "Input-1:");
  }
  {
    hi->beg_statistics();
    hi->Input(node_id_2, X, shareX);
    hi->end_statistics("RTT Input-2(k=" + to_string(X.size()) + "):");

    hi->RevealAndPrint(shareX, "Input-2:");
  }

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  HELIX_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);
