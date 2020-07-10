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

#include "cc/modules/io/include/internal/comm.h"

#include "cc/modules/io/include/internal/connection.h"
#include "cc/modules/io/include/internal/socket.h"
#include "cc/modules/io/include/internal/msg_id.h"

namespace rosetta {
namespace io {

class TCPClient : public Socket {
 public:
  TCPClient(const std::string& ip, int port) : ip_(ip), port_(port) {}
  ~TCPClient() {
    close();
  }

 public:
  bool connected() const {
    return connected_;
  }
  void setcid(int cid) {
    cid_ = cid;
  }
  void setsid(int sid) {
    sid_ = sid;
  }

 public:
  /**
   * \param timeout ms
   */
  bool connect(int64_t timeout = -1L);
  void close();

 public:
  /**
   * \param timeout ms
   */
  size_t send(const char* data, size_t len, int64_t timeout = -1L);
  size_t recv(char* data, size_t len, int64_t timeout = -1L);

 protected:
  string ip_ = "";
  int port_ = 0;
  int fd_ = -1;
  bool connected_ = false;
  Connection* conn_ = nullptr;

  int cid_ = 0; // client id
  int sid_ = 0; // server id (connect to)

  SSL_CTX* ctx_ = nullptr;
  SSL* ssl_ = nullptr;

#if USE_LIBEVENT_AS_BACKEND
  // only for libevent
 private:
  void send_client_id() {
    cout << "send client id [" << cid_ << "]" << endl;
    send((const char*)&cid_, sizeof(cid_));
  }

  int tcp_connect_to_server(const string& ip, int port);

 private:
  /// callbacks here
  static void onTimeout(evutil_socket_t fd, short what, void* ctx);
  static void onRead(struct bufferevent* bev, void* ctx);
  static void onWrite(struct bufferevent* bev, void* ctx);
  static void onEvent(struct bufferevent* bev, short what, void* ctx);

 protected:
  bool pre_exit = false;
  bool reconnect_ = true;

  thread* thread_ = nullptr;
  thread_params* tps_ = nullptr;

  std::condition_variable init_cv_;
  std::mutex init_mtx_;
#endif
};

class SSLClient : public TCPClient {
  using TCPClient::TCPClient;

 public:
  SSLClient(const std::string& ip, int port);
  ~SSLClient();
};

} // namespace io
} // namespace rosetta