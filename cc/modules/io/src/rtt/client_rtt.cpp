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
#include <chrono>
#include <thread>
using namespace std::chrono;

#if !USE_LIBEVENT_AS_BACKEND
namespace rosetta {
namespace io {

bool TCPClient::connect(int64_t timeout) {
  if (!init_ssl())
    return false;

  ip_ = Socket::gethostip(host_);
  if (ip_ == "") {
    log_error << "Can not get right IP by " << host_ << endl;
    return false;
  }

  log_info << "client[" << cid_ << "] is ready to connect to server[" << ip_ << ":" << port_ << "]"
           << endl;

  ///////////////////////////////////////////
  //! @todo retries 3 times default
  int conn_retries = 3;
  if (conn_retries <= 0)
    conn_retries = 1;
  for (int k = 0; k < conn_retries; k++) {
    struct sockaddr_in server;

    fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (fd_ < 0) {
      log_warn << "client create socket failed" << endl;
      continue;
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(ip_.c_str());
    server.sin_port = htons(port_);

    int err = -1;

    set_sendbuf(fd_, default_buffer_size());
    set_recvbuf(fd_, default_buffer_size());
    set_nodelay(fd_, 1);
    set_linger(fd_);

    // timeout = netutil::client_init_timeout();
    timeout = -1;
    if (timeout < 0)
      timeout = 1000 * 1000000; // 100w s

    //! in vm, connect api is not in blocking, even under blocking mode!!!
    int64_t elapsed = 0;
    auto beg = system_clock::now();
    connected_ = false;
    while ((timeout < 0) || (elapsed <= timeout)) {
      if (timeout > 0) {
        int64_t tout = timeout - elapsed;
        set_send_timeout(fd_, tout);
      } else {
        break;
      }

      err = ::connect(fd_, (struct sockaddr*)&server, sizeof(server));
      if (err == 0) {
        connected_ = true;
        break;
      }
      // #define	EISCONN		106	/* Transport endpoint is already connected */
      if (errno == EISCONN) {
        connected_ = true;
        break;
      }

      // #define	ECONNREFUSED	111	/* Connection refused */
      if (errno == ECONNREFUSED) {
        //! continue;
      }
      std::this_thread::sleep_for(chrono::milliseconds(500));
      auto end = system_clock::now();
      elapsed = duration_cast<duration<int64_t, std::milli>>(end - beg).count();
    }

    if (!connected_) {
      if (elapsed > timeout) {
        log_warn << "client[" << cid_ << "] connect to server[" << ip_ << ":" << port_
                 << "] timeout." << endl;
        ::close(fd_);
        continue;
      }
      log_error << "client[" << cid_ << "] connect to server[" << ip_ << ":" << port_ << "] failed."
                << endl;
      ::close(fd_);
      continue;
    }

    // send client id to client
    int tmpcid = cid_ ^ 0x1234;
    log_info << "client will send cid:" << cid_ << " tmpcid:" << tmpcid << " to server" << endl;
    ssize_t ret = ::write(fd_, (const char*)&tmpcid, 4);
    if (ret != 4) {
      log_error << "client send cid error. ret:" << ret << ", errno:" << errno << endl;
      ::close(fd_);
      continue;
    }
    if (verbose_ > 0) {
      cout << "client send cid[" << cid_ << "] to server[" << sid_ << "]"
           << " errno:" << errno << endl;
    }

    if (is_ssl_socket_)
      conn_ = new SSLConnection(fd_, 0, false);
    else
      conn_ = new Connection(fd_, 0, false);
    conn_->ctx_ = ctx_;

    // set_send_timeout(fd_, netutil::recv_o_send_timeout());
    // set_recv_timeout(fd_, netutil::recv_o_send_timeout());

    set_nonblocking(fd_, true);
    if (conn_->handshake()) {
      return true;
    }

    delete conn_;
    conn_ = nullptr;

    int64_t interval = 1000;
    if (timeout > elapsed) {
      interval += timeout - elapsed;
    }
    if (interval > 5000) {
      interval = 5000;
    }
    log_warn << "client[" << cid_ << "] will retry connect to server[" << ip_ << ":" << port_
             << "]. retries:" << k << ", at " << interval << "(ms) later." << endl;
    std::this_thread::sleep_for(chrono::milliseconds(interval));
  }
  ///////////////////////////////////////////
  return false;
}

void TCPClient::close() {
  if (connected_) {
    log_debug << getpid() << " " << cid_ << " client closing." << conn_ << endl;
    connected_ = false;

    delete conn_;
    conn_ = nullptr;
    log_debug << getpid() << " " << cid_ << " client closed." << conn_ << endl;
  }
}

} // namespace io
} // namespace rosetta
#endif
