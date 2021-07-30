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
#include "cc/modules/protocol/mpc/comm/include/mpc_protocol.h"

namespace rosetta {
namespace helix {
class HelixInternal;
}

class HelixImpl : public MpcProtocol {
 public:
  HelixImpl(const string& task_id="") : MpcProtocol("Helix", 3, task_id) {}

  int InitAesKeys();

 public:
  shared_ptr<ProtocolOps> GetOps(const msg_id_t& msgid);
  shared_ptr<helix::HelixInternal> GetInternal(const msg_id_t& msgid);
};

class HelixProtocolFactory : public IProtocolFactory {
 public:
  HelixProtocolFactory() {}

 public:
  shared_ptr<ProtocolBase> Create(const string& task_id) { return std::make_shared<HelixImpl>(task_id); }
};

} // namespace rosetta
