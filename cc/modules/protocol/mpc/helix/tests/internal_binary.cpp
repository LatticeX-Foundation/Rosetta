#include "helix__test.h"

void run(int partyid) {
  HELIX_PROTOCOL_INTERNAL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  /**
   * All basic Binary OP(s), eg.
   * Add/Sub/Mul/Div/Compare(s)
   * 
   * VV variable vs variable
   * CV constant vs variable
   * VC variable vs constant
   */
  {
    vector<double> X = {-1.01, -2.00, -3.01, 0, 1.3, 2.02, 3.14, +2, -0.01};
    vector<double> Y = {-1.00, -2.01, -3.01, 0, 1.3, 2.03, 3.12, -2, +0.01};
    size_t size = X.size();
    vector<Share> shareX, shareY;
    hi->Input(node_id_0, X, shareX);
    hi->Input(node_id_1, Y, shareY);

#define helix_binary_f(op)                                                  \
  do {                                                                      \
    string tag(#op);                                                        \
    {                                                                       \
      vector<Share> shareZ;                                                 \
      hi->beg_statistics();                                                 \
      hi->op(shareX, shareY, shareZ);                                       \
      hi->end_statistics("RTT VV " + tag + "(k=" + to_string(size) + "):"); \
      hi->RevealAndPrint2(shareZ, tag + "-shareZ:");                        \
    }                                                                       \
    {                                                                       \
      vector<Share> shareZ;                                                 \
      hi->beg_statistics();                                                 \
      hi->op(shareX, Y, shareZ);                                            \
      hi->end_statistics("RTT VC " + tag + "(k=" + to_string(size) + "):"); \
      hi->RevealAndPrint2(shareZ, tag + "-shareZ:");                        \
    }                                                                       \
    {                                                                       \
      vector<Share> shareZ;                                                 \
      hi->beg_statistics();                                                 \
      hi->op(X, shareY, shareZ);                                            \
      hi->end_statistics("RTT CV " + tag + "(k=" + to_string(size) + "):"); \
      hi->RevealAndPrint2(shareZ, tag + "-shareZ:");                        \
    }                                                                       \
  } while (0)

    helix_binary_f(Add);
    helix_binary_f(Sub);
    helix_binary_f(Mul);
    helix_binary_f(Div);
    helix_binary_f(Floordiv);
    helix_binary_f(Truediv);

    helix_binary_f(Equal);
    helix_binary_f(NotEqual);
    helix_binary_f(Less);
    helix_binary_f(LessEqual);
    helix_binary_f(Greater);
    helix_binary_f(GreaterEqual);
#undef helix_binary_f
  }

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  HELIX_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);
