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

#include "cc/modules/protocol/mpc/comm/include/mpc_helper.h"
#include "cc/modules/protocol/mpc/snn/src/internal/AESObject.h"
#include "cc/modules/protocol/mpc/snn/include/snn_tools.h"
#include <set>
#include <map>
#include <memory>
#include <mutex>

namespace rosetta {
namespace snn {

struct AESKeyStringsV2 {
 public:
  void print();
  // default global aes keystring
  std::string key_0 = "F0000000000000000000000000000000";
  std::string key_a = "F000000000000000000000000000000A";
  std::string key_b = "F000000000000000000000000000000B";
  std::string key_c = "F000000000000000000000000000000C";
  std::string key_ab = "F00000000000000000000000000000AB";
  std::string key_ac = "F00000000000000000000000000000AC";
  std::string key_bc = "F00000000000000000000000000000BC";
  std::string key_cd = "F00000000000000000000000000000CD";
  std::string key_private = "F000000000000000000000000PRIVATE";
};

/**
 * This class('s objects) will be cached by a map <msg-id, objs>\n
 * Because OP-call-chain grouped by msg-id,\n
 * so, each different msg-id will use an unique aesobjects\n
 */
struct AESObjectsV2 {
public:
  std::shared_ptr<AESObject> aes_randseed = nullptr; // A and B and C
  std::shared_ptr<AESObject> aes_common = nullptr; // A and B
  std::shared_ptr<AESObject> aes_indep = nullptr; // A or B or C
  std::shared_ptr<AESObject> aes_a_1 = nullptr; // A and C
  std::shared_ptr<AESObject> aes_a_2 = nullptr; // B and C
  std::shared_ptr<AESObject> aes_b_1 = nullptr; // A and C
  std::shared_ptr<AESObject> aes_b_2 = nullptr; // B and C
  std::shared_ptr<AESObject> aes_c_1 = nullptr; // A and C
  std::shared_ptr<AESObject> aes_private = nullptr;

  AESKeyStringsV2 keys;


public:
  AESObjectsV2(const AESKeyStringsV2& ks) : keys(ks) {}
  AESObjectsV2() {}

  void set_keys(const AESKeyStringsV2& ks) { keys = ks; }
  int init_aes(int pid, const msg_id_t& msg_id);
};

// controller to manage keys and prg-objects for a protocol.
class SnnAesobjectsController {
 private:
  std::string task_id_;
  std::mutex mutex_;
  AESKeyStringsV2 keys_;
  std::map<msg_id_t, std::shared_ptr<AESObjectsV2>> msgid_mpc_aesobj_;
  int party_id_ = -1;

 public:
  SnnAesobjectsController() {}
  // SnnAesobjectsController(const string& task, int party_id)
  //     : task_id_(task), party_id_(party_id) {}

  std::shared_ptr<AESObjectsV2> Get(const msg_id_t& msg_id);

  AESObjectsV2 GetKeys() { return keys_; }
  void SetKeys(const AESKeyStringsV2& keys) { keys_ = keys; }

  void Init(int party_id, const AESKeyStringsV2& keys);
  void Reset();
};

} // namespace snn
} // namespace rosetta