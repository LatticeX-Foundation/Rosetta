#include "wvr_test.h"


void softmax(rosetta::WolverineProtocol & WVR0, const vector<double> &a, int rows, int cols, const vector<double> &expect_a) {
  msg_id_t msgid("softmax ....");
  attr_type attr;
  attr["rows"] = to_string(rows);
  attr["cols"] = to_string(cols);

  vector<double> res_a(a.size());
  vector<string> str_a(a.size());
  vector<string> input_str, out_str;

  for(auto i = 0; i < a.size(); ++i) {
    str_a[i] = to_string(a[i]);
  }
  print_vec(str_a, 10, "[CC] input_str a");

  WVR0.GetOps(msgid)->PrivateInput(ALICE - 1, a, input_str);

  SimpleTimer timer;
  size_t count = 2;
  for (size_t i = 0; i < count; i++)
  {
  WVR0.GetOps(msgid)->Softmax(input_str, out_str, &attr);
  }
  cout << "Softmax " << count << " times, cost: " << timer.elapse() << endl; 
  WVR0.GetOps(msgid)->Reveal(out_str, res_a);
  print_vec(expect_a, expect_a.size(), "[PP] softmax a expect result:");
  print_vec(res_a, res_a.size(), "[PP] softmax a result plain:");
  
}


void run(int partyid) {
  WVR_PROTOCOL_TEST_INIT(partyid);


  msg_id_t msgid("softmax ....");

  vector<double> a = {0.1, 0.3, 0.5, 0.2, 0.4, 0.6};
  vector<double> a2 = {-1.0, 2.0, -3.0, 4.0, -5.0, 6.0};
  vector<double> a3 = { 10.5, 0.23, 8., 7., 6., -10.73, -9., 8., 0.125};
  vector<double> expect_a{0.2693075, 0.3289329, 0.40175956, 0.2693075, 0.32893288, 0.40175956}; 
  vector<double> expect_a2{4.7123417e-02, 9.4649917e-01, 6.3774614e-03, 1.1920116e-01, 1.4710592e-05, 8.8078409e-01};
  vector<double> expect_a3{9.24112201e-01, 3.20272884e-05, 7.58557469e-02, 7.31058598e-01, 2.68941432e-01, 1.45851615e-08, 
                           4.13836396e-08, 9.99619961e-01, 3.79984500e-04};

  cout << "---------------------------------" << endl;
  softmax(WVR0, a, 2, 3, expect_a);

  cout << "---------------------------------" << endl;
  softmax(WVR0, a2, 2, 3, expect_a2);

  cout << "---------------------------------" << endl;
  softmax(WVR0, a3, 3, 3, expect_a3);

  log_info << "softmax check and ok.";


  WVR_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_ZK_TEST(run);