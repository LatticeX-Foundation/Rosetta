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
#define helix_binary_f(op, size0, times)                                                       \
  do {                                                                                         \
    size_t size = size0;                                                                       \
    vector<double> X;                                                                          \
    vector<double> Y;                                                                          \
    random_vector(X, size);                                                                    \
    random_vector(Y, size);                                                                    \
                                                                                               \
    vector<Share> shareX, shareY, shareZ(size);                                                \
    hi->Input(node_id_0, X, shareX);                                                                   \
    hi->Input(node_id_1, Y, shareY);                                                                   \
                                                                                               \
    string tag(#op);                                                                           \
    hi->beg_statistics();                                                                      \
    for (int i = 0; i < times; i++) {                                                          \
      hi->op(shareX, shareY, shareZ);                                                          \
    }                                                                                          \
    hi->end_statistics(                                                                        \
      "PERF-RTT VV " + tag + "(k=" + to_string(size) + "),(times=" + to_string(times) + "):"); \
                                                                                               \
    /* constants */                                                                            \
    vector<double> CX, CY;                                                                     \
    {                                                                                          \
      CX.resize(size);                                                                         \
      CY.resize(size);                                                                         \
      srand(1);                                                                                \
      for (int i = 0; i < size; i++) {                                                         \
        CX[i] = (rand() % 300) / 100.0 + 3.0;                                                  \
      }                                                                                        \
      srand(11);                                                                               \
      for (int i = 0; i < size; i++) {                                                         \
        CY[i] = (rand() & 600) / 200.0 + 3.0;                                                  \
      }                                                                                        \
    }                                                                                          \
    hi->beg_statistics();                                                                      \
    for (int i = 0; i < times; i++) {                                                          \
      hi->op(CX, shareY, shareZ);                                                              \
    }                                                                                          \
    hi->end_statistics(                                                                        \
      "PERF-RTT CV " + tag + "(k=" + to_string(size) + "),(times=" + to_string(times) + "):"); \
                                                                                               \
    hi->beg_statistics();                                                                      \
    for (int i = 0; i < times; i++) {                                                          \
      hi->op(shareX, CY, shareZ);                                                              \
    }                                                                                          \
    hi->end_statistics(                                                                        \
      "PERF-RTT VC " + tag + "(k=" + to_string(size) + "),(times=" + to_string(times) + "):"); \
  } while (0)

#define test_all_helix_binary(size)   \
  helix_binary_f(Add, size, 100);     \
  helix_binary_f(Sub, size, 100);     \
  helix_binary_f(Mul, size, 100);     \
  helix_binary_f(Truediv, size, 3);   \
  helix_binary_f(Floordiv, size, 3);  \
                                      \
  helix_binary_f(Equal, size, 3);     \
  helix_binary_f(NotEqual, size, 3);  \
  helix_binary_f(Less, size, 3);      \
  helix_binary_f(LessEqual, size, 3); \
  helix_binary_f(Greater, size, 3);   \
  helix_binary_f(GreaterEqual, size, 3)

    test_all_helix_binary(1);
    test_all_helix_binary(11);
    test_all_helix_binary(111);
    test_all_helix_binary(1111);
    test_all_helix_binary(11111);

#undef test_all_helix_binary
#undef helix_binary_f
  }

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  HELIX_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);
