#include "cc/modules/protocol/mpc/tests/tasks/mpc__test_tasks.h"
#include <thread>
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
    msg_id_t msgid(task_id + "--All Unary OP(s) Snn or (share,share) Task");
    // cout << __FUNCTION__ << " " << msgid << endl;
    vector<double> X = {-1.01, -2.0, 2.3, 2, 0.01, 0, -0.03, 0.031, 100};
    vector<double> Negative_X = {1.01 ,  2.   , -2.3  , -2.   , -0.01 , -0.   ,  0.03 , -0.031, -100};
    vector<double> Square_X = {1.0201, 4.0, 5.289999999999999, 4.0, 0.0001, 0.0, 0.0009, 0.0009609999999999999, 10000};
    vector<double> Relu_X = {0.0, 0.0, 2.3, 2.0, 0.01, 0.0, 0.0, 0.031, 100};
    vector<double> ReluPrime_X = {0.0, 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 1.0, 1.0};
    vector<double> Sigmoid_X = {0.267, 0.119, 0.909, 0.881, 0.502, 0.5, 0.4925, 0.508, 1.0};
    vector<double> Abs_X = {1.01, 2.0, 2.3, 2, 0.01, 0, 0.03, 0.031, 100.0};

    vector<string> shareX, shareY;
    mpc_proto->GetOps(msgid)->PrivateInput(node_id_0, X, shareX);
    attr_type attr;
  #define mpc_unary_f(op)                                                    \
    do {                                                                       \
      string tag(#op);                                                         \
      {                                                                        \
        mpc_proto->GetOps(msgid)->op(shareX, shareY, &attr);     \
        vector<double> Y;                                \
        mpc_proto->GetOps(msgid)->Reveal(shareY, Y, &reveal_attr);           \
        SIMPLE_AROUND_EQUAL_T(Y, op##_X, tag+"="+ node_id + "=T" + task_id);\
      }                                                                        \
    } while (0)

    mpc_unary_f(Abs);
    mpc_unary_f(Negative);
    mpc_unary_f(Square);
    mpc_unary_f(Relu);
    mpc_unary_f(ReluPrime);
    mpc_unary_f(Sigmoid);
  #undef mpc_unary_f
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
