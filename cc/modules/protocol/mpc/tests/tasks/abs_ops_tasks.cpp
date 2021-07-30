#include "cc/modules/protocol/mpc/tests/tasks/mpc__test_tasks.h"
#include <thread>
#include "cc/third_party/eigen/Eigen/Core"
#include "cc/modules/common/include/utils/simple_timer.h"

static vector<double> expect_abs(const vector<double>& x) {
  vector<double> expect(x.size(), 0.0);
  for (size_t i = 0; i < x.size(); i++)
  {
    expect[i] = std::abs(x[i]);
  }
  
  return expect;
}

static vector<double> expect_abs_prime(const vector<double>& x) {
  vector<double> expect(x.size(), 0.0);
  for (size_t i = 0; i < x.size(); i++)
  {
    expect[i] = x[i] >= 0.0 ? 1 : -1;
  }
  
  return expect;
}

void mpc_log_ops_task_code(shared_ptr<MpcProtocol> mpc_proto, shared_ptr<attr_type> attr) {
  mpc_proto->Init(attr->at("logfile"));
  
  DEFINE_TEST_VARIABLES(mpc_proto->GetMpcContext(), attr);
  
  attr_type &reveal_attr = (*attr);
  /**
   * All basic Binary OP(s), eg.
   * Add/Sub/Mul/Div/Compare(s)
   */
  {
    msg_id_t msgid(task_id + "All abs-ops task unittests");
    // input a b
    vector<double> x = {-10.0, -3.0, 10.0, 3.0, 0.003, -0.02};
    vector<string> input_str, out_str;
    mpc_proto->GetOps(msgid)->PrivateInput(node_id_0, x, input_str);
    attr_type attr;
    
    // case 1: Abs
    mpc_proto->GetOps(msgid)->Abs(input_str, out_str, &attr);
    // reveal c
    vector<double> c;
    mpc_proto->GetOps(msgid)->Reveal(out_str, c, &reveal_attr);
    auto expect_abs_ = expect_abs(x);
    // print_vec(c, c.size(), "Abs result plain:");
    // print_vec(expect_abs_, expect_abs_.size(), "Abs result expect:");
    HD_AROUND_EQUAL_T(c, expect_abs_, "Abs="+node_id + "=T" + task_id);

    // case 2: AbsPrime
    mpc_proto->GetOps(msgid)->AbsPrime(input_str, out_str, &attr);
    // reveal c
    mpc_proto->GetOps(msgid)->Reveal(out_str, c, &reveal_attr);
    auto expect_abs_prime_ = expect_abs_prime(x);
    // print_vec(c, c.size(), "AbsPrime result plain:");
    // print_vec(expect_abs_prime_, expect_abs_prime_.size(), "AbsPrime result expect:");
    HD_AROUND_EQUAL_T(c, expect_abs_prime_, "Abs="+node_id + "=T" + task_id);
  }
}

void run(int partyid) {
  int task_num = TEST_TASK_NUM;
  vector<MpcTaskCodeFunc> task_codes(task_num, mpc_log_ops_task_code);
  string logfile = "log/" + get_file_name(__FILENAME__) + "-" + string(MPC_TEST_PROTOCOL_NAME) + "-" + to_string(partyid) + ".log"; 
  run_task_threads_main(partyid, logfile, task_codes);

  log_info << "-----> " << __FILE__ << " TASK WITH TASK_NUM: " << task_num << " TEST Done !!!!\n\n";
}

RUN_MPC_TEST(run);
