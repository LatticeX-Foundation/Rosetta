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

#include "cc/modules/common/include/utils/perf_stats.h"
#include "cc/modules/protocol/public/include/protocol_ops.h"

#include <memory>
#include <vector>
#include <string>
#include <iostream>
#include <mutex>

namespace rosetta {
namespace io {
class BasicIO;
}
//! the unique NET_IO defined here!
using NET_IO = io::BasicIO;

/**
 * This is the base interface class for all secure cryptographic protocols
 */
class ProtocolBase {
 public:
  /**
   * @brief: constructor, and you can name the protocol.
   */
  ProtocolBase(const string& protocol_name) : _protocol_name(protocol_name), _net_io(nullptr){};
  virtual ~ProtocolBase() = default;

  /**
   * @desc: to init and activate this protocol. 
   *         Start the underlying network and prepare resources.
   * @param:
   *     config_json_str: a string which contains the protocol-specific config.
   * @return:
   *     0 if success, otherwise some errcode
   * @note:
   *   The partyID for MPC protocol is also included in the config_json_str,
   *   you may need extract it.
   */
  virtual int Init(string config_json_str = "");

  /**
   * @desc: to uninit and deactivate this protocol.
   * @return:
   *     0 if success, otherwise some errcode
   */
  virtual int Uninit();

  /**
   * @desc: after initialization, get the actual operation interface of this protocol
   * @param:
   *     op_token: an optional string to differentiate each other
   * @return:
   *     the Operations interface, the Ops whithin have the same token
   */
  virtual shared_ptr<ProtocolOps> GetOps(const string& op_token = "") {
    THROW_NOT_IMPL_FN(__func__);
  }

  /**
   * @desc: after initialization, get the network channel for this protocol
   * that can be used to send and receive data in Ops.
   */
  virtual shared_ptr<NET_IO> GetNetHandler() { THROW_NOT_IMPL_FN(__func__); }

  /**
   *  @desc: get the name of this cryptographical protocol
   */
  virtual string Name() const { return _protocol_name; }

  /**
   * return party id
   */
  int GetPartyId() const { return my_party_id; }

  /**
   * get current performance statistic info
   * 
   * @code
   * auto ps0 = GetPerfStats();
   * // some code
   * auto ps1 = GetPerfStats();
   * auto ps = ps1 - ps0;
   * @endcode
   */
  virtual PerfStats GetPerfStats() { return PerfStats(); }
  virtual void StartPerfStats() {}

 protected:
  int my_party_id = -1;
  bool _is_inited = false;
  string _protocol_name = "";

  std::mutex _status_mtx;
  shared_ptr<NET_IO> _net_io = nullptr;

  unordered_map<string, string> config_map;

  PerfStats perf_stats_;
};
} // namespace rosetta
