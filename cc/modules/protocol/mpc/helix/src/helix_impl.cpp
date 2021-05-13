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
#include "cc/modules/io/include/net_io.h"

#include <stdexcept>
#include <string>
#include <vector>
using namespace std;

namespace rosetta {
using namespace rosetta::helix;

int HelixImpl::_init_aeskeys() {
  msg_id_t msg__seed_msg_id(seed_msg_id);
  auto hi = GetHelixInternel(msg__seed_msg_id);
  gseed = make_shared<RttPRG>();
  hi->gseed = gseed;
  hi->SyncPRGKey();
  return 0;
}

shared_ptr<ProtocolOps> HelixImpl::GetOps(const msg_id_t& msgid) {
  //! @todo optimized
  auto helix_ops_ptr = make_shared<HelixOpsImpl>(msgid);
  helix_ops_ptr->io = GetNetHandler();
  helix_ops_ptr->hi = GetHelixInternel(msgid);
  helix_ops_ptr->op_config_map = config_map;
  return std::dynamic_pointer_cast<ProtocolOps>(helix_ops_ptr);
}

shared_ptr<helix::HelixInternal> HelixImpl::GetHelixInternel(const msg_id_t& msgid) {
  auto o = make_shared<HelixInternal>(my_party_id, GetNetHandler(), msgid);
  if (gseed != nullptr)
    o->gseed = gseed; //! @todo use api
  return o;
}
} // namespace rosetta
