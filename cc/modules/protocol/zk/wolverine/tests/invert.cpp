#include "wvr_test.h"

void run(int partyid) {
  WVR_PROTOCOL_TEST_INIT(partyid);


  msg_id_t msgid("invert ....");

  vector<double> a =          {     10.5,    0.23,     8,        7,        6,     -10.73,        -9,     8,      20.5,        -6, 65530.2345};
  vector<double> expect_inv = {0.0952381, 4.34783, 0.125, 0.142857, 0.166667, -0.0931966, -0.111111, 0.125, 0.0487805, -0.166667, 0.00001526}; 
  vector<string> invert;

  vector<string> str_a(a.size());
  vector<string> str_res(a.size());
  vector<string> input_str[2], out_str;


  WVR0.GetOps(msgid)->PrivateInput(ALICE - 1, a, input_str[0]);


  for(auto i = 0; i < a.size(); ++i) {
    str_a[i] = to_string(a[i]);
  }
  print_vec(str_a, 20, "[CC] input_str a");


  WVR0.GetOps(msgid)->Invert(input_str[0], out_str);
  WVR0.GetOps(msgid)->Reveal(out_str, str_res);
  print_vec(expect_inv, expect_inv.size(), "[PP] invert expect result:");
  print_vec(str_res, expect_inv.size(), "[PP] invert result plain:");


  log_info << "invert check and ok.";


  WVR_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_ZK_TEST(run);