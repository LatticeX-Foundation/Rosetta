#include "snn__test.h"
#include "cc/third_party/eigen/Eigen/Core"

using DoubleMatrix = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

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

void run(int partyid) {
  SNN_PROTOCOL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  
  const static int M = 2;
  const static int K = 2;
  const static int N = 2;

  vector<double> X, Y, Z;
  DoubleMatrix DX(M, K), DY(K, N), DZ(M,N);

  random_vector(X, M *  K, -1.0, 1.0, 1689);
  random_vector(Y,  K * N, -1.0, 1.0, 1699);
  memcpy((void*)DX.data(), (void*)X.data(), X.size()*sizeof(double));
  memcpy((void*)DY.data(), (void*)Y.data(), Y.size()*sizeof(double));

  DZ = DX * DY;
  cout << "DX: " << DX << endl;
  cout << "DY: " << DY << endl;
  cout << "DZ: " << DZ << endl;

  msg_id_t msgid("Matmul (share,share)");
  cout << __FUNCTION__ << " " << msgid << endl;

  vector<string> strX(X.size()), strY(X.size()), strZ(X.size());
  snn0.GetOps(msgid)->PrivateInput(net_io->GetNodeId(0),  X, strX);
  snn0.GetOps(msgid)->PrivateInput(net_io->GetNodeId(1), Y, strY);
  log_info << __FUNCTION__ << " " << msgid ;

  attr_type attr;
  attr["m"] = std::to_string(M);
  attr["k"] = std::to_string(K);
  attr["n"] = std::to_string(N);

  SimpleTimer timer;
  snn0.GetOps(msgid)->Matmul(strX, strY, strZ, &attr);
  attr.clear();
  vector<string> receivers = {"P0", "P1"};
  attr["receive_parties"] = receiver_parties_pack(receivers);
  
  snn0.GetOps(msgid)->Reveal(strZ, Z, &attr);

  print_matrix(X, M, K, "reveal X: ");
  print_matrix(Y, K, N, "reveal Y: ");
  print_matrix(Z, M, N, "reveal Z: ");
  
  cout << "expect Z: \n"  << DZ << endl;
  cout << ">>>>>>>>>>>>>>>>>>> timer:" << timer.elapse() << endl;
  
  SNN_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);
