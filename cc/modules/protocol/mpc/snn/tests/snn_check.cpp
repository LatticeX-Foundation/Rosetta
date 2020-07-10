#include "snn__test.h"

void run(int partyid) {
  SNN_PROTOCOL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////

  vector<double> X = {-1.01, -2.00, -3.01, 0, 1.3, 2.02, 3.14, +2, -0.01};
  vector<double> Y = {-1.00, -2.01, -3.01, 0, 1.3, 2.03, 3.12, -2, +0.01};
  size_t size = X.size();
  print_vec(X, 10, "X");
  print_vec(Y, 10, "Y");

  string msgid("All basic Binary OP(s) (share,share)");
  cout << __FUNCTION__ << " " << msgid << endl;

  vector<string> strX, strY, strZ;
  //
  snn0.GetOps(msgid)->PrivateInput(0, X, strX);
  print_vec(strX, 10, "strX");

  vector<string> zX(strX.size());
  snn0.GetOps(msgid)->Reveal(strX, zX);
  print_vec(zX, 10, "zX");

  //
  snn0.GetOps(msgid)->PrivateInput(1, Y, strY);
  print_vec(strY, 10, "strY");

  vector<string> zY(strY.size());
  snn0.GetOps(msgid)->Reveal(strY, zY);
  print_vec(zY, 10, "zY");

  //
  snn0.GetOps(msgid)->Mul(strX, strY, strZ);
  print_vec(strZ, 10, "strZ");

  vector<string> zZ(strZ.size());
  snn0.GetOps(msgid)->Reveal(strZ, zZ);
  print_vec(zZ, 10, "zZ");

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  SNN_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_HELIX_TEST(run);