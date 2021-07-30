#include "helix__test.h"

void run(int partyid) {
  HELIX_PROTOCOL_INTERNAL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////

  {
    // -5.0,-4.5,-4.0,-3.5,-2.5,-1.5,-0.5,0,0.5,1.5,2.5,3.5,4.0,4.5
    vector<double> X = {-5.0, -4.5, -4.0, -3.5, -2.5, -1.5, -0.5, 0, 0.5, 1.5, 2.5, 3.5, 4.0, 4.5};
    //vector<double> X = {-2.5, 1.5, 3.5};
    vector<Share> shareX, shareY;
    hi->Input(node_id_2, X, shareX);

    hi->beg_statistics();
    hi->Sigmoid(shareX, shareY);
    hi->end_statistics("RTT Sigmoid(k=" + to_string(X.size()) + "):");

    hi->RevealAndPrint(shareY, "shareY:");
    hi->RevealAndPrint2(shareY, "shareY(scaled):");
  }

#if PERFORMANCE_TEST
  {
    size_t size = 13579;
    vector<double> X;
    random_vector(X, size, -5.0, 5.0);

    vector<Share> shareX, shareY;
    hi->Input(node_id_2, X, shareX);

    hi->beg_statistics();
    hi->Sigmoid(shareX, shareY);
    hi->end_statistics("PERF-RTT Sigmoid-2(k=" + to_string(X.size()) + "):");
  }
#endif

  if (false) {
    int size = 2000;
    int NX = 100;

    vector<double> X;
    random_vector(X, size, -5.0, 5.0);

    vector<Share> shareX, shareY;
    hi->Input(node_id_2, X, shareX);

    for (int i = 0; i < 50; i++) {
      hi->Sigmoid(shareX, shareY);
    }

    SimpleTimer timer;
    for (int i = 0; i < NX; i++) {
      hi->Sigmoid(shareX, shareY);
    }
    cout << "performance sigmoid-avg:" << timer.us_elapse() / NX << endl;
  }

#if OUTPUT_FOR_COMPARE_WITH_STD_SIGMOID
  {
    // for compare with standard sigmoid
    vector<double> X;
    for (double d = -5.0; d < 5.0; d += 0.002) {
      X.push_back(d);
    }
    size_t size = X.size();
    vector<Share> shareX, shareY;
    hi->Input(node_id_2, X, shareX);

    hi->beg_statistics();
    hi->Sigmoid(shareX, shareY);
    hi->end_statistics("RTT Sigmoid-2(k=" + to_string(X.size()) + "):");

    vector<double> pY;
    hi->Reveal(shareY, pY);
    if (player == PARTY_0) {
      tofile(pY, "y.csv");
    }
  }
#endif

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  HELIX_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);