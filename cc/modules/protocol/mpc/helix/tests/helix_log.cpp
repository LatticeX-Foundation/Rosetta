#include "helix__test.h"

void run(int partyid) {
  HELIX_PROTOCOL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  msg_id_t msgid("log");
  // input a b
  vector<double> a = {1, 2, 3, 4.0, 1.5, 0.01, 0.001, 0.00013, 10, 2.71828};
  vector<double> LOG_A = {0.0, 0.693, 1.099, 1.386, 0.405, -4.605, -6.908, -8.948, 2.303, 1.0};
  vector<double> LOG1P_A = {0.693, 1.099, 1.386, 1.609, 0.916, 0.01, 0.001, 0.00013, 2.398, 1.313};
  
  vector<string> input_str, out_str;
  helix0.GetOps(msgid)->PrivateInput(node_id_0, a, input_str);
  print_vec(input_str, 10, "input_str");
  attr_type attr;
  vector<double> c;
  
  // case 0: const string
  vector<string> CONS_STR = {"1.1", "4.0"};
  vector<double> LOG_CONST = {0.095, 1.386};
  helix0.GetOps(msgid)->Log(CONS_STR, out_str, &attr);
  helix0.GetOps(msgid)->Reveal(out_str, c, &reveal_attr);
  print_vec(c, 10, "Log CONST result plain:"); 
  print_vec(LOG_CONST, 10, "LOG const expected:");
  // case 1: Log
  helix0.GetOps(msgid)->Log(input_str, out_str, &attr);
  helix0.GetOps(msgid)->Reveal(out_str, c, &reveal_attr);
  print_vec(c, 10, "Log result plain:");
  print_vec(LOG_A, 10, "Log expected:");

  // case 2: HLog
  helix0.GetOps(msgid)->HLog(input_str, out_str, &attr);
  helix0.GetOps(msgid)->Reveal(out_str, c, &reveal_attr);
  print_vec(c, 10, "HLog result plain:");  
  print_vec(LOG_A, 10, "Log expected:");


  // case 3: Log1p
  helix0.GetOps(msgid)->Log1p(input_str, out_str, &attr);
  helix0.GetOps(msgid)->Reveal(out_str, c, &reveal_attr);
  print_vec(c, 10, "Log1p result plain:");  
  print_vec(LOG1P_A, 10, "Log1p expected:");

  HELIX_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);