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
#include <algorithm>
using namespace std;

#include "cc/modules/protocol/public/include/protocol_manager.h"
#include "cc/modules/common/include/utils/rtt_logger.h"
#include "cc/modules/common/include/utils/rtt_exceptions.h"


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
    if (origin_stdout != -1)
      restore_stdout();
  }

  bool is_activated(const string& task_id="") const {
    shared_ptr<ProtocolBase> proto = rosetta::ProtocolManager::Instance()->GetProtocol(task_id);
    if (proto.get() != nullptr)
      return proto->IsInit();
    return false;
  }

  int get_party_id(const string& task_id="") const {
    return rosetta::ProtocolManager::Instance()->GetProtocol(task_id)->GetMpcContext()->GetMyRole();
  }

  std::vector<std::string> get_supported_protocols() {
    return rosetta::ProtocolManager::Instance()->GetSupportedProtocols();
  }

  std::string get_default_protocol_name() {
    return rosetta::ProtocolManager::Instance()->GetDefaultProtocolName();
  }

  int activate(std::string protocol_name, const string& task_id="") {
    return rosetta::ProtocolManager::Instance()->ActivateProtocol(
      protocol_name, task_id);
  }

  void set_float_precision(int float_precision, const string& task_id="") {
    rosetta::ProtocolManager::Instance()->SetFloatPrecision(float_precision, task_id);
  }

  void get_float_precision(const string& task_id="") {
    rosetta::ProtocolManager::Instance()->GetFloatPrecision(task_id);
  }

  void set_saver_model(const vector<string> model_nodes, const string& task_id="") {
    rosetta::ProtocolManager::Instance()->SetSaverModel(model_nodes, task_id);
  }

  vector<string> get_saver_model(const string& task_id="") {
    return rosetta::ProtocolManager::Instance()->GetSaverModel(task_id);
  }

  void set_restore_model(const vector<string>& model_nodes, const string& task_id="") {
    rosetta::ProtocolManager::Instance()->SetRestoreModel(model_nodes, task_id);
  }

  vector<string> get_restore_model(const string& task_id="") {
    return rosetta::ProtocolManager::Instance()->GetRestoreModel(task_id);
  }

  std::string get_protocol_name(const string& task_id="") {
    return rosetta::ProtocolManager::Instance()->GetProtocolName(task_id);
  }

  int deactivate(const string& task_id="") { return rosetta::ProtocolManager::Instance()->DeactivateProtocol(task_id); }

  uint64_t rand_seed(const string& task_id) {
    if (!is_activated(task_id)) {
      cerr << "have not activated!" << endl;
      throw;
    }

    msg_id_t msg__seed_msg_id(seed_msg_id);
    auto randop = rosetta::ProtocolManager::Instance()->GetProtocol(task_id)->GetOps(msg__seed_msg_id);
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
  void set_logfile(const std::string& logfile, const std::string& task_id="") { Logger::Get().set_filename(logfile, task_id); }
  void set_logpattern(const std::string& pattern) { Logger::Get().set_pattern(pattern);}
  // Note: LogLevel \in { Trace, Debug, Audit, Info, Warn, Error, Fatal, off };
  void set_loglevel(int loglevel) { Logger::Get().set_level((int)loglevel % 7); }

  // stats
  void start_perf_stats(const string& task_id) {
    if (is_activated(task_id)) {
      rosetta::ProtocolManager::Instance()->GetProtocol(task_id)->StartPerfStats();
    }
  }
  std::string get_perf_stats(bool pretty = false, const string& task_id="") {
    if (!is_activated(task_id)) {
      return "{}";
    }
    auto stats = rosetta::ProtocolManager::Instance()->GetProtocol(task_id)->GetPerfStats();
    return stats.to_json(pretty);
  }

  // associate task id with unique id
  void mapping_id(const uint64_t& unique_id, const string& task_id="") {
    rosetta::ProtocolManager::Instance()->MappingID(unique_id, task_id);
  }

  string query_mapping_id(const uint64_t& unique_id) {
    return rosetta::ProtocolManager::Instance()->QueryMappingID(unique_id);
  }

  void unmapping_id(const uint64_t& unique_id) {
    return rosetta::ProtocolManager::Instance()->UnmappingID(unique_id);
  }

};
