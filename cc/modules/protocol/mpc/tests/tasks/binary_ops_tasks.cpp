#include "cc/modules/protocol/mpc/tests/tasks/mpc__test_tasks.h"
#include <thread>
#include "cc/third_party/eigen/Eigen/Core"
#include "cc/modules/common/include/utils/simple_timer.h"

void mpc_binary_task_code(shared_ptr<MpcProtocol> mpc_proto, shared_ptr<attr_type> attr) {
  mpc_proto->Init(attr->at("logfile"));

  DEFINE_TEST_VARIABLES(mpc_proto->GetMpcContext(), attr);
  attr_type &reveal_attr = (*attr);
  /**
   * All basic Binary OP(s), eg.
   * Add/Sub/Mul/Div/Compare(s)
   */
  {
    msg_id_t msgid(task_id + "=All basic Binary OP(s) (share,share)");
    // cout << __FUNCTION__ << " " << msgid << endl;

    vector<double> X = {-1.01, -2.00, -3.01, 1.3, 2.02, 3.14, +2, -0.01, 19.0, 10.0};
    vector<double> Y = {-1.00, -2.01, -3.01, 1.3, 2.03, 3.12, -2, +0.01, 10.0, 10.0};
    vector<double> CONST_POW_N = {0, 1, 2, 4, 2, 3, 2, 0, 3, 2};
    vector<double> Add_X = {-2.01, -4.01, -6.02, 2.6, 4.05, 6.26, 0., 0., 29., 20.};
    vector<double> Sub_X = {-0.01, 0.01, 0., 0., -0.01, 0.02, 4., -0.02, 9., 0};
    vector<double> Pow_X = {1.000, -2.000, 9.060, 2.856, 4.080, 31, 4.000, 1.000, 6859, 100};
    vector<double> Mul_X = {1.010, 4.020, 9.060, 1.690, 4.101, 9.797, -4.000, -0.0001, 190, 100};
    vector<double> Truediv_X = {1.01,      0.99502486, 1.,  1.,  0.9950739,
                                1.0064104, -1.,        -1., 1.9, 1.}; //the fourth `nan` set to -2
    vector<double> Floordiv_X = {1., 0.,  1.,  1., 0.,
                                 1., -1., -1., 1., 1}; //the fourth `nan` set to -2
    vector<double> Div_X = {1.01,       0.99502488, 1.,  1.,  0.99507389,
                            1.00641026, -1.,        -1., 1.9, 1.}; //the fourth `nan` set to -2

    vector<double> Greater_X = {0, 1, 0, 0, 0, 1, 1, 0, 1, 0};
    vector<double> GreaterEqual_X = {0, 1, 1, 1, 0, 1, 1, 0, 1, 1};
    vector<double> Equal_X = {0, 0, 1, 1, 0, 0, 0, 0, 0, 1};
    vector<double> NotEqual_X = {1, 1, 0, 0, 1, 1, 1, 1, 1, 0};
    vector<double> LessEqual_X = {1, 0, 1, 1, 1, 0, 0, 1, 0, 1};
    vector<double> Less_X = {1, 0, 0, 0, 1, 0, 0, 1, 0, 0};

    // vector<double> X = {-1.02, -2.00, -3.01, 0, 1.3, 2.02, 3.14, +2, -0.01, 19.0, 10.0};
    // vector<double> Y = {-1.00, -2.02, -3.01, 0, 1.3, 2.04, 3.12, -2, +0.01, 10.0, 10.0};
    size_t size = X.size();

    vector<string> strX, strY, strZ;
    mpc_proto->GetOps(msgid)->PrivateInput(node_id_0, X, strX);
    mpc_proto->GetOps(msgid)->PrivateInput(node_id_1, Y, strY);

    // print_vec(X, 20, "Task: " + mpc_proto->GetMpcContext()->TASK_ID + " == input X");
    // print_vec(Y, 20, "Task: " + mpc_proto->GetMpcContext()->TASK_ID + " == input Y");

    int float_precision = mpc_proto->GetMpcContext()->FLOAT_PRECISION;
    vector<string> literalX, literalY, const_pow_literalY;
    convert_double_to_literal_str(X, literalX, float_precision);
    convert_double_to_literal_str(Y, literalY, float_precision);
    convert_double_to_literal_str(CONST_POW_N, const_pow_literalY, float_precision);

#define snn_binary_f(op)                                        \
  do {                                                            \
    string tag(#op);                                              \
    {                                                             \
      mpc_proto->GetOps(msgid)->op(strX, strY, strZ);                 \
      vector<double> Z;                                           \
      mpc_proto->GetOps(msgid)->Reveal(strZ, Z, &reveal_attr);                      \
      SIMPLE_AROUND_EQUAL_T(Z, op##_X, tag + "=" + node_id + "=T" + task_id);\
    }                                                             \
    {                                                             \
      attr_type attr;                                             \
      attr["lh_is_const"] = "1";                                  \
      mpc_proto->GetOps(msgid)->op(literalX, strY, strZ, &attr);             \
      vector<double> Z;                                           \
      mpc_proto->GetOps(msgid)->Reveal(strZ, Z, &reveal_attr);                      \
      SIMPLE_AROUND_EQUAL_T(Z, op##_X, tag + "(const, private)=" + node_id + "=T" + task_id);\
    }                                                                                                                      \
    {                                                             \
      attr_type attr;                                             \
      attr["rh_is_const"] = "1";                                  \
      mpc_proto->GetOps(msgid)->op(strX, literalY, strZ, &attr);      \
      vector<double> Z;                                           \
      mpc_proto->GetOps(msgid)->Reveal(strZ, Z, &reveal_attr);                      \
      SIMPLE_AROUND_EQUAL_T(Z, op##_X, tag + "(private, const)=" + node_id + "=T" + task_id);\
    }                                                                                                                   \
  } while (0)

  #define snn_binary_f_rh_const(op)                                        \
  do {                                                            \
    string tag(#op);                                              \
    {                                                             \
      attr_type attr;                                             \
      attr["rh_is_const"] = "1";                                  \
      mpc_proto->GetOps(msgid)->op(strX, const_pow_literalY, strZ, &attr);      \
      vector<double> Z;                                           \
      mpc_proto->GetOps(msgid)->Reveal(strZ, Z, &reveal_attr);                      \
      AROUND_EQUAL_T(Z, op##_X, 0.1, tag + "(private,const)=" + node_id + "=T" + task_id); \
    }                                                                                                                   \
  } while (0)

    /***********    basic binary ops test  ***********/
    snn_binary_f(Add);
    snn_binary_f(Sub);
    snn_binary_f(Mul);
    snn_binary_f(Div);
    // snn_binary_f(Floordiv);
    snn_binary_f(Truediv);

    snn_binary_f_rh_const(Pow);

    // /***********    basic compare binary ops  ***********/
    snn_binary_f(Equal);
    snn_binary_f(NotEqual);
    snn_binary_f(Less);
    snn_binary_f(LessEqual);
    snn_binary_f(Greater);
    snn_binary_f(GreaterEqual);

#undef snn_binary_f
  }
}

void run(int partyid) {
  int task_num = TEST_TASK_NUM;
  vector<MpcTaskCodeFunc> task_codes(task_num, mpc_binary_task_code);
  string logfile = "log/" + get_file_name(__FILENAME__) + "-" + string(MPC_TEST_PROTOCOL_NAME) + to_string(partyid) + ".log"; 
  run_task_threads_main(partyid, logfile, task_codes);

  log_info << "-----> " << __FILE__ << " TASK WITH TASK_NUM: " << task_num << " TEST Done !!!!\n\n";
}

RUN_MPC_TEST(run);
