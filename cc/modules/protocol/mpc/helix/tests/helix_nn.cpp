#include "helix__test.h"

void run(int partyid) {
  HELIX_PROTOCOL_TEST_INIT(partyid);

  msg_id_t msgid("log");
  // input a b
  vector<double> logits = {-10.0, -3.0, 10.0, 3.0,  5.0,  3.1, 0.003, -0.02};
  vector<double> labels = {  0.0,  0.0,  1.0, 1.0, -1.0, -1.0,  -1.0,   1.0};
  vector<double> SCE = {0.0, 0.04, 0.0001, 0.049, 10.007, 6.244, 0.698, 0.703};
  vector<string> input_str1, input_str2, out_str;
  helix0.GetOps(msgid)->PrivateInput(node_id_0, logits, input_str1);
  helix0.GetOps(msgid)->PrivateInput(node_id_0, labels, input_str2);
  print_vec(logits, 10, "logits:");
  print_vec(labels, 10, "labels:");
  attr_type attr;
  
  // case 1: Log
  helix0.StartPerfStats();
  helix0.GetOps(msgid)->SigmoidCrossEntropy(input_str1, input_str2, out_str, &attr);
  cout << "SCE PERF:" << helix0.GetPerfStats().to_json(true) << endl;
  // reveal c
  vector<double> c;
  helix0.GetOps(msgid)->Reveal(out_str, c, &reveal_attr);
  print_vec(c, 10, "SigmoidCrossEntropy result plain:");
  print_vec(SCE, 10, "SCE expected:");

  HELIX_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);