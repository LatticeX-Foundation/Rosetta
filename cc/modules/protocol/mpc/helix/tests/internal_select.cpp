#include "helix__test.h"

void run(int partyid) {
  HELIX_PROTOCOL_INTERNAL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  {
    vector<double> X = {-1.1, 1.2};
    vector<double> Y = {-1.4, 1.3};

    vector<double> x = {-3.0, 4.0};
    vector<double> y = {2.0, -5.0};

    /**
     * Cond = X - Y = {0.3, -0.1} = {1, 0}
     * 
     *  res = x*Cond + y*(1-Cond) = (x-y)*Cond + y
     *      = {-5.0, 9.0} * {1, 0} + {2, -5.0} = {-3, -5.0}
     */

    int size = X.size();
    vector<Share> shareX, shareY;
    hi->Input(node_id_2, X, shareX);
    hi->Input(node_id_2, Y, shareY);
    vector<Share> shareZ;

    // X ?> Y
    hi->Greater(shareX, shareY, shareZ); //
    hi->RevealAndPrint2(shareZ, "Greater shareZ:");

    // x - y
    auto x_minus_y = x - y;
    print_vec(x_minus_y, 10, "x_minus_y");

    // (x-y)*Cond
    vector<Share> prod;
    hi->Scale(shareZ, hi->GetMpcContext()->FLOAT_PRECISION);
    hi->Mul(shareZ, x_minus_y, prod);

    // (x-y)*Cond + y
    vector<Share> res;
    hi->Add(prod, y, res);
    hi->RevealAndPrint(res, "Select1Of2 Add:");
  }

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  HELIX_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);
