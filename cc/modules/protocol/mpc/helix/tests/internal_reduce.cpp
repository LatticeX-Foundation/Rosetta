#include "helix__test.h"

void run(int partyid) {
  HELIX_PROTOCOL_INTERNAL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  vector<double> X = {-3.0, 1.4201, 3.04, 0.2, 0, -1.2223};
  vector<Share> shareX;
  hi->Input(node_id_2, X, shareX);

#define helix_reduce_f(op)                           \
  do {                                               \
    string tag(#op);                                 \
    {                                                \
      vector<Share> shareY;                          \
      hi->beg_statistics();                          \
      hi->op(shareX, shareY, 2, 3);                  \
      hi->end_statistics("RTT " + tag + "(k=2,3):"); \
      hi->RevealAndPrint(shareY, "shareY:");         \
    }                                                \
  } while (0)

  helix_reduce_f(AddN);
  helix_reduce_f(Sum);
  helix_reduce_f(Mean);
  helix_reduce_f(Min);
  helix_reduce_f(Max);
#undef helix_reduce_f

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  HELIX_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);