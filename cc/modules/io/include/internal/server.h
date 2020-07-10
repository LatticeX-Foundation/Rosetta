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

class TCPServer : public Socket {
 public:
  TCPServer() {
    main_buffer_ = new char[1024 * 1024 * 2];
  }
  virtual ~TCPServer() {
    stop();
    delete[] main_buffer_;
  }

 public:
  bool start(int port, int64_t timeout = -1L);
  void as_server();
  bool stop();

 public:
  size_t send(int cid, const char* data, size_t len, int64_t timeout = -1L);
  size_t recv(int cid, char* data, size_t len, int64_t timeout = -1L);

  size_t send(int cid, const msg_id_t& msg_id, const char* data, size_t len, int64_t timeout = -1L);
  size_t recv(int cid, const msg_id_t& msg_id, char* data, size_t len, int64_t timeout = -1L);

  /**
   * about certifications
   */
  virtual bool init_ssl() {
    return true;
  }
  void set_server_cert(string server_cert) {
    server_cert_ = server_cert;
  }
  void set_server_prikey(string server_prikey, string password = "") {
    server_prikey_ = server_prikey;
    server_prikey_password_ = password;
  }

 protected:
  Connection* find_connection(int cid);
  Connection* find_connection(int cid, int64_t& timeout);

 protected:
#if !USE_LIBEVENT_AS_BACKEND
  bool init();
  int create_server(int port);
  void loop_once(int efd, int waitms);
  void loop_main();

  void handle_accept(Connection* conn);
  void handle_read(Connection* conn);
  void handle_write(Connection* conn);
  void handle_error(Connection* conn);

 protected:
  Connection* listen_conn_ = nullptr;
  std::thread loop_thread_;
  int epollfd_ = -1;
  int listenfd_ = -1;
#endif

 protected:
  SSL_CTX* ctx_ = nullptr;
  std::mutex connections_mtx_;
  std::map<int, Connection*> connections_;
  char* main_buffer_ = nullptr;
  int port_ = 0;
  int stop_ = 0;
  bool stoped_ = false;

  string server_cert_;
  string server_prikey_;
  string server_prikey_password_;

  std::condition_variable init_cv_;
  std::mutex init_mtx_;

#if USE_LIBEVENT_AS_BACKEND
  ////////////////// event
 protected:
  std::mutex set_client_id_mtx_;
  struct event* ev_timer = nullptr;

  thread* thread_ = nullptr;
  thread_params* tps_ = nullptr;

 private:
  /// callbacks here
  static void onTimer(evutil_socket_t fd, short what, void* ctx);
  static void onTimeout(evutil_socket_t fd, short what, void* ctx);
  static void onSignal(evutil_socket_t sig, short what, void* ctx);

  static void onRead(struct bufferevent* bev, void* ctx);
  static void onWrite(struct bufferevent* bev, void* ctx);
  static void onEvent(struct bufferevent* bev, short what, void* ctx);
  static void onAccept(
    struct evconnlistener* listener, evutil_socket_t fd, struct sockaddr* address, int socklen,
    void* ctx);
#endif
};

class SSLServer : public TCPServer {
  using TCPServer::TCPServer;

 public:
  virtual bool init_ssl();
  SSLServer();
  virtual ~SSLServer();
};

} // namespace io
} // namespace rosetta
