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
#include "cc/modules/protocol/mpc/helix/include/helix_impl.h"
#include "cc/modules/protocol/mpc/helix/include/helix_ops_impl.h"
#include "cc/modules/protocol/mpc/helix/include/helix_internal.h"

#include <stdexcept>
#include <string>
#include <vector>
using namespace std;

namespace rosetta {
using namespace rosetta::helix;

int HelixImpl::InitAesKeys() {
  msg_id_t msg__seed_msg_id(GetMpcContext()->TASK_ID + "=" + seed_msg_id_);
  auto hi = GetInternal(msg__seed_msg_id);
  gseed_ = make_shared<RttPRG>();
  hi->gseed = gseed_;
  key_prg_controller_ = hi->SyncPRGKey();
  return 0;
}

shared_ptr<ProtocolOps> HelixImpl::GetOps(const msg_id_t& msgid) {
  //! @todo optimized
  auto helix_ops_ptr = make_shared<HelixOpsImpl>(msgid, context_);
  helix_ops_ptr->io = GetNetHandler();
  helix_ops_ptr->hi = GetInternal(msgid);
  
  return helix_ops_ptr;
}

shared_ptr<helix::HelixInternal> HelixImpl::GetInternal(const msg_id_t& msgid) {
  auto o = make_shared<HelixInternal>(
    context_->ROLE_ID, GetNetHandler(), msgid, context_, key_prg_controller_);
  if (gseed_ != nullptr)
    o->gseed = gseed_; //! @todo use api
  return o;
}
} // namespace rosetta
