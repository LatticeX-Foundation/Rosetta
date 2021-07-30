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
#include "cc/modules/protocol/mpc/snn/include/snn_protocol.h"
#include "cc/modules/protocol/mpc/snn/include/snn_ops.h"
#include "cc/modules/protocol/mpc/comm/include/mpc_helper.h"
#include "cc/modules/common/include/utils/rtt_logger.h"
#include "cc/modules/common/include/utils/generate_key.h"
#include "cc/modules/protocol/mpc/snn/src/internal/snn_helper.h"
// #include "cc/modules/protocol/mpc/snn/include/snn_opsets.h"

#include <iostream>
#include <mutex>
#include <thread>

using namespace std;

namespace rosetta {
namespace snn {

std::once_flag g_snn_once_flag_ex;
 
static void snn_init_once_calls()
{
  std::call_once(g_snn_once_flag_ex, [](){ init_mod_prime_tables(); });
}

shared_ptr<ProtocolOps> SnnProtocol::GetOps(const msg_id_t& msgid) {
  //! @todo optimized
  auto snn_ops_ptr = make_shared<SnnProtocolOps>(msgid, context_, gseed_, GetNetHandler());
  snn_ops_ptr->_triple_generator = triple_generator_;
  
  return snn_ops_ptr;
}

shared_ptr<snn::SnnInternal> SnnProtocol::GetInternal(const msg_id_t& msgid) {
  return make_shared<snn::SnnInternal>(msgid, GetMpcContext(), gseed_, GetNetHandler());
}

void SnnProtocol::InitMpcEnvironment() {
  snn_init_once_calls();
}

int SnnProtocol::OfflinePreprocess() {
  tlog_info << "calling SnnProtocol::OfflinePreprocess";

  triple_generator_ = make_shared<SnnTripleGenerator>(GetNetHandler());
  triple_generator_->pre_gen();
  return 0;
}

int SnnProtocol::InitAesKeys() {
  using namespace rosetta::snn;
  {
    // gen private key
    // 3PC: P0-->keyA, keyCD; P1-->keyB; P2-->keyC
    auto my_party_id = context_->ROLE_ID;
    if (my_party_id == 0) {
      aes_keys_.key_a = gen_key_str();
    } else if (my_party_id == 1) {
      aes_keys_.key_b = gen_key_str();
    } else if (my_party_id == 2) {
      aes_keys_.key_c = gen_key_str();
      aes_keys_.key_cd = gen_key_str();
    }
    //usleep(1000);//no need to sleep

    // public aes key
    string kab, kac, kbc, k0;
    k0 = gen_key_str();
    kab = gen_key_str();
    kac = gen_key_str();
    kbc = gen_key_str();

    // msg_id_t msgkey("snn_sync_aes_key_temp_msgid_string");
    msg_id_t msgkey(GetMpcContext()->TASK_ID + "=" + seed_msg_id_);
    gseed_ = make_shared<RttPRG>();
    auto internal = GetInternal(msgkey);
    // hi->SyncPRGKey();
    internal->SyncAesKey(PARTY_A, PARTY_B, kab, kab);
    internal->SyncAesKey(PARTY_A, PARTY_C, kac, kac);
    internal->SyncAesKey(PARTY_B, PARTY_C, kbc, kbc);
    internal->SyncAesKey(PARTY_C, PARTY_A, k0, k0);
    internal->SyncAesKey(PARTY_C, PARTY_B, k0, k0);
    aes_keys_.key_0 = k0;
    aes_keys_.key_ab = kab;
    aes_keys_.key_bc = kbc;
    aes_keys_.key_ac = kac;

    auto task_id = GetMpcContext()->TASK_ID;
    auto role_id = GetMpcContext()->GetMyRole();
    aes_controller_ = std::make_shared<SnnAesobjectsController>();
    aes_controller_->Init(role_id, aes_keys_);
  }
  //usleep(1000);//no need to sleep

  return 0;
}

} // snn
} // namespace rosetta
