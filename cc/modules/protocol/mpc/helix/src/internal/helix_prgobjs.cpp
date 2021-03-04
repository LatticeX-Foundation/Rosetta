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
#include "cc/modules/protocol/mpc/helix/include/helix_def.h"
#include "cc/modules/protocol/mpc/helix/include/helix_util.h"

#include "cc/modules/protocol/mpc/helix/include/helix_prgobjs.h"
namespace rosetta {
MpcPRGKeys MpcPRGKeys::keys;

std::set<msg_id_t> MpcPRGObjs::msig_objs_;
std::mutex MpcPRGObjs::msgid_mpc_prgs_mtx_;
std::map<msg_id_t, std::shared_ptr<MpcPRGObjs>> MpcPRGObjs::msgid_mpc_prgs_;

void MpcPRGKeys::reset() {
  // clang-format off
  key_prg   = "00000000000000000000000000000000";
  key_prg01 = "00000000000000000000000000000000";
  key_prg02 = "00000000000000000000000000000000";
  key_prg12 = "00000000000000000000000000000000";
  key_prg0  = "00000000000000000000000000000000";
  key_prg1  = "00000000000000000000000000000000";
  key_prg2  = "00000000000000000000000000000000";
  // clang-format on
}

void MpcPRGKeys::fmt_print() {
  cout << "K00 " << key_prg << endl;
  cout << "K01 " << key_prg01 << endl;
  cout << "K02 " << key_prg02 << endl;
  cout << "K12 " << key_prg12 << endl;
  cout << " K0 " << key_prg0 << endl;
  cout << " K1 " << key_prg1 << endl;
  cout << " K2 " << key_prg2 << endl;
}

int MpcPRGObjs::init(int player, const msg_id_t& msg_id) {
  //! @todo use different key according to msg_id
  prg = std::make_shared<RttPRG>(MpcPRGKeys::keys.key_prg);
  prg01 = std::make_shared<RttPRG>(MpcPRGKeys::keys.key_prg01);
  prg02 = std::make_shared<RttPRG>(MpcPRGKeys::keys.key_prg02);
  prg12 = std::make_shared<RttPRG>(MpcPRGKeys::keys.key_prg12);
  prg0 = std::make_shared<RttPRG>(MpcPRGKeys::keys.key_prg0);
  prg1 = std::make_shared<RttPRG>(MpcPRGKeys::keys.key_prg1);
  prg2 = std::make_shared<RttPRG>(MpcPRGKeys::keys.key_prg2);

  return 0;
}

std::shared_ptr<MpcPRGObjs> MpcPRGObjs::Get(int player, const msg_id_t& msg_id) {
  if (msig_objs_.count(msg_id) > 0) {
    return msgid_mpc_prgs_[msg_id];
  }

  std::unique_lock<std::mutex> lck(msgid_mpc_prgs_mtx_);
  auto iter = msgid_mpc_prgs_.find(msg_id);
  if (iter != msgid_mpc_prgs_.end()) {
    return iter->second;
  }

  auto prgobjs = std::make_shared<MpcPRGObjs>();
  prgobjs->init(player, msg_id);
  msgid_mpc_prgs_[msg_id] = prgobjs;
  msig_objs_.insert(msg_id);
  return msgid_mpc_prgs_[msg_id];
}

} // namespace rosetta
