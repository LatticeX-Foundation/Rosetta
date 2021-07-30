#include "snn__test.h"

void run(int partyid) {
  SNN_PROTOCOL_TEST_INIT(partyid);
  // mimic 8^3 * 8^2
  vector<double> X(100000, 8.0 * 8.0 * 8.0);
  vector<double> Y(100000, 8.0 * 8.0);

  vector<double> EXPECT(100000, 32768.0);
  size_t size = X.size();
  
  // vector<double> X = { 8.0, -8.0, 5, -5, 2, -2};
  // vector<double> EXPECT = {0.9997, 0.00033, 0.9933, 0.00669, 0.8808, 0.1192};
  // size_t size = X.size();
  print_vec(X, 10, "X");
  print_vec(Y, 10, "Y");

  attr_type attr;
  vector<string> receivers = {"P0", "P1", "P2"};
  attr["receive_parties"] = receiver_parties_pack(receivers);

  msg_id_t msgid("Mul OP(s) (share,share)");
  cout << __FUNCTION__ << " " << msgid << endl;
  
  vector<string> strX, strY, strZ;
  vector<string> zZ(strZ.size());
  // In theory, the probability of this truncation error is about 1/(2^{64-(15+16+16)}, 
  // i.e. 1/(2^17) = 0.000008
  // we do this 10-timers to make this more likely to happen.
  int wrong_cnt = 0;
  int ITER = 10;
  for(int kkk = 0;  kkk < ITER; ++kkk) {
      snn0.GetOps(msgid)->PrivateInput(node_id_0, X, strX);
      snn0.GetOps(msgid)->PrivateInput(node_id_0, Y, strY);
      snn0.GetOps(msgid)->Mul(strX, strY, strZ);
      snn0.GetOps(msgid)->Reveal(strZ, zZ, &attr);
      print_vec(zZ, 10, "SNN Mul plaintext:");
      print_vec(EXPECT, 10, "Mul expected:");
      for(int i = 0; i < size; ++i) {
        auto inner = stol(zZ[i]);
        auto expected = long(EXPECT[i]);
        if (inner != expected) {
          wrong_cnt++;
          cout << i << "-th item wrong in iter: " << kkk << "!!!" << endl;
          cout << "inner: " << inner << " <-> expected: " << expected << endl;
        }
      }
  }
  cout << "error num:" << wrong_cnt << endl;
  cout << "probability in this case:" << (wrong_cnt * 1.0) / (ITER * size) << endl; 
  SNN_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);