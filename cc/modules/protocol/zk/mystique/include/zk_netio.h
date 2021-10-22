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

#include "emp-tool/emp-tool.h"
#include "emp-zk/emp-zk.h"

#include "cc/modules/iowrapper/include/io_manager.h"
#include "cc/modules/protocol/public/include/protocol_manager.h"

using EMPNetIO = emp::NetIO;
typedef BoolIO<EMPNetIO> EMP_NET_IO;

class ZKNetIO : public IOChannel<ZKNetIO> {
  shared_ptr<NET_IO> net_io_ = nullptr;
  msg_id_t msg_id_;
  int my_party_id_ = 0;
  int peer_party_id_ = -1;

 public:
  ZKNetIO(shared_ptr<NET_IO> net_io, string msgid_tag = "") : net_io_(net_io) {
    msg_id_ = msg_id_t("zk-" + msgid_tag);
    my_party_id_ = net_io_->GetCurrentPartyId();
    peer_party_id_ = 1 - my_party_id_;
  }

  void sync() { net_io_->sync_with(msg_id_); }
  void flush() {}
  void send_data_internal(const void* data, int len) {
    net_io_->send(peer_party_id_, (const char*)data, len, msg_id_);
  }
  void recv_data_internal(void* data, int len) {
    net_io_->recv(peer_party_id_, (char*)data, len, msg_id_);
  }
};
typedef BoolIO<ZKNetIO> ZK_NET_IO;
