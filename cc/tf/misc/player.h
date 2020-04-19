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
#pragma once

#include <cstdio>
#include <string>
#include <vector>
#include <iostream>
#include <mutex>
#include <fstream>

#include "mpc.h"

class Player {
 public:
  bool inited = false;
  int origin_stdout = -1;
  std::mutex mtx;
  std::streambuf* cout_buf = nullptr;
  std::ofstream of;
  shared_ptr<rosetta::mpc::RandomSeed> randseed = nullptr;
  std::string seed_msg_id = "cc/Player This msg id for global RandomSeed.";
  shared_ptr<rosetta::mpc::PrivateInput> pri_input = nullptr;
  std::string pri_input_msg_id = "cc/Player This msg id for global PrivateInput.";

 public:
  int id = -1;

 public:
  ~Player() {
    uninitialize_mpc();
    if (origin_stdout != -1)
      restore_stdout();
  }

  // a temporary for init mpc comm, keys, ..
  void init_mpc(int party, const std::string& cfgfile) {
    cout << "init_mpc, party: " << party << ", config file: " << cfgfile << endl;
    if (!inited) {
      std::unique_lock<std::mutex> lck(mtx);
      if (!inited) {
        SecureParams params(party, cfgfile);
        initialize_mpc(params);

        // other inits
        randseed = GetMpcOpWithKey(RandomSeed, seed_msg_id);
        pri_input = GetMpcOpWithKey(PrivateInput, pri_input_msg_id);
        id = partyid();

        inited = true;
      }
    }
  }

  mpc_t rand_seed(mpc_t op_seed) {
    if (!inited) {
      cerr << "have not inited!" << endl;
      throw;
    }

    mpc_t seed;
    randseed->Run(seed);
    return seed;
  }

  double private_input(int party_id, double d) {
    if (!inited) {
      cerr << "have not inited!" << endl;
      throw;
    }

    //double ss = 0;
    //pri_input->Run(party_id, d, ss);
    mpc_t t = 0;
    pri_input->Run(party_id, d, t);
    double ss = MpcTypeToFloatBC(t);
    return ss;
  }

  // redirect stdout to external specified log file
  void redirect_stdout(const std::string& logfile) {
    cout_buf = cout.rdbuf();
    of.open(logfile);
    streambuf* fileBuf = of.rdbuf();
    cout.rdbuf(fileBuf);
    origin_stdout = 0;
  }

  void restore_stdout() {
    of.flush();
    of.close();
    cout.rdbuf(cout_buf);
  }
};
