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

// A singleton managing all protocols

#pragma once
#include "cc/modules/protocol/public/include/protocol_base.h"

#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace rosetta {
using std::shared_ptr;
using std::string;
using std::unordered_map;
using std::vector;

class ProtocolManager {
 private:
  ProtocolManager() = default;
  ProtocolManager(const ProtocolManager&) = delete;
  ProtocolManager(ProtocolManager&&) = delete;
  ProtocolManager& operator=(const ProtocolManager&) = delete;
  ProtocolManager& operator=(ProtocolManager&&) = delete;

 public:
  static ProtocolManager* Instance() {
    static ProtocolManager ptcMgr;
    return &ptcMgr;
  }

  /**
   * @desc: Get all the registered protocols
   */
  vector<string> GetSupportedProtocols() {
    vector<string> res;
    //const std::lock_guard<std::mutex> lock(_protocol_mutex);
    for (auto& prtc : _registered_protocols_map) {
      res.push_back(prtc.first);
    }
    return res;
  }

  /**
   * @desc: Regsiter a new backend protocol. Used by C++ internally.
   * @param:
   *     protocol_name, the name that identify this protocol.
   *     protocol_impl, the protocol object.
   * @returns:
   *     0 if success, otherwise errcode.
   */
  int RegisterProtocol(const string& protocol_name, shared_ptr<ProtocolBase> protocol_impl);

  /**
   * @desc: Deregsiter a exisiting backend protocol. Used by C++ internally.
   * @param:
   *     protocol_name, the name that identify this protocol.
   * @returns:
   *     0 if success, otherwise errcode.
   * @note:
   *     normally, there is no reason to use this interface.
   */
  int DeRegisterProtocol(const string& protocol_name);

  /**
   * @desc: Get the current protocol in use. If the use has not activated any one,
   *         the default one will be used.
   * @returns:
   *     The pointer to current protocol in use.
   */
  shared_ptr<ProtocolBase> GetProtocol();

  /**
   * @desc: Get the name of the protocol currently in use
   */
  string GetProtocolName() {
    //const std::lock_guard<std::mutex> lock(_protocol_mutex);
    if (!curr_protocol_name.empty()) {
      return curr_protocol_name;
    } else {
      return "";
    }
  }

  /**
   * @desc: Activate the specific protocol to use, which will init the underlying
   *         network, and change the behavir of
   *         all the TF Ops that you may run later.
   * @param:
   *     protocol_name, the name of the protocol to activate.
   *     protocol_config_json_str, the Json-compatible string that is specific to
   *         the protocol you choose.
   */
  int ActivateProtocol(const string& protocol_name, const string& protocol_config_json_str);

  /**
   *  @desc: to deactivate the current protocol. This will close the underlying
   *      network and other resourses.
   */
  int DeactivateProtocol();

  string GetDefaultProtocolName() { return ProtocolManager::default_protocol_name; }
  bool IsActivated() const { return !curr_protocol_name.empty(); }

 private:
  // For now, we assume that the interfaces of this class will be called infrequent,
  // So for all resources, we use the same mutex lock to control access.
  std::mutex _protocol_mutex;
  // To indidate whether the user have called ActivateProtocol once ever.
  string curr_protocol_name;
  string default_protocol_name{"SecureNN"};
  unordered_map<string, std::shared_ptr<ProtocolBase>> _registered_protocols_map;
};

} // namespace rosetta
