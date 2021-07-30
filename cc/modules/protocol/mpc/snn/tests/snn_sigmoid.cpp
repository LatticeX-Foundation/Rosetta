#include "snn__test.h"
#include <math.h>

vector<double> sigmoid_ideal(const vector<double>& x) {
  vector<double> expect;
  for (auto& elem : x)
    expect.push_back(1.0/(1+exp(-elem)));

  return expect;
}

void run(int partyid) {
  SNN_PROTOCOL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////

  vector<double> X = {-1.5, -2, 0, 2, 3, 1.2, 10, 2.71828};
  vector<double> EXPECT = sigmoid_ideal(X);
  size_t size = X.size();
  print_vec(X, size, "X");

  attr_type attr;
  vector<string> receivers = {"P0", "P1", "P2"};
  attr["receive_parties"] = receiver_parties_pack(receivers);

  msg_id_t msgid("sigmoid OP(s) (share,share)");
  cout << __FUNCTION__ << " " << msgid << endl;

  vector<string> strX, strZ;
  snn0.GetOps(msgid)->PrivateInput(node_id_0, X, strX);
  snn0.GetOps(msgid)->Sigmoid(strX, strZ);
 

  vector<double> reveal_x(strX.size()), plain_z(strZ.size());
  snn0.GetOps(msgid)->Reveal(strZ, reveal_x, &attr);
  // print_vec(reveal_x, size, "SNN PrivateInput Reveal: ");
  print_vec(reveal_x, size, "SNN Sigmoid reveal:");
  print_vec(EXPECT, size, "Sigmoid expected:");
  
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  SNN_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);