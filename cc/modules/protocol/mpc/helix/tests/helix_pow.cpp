#include "helix__test.h"
#include "cmath"

void run(int partyid) {
  HELIX_PROTOCOL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////

  msg_id_t msgid("pow constant");
  cout << __FUNCTION__ << " " << msgid << endl;

  vector<double> X = {-1.01, -2.00, -3.01, 0, 1.3, 2.02, 3.14, +2, -0.01};
  //vector<double> K = {-1.00, -2.01, -3.01, 0, 1.3, 2.03, 3.12, -2, +0.01};
  vector<double> K(X.size(), 10.0);
  
  vector<double> expected_pow(X.size());
  for(int i = 0; i < X.size(); ++i) {
    expected_pow[i] = pow(X[i], K[i]);
  }

  size_t size = X.size();
  print_vec(X, 10, "X");
  print_vec(K, 10, "K");
  print_vec(expected_pow, 10, "expected:");

  vector<string> strX, strK, strZ;
  helix0.GetOps(msgid)->PrivateInput(node_id_0, X, strX);
  // print_vec(strX, 10, "strX");
  helix_double_to_plain_string(K, strK);
  // print_vec(strK, 10, "strK");

  helix0.StartPerfStats();
  helix0.GetOps(msgid)->Pow(strX, strK, strZ);
  auto perf = helix0.GetPerfStats();
  cout << "Pow perf stat:" << perf.to_json(true) << endl;

  vector<double> Z;
  helix0.GetOps(msgid)->Reveal(strZ, Z, &reveal_attr);
  print_vec(Z, 10, "Z");

  // case 2: vectorization speedup
  X.clear();
  X = {5.0};
  K = {3.0};
  print_vec(X, 10, "X");
  print_vec(K, 10, "K");

  helix0.GetOps(msgid)->PrivateInput(node_id_0, X, strX);
  
  print_vec(strX, 10, "strX");
  helix_double_to_plain_string(K, strK);
  print_vec(strK, 10, "strK");

  helix0.GetOps(msgid)->Pow(strX, strK, strZ);
  print_vec(strZ, 10, "strZ");

  helix0.GetOps(msgid)->Reveal(strZ, Z, &reveal_attr);
  print_vec(Z, 10, "revealed Z:");  

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  HELIX_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);