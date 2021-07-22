#include "wvr_test.h"


void exp(rosetta::WolverineProtocol & WVR0, const vector<double> &a, const vector<double> &expect_exp) {
  msg_id_t msgid("exp ....");

  vector<string> str_a(a.size());
  vector<double> res_a(a.size());
  vector<string> input_str[2], out_str;

  std::cout << "begin to PrivateInput..." << endl;
  WVR0.GetOps(msgid)->PrivateInput(ALICE - 1, a, input_str[0]);
  std::cout << "end to PrivateInput..." << endl;

  for(auto i = 0; i < a.size(); ++i) {
    str_a[i] = to_string(a[i]);
  }
  print_vec(str_a, str_a.size(), "[CC] input_str a");

  std::cout << "begin to Exp..." << endl;
  WVR0.GetOps(msgid)->Exp(input_str[0], out_str);
  WVR0.GetOps(msgid)->Reveal(out_str, res_a);

  print_vec(expect_exp, expect_exp.size(), "[PP] exp a expect result:");
  print_vec(res_a, res_a.size(), "[PP] exp a result plain:");

}

void run(int partyid) {
  WVR_PROTOCOL_TEST_INIT(partyid);
  

  std::vector<double> a = { 10.5, 0.23, 8, 7, 6, -10.73, -9,  8, 1.5, -6};
  std::vector<double> a2 = { 0.105, 0.23, 0.8, -0.7, -0.6, -1.73, 1.12, -0.8, -0.205, 
						                -1.6,  0.920, -0.920, 0.31, -0.31, 0.418, -0.418 };
  vector<double> expect_exp{36315.5, 1.2586, 2980.96, 1096.63, 403.429, 2.18786e-05, 0.00012341, 2980.96, 4.48169, 0.00247875}; 
  vector<double> expect_exp2{1.11071, 1.2586, 2.22554, 0.496585, 0.548812, 0.177284, 3.06485, 0.449329, 0.814647, 0.201897, 2.50929, 0.398519, 1.36343, 0.733447, 1.51892, 0.658362};

  cout << "---------------------------------" << endl;
  exp(WVR0, a, expect_exp);

  cout << "---------------------------------" << endl;
  exp(WVR0, a2, expect_exp2);

  log_info << "exp check and ok.";


  WVR_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_ZK_TEST(run);