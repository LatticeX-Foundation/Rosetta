#include "wvr_test.h"


void mul(rosetta::WolverineProtocol & WVR0, const vector<double> &a, const vector<double> &b, const vector<double> &expect_c) {
  msg_id_t msgid("mul ....");

  vector<string> input_str[2], out_str;
  vector<string> div;
  vector<string> str_a(a.size()), str_b(b.size());
  vector<double> res_a(a.size());

  WVR0.GetOps(msgid)->PrivateInput(ALICE - 1, a, input_str[0]);
  WVR0.GetOps(msgid)->PrivateInput(ALICE - 1, b, input_str[1]);

  for(auto i = 0; i < a.size(); ++i) {
    str_a[i] = to_string(a[i]);
    str_b[i] = to_string(b[i]);
  }
  print_vec(str_a, str_a.size(), "[CC] input_str a");
  print_vec(str_b, str_b.size(), "[CC] input_str b");

  WVR0.GetOps(msgid)->Mul(input_str[0], input_str[1], out_str);
  WVR0.GetOps(msgid)->Reveal(out_str, res_a);
  print_vec(expect_c, expect_c.size(), "[PP] mul expect result:");
  print_vec(res_a, res_a.size(), "[PP] mul result plain:");

}


void run(int partyid) {
  WVR_PROTOCOL_TEST_INIT(partyid);


  vector<double> a = { 10.5, 0.23, 8, 7, 6, -10.73, -9,  8, 0, -10000,   263.662 };
  vector<double> b = { 1.2, 6.32, 4, 4, 12,  1.42, -2, -3, 4, 0.0004, 343276.00 };
  vector<double> expect_c{  12.6, 1.4536, 32,   28,  72, -15.2366,  18,    -24,  0,        -4.0, 90508836.712};

  
  cout << "---------------------------------" << endl;
  mul(WVR0, a, b, expect_c);


  log_info << "Mul check and ok.";


  WVR_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_ZK_TEST(run);