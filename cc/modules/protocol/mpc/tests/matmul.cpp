// ==============================================================================
// Copyright 2020 The LatticeX Foundation
// This file is part of the Rosetta library.
//
// The Rosetta library is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// The Rosetta library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with the Rosetta library. If not, see <http://www.gnu.org/licenses/>.
// ==============================================================================
// only for disable vscode warnings
#ifndef PROTOCOL_MPC_TEST
#define PROTOCOL_MPC_TEST_SNN 1
#endif

#include "cc/modules/protocol/mpc/tests/test.h"
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

static void run(int partyid) {
  PROTOCOL_MPC_TEST_INIT(partyid);
  {
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
    // cout << "DX: " << DX << endl;
    // cout << "DY: " << DY << endl;
    // cout << "DZ: " << DZ << endl;

    msg_id_t msgid("Matmul (share,share)");
    cout << __FUNCTION__ << " " << msgid << endl;

    vector<string> strX(X.size()), strY(X.size()), strZ(X.size());
    mpc_proto->GetOps(msgid)->PrivateInput(net_io->GetNodeId(0),  X, strX);
    // mpc_proto->GetOps(msgid)->PrivateInput(net_io->GetNodeId(1), Y, strY);
    mpc_proto->GetOps(msgid)->PrivateInput(net_io->GetNodeId(2), Y, strY);
    log_info << __FUNCTION__ << " " << msgid.str();

    attr_type attr;
    attr["m"] = std::to_string(M);
    attr["k"] = std::to_string(K);
    attr["n"] = std::to_string(N);

    SimpleTimer timer;
    mpc_proto->GetOps(msgid)->Matmul(strX, strY, strZ, &attr);
    attr.clear();
    vector<string> receivers = {"P0", "P1", "P2"};
    attr["receive_parties"] = receiver_parties_pack(receivers);
    
    mpc_proto->GetOps(msgid)->Reveal(strZ, Z, &attr);

    // print_matrix(X, M, K, "reveal X: ");
    // print_matrix(Y, K, N, "reveal Y: ");
    // print_matrix(Z, M, N, "reveal Z: ");

    vector<double> expect(DZ.data(), DZ.data() + DZ.size());
    HD_AROUND_EQUAL_T(Z, expect, "Matmul=P"+std::to_string(partyid));
    cout << ">>>>>>>>>>>>>>>>>>> Matmul test cost:" << timer.elapse()  << "seconds" << endl;
  }
  PROTOCOL_MPC_TEST_UNINIT(partyid);

  printf("\n----------------------  end Matmul test, party: %d --------------\n", partyid);
}

RUN_MPC_TEST(run);
