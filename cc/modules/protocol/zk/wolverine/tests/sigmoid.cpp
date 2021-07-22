#include "wvr_test.h"

void run(int partyid) {
  WVR_PROTOCOL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  // Also ref python/latticex/rosetta/secure/decorator/test_cases/conv2d_*.py

  msg_id_t msgid("sigmoid ....");
  {
    // a = [1.21933774, 1.66604676, 1.26618940, 1.29514586, 1.71182663, 1.44599081, 1.95028897, 1.29123498, 1.81044357, 1.13242592]
    vector<double> a, c;
    int64_t size = 10;
    random_vector(a, size, 1.0, 2.0);
    print_vector(a, "Sigmoid input", 5, 8);
    vector<string> sa(a.size());
    vector<string> sc;
    WVR0.GetOps(msgid)->PrivateInput(ALICE - 1, a, sa);
    WVR0.GetOps(msgid)->Sigmoid(sa, sc);
    WVR0.GetOps(msgid)->Reveal(sc, c);
    print_vector(c, "Sigmoid output", 5, 8);
  }
  {
    /**
     * input [ 0.16156076, -4.10311569, -4.10077932, 0.44367139, 5.75057091, 9.67613485, -2.37992174, -7.71311783, 0.05045872, -1.29939185 ]
     *    zk [ 0.54013062,  0.01683044,  0.01689148, 0.60902405, 0.99702454, 0.99995422,  0.08586121,  0.00048828, 0.51174927,  0.21560669 ]
     *    tf [ 0.5403026,   0.01625261,  0.01629004, 0.6091335,  0.99682915, 0.99993724,  0.08471662,  0.00044674, 0.512612,    0.2142674  ]
     */
    vector<double> a, c;
    int64_t size = 10;
    random_vector(a, size, -10.0, 10.0);
    print_vector(a, "Sigmoid input", 5, 8);
    vector<string> sa(a.size());
    vector<string> sc;
    WVR0.GetOps(msgid)->PrivateInput(ALICE - 1, a, sa);
    WVR0.GetOps(msgid)->Sigmoid(sa, sc);
    WVR0.GetOps(msgid)->Reveal(sc, c);
    print_vector(c, "Sigmoid output", 5, 8);
  }
  log_info << "Sigmoid check and ok.";

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  WVR_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_ZK_TEST(run);