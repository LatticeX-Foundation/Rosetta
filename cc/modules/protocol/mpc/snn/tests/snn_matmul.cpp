#include "snn__test.h"

void run(int partyid) {
  SNN_PROTOCOL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////

  vector<double> X, Y, Z;
  random_vector(X, 64 * 42, -1.0, 1.0);
  random_vector(Y, 42 * 1, -1.0, 1.0);
  size_t size = X.size();

  string msgid("Matmul (share,share)");
  cout << __FUNCTION__ << " " << msgid << endl;

  vector<string> strX(X.size()), strY(X.size()), strZ(X.size());
  snn0.GetOps(msgid)->PrivateInput(0, X, strX);
  snn0.GetOps(msgid)->PrivateInput(1, Y, strY);
  cout << __FUNCTION__ << " " << msgid << endl;

  attr_type attr;
  attr["m"] = "64";
  attr["k"] = "42";
  attr["n"] = "1";

  SimpleTimer timer;
  snn0.GetOps(msgid)->Matmul(strX, strY, strZ, &attr);
  cout << ">>>>>>>>>>>>>>>>>>> timer:" << timer.elapse() << endl;
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  SNN_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);
