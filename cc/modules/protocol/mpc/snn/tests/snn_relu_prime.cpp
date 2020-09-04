#include "snn__test.h"

static inline string to_hex(const void* v, int len) {
  string hex(2 * len, 0);
  unsigned char* p = (unsigned char*)v;
  for (int i = 0; i < len; ++i) {
    unsigned char p0;
    unsigned char p1;
    p0 = p[len - 1 - i] & 0x0000000F;
    p1 = p[len - 1 - i] >> 4 & 0x0000000F;
    hex[2 * i + 1] = p0 >= 0 && p0 <= 9 ? p0 + '0' : p0 - 10 + 'A';
    hex[2 * i] = p1 >= 0 && p1 <= 9 ? p1 + '0' : p1 - 10 + 'A';
  }

  // cout << "v to hex: " << hex << endl;

  return hex;
}

void run(int partyid) {
  SNN_PROTOCOL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////

  vector<double> X = {-1.5, -2, 2, 3, 1.2, 10, 2.71828};
  vector<double> EXPECT = {0, 0, 1, 1, 1,   1, 1};
  size_t size = X.size();
  print_vec(X, size, "X");

  string msgid("relu-prime OP(s) (share,share)");
  cout << __FUNCTION__ << " " << msgid << endl;

  vector<string> strX, strZ, plainX(X.size());
  snn0.GetOps(msgid)->PrivateInput(0, X, strX);
  snn0.GetOps(msgid)->Reveal(strX, plainX);
  snn0.GetOps(msgid)->ReluPrime(strX, strZ);
  for (auto i = 0; i < strZ.size(); ++i) {
    cout << "z binary-" << i <<": " << to_hex(strZ[i].data(), strZ[i].size()-1) << endl;
  }

  vector<string> zZ(strZ.size());
  snn0.GetOps(msgid)->Reveal(strZ, zZ);
  print_vec(plainX, size, "SNN PrivateInput Reveal: ");
  print_vec(zZ, size, "SNN ReluPrime plaintext:");
  print_vec(EXPECT, size, "ReluPrime expected:");
  
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  SNN_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);