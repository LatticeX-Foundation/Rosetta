#include "helix__test.h"

void run(int partyid) {
  HELIX_PROTOCOL_TEST_INIT(partyid);
  msg_id_t msgid("abs");
  // input a b
  vector<double> x = {-10.0, -3.0, 10.0, 3.0, 0.003, -0.02};
  vector<string> input_str, out_str;
  helix0.GetOps(msgid)->PrivateInput(node_id_0, x, input_str);
  print_vec(input_str, 10, "X:");
  attr_type attr;
  
  // case 1: Abs
  helix0.GetOps(msgid)->Abs(input_str, out_str, &attr);
  print_vec(out_str, 10, "Abs output cipher");
  // reveal c
  vector<double> c;
  helix0.GetOps(msgid)->Reveal(out_str, c, &reveal_attr);
  print_vec(c, 10, "Abs result plain:");

  // case 2: AbsPrime
  helix0.GetOps(msgid)->AbsPrime(input_str, out_str, &attr);
  print_vec(out_str, 10, "AbsPrime output cipher");
  // reveal c
  helix0.GetOps(msgid)->Reveal(out_str, c, &reveal_attr);
  print_vec(c, 10, "AbsPrime result plain:");

  HELIX_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);