#include "cc/modules/protocol/mpc/tests/tasks/mpc__test_tasks.h"
#include <thread>
#include "cc/third_party/eigen/Eigen/Core"
#include "cc/modules/common/include/utils/simple_timer.h"

using DoubleMatrix = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

const static int kThreadSize = 4;
const static int M = 2;
const static int K = 2;
const static int N = 2;

template<class T>
void print_matrix(const vector<T>& vec, int r, int c, const string& name="no-name") {
  cout << "matrix(" << r <<"," << c <<") " << name << ":" << endl;
  for (int i = 0; i < r; ++i)
  {
    for (int j = 0; j < c; ++j)
    {
      cout << vec[i*c+j] << "\t";
    }
    cout << endl;
  }
  cout << endl;
}


void mpc_matmul_task_code(shared_ptr<MpcProtocol> mpc_proto, shared_ptr<attr_type> attr) {
  DEFINE_TEST_VARIABLES(mpc_proto->GetMpcContext(), attr);

  mpc_proto->Init(attr->at("logfile"));//logfile);
  shared_ptr<NET_IO> net_io = mpc_proto->GetNetHandler();

  vector<double> X, Y;
  DoubleMatrix DX(M, K), DY(K, N), DZ(M,N);

  random_vector(X, M *  K, -1.0, 1.0, 1689);
  random_vector(Y,  K * N, -1.0, 1.0, 1699);
  memcpy((void*)DX.data(), (void*)X.data(), X.size()*sizeof(double));
  memcpy((void*)DY.data(), (void*)Y.data(), Y.size()*sizeof(double));

  DZ = DX * DY;

  msg_id_t msgid(task_id + ": Matmul_thread (share,share)-");

  vector<string> strX(X.size()), strY(Y.size()), strZ;
  mpc_proto->GetOps(msgid)->PrivateInput(mpc_proto->GetNetHandler()->GetNodeId(0), X, strX);
  mpc_proto->GetOps(msgid)->PrivateInput(mpc_proto->GetNetHandler()->GetNodeId(1), Y, strY); // NOTE: from 1 to 0

  SimpleTimer timer;
  // attr_type attr;
  (*attr)["m"] = std::to_string(M);
  (*attr)["k"] = std::to_string(K);
  (*attr)["n"] = std::to_string(N);
  mpc_proto->GetOps(msgid)->Matmul(strX, strY, strZ, attr.get());

  vector<double> Z;
  mpc_proto->GetOps(msgid)->Reveal(strZ, Z, attr.get());

  // print_matrix(Z, M, N, node_id + ", " + task_id + " --- reveal Z: ");
  // cout << node_id << " => task: " << task_id << "--- " << "expect Z: \n"  << DZ << endl;
  // cout << node_id << " => task: " << task_id << " cost time:" << timer.elapse() << endl;

  vector<double> expect(DZ.data(), DZ.data() + DZ.size());
  HD_AROUND_EQUAL_T(Z, expect, "Matmul="+ node_id + "=T" + task_id);
}


void run(int partyid) {
  int task_num = TEST_TASK_NUM;
  vector<MpcTaskCodeFunc> task_codes(task_num, mpc_matmul_task_code);
  string logfile = "log/" + get_file_name(__FILENAME__) + "-" + string(MPC_TEST_PROTOCOL_NAME) + to_string(partyid) + ".log";  
  run_task_threads_main(partyid, logfile, task_codes);

  log_info << "-----> " << __FILE__ << " TASK WITH TASK_NUM: " << task_num << " TEST Done !!!!\n\n";
}

RUN_MPC_TEST(run);
