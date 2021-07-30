#include "helix__test.h"

void run(int partyid) {
  HELIX_PROTOCOL_INTERNAL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////

  // todo here
  {
    vector<double> X = {-1.01, -2.0, 2.3, 2, 0.01, 0, -0.03, 0.031};
    vector<double> Y = {-1.01, -2.0, 2.3, 2, 0.01, 0, -0.03, 0.031};
    vector<Share> shareX, shareY;
    hi->Input(node_id_2, X, shareX);
    hi->Input(node_id_2, Y, shareY);

    vector<Share> shareZ;
    int J = 2;
    int I = X.size() / J;

    hi->beg_statistics();
    hi->InnerProducts(shareX, shareY, shareZ, J);
    hi->end_statistics("RTT InnerProducts(k=" + to_string(I) + ", j=" + to_string(J) + "):");

    hi->RevealAndPrint(shareZ, "shareZ:");
  }

#if PERFORMANCE_TEST
  {
    size_t size = 1357 * 9;

    vector<double> X;
    vector<double> Y;
    random_vector(X, size);
    random_vector(Y, size);

    vector<Share> shareX, shareY;
    hi->Input(node_id_2, X, shareX);
    hi->Input(node_id_2, Y, shareY);

    vector<Share> shareZ;
    int J = 9;
    int I = X.size() / J;

    hi->beg_statistics();
    hi->InnerProducts(shareX, shareY, shareZ, J);
    hi->end_statistics("PERF-RTT InnerProducts(k=" + to_string(I) + ", j=" + to_string(J) + "):");
  }
#endif

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  HELIX_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);
