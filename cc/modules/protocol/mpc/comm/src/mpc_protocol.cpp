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
#include "cc/modules/protocol/mpc/comm/include/mpc_protocol.h"
#include "cc/modules/protocol/mpc/comm/include/mpc_util.h"
#include "cc/modules/protocol/mpc/comm/include/config.h"
#include "cc/modules/io/include/ex.h"

#include <stdexcept>
#include <string>
#include <vector>
using namespace std;

namespace rosetta {
int MpcProtocol::Init(std::string config_json_str) {
  //! @todo optimized
  log_debug << "MpcProtocol::Init with config:" << config_json_str << endl;
  if (!_is_inited) {
    std::unique_lock<std::mutex> lck(_status_mtx);
    if (!_is_inited) {
      _init_config(config_json_str);
      my_party_id = config->PARTY;
      config->fmt_print();
      _initialize_mpc_enviroment();
      _init_with_config();
      _init_aeskeys();
      _is_inited = true;
    }
  }
  log_debug << "MpcProtocol::Init done!" << endl;
  log_info << "Rosetta: Protocol [" << _protocol_name << "] backend initialization succeeded!"
           << endl;
  return 0;
}
int MpcProtocol::Init(int partyid, std::string config_json_str) {
  return Init(partyid, config_json_str, "helix-default-" + to_string(partyid) + ".log");
}
int MpcProtocol::Init(int partyid, string config_json_str, string logfile) {
  if (logfile != "")
    rosetta::redirect_stdout(logfile);

  //! @todo optimized
  log_debug << "MpcProtocol::Init2 with config:" << config_json_str << endl;
  if (!_is_inited) {
    std::unique_lock<std::mutex> lck(_status_mtx);
    if (!_is_inited) {
      _init_config(partyid, config_json_str);
      my_party_id = config->PARTY;
      config->fmt_print();
      _initialize_mpc_enviroment();
      _init_with_config();
      _init_aeskeys();
      _is_inited = true;
    }
  }
  log_info << "Rosetta: MpcProtocol::Init with CONFIG file done!" << endl;
  return 0;
}

int MpcProtocol::_init_config(int partyid, const std::string& config_json) {
  config = make_shared<RosettaConfig>(partyid, config_json);
  return 0;
}
int MpcProtocol::_init_config(const std::string& config_json) {
  config = make_shared<RosettaConfig>(config_json);
  return 0;
}
int MpcProtocol::_init_aeskeys() {
  //
  return 0;
}
int MpcProtocol::_init_with_config() {
  FLOAT_PRECISION_M = config->mpc.FLOAT_PRECISION_M;
  log_debug << "MpcProtocol::_init_with_config FLOAT_PRECISION_M:" << FLOAT_PRECISION_M
            << ", my_party_id: " << my_party_id << endl;

  int parties = 3;
  vector<int> ports;
  vector<string> ips;
  for (int i = 0; i < parties; i++) {
    ports.push_back(config->mpc.P[i].PORT);
    ips.push_back(config->mpc.P[i].HOST);
  }

  _net_io = make_shared<ParallelIO>(parties, my_party_id, 1, ports, ips);
  _net_io->set_server_cert(config->mpc.SERVER_CERT);
  _net_io->set_server_prikey(config->mpc.SERVER_PRIKEY);
  _net_io->init();

  config_map["save_mode"] = to_string(config->mpc.SAVER_MODE);

  msg_id_t msgid("this message id for synchronize P0/P1/P2 init");
  log_debug << __FUNCTION__ << " beg sync :" << time(0) << endl;
  _net_io->sync_with(msgid);
  log_debug << __FUNCTION__ << " end sync :" << time(0) << endl;

  return 0;
}

int MpcProtocol::Uninit() {
  log_debug << "MpcProtocol::Uninit()" << endl;
  std::unique_lock<std::mutex> lck(_status_mtx);
  if (_is_inited) {
    _net_io->statistics();

    msg_id_t msgid("this message id for synchronize P0/P1/P2 uninit");

    // the following time(0) will show the sync beg/end
    log_debug << __FUNCTION__ << " beg sync :" << time(0) << endl;
    _net_io->sync_with(msgid);
    log_debug << __FUNCTION__ << " end sync :" << time(0) << endl;

    _net_io->close();
    _net_io.reset();
    rosetta::restore_stdout();
    log_info << "Rosetta: Protocol [" << _protocol_name << "] backend has been released." << endl;
    _is_inited = false;
  }
  return 0;
}

PerfStats MpcProtocol::GetPerfStats() {
  PerfStats perf_stats;
  if (!_is_inited) {
    return perf_stats;
  }

  //! Network
  io::NetStat net_stat = _net_io->net_stat();
  perf_stats.s.bytes_sent = net_stat.bytes_sent();
  perf_stats.s.bytes_recv = net_stat.bytes_received();
  perf_stats.s.msg_sent = net_stat.message_sent();
  perf_stats.s.msg_recv = net_stat.message_received();

  //! Time @todo

  return perf_stats;
}

} // namespace rosetta
