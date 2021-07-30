#include "helix__test.h"

void run(int partyid) {
  HELIX_PROTOCOL_TEST_INIT(partyid);
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
    size_t size = X.size();
    print_vec(X, 20, "X");
    print_vec(Y, 20, "Y");

    vector<string> strX, strY, strZ;
    helix0.GetOps(msgid)->PrivateInput(node_id_0, X, strX);
    print_vec(strX, 20, "strX");
    helix0.GetOps(msgid)->PrivateInput(node_id_1, Y, strY);
    print_vec(strY, 20, "strY");

    vector<string> literalX, literalY;
    helix_double_to_plain_string(X, literalX);
    helix_double_to_plain_string(Y, literalY);
    print_vec(literalX, 20, "literalX");
    print_vec(literalY, 20, "literalY");

#define helix_binary_f(op)                                        \
  do {                                                            \
    string tag(#op);                                              \
    {                                                             \
      helix0.GetOps(msgid)->op(strX, strY, strZ);                 \
      print_vec(strZ, 20, "strX " + tag + " strY = strZ");        \
      vector<double> Z;                                           \
      helix0.GetOps(msgid)->Reveal(strZ, Z, &reveal_attr);                      \
      print_vec(Z, 20, tag + " Z");                               \
    }                                                             \
    {                                                             \
      helix0.GetOps(msgid)->op(literalX, strY, strZ);             \
      print_vec(strZ, 20, "literalX " + tag + " strY = strZ");    \
      vector<double> Z;                                           \
      helix0.GetOps(msgid)->Reveal(strZ, Z, &reveal_attr);                      \
      print_vec(Z, 20, tag + " Z");                               \
    }                                                             \
    {                                                             \
      helix0.GetOps(msgid)->op(strY, literalX, strZ);             \
      print_vec(strZ, 20, "strY " + tag + " literalX = strZ");    \
      vector<double> Z;                                           \
      helix0.GetOps(msgid)->Reveal(strZ, Z, &reveal_attr);                      \
      print_vec(Z, 20, tag + " Z");                               \
    }                                                             \
    {                                                             \
      attr_type attr;                                             \
      attr["rh_is_const"] = "1";                                  \
      helix0.GetOps(msgid)->op(strY, literalX, strZ, &attr);      \
      print_vec(strZ, 20, "strY " + tag + " literalX(c) = strZ"); \
      vector<double> Z;                                           \
      helix0.GetOps(msgid)->Reveal(strZ, Z, &reveal_attr);                      \
      print_vec(Z, 20, tag + " Z");                               \
    }                                                             \
    {                                                             \
      attr_type attr;                                             \
      attr["lh_is_const"] = "1";                                  \
      helix0.GetOps(msgid)->op(literalX, strY, strZ, &attr);      \
      print_vec(strZ, 20, "literalX(c) " + tag + " strY = strZ"); \
      vector<double> Z;                                           \
      helix0.GetOps(msgid)->Reveal(strZ, Z, &reveal_attr);                      \
      print_vec(Z, 20, tag + " Z");                               \
    }                                                             \
  } while (0)

    helix_binary_f(Add);
    helix_binary_f(Sub);
    helix_binary_f(Mul);
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
