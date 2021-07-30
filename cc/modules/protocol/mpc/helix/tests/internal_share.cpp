#include "helix__test.h"

void run(int partyid) {
  HELIX_PROTOCOL_INTERNAL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////

  {
    vector<double> X = {-12.23, -4.51, -3.05, -0.01, 1.05, 0.002, 3.33, 12.5};
    vector<mpc_t> XF;

    helix0.GetMpcContext()->FLOAT_PRECISION = 13;
    convert_plain_to_fixpoint(X, XF, helix0.GetMpcContext()->FLOAT_PRECISION);
    print_vec(XF, 32, "13 bits:");

    helix0.GetMpcContext()->FLOAT_PRECISION = 18;
    convert_plain_to_fixpoint(X, XF, helix0.GetMpcContext()->FLOAT_PRECISION);
    print_vec(XF, 32, "18 bits:");
  }
  helix0.GetMpcContext()->FLOAT_PRECISION = 13;
  cout << "use 13 bits:" << endl;
  {
    vector<double> X = {-12.23, -4.51, -3.05, -0.01, 1.05, 0.002, 3.33, 12.5};
    vector<Share> shareX;
    hi->Input(node_id_2, X, shareX);
    hi->RevealAndPrint(shareX, "ShareX:");
    cout << "X shareX:" << shareX << endl;

    cout << "1. convert to binary-string, and covert to back." << endl;
    {
      vector<Share> shareR(X.size());
      for (int i = 0; i < shareX.size(); i++) {
        string ss;
        shareX[i].output(ss);
        cout << "ss.size:" << ss.size() << endl;
        shareR[i].input(ss);
      }
      hi->RevealAndPrint(shareR, "shareR1:");
    }
    cout << "2. convert to human-readable-string, and covert to back." << endl;
    {
      vector<Share> shareR(X.size());
      for (int i = 0; i < shareX.size(); i++) {
        string ss;
        shareX[i].output(ss, true);
        cout << "ss:" << ss << ",size:" << ss.size() << endl;
        shareR[i].input(ss, true);
      }
      hi->RevealAndPrint(shareR, "shareR2:");
    }
  }
  helix0.GetMpcContext()->FLOAT_PRECISION = 18;
  cout << "use 18 bits:" << endl;
  {
    vector<double> X = {-12.23, -4.51, -3.05, -0.01, 1.05, 0.002, 3.33, 12.5};
    vector<Share> shareX;
    hi->Input(node_id_2, X, shareX);
    hi->RevealAndPrint(shareX, "ShareX:");

    cout << "1. convert to binary-string, and covert to back." << endl;
    {
      vector<Share> shareR(X.size());
      for (int i = 0; i < shareX.size(); i++) {
        string ss;
        shareX[i].output(ss);
        cout << "ss.size:" << ss.size() << endl;
        shareR[i].input(ss);
      }
      hi->RevealAndPrint(shareR, "shareR1:");
    }
    cout << "2. convert to human-readable-string, and covert to back." << endl;
    {
      vector<Share> shareR(X.size());
      for (int i = 0; i < shareX.size(); i++) {
        string ss;
        shareX[i].output(ss, true);
        cout << "ss:" << ss << ",size:" << ss.size() << endl;
        shareR[i].input(ss, true);
      }
      hi->RevealAndPrint(shareR, "shareR2:");
    }
  }

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  HELIX_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);
