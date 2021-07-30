#include "snn__test.h"

void run(int partyid) {
  SNN_PROTOCOL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////

  vector<double> X = {    1.5,    0.01,   0.001, 0.00013, 10, 2.71828};
  vector<double> EXPECT = {0.405, -4.605, -6.908, -8.948, 2.303, 1.000};
  size_t size = X.size();
  print_vec(X, 10, "X");

  attr_type attr;
  vector<string> receivers = {"P0", "P1", "P2"};
  attr["receive_parties"] = receiver_parties_pack(receivers);

  msg_id_t msgid("All basic Binary OP(s) (share,share)");
  cout << __FUNCTION__ << " " << msgid << endl;
    vector<string> strX, strZ;
  vector<string> zZ(strZ.size());

  vector<string> half_share_X = {"2.0", "10.0"};
  print_vec(half_share_X, 10, "Half_share {2, 10}");
  vector<string> half_share_log(size);
  snn0.GetOps(msgid)->Log(half_share_X, half_share_log);
  print_vec(half_share_log, 10, "Log half_share cipher:");
  vector<string> half_share_revealed(size);
  snn0.GetOps(msgid)->Reveal(half_share_log, half_share_revealed, &attr);
  print_vec(half_share_revealed, 10, "Log half_share revealed zZ:");  



  snn0.GetOps(msgid)->PrivateInput(node_id_0, X, strX);

  snn0.GetOps(msgid)->Log(strX, strZ);


  snn0.GetOps(msgid)->Reveal(strZ, zZ, &attr);
  print_vec(zZ, 10, "SNN Log plaintext:");
  print_vec(EXPECT, 10, "Log expected:");

  snn0.StartPerfStats();
  snn0.GetOps(msgid)->HLog(strX, strZ);
  print_vec(strZ, 10, "HLog strZ:");
  auto perf = snn0.GetPerfStats();
  cout << "HLog perf stat:" << perf.to_json(true) << endl;

  snn0.GetOps(msgid)->Reveal(strZ, zZ, &attr);
  print_vec(zZ, 10, "SNN HLog plaintext:");
  print_vec(EXPECT, 10, "HLog expected:");


  vector<double> logits = {-10.0, -3.0, 10.0, 3.0,  5.0,  3.1, 0.003, -0.02};
  vector<double> labels = {  0.0,  0.0,  1.0, 1.0, -1.0, -1.0,  -1.0,   1.0};
  vector<double> SCE = {0.0, 0.04, 0.0001, 0.049, 10.007, 6.244, 0.698, 0.703};

  vector<string> input_str1, input_str2, out_str;
  snn0.GetOps(msgid)->PrivateInput(node_id_0, logits, input_str1);
  snn0.GetOps(msgid)->PrivateInput(node_id_0, labels, input_str2);
  print_vec(logits, 10, "logits:");
  print_vec(labels, 10, "labels:");
  // attr_type attr;
  
  // case 1: Log
  snn0.StartPerfStats();
  snn0.GetOps(msgid)->SigmoidCrossEntropy(input_str1, input_str2, out_str, &attr);
  cout << "SCE PERF:" << snn0.GetPerfStats().to_json(true) << endl;
  // reveal c
  vector<double> c;
  snn0.GetOps(msgid)->Reveal(out_str, c, &attr);
  print_vec(c, 10, "SigmoidCrossEntropy result plain:");
  print_vec(SCE, 10, "SCE expected:");

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  SNN_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);