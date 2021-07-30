#include "helix__test.h"

void run(int partyid) {
  HELIX_PROTOCOL_INTERNAL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////

  // (dummy) real dataset (n x d)
  // shape = (n, d) --> n samples with d features
  int n = 10, d = 11 / (partyid + 1);
  vector<vector<double>> X(n, vector<double>(d, 0));
  vector<double> Xx = {0, 1, -1, -0.3, 2, 3, -4};
  {
    //dummy
    for (int i = 0; i < n; i++) {
      for (int j = 0; j < d; j++) {
        // X[i][j] = ((i * d + j) * (player + 1)) / 10.0;
        // X[i][j] = i * d + j;
        int idx = i * d + j;
        if (idx < Xx.size())
          X[i][j] = Xx[idx];
        else
          X[i][j] = idx;
      }
    }
  }
  print_shape(X);

  // sync each party's n(samples) and d(features)
  int nA, nB, nC, dA, dB, dC;
  nA = nB = nC = n;
  dA = 11 / (0 + 1);
  dB = 11 / (1 + 1);
  dC = 11 / (2 + 1);
  cout << "dA,dB,dC: " << dA << "," << dB << "," << dC << endl;

  // call `Input`, get parivate-inputs
  // nA == nB == nC == n, use a PRF(common) to generate
  vector<vector<Share>> shareX0, shareX1, shareX2;

  if (dA > 0) { // if P0 owns data
    if (partyid != PARTY_0) {
      vector<vector<double>> tmpX(nA, vector<double>(dA, 0));
      hi->Input(node_id_0, tmpX, shareX0);
    } else {
      hi->Input(node_id_0, X, shareX0);
    }
  }
  {
    vector<Share> shareX;
    flatten(shareX0, shareX);

    vector<double> plain(shareX.size());
    hi->Reveal(shareX, plain, reveal_attr["receive_parties"]);
    print_vec(plain, 7, "shareX0-double");
  }

  if (dB > 0) { // if P1 owns data
    if (partyid != PARTY_1) {
      vector<vector<double>> tmpX(nB, vector<double>(dB, 0));
      hi->Input(node_id_1, tmpX, shareX1);
    } else {
      hi->Input(node_id_1, X, shareX1);
    }
  }

  {
    vector<Share> shareX;
    flatten(shareX1, shareX);

    vector<double> plain(shareX.size());
    hi->Reveal(shareX, plain, reveal_attr["receive_parities"]);
    print_vec(plain, 7, "shareX1-double");
  }

  if (dC > 0) { // if P2 owns data
    if (partyid != PARTY_2) {
      vector<vector<double>> tmpX(nC, vector<double>(dC, 0));
      hi->Input(node_id_2, tmpX, shareX2);
    } else {
      hi->Input(node_id_2, X, shareX2);
    }
  }

  {
    vector<Share> shareX;
    flatten(shareX2, shareX);

    vector<double> plain(shareX.size());
    hi->Reveal(shareX, plain, reveal_attr["receive_parties"]);
    print_vec(plain, 7, "shareX2-double");
  }

  // each party locally combine shareX0, shareX1, shareX2
  vector<vector<Share>> shareXX;
  hi->CombineInputInVertical(shareX0, shareX1, shareX2, shareXX);
  print_shape(shareXX);

  // flatten, vectorize, row first
  vector<Share> shareX;
  flatten(shareXX, shareX);
  hi->RevealAndPrint(shareX, "shareX-double");

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  HELIX_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);
