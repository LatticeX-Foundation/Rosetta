#include "snn__test.h"

void run(int partyid) {
  SNN_PROTOCOL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  /**
   * All basic Binary OP(s), eg.
   * Add/Sub/Mul/Div/Compare(s)
   */
  {
    msg_id_t msgid("All basic Binary OP(s) (share,share)");
    cout << __FUNCTION__ << " " << msgid << endl;

    vector<double> X = {-1.01, -2.00, -3.01, 0, 1.3, 2.02, 3.14, +2, -0.01, 19.0, 10.0};
    vector<double> Y = {-1.00, -2.01, -3.01, 0, 1.3, 2.03, 3.12, -2, +0.01, 10.0, 10.0};
    // vector<double> X = {-1.02, -2.00, -3.01, 0, 1.3, 2.02, 3.14, +2, -0.01, 19.0, 10.0};
    // vector<double> Y = {-1.00, -2.02, -3.01, 0, 1.3, 2.04, 3.12, -2, +0.01, 10.0, 10.0};
    size_t size = X.size();

    vector<string> strX, strY, strZ;
    snn0.GetOps(msgid)->PrivateInput(node_id_0, X, strX);
    snn0.GetOps(msgid)->PrivateInput(node_id_1, Y, strY);

    print_vec(X, 20, "input X");
    print_vec(Y, 20, "input Y");

    int float_precision = snn0.GetMpcContext()->FLOAT_PRECISION;
    vector<string> literalX, literalY;
    convert_double_to_literal_str(X, literalX, float_precision);
    convert_double_to_literal_str(Y, literalY, float_precision);

#define snn_binary_f(op)                                        \
  do {                                                            \
    string tag(#op);                                              \
    {                                                             \
      snn0.GetOps(msgid)->op(strX, strY, strZ);                 \
      vector<double> Z;                                           \
      snn0.GetOps(msgid)->Reveal(strZ, Z, &reveal_attr);                      \
      print_vec(Z, 20, "strX " + tag + " strY = reveal(strZ)");                               \
    }                                                             \
    {                                                             \
      attr_type attr;                                             \
      attr["lh_is_const"] = "1";                                  \
      snn0.GetOps(msgid)->op(literalX, strY, strZ, &attr);             \
      vector<double> Z;                                           \
      snn0.GetOps(msgid)->Reveal(strZ, Z, &reveal_attr);                      \
      print_vec(Z, 20, "literalX " + tag + " strY = reveal(strZ)");                               \
    }                                                                                                                      \
    {                                                             \
      attr_type attr;                                             \
      attr["rh_is_const"] = "1";                                  \
      snn0.GetOps(msgid)->op(strX, literalY, strZ, &attr);      \
      vector<double> Z;                                           \
      snn0.GetOps(msgid)->Reveal(strZ, Z, &reveal_attr);                      \
      print_vec(Z, 20, "strX " + tag + " literalY(c) = reveal(strZ)");                               \
    }                                                                                                                   \
  } while (0)

    /***********    basic binary ops test  ***********/
    snn_binary_f(Add);
    snn_binary_f(Sub);
    snn_binary_f(Mul);
    snn_binary_f(Floordiv);
    snn_binary_f(Truediv);

    /***********    basic compare binary ops  ***********/
    snn_binary_f(Equal);
    snn_binary_f(NotEqual);
    snn_binary_f(Less);
    snn_binary_f(LessEqual);
    snn_binary_f(Greater);
    snn_binary_f(GreaterEqual);

#undef snn_binary_f
  }

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  SNN_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);
