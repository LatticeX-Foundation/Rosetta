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

#include "test.h"

void run(int partyid) {
  PROTOCOL_MPC_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  // clang-format off
  vector<double> X1 = {1, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1};
  vector<double> X2 = {0, 1, 1, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0};
  /////// X = X1>X2 = {1, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1};
  vector<double> Y1 = {1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 0, 0, 1};
  vector<double> Y2 = {0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 1, 1, 0};
  /////// Y = Y1>Y2 = {1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 0, 0, 1};
  //
  ///////////// AND : {1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1}
  ///////////// XOR : {0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0}
  /////////////  OR : {1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1}
  //////////// NOTx : {0, 1, 1, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0}
  // clang-format on

  msg_id_t msgid("Logical ops");
  cout << __FUNCTION__ << " " << msgid << endl;

  vector<string> strX1, strY1, strX2, strY2, strZ1, strZ2;
  mpc_proto->GetOps(msgid)->PrivateInput(0, X1, strX1);
  mpc_proto->GetOps(msgid)->PrivateInput(1, X2, strX2);
  mpc_proto->GetOps(msgid)->PrivateInput(2, Y1, strY1);
  mpc_proto->GetOps(msgid)->PrivateInput(0, Y2, strY2);
  mpc_proto->GetOps(msgid)->Greater(strX1, strX2, strZ1); // return X1 > X2
  mpc_proto->GetOps(msgid)->Greater(strY1, strY2, strZ2); // return Y1 > Y2

  //
  vector<double> Z;
  vector<string> strZ;
  mpc_proto->GetOps(msgid)->Reveal(strZ1, Z);
  print_vector(Z, "Z1", 10, 0);
  mpc_proto->GetOps(msgid)->Reveal(strZ2, Z);
  print_vector(Z, "Z2", 10, 0);

  {
    // variable vs variable
    mpc_proto->GetOps(msgid)->AND(strZ1, strZ2, strZ);
    mpc_proto->GetOps(msgid)->Reveal(strZ, Z);
    print_vector(Z, "AND Z", 10, 0);

    mpc_proto->GetOps(msgid)->XOR(strZ1, strZ2, strZ);
    mpc_proto->GetOps(msgid)->Reveal(strZ, Z);
    print_vector(Z, "XOR Z", 10, 0);

    mpc_proto->GetOps(msgid)->OR(strZ1, strZ2, strZ);
    mpc_proto->GetOps(msgid)->Reveal(strZ, Z);
    print_vector(Z, " OR Z", 10, 0);

    mpc_proto->GetOps(msgid)->NOT(strZ1, strZ);
    mpc_proto->GetOps(msgid)->Reveal(strZ, Z);
    print_vector(Z, "NOT Z1", 10, 0);
  }

  {
    // constant vs variable
    vector<string> strZ1 = {"1", "0", "0", "0", "1", "1", "1", "0", "1", "0", "1", "0", "1"};
    attr_type attr;
    attr["lh_is_const"] = "1";
    mpc_proto->GetOps(msgid)->AND(strZ1, strZ2, strZ, &attr);
    mpc_proto->GetOps(msgid)->Reveal(strZ, Z);
    print_vector(Z, ".AND Z", 10, 0);

    mpc_proto->GetOps(msgid)->XOR(strZ1, strZ2, strZ, &attr);
    mpc_proto->GetOps(msgid)->Reveal(strZ, Z);
    print_vector(Z, ".XOR Z", 10, 0);

    mpc_proto->GetOps(msgid)->OR(strZ1, strZ2, strZ, &attr);
    mpc_proto->GetOps(msgid)->Reveal(strZ, Z);
    print_vector(Z, ". OR Z", 10, 0);
  }
  {
    // variable vs constant
    vector<string> strZ2 = {"1", "0", "1", "1", "0", "1", "0", "0", "1", "0", "0", "0", "1"};
    attr_type attr;
    attr["rh_is_const"] = "1";
    mpc_proto->GetOps(msgid)->AND(strZ1, strZ2, strZ, &attr);
    mpc_proto->GetOps(msgid)->Reveal(strZ, Z);
    print_vector(Z, "AND. Z", 10, 0);

    mpc_proto->GetOps(msgid)->XOR(strZ1, strZ2, strZ, &attr);
    mpc_proto->GetOps(msgid)->Reveal(strZ, Z);
    print_vector(Z, "XOR. Z", 10, 0);

    mpc_proto->GetOps(msgid)->OR(strZ1, strZ2, strZ, &attr);
    mpc_proto->GetOps(msgid)->Reveal(strZ, Z);
    print_vector(Z, " OR. Z", 10, 0);
  }
  ////
  //// unexpected behavier
  {
    cout << "unexpected behavier" << endl;
    // variable vs constant
    vector<string> strZ2 = {"1", "0", "3", "1", "0", "2", "0", "5", "1", "0", "0", "7", "1"};
    attr_type attr;
    attr["rh_is_const"] = "1";
    mpc_proto->GetOps(msgid)->AND(strZ1, strZ2, strZ, &attr);
    mpc_proto->GetOps(msgid)->Reveal(strZ, Z);
    print_vector(Z, "AND. Z", 10, 0);

    mpc_proto->GetOps(msgid)->XOR(strZ1, strZ2, strZ, &attr);
    mpc_proto->GetOps(msgid)->Reveal(strZ, Z);
    print_vector(Z, "XOR. Z", 10, 0);

    mpc_proto->GetOps(msgid)->OR(strZ1, strZ2, strZ, &attr);
    mpc_proto->GetOps(msgid)->Reveal(strZ, Z);
    print_vector(Z, " OR. Z", 10, 0);
  }

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  PROTOCOL_MPC_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);