#include "snn__test.h"
#include <string>
using namespace std;
void run(int partyid) {
  SNN_PROTOCOL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////

  vector<double> X = {1.5, 0.31, -10.0, -0.02, -2, 19.0, 10.0};
  vector<double> Y = {1.5, 0.01, -10.0, -0.21, 10, 10.0, 10.0};
  vector<double> EQUAL_EXPECT = {1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0};
  vector<double> NEQUAL_EXPECT = {0.0, 1.0, 0.0, 1.0, 1.0, 1.0, 0.0};

  size_t size = X.size();
  print_vec(X, 10, "Input X");
  print_vec(Y, 10, "Input Y");
  
  // Logger::Get().log_to_stdout(true);
  // Logger::Get().set_filename("SNN_Equal_" + to_string(partyid) + ".log");
  // Logger::Get().set_level(1);
  msg_id_t msgid("All basic Binary OP(s) (share,share)");
  cout << __FUNCTION__ << " " << msgid << endl;
  
  vector<string> strX(X.size()), strY(X.size()), strZ(X.size());
  snn0.GetOps(msgid)->PrivateInput(node_id_0, X, strX);
  snn0.GetOps(msgid)->PrivateInput(node_id_1, Y, strY);
  cout << __FUNCTION__ << " " << msgid << endl;  
  vector<string> zZ(strZ.size());

  ///// Utils test:  
  // auto ptr = std::make_shared<rosetta::snn::DotProduct>(msgid, snn0.GetNetHandler());
  // vector<small_mpc_t> a = {1, 1, 0, 0, 1};
  // vector<small_mpc_t> b = {0, 1, 0, 1, 0};
  // vector<small_mpc_t> c(a.size());

  // ptr->BitMul(a, b, c, a.size());

  snn0.GetOps(msgid)->Equal(strX, strY, strZ);

  attr_type attr;
  vector<string> receivers = {"P0", "P1", "P2"};
  attr["receive_parties"] = receiver_parties_pack(receivers);
  snn0.GetOps(msgid)->Reveal(strZ, zZ, &attr);
  print_vec(zZ, 10, "SNN Equal plaintext:");
  print_vec(EQUAL_EXPECT, 10, "Equal expected:");

  snn0.GetOps(msgid)->NotEqual(strX, strY, strZ);
  snn0.GetOps(msgid)->Reveal(strZ, zZ, &attr);
  print_vec(zZ, 10, "SNN NotEqual plaintext:");
  print_vec(NEQUAL_EXPECT, 10, "NotEqual expected:");

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  SNN_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);