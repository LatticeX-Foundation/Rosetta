#include "helix__test.h"

void run(int partyid) {
  HELIX_PROTOCOL_INTERNAL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  /**
   * Abs/Negative/Square/ReLU/DReLU
   */

  vector<double> X = {-1.01, -2.0, 2.3, 2, 0.01, 0, -0.03, 0.031};
  vector<Share> shareX, shareY;
  hi->Input(node_id_2, X, shareX);
#define helix_unary_f(op)                                                    \
  do {                                                                       \
    string tag(#op);                                                         \
    {                                                                        \
      vector<Share> shareY;                                                  \
      hi->beg_statistics();                                                  \
      hi->op(shareX, shareY);                                                \
      hi->end_statistics("RTT " + tag + "(k=" + to_string(X.size()) + "):"); \
      hi->RevealAndPrint2(shareY, "shareY:");                                \
    }                                                                        \
  } while (0)

  //helix_unary_f(Abs);
  helix_unary_f(Negative);
  helix_unary_f(Square);
  helix_unary_f(ReLU);
  helix_unary_f(DReLU);
  helix_unary_f(Sigmoid);
#undef helix_unary_f

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  HELIX_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);