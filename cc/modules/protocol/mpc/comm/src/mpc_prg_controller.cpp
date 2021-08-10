

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
#include "cc/modules/common/include/utils/helper.h"
#include "cc/modules/common/include/utils/model_tool.h"
#include "cc/modules/common/include/utils/simple_timer.h"
#include "cc/modules/protocol/public/include/protocol_base.h"
#include "cc/modules/protocol/utility/include/prg.h"
#include "cc/modules/protocol/mpc/comm/include/mpc_prg_controller.h"

namespace rosetta {
void MpcPRGKeysV2::reset() {
  // clang-format off
  key_prg   = "00000000000000000000000000000000";
  key_prg01 = "00000000000000000000000000000000";
  key_prg02 = "00000000000000000000000000000000";
  key_prg12 = "00000000000000000000000000000000";
  key_prg0  = "00000000000000000000000000000000";
  key_prg1  = "00000000000000000000000000000000";
  key_prg2  = "00000000000000000000000000000000";
  key_a0.clear();
  key_a1.clear();
  // clang-format on
}

void MpcPRGKeysV2::fmt_print() {
  cout << "K00 " << key_prg << endl;
  cout << "K01 " << key_prg01 << endl;
  cout << "K02 " << key_prg02 << endl;
  cout << "K12 " << key_prg12 << endl;
  cout << " K0 " << key_prg0 << endl;
  cout << " K1 " << key_prg1 << endl;
  cout << " K2 " << key_prg2 << endl;
  for (auto iter = key_a0.begin(); iter != key_a0.end(); iter++) {
    cout << "A0 " << iter->first << " " << iter->second << endl;
  }
  for (auto iter = key_a1.begin(); iter != key_a1.end(); iter++) {
    cout << "A1 " << iter->first << " " << iter->second << endl;
  }
}

int MpcPRGObjsV2::Init(const msg_id_t& msg_id, const MpcPRGKeysV2& prg_keys) {
  msgid = msg_id;

  //! @todo use different key according to msg_id
  prg = std::make_shared<RttPRG>(prg_keys.key_prg);
  prg01 = std::make_shared<RttPRG>(prg_keys.key_prg01);
  prg02 = std::make_shared<RttPRG>(prg_keys.key_prg02);
  prg12 = std::make_shared<RttPRG>(prg_keys.key_prg12);
  prg0 = std::make_shared<RttPRG>(prg_keys.key_prg0);
  prg1 = std::make_shared<RttPRG>(prg_keys.key_prg1);
  prg2 = std::make_shared<RttPRG>(prg_keys.key_prg2);
  prg_a0.clear();
  prg_a1.clear();
  for (auto iter = prg_keys.key_a0.begin(); iter != prg_keys.key_a0.end(); iter++) {
    prg_a0.insert(std::pair<std::string, std::shared_ptr<RttPRG>>(iter->first, std::make_shared<RttPRG>(iter->second)));
  }
  for (auto iter = prg_keys.key_a1.begin(); iter != prg_keys.key_a1.end(); iter++) {
    prg_a1.insert(std::pair<std::string, std::shared_ptr<RttPRG>>(iter->first, std::make_shared<RttPRG>(iter->second)));
  }

  return 0;
}

void MpcKeyPrgController::Init(const MpcPRGKeysV2& keys) {
  std::unique_lock<std::mutex> lck(mutex_);
  keys_ = keys;
}

std::shared_ptr<MpcPRGObjsV2> MpcKeyPrgController::Get(const msg_id_t& msg_id) {
  {
    std::unique_lock<std::mutex> lck(mutex_);
    auto iter = msgid_mpc_prgs_.find(msg_id);
    if (iter != msgid_mpc_prgs_.end()) {
      return iter->second;
    }
  }

  auto prgobjs = std::make_shared<MpcPRGObjsV2>();
  prgobjs->Init(msg_id, keys_);
  {
    std::unique_lock<std::mutex> lck(mutex_);
    msgid_mpc_prgs_[msg_id] = prgobjs;
  }
  return prgobjs;
}

void MpcKeyPrgController::Reset() {
  std::unique_lock<std::mutex> lck(mutex_);
  msgid_mpc_prgs_.clear();
}

} // namespace rosetta
