#include "cc/modules/protocol/mpc/tests/tasks/mpc__test_tasks.h"
#include <thread>
#include "cc/third_party/eigen/Eigen/Core"
#include "cc/modules/common/include/utils/simple_timer.h"

void mpc_reduce_task_code(shared_ptr<MpcProtocol> mpc_proto, shared_ptr<attr_type> attr) {
  mpc_proto->Init(attr->at("logfile"));
  
  DEFINE_TEST_VARIABLES(mpc_proto->GetMpcContext(), attr);
  attr_type &reveal_attr = (*attr);
  /**
   * All basic Binary OP(s), eg.
   * Add/Sub/Mul/Div/Compare(s)
   */
  {
    msg_id_t msgid(task_id + "=All reduce OP(s) (share,share) snn or helix");
    // cout << __FUNCTION__ << " " << msgid << "rows: " << 2 << ", col:" << 5 << endl;

    vector<double> X = {-1.01, -2.00, -3.01, 0, 1.3, 2.02, 3.14, +2, -0.01, 6.54321};
    vector<double> AddN_X = {1.01,  1.1400001, -1.01, -0.01, 7.84321};
    vector<double> Sum_X = {-4.7200003, 13.69321};
    vector<double> Mean_X = {-0.94400007,  2.738642};
    vector<double> Max_X = {1.3, 6.54321};
    vector<double> Min_X = {-3.01, -0.01};
    
    size_t size = X.size();
    // print_vec(X, 10, "X");

    vector<string> strX, strY, strZ;
    mpc_proto->GetOps(msgid)->PrivateInput(node_id_0, X, strX);

#define mpc_reduce_f(op)                               \
  do {                                                 \
    string tag(#op);                                   \
    attr_type attr;                                    \
    attr["rows"] = "2";                                \
    attr["cols"] = "5";                                \
    {                                                  \
      mpc_proto->GetOps(msgid)->op(strX, strY, &attr); \
      vector<double> Y;                                \
      mpc_proto->GetOps(msgid)->Reveal(strY, Y, &reveal_attr);\
      HD_AROUND_EQUAL_T(Y, op##_X, tag+"="+ node_id + "=T" + task_id);\
    }                                                  \
  } while (0)

    mpc_reduce_f(AddN);
    mpc_reduce_f(Sum);
    mpc_reduce_f(Mean);
    mpc_reduce_f(Max);
    mpc_reduce_f(Min);
#undef mpc_reduce_f
  }
}

void run(int partyid) {
  int task_num = TEST_TASK_NUM;
  vector<MpcTaskCodeFunc> task_codes(task_num, mpc_reduce_task_code);
  string logfile = "log/" + get_file_name(__FILENAME__) + "-" + string(MPC_TEST_PROTOCOL_NAME) + to_string(partyid) + ".log"; 
  run_task_threads_main(partyid, logfile, task_codes);

  log_info << "-----> " << __FILE__ << " TASK WITH TASK_NUM: " << task_num << " TEST Done !!!!\n\n";
}

RUN_MPC_TEST(run);
