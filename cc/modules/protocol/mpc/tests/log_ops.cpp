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

void static run(int partyid) {
  PROTOCOL_MPC_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  msg_id_t msgid("mpc-log-ops-unittests");
  vector<double> hd_a = {1,    2,     3,     4.0, 1.5,
                         0.01, 0.001, 0.003, 100, 2.71828}; //between [0.0001, 10)
  vector<double> a = {1, 2, 3, 4.0, 1.5, 0.01, 0.001, 0.003, 8, 2.71828};
  vector<double> LOG_HD_A = {0.0, 0.693, 1.099, 1.386, 0.405, -4.605, -6.908, -5.809, 4.605, 1.0};
  vector<double> LOG_A = {0.0, 0.693, 1.099, 1.386, 0.405, -4.605, -6.908, -5.809, 2.197, 1.0};
  vector<double> LOG1P_A = {0.693, 1.099, 1.386, 1.609, 0.916, 0.01, 0.001, 0.00299, 2.303, 1.313};

  vector<string> input_str, hd_input_str, out_str;
  mpc_proto->GetOps(msgid)->PrivateInput(node_id_0, hd_a, hd_input_str);
  mpc_proto->GetOps(msgid)->PrivateInput(node_id_0, a, input_str);
  // print_vec(a, 10, "Task: " + mpc_proto->GetMpcContext()->TASK_ID + " == input plain");
  attr_type attr;
  vector<double> c;

  // case 0: const string
  vector<string> CONS_STR = {"1.1", "4.0"};
  vector<double> LOG_CONST = {0.095, 1.386};
  mpc_proto->GetOps(msgid)->Log(CONS_STR, out_str, &attr);
  mpc_proto->GetOps(msgid)->Reveal(out_str, c, &reveal_attr);
  // print_vec(c, 10, "Task: " + mpc_proto->GetMpcContext()->TASK_ID +  " == Log CONST result plain:");
  // print_vec(LOG_CONST, 10, "Task: " + mpc_proto->GetMpcContext()->TASK_ID + " LOG const expected:");
  AROUND_EQUAL_T(c, LOG_CONST, 0.5, "Log(const)=" + node_id + "=T" + task_id);

  // case 1: Log
  mpc_proto->GetOps(msgid)->Log(input_str, out_str, &attr);
  mpc_proto->GetOps(msgid)->Reveal(out_str, c, &reveal_attr);
  // print_vec(c, 10, "Task: " + mpc_proto->GetMpcContext()->TASK_ID + " == Log result plain:");
  // print_vec(LOG_A, 10, "Task: " + mpc_proto->GetMpcContext()->TASK_ID + " == Log expected:");
  AROUND_EQUAL_T(c, LOG_A, 1.2, "Log=" + node_id + "=T" + task_id);

  // case 2: HLog
  mpc_proto->GetOps(msgid)->HLog(hd_input_str, out_str, &attr);
  mpc_proto->GetOps(msgid)->Reveal(out_str, c, &reveal_attr);
  // print_vec(c, 10, "Task: " + mpc_proto->GetMpcContext()->TASK_ID + " == HLog result plain:");
  // print_vec(LOG_A, 10, "Task: " + mpc_proto->GetMpcContext()->TASK_ID + " == Log expected:");
  AROUND_EQUAL_T(c, LOG_HD_A, 0.1, "HLog=" + node_id + "=T" + task_id);

  // case 3: Log1p
  mpc_proto->GetOps(msgid)->Log1p(input_str, out_str, &attr);
  mpc_proto->GetOps(msgid)->Reveal(out_str, c, &reveal_attr);
  // print_vec(c, 10, "Task: " + mpc_proto->GetMpcContext()->TASK_ID + " == Log1p result plain:");
  // print_vec(LOG1P_A, 10, "Task: " + mpc_proto->GetMpcContext()->TASK_ID + " == Log1p expected:");
  AROUND_EQUAL_T(c, LOG1P_A, 0.5, "Log1p=" + node_id + "=T" + task_id);

  // case 4: SigmoidCrossEntropy
  // input a b
  vector<double> logits = {-10.0, -3.0, 10.0, 3.0, 5.0, 3.1, 0.003, -0.02};
  vector<double> labels = {0.0, 0.0, 1.0, 1.0, -1.0, -1.0, -1.0, 1.0};
  vector<double> SCE = {0.0, 0.04, 0.0001, 0.049, 10.007, 6.244, 0.698, 0.703};
  vector<string> input_str1, input_str2;
  mpc_proto->GetOps(msgid)->PrivateInput(node_id_0, logits, input_str1);
  mpc_proto->GetOps(msgid)->PrivateInput(node_id_0, labels, input_str2);
  // if (node_id == node_id_0) print_vec(logits, 10, "logits:");
  // if (node_id == node_id_0) print_vec(labels, 10, "labels:");
  // attr_type attr;

  mpc_proto->GetOps(msgid)->SigmoidCrossEntropy(input_str1, input_str2, out_str, &attr);

  // reveal c
  mpc_proto->GetOps(msgid)->Reveal(out_str, c, &reveal_attr);
  // if (node_id == node_id_0) print_vec(c, 10, "SigmoidCrossEntropy result plain:");
  // if (node_id == node_id_0) print_vec(SCE, 10, "SCE expected:");
  AROUND_EQUAL_T(c, SCE, 0.1, "SigmoidCrossEntropy=" + node_id + "=T" + task_id);

  PROTOCOL_MPC_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);