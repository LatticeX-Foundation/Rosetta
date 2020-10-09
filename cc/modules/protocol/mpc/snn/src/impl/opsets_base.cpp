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
#include "cc/modules/protocol/mpc/snn/include/opsets_base.h"
#include "cc/modules/protocol/mpc/snn/include/snn_opsets.h"

namespace rosetta {
namespace snn {
 void OpBase_::sendBitVector(const vector<small_mpc_t>& vec, size_t player, size_t size) {
    int len = (size + 7) / 8;
    unsigned char* ptr = new unsigned char[len];
    memset(ptr, 0, len);
    int i = 0;
    for (int j = 0; j < len; j++) {
      for (int k = 0; k < 8; k++) {
        if (i < size) {
          ptr[j] = (((vec[i++] & 0x1) << k) & 0xFF) | ptr[j];
        }
      }
    }
    int ret = io->send(player, (const char*)ptr, len, msg_id());
    delete[] ptr;
 }

void OpBase_::receiveTwoBitVector(vector<small_mpc_t>& vec_a,
                                vector<small_mpc_t>& vec_b,
                                size_t player, size_t size_a, size_t size_b) {
    vector<small_mpc_t> temp(size_a + size_b);
    receiveBitVector(temp, player, size_a + size_b);

    for (size_t i = 0; i < size_a; ++i)
      vec_a[i] = temp[i];

    for (size_t i = 0; i < size_b; ++i)
      vec_b[i] = temp[size_a + i];
 }
 

 void OpBase_::sendTwoBitVector(const vector<small_mpc_t>& vec_a,
                                const vector<small_mpc_t>& vec_b,
                                size_t player, size_t size_a, size_t size_b) {
    vector<small_mpc_t> temp(size_a + size_b);
    for (size_t i = 0; i < size_a; ++i)
      temp[i] = vec_a[i];
    for (size_t i = 0; i < size_b; ++i)
      temp[size_a + i] = vec_b[i];
    
    sendBitVector(temp, player, size_a + size_b);
 }

 void OpBase_::receiveBitVector(vector<small_mpc_t>& vec, size_t player, size_t size) {
    int len = (size + 7) / 8;
    unsigned char* ptr = new unsigned char[len];
    memset(ptr, 0, len);
    int ret = io->recv(player, (char*)ptr, len, msg_id());

    int i = 0;
    for (int j = 0; j < len; j++) {
      for (int k = 0; k < 8; k++) {
        if (i < size) {
          vec[i++] = (ptr[j] >> k) & 0x01;
        }
      }
    }
    delete[] ptr;
 }

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
  io->send(player, buf, length, msg_id());
#else
  io->send(player, buf, length, 0);
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
  io->recv(player, buf, length, msg_id());
#else
  io->recv(player, buf, length, 0);
#endif
}

void OpBase_::synchronize(int length) {
#if USE_NETIO_WITH_MESSAGEID
  cerr << "error! please use void OpBase_::synchronize(const msg_id_t& msg_id)" << endl;
  throw;
#endif
  io->sync();
}
void OpBase_::synchronize(const msg_id_t& msg_id) { io->sync_with(msg_id); }

} // namespace snn
} // namespace rosetta
