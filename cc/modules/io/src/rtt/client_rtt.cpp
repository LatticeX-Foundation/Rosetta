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

#include "cc/modules/io/include/internal/client.h"
#include "cc/modules/common/include/utils/helper.h"

#if !USE_LIBEVENT_AS_BACKEND
namespace rosetta {
namespace io {
bool TCPClient::connect(int64_t timeout) {
  struct sockaddr_in server;
  int n = 0;

  fd_ = socket(AF_INET, SOCK_STREAM, 0);

  memset(&server, 0, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr(ip_.c_str());
  server.sin_port = htons(port_);

  log_debug << "client[" << cid_ << "] connect to server[" << ip_ << ":" << port_ << "]" << endl;
  int err = -1;

  set_sendbuf(fd_, default_buffer_size());
  set_recvbuf(fd_, default_buffer_size());
  set_nodelay(fd_, 1);
  set_linger(fd_);

  while (1) {
    ::connect(fd_, (struct sockaddr*)&server, sizeof(server));
    if (err < 0) {
      // #define	ECONNREFUSED	111	/* Connection refused */
      if (errno == ECONNREFUSED) {
        usleep(10000);
        continue;
      }
      // #define	EISCONN		106	/* Transport endpoint is already connected */
      if (errno == EISCONN) {
        break;
      }
      usleep(10000);
      //cout << "connect err:" << err << " errno:" << errno << endl;
    } else {
      break;
    }
  }
  connected_ = true;

  if (is_ssl_socket_)
    conn_ = new SSLConnection(fd_, 0, false);
  else
    conn_ = new Connection(fd_, 0, false);
  conn_->ctx_ = ctx_;

  // send client id to client
  int ret = ::write(fd_, (const char*)&cid_, 4);
  if (ret != 4) {
    cerr << "client send cid error ::write(fd_, (const char*)&cid_, 4);" << endl;
    exit(0);
  }
  if (verbose_ > 0) {
    cout << "client send cid[" << cid_ << "] to server[" << sid_ << "]"
         << " errno:" << errno << endl;
  }

  set_nonblocking(fd_, true);
  conn_->handshake();

  return true;
}

void TCPClient::close() {
  if (connected_) {
    log_debug << "client closing." << endl;
    connected_ = false;

    delete conn_;
    conn_ = nullptr;
    ::close(fd_);
    log_debug << "client closed." << endl;
  }
}
} // namespace io
} // namespace rosetta
#endif
