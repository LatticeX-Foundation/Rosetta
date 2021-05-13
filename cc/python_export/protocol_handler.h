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
using namespace std;

#include "cc/modules/protocol/public/protocol_manager.h"

class ProtocolHandler {
 public:
  int origin_stdout = -1;
  std::mutex mtx;
  std::streambuf* cout_buf = nullptr;
  std::ofstream of;
  std::string seed_msg_id = "cc/Player This msg id for global RandomSeed.";
  std::string pri_input_msg_id = "cc/Player This msg id for global PrivateInput.";

 public:
  ProtocolHandler() {
    std::cout.rdbuf()->pubsetbuf(nullptr, 0);
    setvbuf(stdout, NULL, _IOLBF, 0);
  }

  ~ProtocolHandler() {
    rosetta::ProtocolManager::Instance()->DeactivateProtocol();

    if (origin_stdout != -1)
      restore_stdout();
  }

  bool is_activated() const { return rosetta::ProtocolManager::Instance()->IsActivated(); }

  int get_party_id() const {
    return rosetta::ProtocolManager::Instance()->GetProtocol()->GetPartyId();
  }

  std::vector<std::string> get_supported_protocols() {
    return rosetta::ProtocolManager::Instance()->GetSupportedProtocols();
  }

  std::string get_default_protocol_name() {
    return rosetta::ProtocolManager::Instance()->GetDefaultProtocolName();
  }

  int activate(std::string protocol_name, std::string protocol_config_str) {
    return rosetta::ProtocolManager::Instance()->ActivateProtocol(
      protocol_name, protocol_config_str);
  }

  std::string get_protocol_name() {
    return rosetta::ProtocolManager::Instance()->GetProtocolName();
  }

  int deactivate() { return rosetta::ProtocolManager::Instance()->DeactivateProtocol(); }

  uint64_t rand_seed(uint64_t op_seed = 0) {
    if (!is_activated()) {
      cerr << "have not actived!" << endl;
      throw;
    }

    msg_id_t msg__seed_msg_id(seed_msg_id);
    auto randop = rosetta::ProtocolManager::Instance()->GetProtocol()->GetOps(msg__seed_msg_id);
    uint64_t seed = (uint64_t)randop->RandSeed();
    return seed;
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

  void log_to_stdout(bool flag) { Logger::Get().log_to_stdout(flag); }
  void set_logfile(const std::string& logfile) { Logger::Get().set_filename(logfile); }
  // Note: LogLevel \in { Cout = 0, Trace, Debug, Info, Warn, Error, Fatal };
  void set_loglevel(int loglevel) { Logger::Get().set_level(loglevel); }

  // stats
  void start_perf_stats() {
    if (is_activated()) {
      rosetta::ProtocolManager::Instance()->GetProtocol()->StartPerfStats();
    }
  }
  std::string get_perf_stats(bool pretty = false) {
    if (!is_activated()) {
      return "{}";
    }
    auto stats = rosetta::ProtocolManager::Instance()->GetProtocol()->GetPerfStats();
    return stats.to_json(pretty);
  }
};
