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
#include "comm.h"
#include "msg_id.h"
#include "socket.h"
#include "cycle_buffer.h"

namespace rosetta {
namespace io {

struct Connection {
 public:
  Connection(int _fd, int _events, bool _is_server);
  virtual ~Connection() {}

 public:
  virtual void handshake() {}
  bool is_server() const {
    return is_server_;
  }

 public:
  size_t send(const char* data, size_t len, int64_t timeout = -1L);
  size_t recv(char* data, size_t len, int64_t timeout = -1L);
  size_t recv(const msg_id_t& msg_id, char* data, size_t len, int64_t timeout = -1L);

  // Read & Write
 public:
  ssize_t peek(int sockfd, void* buf, size_t len);
  int readn(int connfd, char* vptr, int n);
  int writen(int connfd, const char* vptr, size_t n);

  virtual ssize_t readImpl(int fd, char* data, size_t len) {
    int ret = ::read(fd, data, len);
    return ret;
  }
  virtual ssize_t writeImpl(int fd, const char* data, size_t len) {
    int ret = ::write(fd, data, len);
    return ret;
  }

 public:
  enum State {
    Invalid = 1,
    Handshaking,
    Handshaked,
    Connecting,
    Connected,
    Closing,
    Closed,
    Failed,
  };
  State state_;

  int verbose_ = 0;
  int fd_ = -1;
  int events_ = 0;
  bool is_server_ = false;
  string client_ip_ = "";
  int client_port_ = 0;

  //! buffer manage
  //! for all messages
  shared_ptr<cycle_buffer> buffer_ = nullptr;
  //! for one message which id is msg_id_t
  map<msg_id_t, shared_ptr<cycle_buffer>> mapbuffer_;
  mutex mapbuffer_mtx_;

#if USE_LIBEVENT_AS_BACKEND
  bool has_set_client_id = false;
  mutex set_client_id_mtx_;
  void* thread_ptr_ = nullptr; // do not delete this pointer in this class
  void* obj_ptr_ = nullptr; // do not delete this pointer in this class
  struct bufferevent* bev_ = nullptr; // buffevent, do not delete this pointer in this class
#endif

  SSL_CTX* ctx_ = nullptr; // do not delete this pointer in this class
};

class SSLConnection : public Connection {
  using Connection::Connection;
  SSL* ssl_ = nullptr;
  mutex ssl_rw_mtx_;

 public:
  ~SSLConnection();
#if USE_LIBEVENT_AS_BACKEND
  virtual void handshake() {}
#else
  virtual void handshake();
  virtual ssize_t readImpl(int fd, char* data, size_t len) {
    int rd = 0;
    {
      unique_lock<mutex> lck(ssl_rw_mtx_);
      rd = SSL_read(ssl_, data, len);
      int ssle = SSL_get_error(ssl_, rd);
      if (rd < 0 && ssle != SSL_ERROR_WANT_READ) {
        cerr << "ssl readImpl error:" << errno << endl;
      }
    }
    return rd;
  }
  virtual ssize_t writeImpl(int fd, const char* data, size_t len) {
    int wd = 0;
    {
      unique_lock<mutex> lck(ssl_rw_mtx_);
      wd = SSL_write(ssl_, data, len);
      int ssle = SSL_get_error(ssl_, wd);
      if (wd < 0 && ssle != SSL_ERROR_WANT_WRITE) {
        cerr << "ssl writeImpl error:" << errno << endl;
      }
    }
    return wd;
  }
#endif
};

} // namespace io
} // namespace rosetta
