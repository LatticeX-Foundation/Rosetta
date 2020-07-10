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

/**
 * @todo
 * This file and its implement will be migrate to the `protocol/` directory.
 * 
 */
#include <unistd.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
using namespace rapidjson;

#include "cc/modules/common/include/utils/logger.h"

namespace rosetta {

class PSIConfig {
 public:
  std::string to_string();

 public:
  string HOST = "127.0.0.1"; // server IP
  int PORT = 12345; // the communication port
  int RECV_PARTY = 2; // which party receive the result. 0-ALICE; 1-BOB; 2-BOTH
};

class MPCConfig {
#define MPC_PARTIES 3
 public:
  MPCConfig();
  std::string to_string();

 public:
  int FLOAT_PRECISION_M = 13;
  struct Node {
    string NAME;
    string HOST;
    int PORT;
  };
  Node P[MPC_PARTIES];

  // By default, the local ciphertext values are saved in model.
  // Currently, only support it as 3-bit bitmap:[P2 P1 P0]
  // 0: none, all local ciphertext
  // 1: P0
  // 2: P1
  // 4: P2
  // 3: P0 and P1
  // 5: P0 and P2
  // 6: P1 and P2
  // 7: P0, P1 and P2
  int SAVER_MODE = 0;

  // server and client certifications
  string SERVER_CERT = "certs/server-nopass.cert";
  string SERVER_PRIKEY = "certs/server-prikey";
  string SERVER_PRIKEY_PASSWORD = "123456";
};

class RosettaConfig {
 public:
  RosettaConfig() {}
  RosettaConfig(int argc, char* argv[]);
  RosettaConfig(int party, const string& config_json);
  RosettaConfig(const string& config_json); // use PARTY_ID in config_json
  RosettaConfig(const PSIConfig& psiconfig) : psi(psiconfig) {}
  RosettaConfig(const MPCConfig& mpcconfig) : mpc(mpcconfig) {}

 private:
  bool load(int party, const string& config_file);
  bool parse(Document& doc);

 public:
  PSIConfig& getPsiConfig() { return psi; }
  MPCConfig& getMpcConfig() { return mpc; }
  void fmt_print();

 public:
  int PARTY = -1; // NOT USE AT PRESENT
  PSIConfig psi;
  MPCConfig mpc;
};

} // namespace rosetta
