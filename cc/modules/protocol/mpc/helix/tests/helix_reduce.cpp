#include "helix__test.h"

void run(int partyid) {
  HELIX_PROTOCOL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  /**
   * All basic Reduce OP(s)
   * Sum/Mean/AddN/...
   */

  {
    msg_id_t msgid("All basic Reduce OP(s) (share,share)");
    cout << __FUNCTION__ << " " << msgid << endl;

    vector<double> X = {-1.01, -2.00, -3.01, 0, 1.3, 2.02, 3.14, +2, -0.01, 6.54321};
    size_t size = X.size();
    print_vec(X, 10, "X");

    vector<string> strX, strY, strZ;
    helix0.GetOps(msgid)->PrivateInput(node_id_0, X, strX);
    print_vec(strX, 10, "strX");

    vector<string> literalX;
    helix_double_to_plain_string(X, literalX);
    print_vec(literalX, 10, "literalX");

#define helix_reduce_f(op)                             \
  do {                                                 \
    string tag(#op);                                   \
    {                                                  \
      helix0.GetOps(msgid)->op(strX, strY);            \
      print_vec(strY, 10, tag + "(strX)");             \
      vector<double> Y;                                \
      helix0.GetOps(msgid)->Reveal(strY, Y, &reveal_attr);           \
      print_vec(Y, 10, tag + " strX -> Y");            \
    }                                                  \
    {                                                  \
      helix0.GetOps(msgid)->op(literalX, strY);        \
      print_vec(strY, 10, tag + "(literalX)");         \
      vector<double> Y;                                \
      helix0.GetOps(msgid)->Reveal(strY, Y, &reveal_attr);           \
      print_vec(Y, 10, tag + " literalX -> Y");        \
    }                                                  \
    attr_type attr;                                    \
    attr["rows"] = "2";                                \
    attr["cols"] = "5";                                \
    {                                                  \
      helix0.GetOps(msgid)->op(strX, strY, &attr);     \
      print_vec(strY, 10, tag + "(strX)");             \
      vector<double> Y;                                \
      helix0.GetOps(msgid)->Reveal(strY, Y, &reveal_attr);           \
      print_vec(Y, 10, tag + " strX -> Y");            \
    }                                                  \
    {                                                  \
      helix0.GetOps(msgid)->op(literalX, strY, &attr); \
      print_vec(strY, 10, tag + "(literalX)");         \
      vector<double> Y;                                \
      helix0.GetOps(msgid)->Reveal(strY, Y, &reveal_attr);           \
      print_vec(Y, 10, tag + " literalX -> Y");        \
    }                                                  \
  } while (0)

    helix_reduce_f(AddN);
    helix_reduce_f(Sum);
    helix_reduce_f(Mean);
    helix_reduce_f(Max);
    helix_reduce_f(Min);
#undef helix_reduce_f
  }

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  HELIX_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);