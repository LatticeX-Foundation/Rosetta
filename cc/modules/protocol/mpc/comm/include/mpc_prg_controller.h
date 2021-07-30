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


#include <string>
#include <iostream>
#include <memory>
#include <map>
#include "cc/modules/common/include/utils/msg_id.h"
#include "cc/modules/protocol/utility/include/prg.h"

using namespace std;

namespace rosetta {

struct MpcPRGKeysV2 {
  // clang-format off
  std::string key_prg   = "FFFF0000000000000000000000000000";
  std::string key_prg01 = "01000000000000000000000000000000";
  std::string key_prg02 = "02000000000000000000000000000000";
  std::string key_prg12 = "12000000000000000000000000000000";
  std::string key_prg0  = "FF000000000000000000000000000000";
  std::string key_prg1  = "FF010000000000000000000000000000";
  std::string key_prg2  = "FF020000000000000000000000000000";
  std::string key_private = "PRIVATE0000000000000000000000000";
  // clang-format on

  void reset();
  void fmt_print();
};


struct MpcPRGObjsV2 {
  /**
   * prg   // P0 and P1 and P2
   * prg01 // P0 and P1
   * prg02 // P0 and P2
   * prg12 // P1 and P2
   * prg0  // P0
   * prg1  // P1
   * prg2  // P2
   */
  std::shared_ptr<RttPRG> prg = nullptr;
  std::shared_ptr<RttPRG> prg01 = nullptr;
  std::shared_ptr<RttPRG> prg02 = nullptr;
  std::shared_ptr<RttPRG> prg12 = nullptr;
  std::shared_ptr<RttPRG> prg0 = nullptr;
  std::shared_ptr<RttPRG> prg1 = nullptr;
  std::shared_ptr<RttPRG> prg2 = nullptr;
  std::shared_ptr<RttPRG> prg_private = nullptr;

  msg_id_t msgid;

 public:
  int Init(const msg_id_t& msg_id, const MpcPRGKeysV2& prg_keys);
};

// controller to manage keys and prg-objects for a protocol.
class MpcKeyPrgController {
private:
  string task_id_;
  std::mutex mutex_;
  MpcPRGKeysV2 keys_;
  std::map<msg_id_t, std::shared_ptr<MpcPRGObjsV2>> msgid_mpc_prgs_;

public:
  MpcKeyPrgController() {}

  std::shared_ptr<MpcPRGObjsV2> Get(const msg_id_t& msg_id);

  MpcPRGKeysV2 GetKeys() { return keys_; }
  void SetKeys(const MpcPRGKeysV2& keys) { keys_ = keys; }

  void Init(const MpcPRGKeysV2& keys);
  void Reset();
};

} // namespace rosetta
