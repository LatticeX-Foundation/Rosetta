#include "helix__test.h"

void run(int partyid) {
  HELIX_PROTOCOL_INTERNAL_TEST_INIT(partyid);
  {
    vector<double> XX = {-1.0, -1.0, 2.0,  3.4, -2.1, 2000, 20000, 20000*1000, 200};
    vector<double> YY = {-3.0, -2.0, 1.0, -2.1,  3.4, 50000, -50000, -50000, -0.02};
    vector<double> expected_result(YY.size());
    for(auto i = 0; i < YY.size(); ++i) {
      expected_result[i] = XX[i] / YY[i];
    }

    vector<double> cond_1 = {-1.0, -1.0, 1.0, 1.0, -1.0, 1.0, 1.0, 1.0, 1.0};
    vector<double> cond_2 = {-1.0, 1.0, -1.0, 1.0, -1.0, 1.0, 1.0, 1.0, 1.0};

    vector<Share> shareX, shareY;
    hi->Input(node_id_2, XX, shareX);
    hi->Input(node_id_2, YY, shareY);
    vector<Share> shareZ;
    
    // pre-case: inner utils
    hi->Greater(shareX, shareY, shareZ);
    hi->RevealAndPrint2(shareZ, "Greater shareZ:");
    vector<Share> share_cond;
    hi->Select1Of2(cond_1, cond_2, shareZ, share_cond);
    //we expect -1. and -1.
    hi->RevealAndPrint(share_cond, "Select1Of2 share_cond:");
    
    hi->Input(node_id_2, XX, shareX);
    hi->Input(node_id_2, YY, shareY);
    print_vec(expected_result, 10, "expected:");
    
    // case 1 : SSS
    hi->Div(shareX, shareY, shareZ);
    hi->RevealAndPrint(shareZ, "Div[SSS] shareZ:");

    // case 2 : SDS
    vector<Share> shareZZ;
    hi->Div(shareX, YY, shareZZ);
    hi->RevealAndPrint(shareZZ, "Div[SDS] shareZ:");
    
    // case 3 : DSS
    vector<Share> shareZZZ;
    hi->Div(XX, shareY, shareZZZ);
    hi->RevealAndPrint(shareZZZ, "Div[DSS] shareZ:");

    // case 4: FloorDiv SSS
    hi->Floordiv(shareX, shareY, shareZ);
    hi->RevealAndPrint(shareZ, "FloorDiv[SSS] shareZ:");

    // case 5 : FloorDiv SDS
    hi->Floordiv(shareX, YY, shareZZ);
    hi->RevealAndPrint(shareZZ, "FloorDiv[SDS] shareZ:");

    // case 6 : FloorDiv DSS
    hi->Floordiv(XX, shareY, shareZZZ);
    hi->RevealAndPrint(shareZZZ, "FloorDiv[DSS] shareZ:");
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
