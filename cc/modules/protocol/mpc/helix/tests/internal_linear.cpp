#include "helix__test.h"

void run(int partyid) {
  HELIX_PROTOCOL_INTERNAL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////

  {
    // binary version
    vector<bit_t> X = {0, 1, 0, 1, 1, 1, 0};
    vector<bit_t> C = {1, 0, 1, 1, 1, 0, 0, 1};
    vector<BitShare> shareX(X.size());
    hi->Input(node_id_2, X, shareX);

    BitShare Y;

    hi->beg_statistics();
    hi->Linear(shareX, C, Y);
    hi->end_statistics("RTT Linear-Bit(k=" + to_string(X.size()) + "):");

    hi->RevealAndPrint(Y, "Bit Linear:");
  }

  {
    // float version
    vector<double> X = {-1.2, 0.1, 0, 3.3};
    vector<Share> shareX(X.size());
    hi->Input(node_id_2, X, shareX);

    vector<double> C = {2.2, 0.2, -1.1, 3, -0.5};
    Share Y;

    hi->beg_statistics();
    hi->Linear(shareX, C, Y);
    hi->end_statistics("RTT Linear-Float(k=" + to_string(X.size()) + "):");

    vector<Share> vY = {Y};
    hi->RevealAndPrint(vY, "Float Linear:");
  }

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  HELIX_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);
