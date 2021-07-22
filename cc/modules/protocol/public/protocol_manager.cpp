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
#include "cc/modules/common/include/utils/logger.h"
#include "cc/modules/protocol/public/protocol_manager.h"
#if ROSETTA_ENABLES_PROTOCOL_MPC_HELIX
#include "cc/modules/protocol/mpc/helix/include/helix_impl.h"
#endif
#if ROSETTA_ENABLES_PROTOCOL_MPC_SECURENN
#include "cc/modules/protocol/mpc/snn/src/snn_protocol.h"
#endif
#include "cc/modules/protocol/mpc/naive/include/naive_impl.h"
#if ROSETTA_ENABLES_PROTOCOL_ZK
#include "cc/modules/protocol/zk/wolverine/include/wolverine_impl.h"
#endif

#include <iostream>

using namespace std;

namespace rosetta {

int ProtocolManager::RegisterProtocol(
  const string& protocol_name,
  shared_ptr<ProtocolBase> protocol_impl) {
  if (protocol_name.empty()) {
    cout << "ERROR! The name of the protocol to register is empty!" << endl;
    return -1;
  }

  if (protocol_impl == nullptr) {
    cout << "ERROR! The pointer of the protocol to register is NULL!" << endl;
    return -2;
  }

  //const std::lock_guard<std::mutex> lock(_protocol_mutex);
  if (_registered_protocols_map.find(protocol_name) != _registered_protocols_map.end()) {
    cout << "ERROR! The protocol has already been regsitered!" << endl;
    return -3;
  }
  _registered_protocols_map[protocol_name] = protocol_impl;
  // cout << "Your protocol, " << protocol_name << ", is registered successfully!" << endl;
  return 0;
}

int ProtocolManager::DeRegisterProtocol(const string& protocol_name) {
  if (protocol_name.empty()) {
    log_error << "ERROR! The name of the protocol to deregister is empty!" << endl;
    return -1;
  }
  //const std::lock_guard<std::mutex> lock(_protocol_mutex);
  if (_registered_protocols_map.find(protocol_name) != _registered_protocols_map.end()) {
    _registered_protocols_map.erase(protocol_name);
    log_info << "INFO: your protocol, " << protocol_name << ", is deregsitered successfully!"
             << endl;
    return 0;
  } else {
    log_error << "ERROR! the protocol you want to deregister, " << protocol_name
              << ", is not regsitered!" << endl;
    return -2;
  }
}

shared_ptr<ProtocolBase> ProtocolManager::GetProtocol() {
  ////const std::lock_guard<std::mutex> lock(_protocol_m);
  if (curr_protocol_name.empty()) {
    log_warn << "ERROR! please activate one protocol before you use it!" << endl;
  }
  auto curr_ptr = _registered_protocols_map[curr_protocol_name];
  //cout << "DEBUG ProtocolManager::GetProtocol() : " << curr_protocol_name << endl;
  return curr_ptr;
}

int ProtocolManager::ActivateProtocol(
  const string& protocol_name,
  const string& protocol_config_json_str) {
  log_debug << "DEBUG: PM ActivateProtocol " << protocol_name
            << " with config: " << protocol_config_json_str << endl;

  //const std::lock_guard<std::mutex>  lock(_protocol_mutex);
  if (curr_protocol_name == protocol_name) {
    log_warn << "WARN! The protocol [" << protocol_name << "] has already being used NOW!" << endl;
    return 0;
  }
  if (_registered_protocols_map.find(protocol_name) == _registered_protocols_map.end()) {
    log_error << "ERROR! The protocol has not been regsitered!" << endl;
    return -2;
  }
  auto new_prtc = _registered_protocols_map[protocol_name];

  int ret = DeactivateProtocol();
  if (ret != 0) {
    return ret;
  }
  log_debug << "Deactivate done ,begin init" << endl;
  ret = new_prtc->Init(protocol_config_json_str);
  log_debug << "init ret:" << ret << endl;
  if (ret != 0) {
    log_error << "fail to init protocol:" << protocol_name << endl;
    return ret;
  }

  curr_protocol_name = protocol_name;
  log_debug << "ProtocolManager::ActivateProtocol Done!" << endl;
  return 0;
}

int ProtocolManager::DeactivateProtocol() {
  log_debug << "DEBUG: PM DeactivateProtocol" << endl;
  //const std::lock_guard<std::mutex> lock(_protocol_m);
  if (curr_protocol_name.empty()) {
    return 0;
  }
  log_debug << "current protcol: " << curr_protocol_name << endl;
  auto new_prtc = _registered_protocols_map[curr_protocol_name];
  if (!new_prtc->Uninit()) {
    curr_protocol_name = "";
    return 0;
  } else {
    log_error << "ERROR! fail to deactive current protocol: " << curr_protocol_name << endl;
    return -1;
  }
}

// For static registering!
/**
 * @note: you can also register your customized cryptographic protocol here
 * if you has implemented your protocol according to our protocol interface.
 */
template <typename Protocol>
struct ProtocolRegistrar {
  ProtocolRegistrar(const string& name) {
    auto impl_ptr = make_shared<Protocol>();
    auto base_ptr = std::dynamic_pointer_cast<ProtocolBase>(impl_ptr);
    ProtocolManager::Instance()->RegisterProtocol(name, base_ptr);
  }
};

#define REGISTER_SECURE_PROTOCOL(Prot, Name) \
  static ProtocolRegistrar<Prot> _protocol_registrar_##Prot(Name)

#if ROSETTA_ENABLES_PROTOCOL_MPC_HELIX
REGISTER_SECURE_PROTOCOL(HelixImpl, "Helix");
#endif
#if ROSETTA_ENABLES_PROTOCOL_MPC_SECURENN
REGISTER_SECURE_PROTOCOL(SnnProtocol, "SecureNN");
#endif
REGISTER_SECURE_PROTOCOL(NaiveProtocol, "Naive");

#if ROSETTA_ENABLES_PROTOCOL_ZK
REGISTER_SECURE_PROTOCOL(WolverineProtocol, "Wolverine");
#endif

} // namespace rosetta
