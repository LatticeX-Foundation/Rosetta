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
#include "cc/modules/protocol/mpc/snn/src/snn_protocol.h"
#include "cc/modules/protocol/mpc/comm/include/mpc_helper.h"
#include "cc/modules/common/include/utils/logger.h"
#include "cc/modules/common/include/utils/generate_key.h"
#include "cc/modules/protocol/mpc/snn/src/snn_protocol_ops.h"
#include "cc/modules/protocol/mpc/snn/src/internal/snn_helper.h"
#include "cc/modules/protocol/mpc/snn/include/snn_opsets.h"
#include "cc/modules/protocol/mpc/snn/include/snn_config.h"

#include <iostream>
using namespace std;

namespace rosetta {
shared_ptr<ProtocolOps> SnnProtocol::GetOps(const string& op_token) {
  //! @todo optimized
  auto snn_ops_ptr = make_shared<SnnProtocolOps>(op_token);
  snn_ops_ptr->net_io_ = GetNetHandler();
  snn_ops_ptr->op_config_map = config_map;
  auto o = std::dynamic_pointer_cast<ProtocolOps>(snn_ops_ptr);
  return o;
}

void SnnProtocol::_initialize_mpc_enviroment() {
  partyNum = my_party_id;
  NUM_OF_PARTIES = 3; /// set to 3PC
  initializeMPC();
}
int SnnProtocol::_init_aeskeys() {
  using namespace rosetta::mpc;
  {
    // gen private key
    // 3PC: P0-->keyA, keyCD; P1-->keyB; P2-->keyC
    if (my_party_id == 0) {
      AESKeyStrings::keys.key_a = gen_key_str();
    } else if (my_party_id == 1) {
      AESKeyStrings::keys.key_b = gen_key_str();
    } else if (my_party_id == 2) {
      AESKeyStrings::keys.key_c = gen_key_str();
      AESKeyStrings::keys.key_cd = gen_key_str();
    }
    //usleep(1000);//no need to sleep

    // public aes key
    string kab, kac, kbc, k0;
    k0 = gen_key_str();
    kab = gen_key_str();
    kac = gen_key_str();
    kbc = gen_key_str();

    string msgkey("l---------=+++");
    auto sync_aes_key = std::make_shared<rosetta::snn::SyncAesKey>(msgkey, GetNetHandler());

    sync_aes_key->Run(PARTY_A, PARTY_B, kab, kab);
    sync_aes_key->Run(PARTY_A, PARTY_C, kac, kac);
    sync_aes_key->Run(PARTY_B, PARTY_C, kbc, kbc);
    sync_aes_key->Run(PARTY_C, PARTY_A, k0, k0);
    sync_aes_key->Run(PARTY_C, PARTY_B, k0, k0);
    AESKeyStrings::keys.key_0 = k0;
    AESKeyStrings::keys.key_ab = kab;
    AESKeyStrings::keys.key_bc = kbc;
    AESKeyStrings::keys.key_ac = kac;
  }
  //usleep(1000);//no need to sleep

  return 0;
}
} // namespace rosetta
