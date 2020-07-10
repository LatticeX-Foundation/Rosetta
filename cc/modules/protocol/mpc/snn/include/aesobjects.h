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
#include "cc/modules/protocol/mpc/snn/include/mpc_tools.h"

extern int partyNum;
// it seems that this is not used in Helix?
namespace rosetta {
namespace mpc {

class AESKeyStrings {
 public:
  static AESKeyStrings keys; // global aes keys

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
};

/**
 * This class('s objects) will be cached by a map <msg-id, objs>\n
 * Because OP-call-chain grouped by msg-id,\n
 * so, each different msg-id will use an unique aesobjects\n
 */
class AESObjects {
  static std::mutex msgid_aesobjs_mtx_;
  static std::map<msg_id_t, std::shared_ptr<AESObjects>> msgid_aesobjs_;

 public:
  static std::shared_ptr<AESObjects> Get(const msg_id_t& msg_id) {
    std::unique_lock<std::mutex> lck(msgid_aesobjs_mtx_);
    auto iter = msgid_aesobjs_.find(msg_id);
    if (iter != msgid_aesobjs_.end()) {
      return iter->second;
    }

    auto aesobjs = std::make_shared<AESObjects>();
    aesobjs->init_aes(partyNum, msg_id);
    msgid_aesobjs_[msg_id] = aesobjs;
    return msgid_aesobjs_[msg_id];
  }

 private:
  int init_aes(int pid, const msg_id_t& msg_id);

 public:
  std::shared_ptr<AESObject> aes_randseed = nullptr; // A and B and C
  std::shared_ptr<AESObject> aes_common = nullptr; // A and B
  std::shared_ptr<AESObject> aes_indep = nullptr; // A or B or C
  std::shared_ptr<AESObject> aes_a_1 = nullptr; // A and C
  std::shared_ptr<AESObject> aes_a_2 = nullptr; // B and C
  std::shared_ptr<AESObject> aes_b_1 = nullptr; // A and C
  std::shared_ptr<AESObject> aes_b_2 = nullptr; // B and C
  std::shared_ptr<AESObject> aes_c_1 = nullptr; // A and C
};
} // namespace mpc
} // namespace rosetta