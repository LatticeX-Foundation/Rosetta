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
  msg_id_t msgid(task_id + "=All Logical OP(s) (share,share)");
  // cout << __FUNCTION__ << " " << msgid << endl;

  vector<double> X1 = {1, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1};
  vector<double> X2 = {0, 1, 1, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0};
  /////// X = X1>X2 = {1, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1};
  vector<double> Y1 = {1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 0, 0, 1};
  vector<double> Y2 = {0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 1, 1, 0};
  /////// Y = Y1>Y2 = {1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 0, 0, 1};
  //
  ///////////// AND : {1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1}
  vector<double> expect_and = {1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1};
  ///////////// XOR : {0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0}
  vector<double> expect_xor = {0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0};
  /////////////  OR : {1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1}
  vector<double> expect_or = {1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1};
  //////////// NOTx : {0, 1, 1, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0}
  vector<double> expect_not = {0, 1, 1, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0};
  // clang-format on

  vector<string> strX1, strY1, strX2, strY2, strZ1, strZ2;
  mpc_proto->GetOps(msgid)->PrivateInput(node_id_0, X1, strX1);
  mpc_proto->GetOps(msgid)->PrivateInput(node_id_1, X2, strX2);
  // mpc_proto->GetOps(msgid)->PrivateInput(node_id_2, Y1, strY1);
  mpc_proto->GetOps(msgid)->PrivateInput(node_id_0, Y1, strY1);
  mpc_proto->GetOps(msgid)->PrivateInput(node_id_0, Y2, strY2);
  mpc_proto->GetOps(msgid)->Greater(strX1, strX2, strZ1); // return X1 > X2
  mpc_proto->GetOps(msgid)->Greater(strY1, strY2, strZ2); // return Y1 > Y2

  //
  vector<double> Z;
  vector<string> strZ;
  mpc_proto->GetOps(msgid)->Reveal(strZ1, Z, &reveal_attr);
  print_vector(Z, "Task: " + mpc_proto->GetMpcContext()->TASK_ID + " X1>X2 = Z1: ", 10, 0);
  mpc_proto->GetOps(msgid)->Reveal(strZ2, Z, &reveal_attr);
  print_vector(Z, "Task: " + mpc_proto->GetMpcContext()->TASK_ID + " Y1>Y2 = Z2: ", 10, 0);

  {
    // variable vs variable
    mpc_proto->GetOps(msgid)->AND(strZ1, strZ2, strZ);
    mpc_proto->GetOps(msgid)->Reveal(strZ, Z, &reveal_attr);
    // print_vector(Z, "Task: " + mpc_proto->GetMpcContext()->TASK_ID + " AND Z", 10, 0);
    // print_vector(expect_and, "Task: " + mpc_proto->GetMpcContext()->TASK_ID + " expect AND Z", 10, 0);
    HD_AROUND_EQUAL_T(Z, expect_and, "AND=" + node_id + "=T" + task_id);

    mpc_proto->GetOps(msgid)->XOR(strZ1, strZ2, strZ);
    mpc_proto->GetOps(msgid)->Reveal(strZ, Z, &reveal_attr);
    // print_vector(Z, "Task: " + mpc_proto->GetMpcContext()->TASK_ID + " XOR Z", 10, 0);
    // print_vector(expect_xor, "Task: " + mpc_proto->GetMpcContext()->TASK_ID + " expect XOR Z", 10, 0);
    HD_AROUND_EQUAL_T(Z, expect_xor, "XOR=" + node_id + "=T"+ task_id);

    mpc_proto->GetOps(msgid)->OR(strZ1, strZ2, strZ);
    mpc_proto->GetOps(msgid)->Reveal(strZ, Z, &reveal_attr);
    // print_vector(Z, " OR Z", 10, 0);
    // print_vector(expect_or, "expect OR Z", 10, 0);
    HD_AROUND_EQUAL_T(Z, expect_or, "OR=" + node_id + "=T"+ task_id);

    mpc_proto->GetOps(msgid)->NOT(strZ1, strZ);
    mpc_proto->GetOps(msgid)->Reveal(strZ, Z, &reveal_attr);
    // print_vector(Z, "NOT Z1", 10, 0);
    // print_vector(expect_not, "expect NOT Z", 10, 0);
    HD_AROUND_EQUAL_T(Z, expect_not, "NOT=" + node_id + "=T" + task_id);
  }

  {
    // constant vs variable
    vector<string> strZ1 = {"1", "0", "0", "0", "1", "1", "1", "0", "1", "0", "1", "0", "1"};
    attr_type attr;
    attr["lh_is_const"] = "1";
    mpc_proto->GetOps(msgid)->AND(strZ1, strZ2, strZ, &attr);
    mpc_proto->GetOps(msgid)->Reveal(strZ, Z, &reveal_attr);
    // print_vector(Z, "Task: " + mpc_proto->GetMpcContext()->TASK_ID + " lh_const AND Z", 10, 0);
    // print_vector(expect_and, "Task: " + mpc_proto->GetMpcContext()->TASK_ID +  " expect AND Z", 10, 0);
    HD_AROUND_EQUAL_T(Z, expect_and, "AND(const, private)=" + node_id + "=T" + task_id);

    mpc_proto->GetOps(msgid)->XOR(strZ1, strZ2, strZ, &attr);
    mpc_proto->GetOps(msgid)->Reveal(strZ, Z, &reveal_attr);
    // print_vector(Z, "Task: " + mpc_proto->GetMpcContext()->TASK_ID + " lh_const XOR Z", 10, 0);
    // print_vector(expect_xor, "Task: " + mpc_proto->GetMpcContext()->TASK_ID + " expect XOR Z", 10, 0);
    HD_AROUND_EQUAL_T(Z, expect_xor, "XOR(const, private)=" + node_id + "=T" + task_id);

    mpc_proto->GetOps(msgid)->OR(strZ1, strZ2, strZ, &attr);
    mpc_proto->GetOps(msgid)->Reveal(strZ, Z, &reveal_attr);
    // print_vector(Z, "Task: " + mpc_proto->GetMpcContext()->TASK_ID + " lh_const OR Z", 10, 0);
    // print_vector(expect_or, "Task: " + mpc_proto->GetMpcContext()->TASK_ID + " expect OR Z", 10, 0);
    HD_AROUND_EQUAL_T(Z, expect_or, "OR(const, private)=" + node_id + "=T" + task_id);
  }
  {
    // variable vs constant
    vector<string> strZ2 = {"1", "0", "1", "1", "0", "1", "0", "0", "1", "0", "0", "0", "1"};
    attr_type attr;
    attr["rh_is_const"] = "1";
    mpc_proto->GetOps(msgid)->AND(strZ1, strZ2, strZ, &attr);
    mpc_proto->GetOps(msgid)->Reveal(strZ, Z, &reveal_attr);
    // print_vector(Z, "Task: " + mpc_proto->GetMpcContext()->TASK_ID + " rh_const AND Z", 10, 0);
    // print_vector(expect_and, "Task: " + mpc_proto->GetMpcContext()->TASK_ID + " expect AND Z", 10, 0);
    HD_AROUND_EQUAL_T(Z, expect_and, "AND(private, const)=" + node_id + "=T" + task_id);

    mpc_proto->GetOps(msgid)->XOR(strZ1, strZ2, strZ, &attr);
    mpc_proto->GetOps(msgid)->Reveal(strZ, Z, &reveal_attr);
    // print_vector(Z, "Task: " + mpc_proto->GetMpcContext()->TASK_ID + " rh_const XOR Z", 10, 0);
    // print_vector(expect_or, "Task: " + mpc_proto->GetMpcContext()->TASK_ID + " expect OR Z", 10, 0);
    HD_AROUND_EQUAL_T(Z, expect_xor, "XOR(private, const)=" + node_id + "=T" + task_id);

    mpc_proto->GetOps(msgid)->OR(strZ1, strZ2, strZ, &attr);
    mpc_proto->GetOps(msgid)->Reveal(strZ, Z, &reveal_attr);
    // print_vector(Z, "Task: " + mpc_proto->GetMpcContext()->TASK_ID + " rh_const OR Z", 10, 0);
    // print_vector(expect_or, "Task: " + mpc_proto->GetMpcContext()->TASK_ID + " expect OR Z", 10, 0);
    HD_AROUND_EQUAL_T(Z, expect_or, "OR(private, const)=" + node_id + "=T" + task_id);
  }
  ////
  //// unexpected behavier
  {
    cout << "Task: " + mpc_proto->GetMpcContext()->TASK_ID + "unexpected behavier" << endl;
    // variable vs constant
    vector<string> strZ2 = {"1", "0", "3", "1", "0", "2", "0", "5", "1", "0", "0", "7", "1"};
    attr_type attr;
    attr["rh_is_const"] = "1";
    mpc_proto->GetOps(msgid)->AND(strZ1, strZ2, strZ, &attr);
    mpc_proto->GetOps(msgid)->Reveal(strZ, Z, &reveal_attr);
    print_vector(Z, "Task: " + mpc_proto->GetMpcContext()->TASK_ID + " AND. Z", 10, 0);

    mpc_proto->GetOps(msgid)->XOR(strZ1, strZ2, strZ, &attr);
    mpc_proto->GetOps(msgid)->Reveal(strZ, Z, &reveal_attr);
    print_vector(Z, "Task: " + mpc_proto->GetMpcContext()->TASK_ID + " XOR. Z", 10, 0);

    mpc_proto->GetOps(msgid)->OR(strZ1, strZ2, strZ, &attr);
    mpc_proto->GetOps(msgid)->Reveal(strZ, Z, &reveal_attr);
    print_vector(Z, "Task: " + mpc_proto->GetMpcContext()->TASK_ID + " OR. Z", 10, 0);
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
