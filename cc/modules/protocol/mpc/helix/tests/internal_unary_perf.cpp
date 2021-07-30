#include "helix__test.h"

void run(int partyid) {
  HELIX_PROTOCOL_INTERNAL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  /**
   * Abs/Negative/Square/ReLU/DReLU
   */

#define helix_unary_f(op, size0, times)                                           \
  do {                                                                            \
    size_t size = size0;                                                          \
    vector<double> X;                                                             \
    random_vector(X, size);                                                       \
                                                                                  \
    vector<Share> shareX, shareY(size);                                           \
    hi->Input(node_id_2, X, shareX);                                                      \
                                                                                  \
    string tag(#op);                                                              \
    {                                                                             \
      vector<Share> shareY;                                                       \
      hi->beg_statistics();                                                       \
      hi->op(shareX, shareY);                                                     \
      hi->end_statistics("PERF-RTT " + tag + "(k=" + to_string(X.size()) + "):"); \
    }                                                                             \
  } while (0)

#define test_all_helix_unary(size)     \
  helix_unary_f(Negative, size, 1000); \
  helix_unary_f(Square, size, 1000);   \
  helix_unary_f(ReLU, size, 1000);     \
  helix_unary_f(DReLU, size, 1000);    \
  helix_unary_f(Sigmoid, size, 10)

  test_all_helix_unary(1);
  test_all_helix_unary(11);
  test_all_helix_unary(111);
  test_all_helix_unary(1111);
  test_all_helix_unary(11111);
#undef test_all_helix_unary
#undef helix_unary_f

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  HELIX_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);