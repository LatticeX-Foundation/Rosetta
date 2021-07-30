#include "snn__test.h"

void run(int partyid) {
  SNN_PROTOCOL_TEST_INIT(partyid);

  msg_id_t msgid("SECURE-NN-NN");
  // input a b
  vector<double> logits = {-10.0, -3.0, 10.0, 3.0,  5.0,  3.1, 0.003, -0.02};
  vector<double> labels = {  0.0,  0.0,  1.0, 1.0, -1.0, -1.0,  -1.0,   1.0};
  vector<double> SCE = {0.0, 0.04, 0.0001, 0.049, 10.007, 6.244, 0.698, 0.703};
  vector<string> input_str1, input_str2, out_str;
  snn0.GetOps(msgid)->PrivateInput(node_id_0, logits, input_str1);
  snn0.GetOps(msgid)->PrivateInput(node_id_0, labels, input_str2);
  print_vec(logits, 10, "logits:");
  print_vec(labels, 10, "labels:");
  attr_type attr;
  
  // case 1: Log
  snn0.StartPerfStats();
  snn0.GetOps(msgid)->SigmoidCrossEntropy(input_str1, input_str2, out_str, &attr);
  cout << "SCE PERF:" << snn0.GetPerfStats().to_json(true) << endl;
  // reveal c
  vector<double> c;
  snn0.GetOps(msgid)->Reveal(out_str, c, &reveal_attr);
  print_vec(c, 10, "SigmoidCrossEntropy result plain:");
  print_vec(SCE, 10, "SCE expected:");

  SNN_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);