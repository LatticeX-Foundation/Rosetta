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
#include "cc/modules/io/include/internal/server.h"

namespace rosetta {
namespace io {
Connection* TCPServer::find_connection(int cid, int64_t& timeout) {
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
      log_info << "receive of find_connection cid[" << cid << "] counter:" << icounter << std::endl;
    }
  } while (elapsed < timeout);
  if (verbose_ > 2) {
    log_info << "receive of find_connection timeout:" << timeout << ", elapsed:" << elapsed
             << std::endl;
  }
  if (conn == nullptr) {
    log_warn << "receive cannot find connection id" << std::endl;
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
ssize_t TCPServer::send(int cid, const char* data, size_t len, int64_t timeout) {
  if (verbose_ > 3)
    cout << "cid:" << cid << " send 1" << endl;

  auto conn = find_connection(cid, timeout);
  if (conn == nullptr) {
    log_error << "TCPServer send cannot find connection!" << endl;
    return -1;
  }

  if (verbose_ > 3)
    cout << "cid:" << cid << " send 2" << endl;

  ssize_t ret = conn->send(data, len);
  return ret;
}

ssize_t TCPServer::recv(int cid, char* data, size_t len, int64_t timeout) {
  if (timeout < 0)
    timeout = 1000 * 1000000;

  if (verbose_ > 3)
    cout << "cid:" << cid << " recv 1" << endl;

  auto conn = find_connection(cid, timeout);
  if (conn == nullptr) {
    log_error << "TCPServer recv cannot find connection!" << endl;
    return -1;
  }

  if (verbose_ > 3)
    cout << "cid:" << cid << " recv 2" << endl;

  ssize_t ret = conn->recv(data, len, timeout);
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
ssize_t TCPServer::send(
  int cid,
  const msg_id_t& msg_id,
  const char* data,
  size_t len,
  int64_t timeout) {
  auto conn = find_connection(cid, timeout);
  if (conn == nullptr) {
    log_error << "TCPServer send cannot find connection!" << endl;
    return -1;
  }

  //! @todo: add a mutex here
  conn->send(msg_id.data(), msg_id_t::Size());
  ssize_t ret = conn->send(data, len);

  return ret;
}
ssize_t TCPServer::recv(int cid, const msg_id_t& msg_id, char* data, size_t len, int64_t timeout) {
  if (timeout < 0)
    timeout = 1000 * 1000000;

  if (verbose_ > 3)
    cout << "msgid: " << msg_id << " cid:" << cid << " recv 1" << endl;

  auto conn = find_connection(cid, timeout);
  if (conn == nullptr) {
    log_error << "TCPServer recv2 cannot find connection!" << endl;
    return -1;
  }

  if (verbose_ > 3)
    cout << "msgid " << msg_id << " cid:" << cid << " recv 2" << endl;

  ssize_t ret = conn->recv(msg_id, data, len, timeout);
  if (ret != len) {
    // if (stop_)
    //   return len;
    string errmsg = "TCPServer connection recv error. msgid: " + msg_id.str() +
      " cid:" + to_string(cid) + " expected:" + to_string(len) + " but got:" + to_string(ret);
    if (ret == E_TIMEOUT) {
      errmsg =
        "TCPServer connection recv timeout. msgid: " + msg_id.str() + " cid:" + to_string(cid);
    } else if (ret == E_UNCONNECTED) {
      errmsg = "TCPServer connection have not connected. msgid: " + msg_id.str() +
        " cid:" + to_string(cid);
    }
    log_error << errmsg << endl;
    return -1;
  }

  if (verbose_ > 3)
    cout << "msgid " << msg_id << " cid:" << cid << " recv 3" << endl;

  return ret;
}
} // namespace io
} // namespace rosetta