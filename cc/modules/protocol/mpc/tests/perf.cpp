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

static void run(int partyid) {
  PROTOCOL_MPC_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  msg_id_t msgid(protocol_name + " performance test");
  cout << __FUNCTION__ << " " << msgid << endl;
  Logger::Get().log_to_stdout(true);

  stringstream ssheader;
  // clang-format off
      ssheader << "Notes: (partyid:" << partyid << ")" << endl;
      ssheader << "    OP: operator name. eg. 0.1.OpName: 0 means variable, 1 means constant." << endl;
      ssheader << "    loops: how many loops to execute." << endl;
      ssheader << "    elapsed(s): the total time spent executing the OP (loops)." << endl;
      ssheader << "    avg-elapse(ms): (elapsed(s)*1000.0)/loops." << endl;
      ssheader << "    shape size: k = r * c = m * K = K * n." << endl;
      ssheader << "    sent data/recv data: IO total bytes. (per operator with size items; including message id)" << endl;
      ssheader << "    sent msgs/recv msgs: IO interface invocation times. (per operator)" << endl;
      ssheader << "+-------------------------+------------+---------------+---------------+----------------------+------------+------------+------------+------------+" << endl;
      ssheader << "|" << setw(25) << "OP "         << "|" << setw(12) << "loops "
               << "|" << setw(15) << "elapsed(s) " << "|" << setw(15) << "avg-elapse(ms) " << "|" << setw(22) << "shape size "
               << "|" << setw(12) << "sent data "  << "|" << setw(12) << "recv data "
               << "|" << setw(12) << "sent msgs "  << "|" << setw(12) << "recv msgs "
               << "|" << endl;
      ssheader << "+-------------------------+------------+---------------+---------------+----------------------+------------+------------+------------+------------+";
  // clang-format on
  stringstream ssender;
  // clang-format off
      ssender  << "+-------------------------+------------+---------------+---------------+----------------------+------------+------------+------------+------------+";
  // clang-format on

  // statitics beg ============
  auto ps_cmp = [](const rosetta::PerfStats& l, const rosetta::PerfStats& r) -> bool {
    return l.s.elapse < r.s.elapse;
  };
  vector<rosetta::PerfStats> vecPerfStats;
  auto copePerf = [&](rosetta::PerfStats& ps) {
    //
    vecPerfStats.push_back(ps);
  };
  auto endPerf = [&]() {
    // print_vec(vecx);
    std::sort(vecPerfStats.begin(), vecPerfStats.end(), ps_cmp);
    // for (auto& ps : vecPerfStats) {
    //   cout.precision(11);
    //   cout << "op:" << ps.name << " " << ps.s.elapse << endl;
    // }
    // cout << vecPerfStats.back().to_json(true) << endl;
    std::string jsonstr = rosetta::PerfStats::to_json(vecPerfStats, true);
    //cout << jsonstr << endl;
    {
      ofstream ofile;
      ofile.open(
        protocol_name + "-party-" + to_string(partyid) + "-perf-ops-sorted-by-elapse.json");
      if (ofile.good()) {
        ofile << jsonstr << endl;
        ofile.close();
      }
    }
  };
  // statitics end ============

#define _perf_test_beg() do {
#define _perf_test_for(op, loops)                                \
  auto nloops = loops;                                           \
  /*nloops = nloops / 10 + 1;*/                                  \
  /*nloops = nloops*1.5;*/                                       \
  std::string opname(#op);                                       \
  stringstream ss;                                               \
  ss.precision(11);                                              \
  ss << "|" << setw(24) << opname << " |" << setw(11) << nloops; \
  auto ps0 = mpc_proto->GetPerfStats();                          \
  SimpleTimer timer;                                             \
  for (int i = 0; i < nloops; i++) {
#define _perf_test_end(shape)                                                                      \
  }                                                                                                \
  timer.stop();                                                                                    \
  auto ps1 = mpc_proto->GetPerfStats();                                                            \
  auto ps = ps1 - ps0;                                                                             \
  ss << " |" << setw(14) << timer.elapse() << " |" << setw(14) << (timer.elapse() * 1000) / nloops \
     << " |" << setw(21) << shape;                                                                 \
  ss << " |" << setw(11) << ps.s.bytes_sent / nloops << " |" << setw(11)                           \
     << ps.s.bytes_recv / nloops << " |" << setw(11) << ps.s.msg_sent / nloops << " |" << setw(11) \
     << ps.s.msg_recv / nloops;                                                                    \
  ss << " |";                                                                                      \
  cout << ss.str() << endl;                                                                        \
  ps = ps / nloops;                                                                                \
  ps.name = opname;                                                                                \
  ps.s.elapse = (timer.elapse() * 1000) / nloops;                                                  \
  copePerf(ps);                                                                                    \
  }                                                                                                \
  while (0)

#define binary_perf_test(lh, rh, sX, sY, op, loops)                                  \
  _perf_test_beg() attr_type attr;                                                   \
  attr["lh_is_const"] = to_string(lh);                                               \
  attr["rh_is_const"] = to_string(rh);                                               \
  _perf_test_for(lh.rh.op, loops) mpc_proto->GetOps(msgid)->op(sX, sY, strZ, &attr); \
  _perf_test_end("k=" + to_string(sX.size()) + ",k=" + to_string(sY.size()))

#define unary_perf_test(op, loops)                                           \
  _perf_test_beg() attr_type attr;                                           \
  _perf_test_for(op, loops) mpc_proto->GetOps(msgid)->op(strX, strZ, &attr); \
  _perf_test_end("k=" + to_string(strX.size()))

#define reduce_perf_test(op, loops, r, c)                                    \
  _perf_test_beg() attr_type attr;                                           \
  attr["rows"] = to_string(r);                                               \
  attr["cols"] = to_string(c);                                               \
  _perf_test_for(op, loops) mpc_proto->GetOps(msgid)->op(strX, strZ, &attr); \
  _perf_test_end("r=" + to_string(r) + ",c=" + to_string(c))

#define matmul_perf_test(op, loops, m, K, n)                                       \
  _perf_test_beg() attr_type attr;                                                 \
  attr["m"] = to_string(m);                                                        \
  attr["k"] = to_string(K);                                                        \
  attr["n"] = to_string(n);                                                        \
  _perf_test_for(op, loops) mpc_proto->GetOps(msgid)->op(strX, strY, strZ, &attr); \
  _perf_test_end("m=" + to_string(m) + ",K=" + to_string(K) + ",n=" + to_string(n))

  //////////////////////////////////////////////////////////////////
  vector<double> X, Y, Z; // variables
  vector<double> CX, CY; // constants
  vector<string> strX, strY, strZ;
  vector<string> strCX, strCY, positiveCY;
  cout << ssheader.str() << endl;
  /**
   * for convenience,
   * k = r * c = m * K = K * n
   * so, only need set m and K.
   */
  int k, r, c, m, K, n = 1;

  // case 1
  m = K = 1;
  // case 2
  m = 33;
  K = 44;
  // .....
  r = n = m;
  c = K;
  k = r * c;

  // initialize
  random_vector(X, k, -3, 3);
  random_vector(Y, k, -3, 3);
  {
    CX.resize(k);
    CY.resize(k);
    strCX.resize(k);
    strCY.resize(k);
    positiveCY.resize(k);
    srand(1);
    for (int i = 0; i < k; i++) {
      CX[i] = (rand() % 300) / 100.0 + 3.0;
      strCX[i] = std::to_string(CX[i]);
    }
    srand(11);
    for (int i = 0; i < k; i++) {
      CY[i] = (rand() & 600) / 200.0 + 3.0;
      strCY[i] = std::to_string(CY[i]);
      positiveCY[i] = std::to_string(abs(CY[i]));
    }
  }
  mpc_proto->GetOps(msgid)->PrivateInput(net_io->GetNodeId(0), X, strX);
  mpc_proto->GetOps(msgid)->PrivateInput(net_io->GetNodeId(1), Y, strY);

  SimpleTimer all_tests_timer;

  // variable vs variable
  binary_perf_test(0, 0, strX, strY, Add, 64);
  binary_perf_test(0, 0, strX, strY, Sub, 128);
  binary_perf_test(0, 0, strX, strY, Mul, 256);
  binary_perf_test(0, 0, strX, strY, Truediv, 5);
  binary_perf_test(0, 0, strX, strY, Floordiv, 5); ////////////////////
  binary_perf_test(0, 0, strX, strY, Less, 32);
  binary_perf_test(0, 0, strX, strY, LessEqual, 32);
  binary_perf_test(0, 0, strX, strY, Equal, 71);
  binary_perf_test(0, 0, strX, strY, NotEqual, 73);
  binary_perf_test(0, 0, strX, strY, Greater, 32);
  binary_perf_test(0, 0, strX, strY, GreaterEqual, 32); ////////////////////
  binary_perf_test(0, 0, strX, strY, SigmoidCrossEntropy, 63);
  // constant vs variable
  binary_perf_test(1, 0, strCX, strY, Add, 64);
  binary_perf_test(1, 0, strCX, strY, Sub, 128);
  binary_perf_test(1, 0, strCX, strY, Mul, 256);
  binary_perf_test(1, 0, strCX, strY, Truediv, 5);
  binary_perf_test(1, 0, strCX, strY, Floordiv, 5); ////////////////////
  binary_perf_test(1, 0, strCX, strY, Less, 32);
  binary_perf_test(1, 0, strCX, strY, LessEqual, 32);
  binary_perf_test(1, 0, strCX, strY, Equal, 71);
  binary_perf_test(1, 0, strCX, strY, NotEqual, 73);
  binary_perf_test(1, 0, strCX, strY, Greater, 32);
  binary_perf_test(1, 0, strCX, strY, GreaterEqual, 32); ////////////////////
  binary_perf_test(1, 0, strCX, strY, SigmoidCrossEntropy, 63);
  // variable vs constant
  binary_perf_test(0, 1, strX, strCY, Add, 64);
  binary_perf_test(0, 1, strX, strCY, Sub, 128);
  binary_perf_test(0, 1, strX, strCY, Mul, 256);
  binary_perf_test(0, 1, strX, strCY, Truediv, 5);
  binary_perf_test(0, 1, strX, strCY, Floordiv, 5); ////////////////////
  binary_perf_test(0, 1, strX, strCY, Less, 32);
  binary_perf_test(0, 1, strX, strCY, LessEqual, 32);
  binary_perf_test(0, 1, strX, strCY, Equal, 71);
  binary_perf_test(0, 1, strX, strCY, NotEqual, 73);
  binary_perf_test(0, 1, strX, strCY, Greater, 32);
  binary_perf_test(0, 1, strX, strCY, GreaterEqual, 32); ////////////////////
  binary_perf_test(0, 1, strX, strCY, SigmoidCrossEntropy, 63);
  binary_perf_test(0, 1, strX, positiveCY, Pow, 11);

  unary_perf_test(Square, 64);
  unary_perf_test(Negative, 128);
  unary_perf_test(Abs, 121);
  unary_perf_test(AbsPrime, 135);
  unary_perf_test(Log, 76);
  unary_perf_test(Log1p, 57);
  unary_perf_test(HLog, 5);
  unary_perf_test(Relu, 257);
  unary_perf_test(ReluPrime, 211);
  unary_perf_test(Sigmoid, 53);

  reduce_perf_test(Mean, 128, r, c);
  reduce_perf_test(Sum, 128, r, c);
  reduce_perf_test(AddN, 128, r, c);
  reduce_perf_test(Max, 61, r, c);
  reduce_perf_test(Min, 63, r, c);

  matmul_perf_test(Matmul, 64, m, K, n);
  cout << ssender.str() << endl;
  cout << "Total elapsed(s): " << all_tests_timer.elapse() << endl;
  endPerf();
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  PROTOCOL_MPC_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);