#include "wvr_test.h"


void div(rosetta::WolverineProtocol & WVR0, const vector<double> &a, const vector<double> &b, const vector<double> &expect_c) {
  msg_id_t msgid("div ....");

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

  WVR0.GetOps(msgid)->Div(input_str[0], input_str[1], out_str);
  WVR0.GetOps(msgid)->Reveal(out_str, res_a);
  print_vec(expect_c, expect_c.size(), "[PP] div expect result:");
  print_vec(res_a, res_a.size(), "[PP] div result plain:");

}


void run(int partyid) {
  WVR_PROTOCOL_TEST_INIT(partyid);


  vector<double> a = {10.5, 0.23, 8, 7, 6, -10.73, -9,  8, 20.5, -6};
  vector<double> b = { 1.2, 6.32, 4, 4, 12,  1.42, -2, -3, 4,  -2.3};
  vector<double> expect_c{8.75, 0.0363924, 2, 1.75, 0.5, -7.55634, 4.5, -2.66667, 5.125, 2.6087};

  vector<double> a2 = {2636.62, 1.52588e-05, 3.43013e+06};
  vector<double> b2 = {3.43276e+06, 3.43276e+06, 3.43276e+06};
  vector<double> expect_c2{7.680758e-4, 4.44505e-12, 0.9992338};

  vector<double> a3 = {2636.62, 1.52588e-05, 199427};
  vector<double> b3 = {202064, 202064, 202064};
  vector<double> expect_c3{0.013048, 7.551468e-11, 0.986949};

  
  cout << "---------------------------------" << endl;
  div(WVR0, a, b, expect_c);

  cout << "---------------------------------" << endl;
  div(WVR0, a2, b2, expect_c2);

  cout << "---------------------------------" << endl;
  div(WVR0, a3, b3, expect_c3);

  log_info << "Div check and ok.";


  WVR_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_ZK_TEST(run);