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
 */
#include <string>
#include <vector>
using namespace std;

#include <rapidjson/document.h>
using rapidjson::Document;

namespace rosetta {

class _Config {
#define TWO_PARTIES 2
#define THREE_PARTIES 3
 public:
  struct Node {
    string DESC;
    string NAME;
    string HOST;
    int PORT;
  };
};

class _2PCConfig : public _Config {
 public:
  Node P[TWO_PARTIES];
};

class _3PCConfig : public _Config {
 public:
  Node P[THREE_PARTIES];
};

class PSIConfig : public _2PCConfig {
  /**
   * Examples:
   * "PSI": {
   *   "P0": {
   *     "DESC": "P0/ALICE, as Server-side.",
   *     "NAME": "PartyA(P0)",
   *     "HOST": "127.0.0.1",
   *     "PORT": 15225 // listen port on P0
   *   },
   *   "P1": {
   *     "DESC": "P1/BOB, as Client-side.",
   *     "NAME": "PartyB(P1)",
   *     "HOST": "127.0.0.1", // P0's HOST
   *     "PORT": 15225 // P0's listen PORT
   *   },
   *   "RECV_PARTY": 2
   * },
   */
 public:
  std::string to_string();

 public:
  int RECV_PARTY = 2; // which party receive the result. 0-ALICE; 1-BOB; 2-BOTH
};

class ZKConfig : public _2PCConfig {
  /**
   * "ZK": {
   *   "P0": {
   *     "DESC": "P0/ALICE/Prover, internal PARTY_ID = 1, as Server-side.",
   *     "NAME": "PartyA(P0)",
   *     "HOST": "127.0.0.1",
   *     "PORT": 16256 // listen port on P0
   *   },
   *   "P1": {
   *     "DESC": "P1/BOB/Verifier, internal PARTY_ID = 2, as Client-side.",
   *     "NAME": "PartyB(P1)",
   *     "HOST": "127.0.0.1", // P0's HOST
   *     "PORT": 16256 // P0's listen PORT
   *   },
   *   "RESTORE_MODE": 1
   * },
   */
 public:
  std::string to_string();

 public:
  /**
   * general descriptions @see MPCConfig, but the restrictions in ZK are:
   *  b01, plain model, only P0(b01) owns the plain model, load as private;
   *  b11(-1), plain model, all parties have the plain model, load as public-constant;
   *  others, not supported
   * you can set RESTORE_MODE to 1 or -1.
   */
  int RESTORE_MODE = 0;
};

class MPCConfig : public _3PCConfig {
 public:
  MPCConfig();
  std::string to_string();

 public:
  int FLOAT_PRECISION_M = 13;
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

  /**
   * for general 3PC
   *  b000(0), cipher model, each party has the secret sharing value of the model.
   *  b001/b010/b100, plain model, only P0(b001) or P1(b010) or P2(b100) owns the plain model, load as private;
   *  b111(-1), plain model, all parties have the plain model, load as public-constant;
   *  others, not supported
   * you can set RESTORE_MODE to 0~7 or -1.
   */
  int RESTORE_MODE = 0;

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
  bool parse_psi(Document& doc);
  bool parse_zk(Document& doc);
  bool parse_mpc(Document& doc);

 public:
  PSIConfig& getPsiConfig() { return psi; }
  ZKConfig& getZkConfig() { return zk; }
  MPCConfig& getMpcConfig() { return mpc; }
  void fmt_print();

 public:
  int PARTY = -1; // NOT USE AT PRESENT
  PSIConfig psi;
  ZKConfig zk;
  MPCConfig mpc;
};

} // namespace rosetta
