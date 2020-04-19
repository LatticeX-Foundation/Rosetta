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
#include "internal/opsets_base.h"
#if USE_NETIO_WITH_MESSAGEID
extern rosetta::io::ParallelIO* g_io;
#else
extern rosetta::io::IO* g_io;
#endif

#include "internal/opsets.h"
namespace rosetta {
namespace mpc {

void OpBase_::sendBuf(int player, const char* buf, int length, int conn /* = 0*/) {
  message_sent_.fetch_add(1);
  bytes_sent_.fetch_add(length);

#if OPEN_MPCOP_DEBUG_AND_CHECK
  {
    string key(msg_id().str());
    unique_lock<mutex> lck(g_key_stat_mtx);
    if (g_key_stat.find(key) != g_key_stat.end()) {
      g_key_stat[key]->message_sent.fetch_add(1);
      g_key_stat[key]->bytes_sent.fetch_add(length);
    }
  }
#endif

#if USE_NETIO_WITH_MESSAGEID
  g_io->send(player, buf, length, msg_id());
#else
  g_io->send(player, buf, length, 0);
#endif
}

void OpBase_::receiveBuf(int player, char* buf, int length, int conn /* = 0*/) {
  message_received_.fetch_add(1);
  bytes_received_.fetch_add(length);

#if OPEN_MPCOP_DEBUG_AND_CHECK
  {
    string key(msg_id().str());
    unique_lock<mutex> lck(g_key_stat_mtx);
    if (g_key_stat.find(key) != g_key_stat.end()) {
      g_key_stat[key]->message_received.fetch_add(1);
      g_key_stat[key]->bytes_received.fetch_add(length);
    }
  }
#endif

#if USE_NETIO_WITH_MESSAGEID
  g_io->recv(player, buf, length, msg_id());
#else
  g_io->recv(player, buf, length, 0);
#endif
}

void OpBase_::synchronize(int length) {
#if USE_NETIO_WITH_MESSAGEID
  cerr << "error! please use void OpBase_::synchronize(const msg_id_t& msg_id)" << endl;
  throw;
#endif
  g_io->sync();
}
void OpBase_::synchronize(const msg_id_t& msg_id) {
  g_io->sync_with(msg_id);
}

} // namespace mpc
} // namespace rosetta
