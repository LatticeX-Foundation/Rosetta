#include "helix__test.h"

void run(int partyid) {
  HELIX_PROTOCOL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////

  {
    msg_id_t msgid("input");

    // input a b
    vector<double> a = {1, 2, 3, 4.0};
    vector<double> b = {5, 6, 7, 8.0};
    vector<string> outa, outb;
    helix0.GetOps(msgid)->PrivateInput(node_id_0, a, outa);
    helix0.GetOps(msgid)->PrivateInput(node_id_1, b, outb);
    print_vec(outa, 10, "outa");
    print_vec(outb, 10, "outb");

    // z = matmul a b
    vector<string> outc;
    attr_type attr;
    attr["m"] = "2";
    attr["k"] = "2";
    attr["n"] = "2";
    helix0.GetOps(msgid)->Matmul(outa, outb, outc, &attr);
    print_vec(outc, 10, "outc");

    // reveal c
    vector<double> c;
    helix0.GetOps(msgid)->Reveal(outc, c, &reveal_attr);
    print_vec(c, 10, "c");
  }

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  HELIX_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);
