#include "snn__test.h"
#include <string>
using namespace std;
void run(int partyid) {
  SNN_PROTOCOL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  /*
  [[2.9 2.2 3.3]
  [2.4 2.8 2.6]
  [2.7 2.5 3.4]]
  */
  vector<double> X = {2.9, 2.2, 3.3, 2.4, 2.8, 2.6, 2.7, 2.5, 3.4};
  vector<double> MAX_EXPECT = {3.3, 2.8, 3.4};
  vector<double> MIN_EXPECT = {2.2, 2.4, 2.5};

  size_t size = X.size();
  print_vec(X, 10, "Input X");

  string msgid("Max_and_Min");
  cout << __FUNCTION__ << " " << msgid << endl;
  
  vector<string> strX(X.size()), strZ(X.size());
  vector<string> zZ(strZ.size());

  snn0.GetOps(msgid)->PrivateInput(0, X, strX);
  snn0.GetOps(msgid)->Reveal(strX, zZ);
  print_vec(zZ, 10, "check PrivateInput plaintext:");

  cout << __FUNCTION__ << " " << msgid << endl;  
  

  attr_type attr;
  attr["rows"] = "3";
  attr["cols"] = "3";

  snn0.GetOps(msgid)->Max(strX, strZ, &attr);
  snn0.GetOps(msgid)->Reveal(strZ, zZ);
  print_vec(zZ, 10, "Max plaintext:");
  print_vec(MAX_EXPECT, 10, "Max expected:");

  snn0.GetOps(msgid)->Min(strX, strZ, &attr);
  snn0.GetOps(msgid)->Reveal(strZ, zZ);
  print_vec(zZ, 10, "SNN Min plaintext:");
  print_vec(MIN_EXPECT, 10, "Min expected:");

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  SNN_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);