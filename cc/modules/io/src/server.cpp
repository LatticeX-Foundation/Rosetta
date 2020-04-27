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
#include "internal/server.h"

namespace rosetta {
namespace io {
Connection* TCPServer::find_connection(int cid, int64_t& timeout) {
  if (timeout < 0) {
    timeout = 999999999999L;
  }

  using namespace std::chrono;
  auto time_beg = system_clock::now();
  Connection* conn = nullptr;
  int icounter = 0;
  int64_t elapsed = 0;
  do {
    icounter++;
    conn = find_connection(cid);
    if (conn != nullptr)
      break;

    std::this_thread::sleep_for(milliseconds(100));

    auto time_end = system_clock::now();
    elapsed = duration_cast<duration<int64_t, std::milli>>(time_end - time_beg).count();

    if ((icounter % 100 == 0) || (verbose_ > 1)) {
      std::cout << "receive data from cid[" << cid << "] counter:" << icounter << std::endl;
    }
  } while (elapsed < timeout);
  if (verbose_ > 2) {
    std::cout << "receive timeout:" << timeout << ", elapsed:" << elapsed << std::endl;
  }
  if (conn == nullptr) {
    std::cerr << "receive cannot find connection id" << std::endl;
  }

  timeout = timeout - elapsed;
  if (timeout < 0)
    timeout = 0;

  return conn;
}

Connection* TCPServer::find_connection(int cid) {
  unique_lock<mutex> lck(connections_mtx_);
  auto iter = connections_.find(cid);
  if (iter == connections_.end()) {
    return nullptr;
  }
  return iter->second;
}

/**
 * @todo not completed supports server's send at present
 */
size_t TCPServer::send(int cid, const char* data, size_t len, int64_t timeout) {
  if (verbose_ > 3)
    cout << "cid:" << cid << " send 1" << endl;

  auto conn = find_connection(cid, timeout);
  if (conn == nullptr)
    return 0;

  if (verbose_ > 3)
    cout << "cid:" << cid << " send 2" << endl;

  int ret = conn->send(data, len);
  return ret;
}

size_t TCPServer::recv(int cid, char* data, size_t len, int64_t timeout) {
  if (timeout < 0) {
    timeout = 999999999999L;
  }

  if (verbose_ > 3)
    cout << "cid:" << cid << " recv 1" << endl;

  auto conn = find_connection(cid, timeout);
  if (conn == nullptr)
    return 0;

  if (verbose_ > 3)
    cout << "cid:" << cid << " recv 2" << endl;

  int ret = conn->recv(data, len, timeout);
  if (ret != len) {
    cerr << "cid:" << cid << " ret != len " << ret << " != " << len << endl;
    throw;
  }
  if (verbose_ > 3)
    cout << "cid:" << cid << " recv 3" << endl;
  return ret;
}

/**
 * @todo not completed supports server's send at present
 */
size_t TCPServer::send(
  int cid, const msg_id_t& msg_id, const char* data, size_t len, int64_t timeout) {
  auto conn = find_connection(cid, timeout);
  if (conn == nullptr)
    return 0;

  //! @todo: add a mutex here
  conn->send(msg_id.data(), msg_id_t::Size());
  int ret = conn->send(data, len);
  return ret;
}
size_t TCPServer::recv(int cid, const msg_id_t& msg_id, char* data, size_t len, int64_t timeout) {
  if (timeout < 0) {
    timeout = 999999999999L;
  }

  if (verbose_ > 3)
    cout << "msgid: " << msg_id << " cid:" << cid << " recv 1" << endl;

  auto conn = find_connection(cid, timeout);
  if (conn == nullptr)
    return 0;

  if (verbose_ > 3)
    cout << "msgid " << msg_id << " cid:" << cid << " recv 2" << endl;

  int ret = conn->recv(msg_id, data, len, timeout);
  if (ret != len) {
    cerr << "msgid " << msg_id << " cid:" << cid << " ret != len " << ret << " != " << len << endl;
    throw;
  }
  if (verbose_ > 3)
    cout << "msgid " << msg_id << " cid:" << cid << " recv 3" << endl;
  return ret;
}
} // namespace io
} // namespace rosetta