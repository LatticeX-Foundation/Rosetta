#include "snn__test.h"

void run(int partyid) {
  SNN_PROTOCOL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////

  vector<double> X = {    1.5,    0.01,   0.001, 0.00013, 10, 2.71828};
  vector<double> EXPECT = {0.405, -4.605, -6.908, -8.948, 2.303, 1.000};
  size_t size = X.size();
  print_vec(X, 10, "X");

  string msgid("All basic Binary OP(s) (share,share)");
  cout << __FUNCTION__ << " " << msgid << endl;
  
  vector<string> half_share_X = {"2.0", "10.0"};
  print_vec(half_share_X, 10, "Half_share {2, 10}");
  vector<string> half_share_log(size);
  snn0.GetOps(msgid)->Log(half_share_X, half_share_log);
  print_vec(half_share_log, 10, "Log half_share cipher:");
  vector<string> half_share_revealed(size);
  snn0.GetOps(msgid)->Reveal(half_share_log, half_share_revealed);
  print_vec(half_share_revealed, 10, "Log half_share revealed zZ:");  

  vector<string> strX, strZ;
  snn0.GetOps(msgid)->PrivateInput(0, X, strX);

  snn0.GetOps(msgid)->Log(strX, strZ);

  vector<string> zZ(strZ.size());
  snn0.GetOps(msgid)->Reveal(strZ, zZ);
  print_vec(zZ, 10, "SNN Log plaintext:");
  print_vec(EXPECT, 10, "Log expected:");

  snn0.GetOps(msgid)->HLog(strX, strZ);
  print_vec(strZ, 10, "HLog strZ:");
  snn0.GetOps(msgid)->Reveal(strZ, zZ);
  print_vec(zZ, 10, "SNN HLog plaintext:");
  print_vec(EXPECT, 10, "HLog expected:");

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  SNN_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_HELIX_TEST(run);