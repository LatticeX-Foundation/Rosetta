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

#include "cc/modules/common/include/utils/helper.h"
#include "cc/modules/common/include/utils/logger.h"
#include "cc/modules/io/include/internal/comm.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string>
#include <sys/types.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <signal.h>

#if USE_LIBEVENT_AS_BACKEND
#include <event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/event-config.h>
#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/http.h>
#include <event2/listener.h>
#include <event2/thread.h>
#include <event2/util.h>
#include <event2/bufferevent_ssl.h>
#endif

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

namespace rosetta {
namespace io {

/**
 * The base class of Server/Client
 */
class Socket {
 protected:
  int verbose_ = 0;

 public:
  Socket();
  virtual ~Socket() = default;

  // 1
 public:
  int set_nonblocking(int fd, bool value) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
      return errno;
    }
    if (value) {
      return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }
    return fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
  }

  //! Options
 protected:
  bool option_reuseaddr_ = true;
  bool option_reuseport_ = true;

 public:
  bool option_reuseaddr() const { return option_reuseaddr_; }
  bool option_reuseport() const { return option_reuseport_; }

 protected:
  int set_reuseaddr(int fd, int optval);
  int set_reuseport(int fd, int optval);
  int set_sendbuf(int fd, int size);
  int set_recvbuf(int fd, int size);
  size_t default_buffer_size_ = 1024 * 1024 * 10;
  size_t default_buffer_size() { return default_buffer_size_; }

 protected:
  int set_linger(int fd);
  int set_nodelay(int fd, int optval);

  //! Helpers
 protected:
  struct timeval gettv(int ms) {
    timeval t;
    t.tv_sec = ms / 1000;
    t.tv_usec = (ms % 1000) * 1000;
    return t;
  }

 protected:
  bool is_ssl_socket_ = false;
  bool running_ = false;
  bool is_running() { return running_; }
};

#if USE_LIBEVENT_AS_BACKEND
typedef struct thread_params_st {
  struct event_base* base = nullptr;
  struct bufferevent* bev = nullptr;
} thread_params;

void enable_event_log();
void enable_threads();
#endif

} // namespace io
} // namespace rosetta
