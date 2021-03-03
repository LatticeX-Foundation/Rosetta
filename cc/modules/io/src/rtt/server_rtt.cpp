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
#include "cc/modules/common/include/utils/perf_stats.h"
#include <chrono>
#include <iostream>
using namespace std;
using namespace std::chrono;

#if !USE_LIBEVENT_AS_BACKEND
namespace rosetta {
namespace io {
//#define EPOLL_EVENTS (EPOLLIN | EPOLLERR)
#define EPOLL_EVENTS (EPOLLIN | EPOLLERR | EPOLLET)

void handleInterrupt(int sig) { cout << "Ctrl C" << endl; }
namespace {

void epoll_add(int efd, Connection* conn) {
  struct epoll_event ev;
  memset(&ev, 0, sizeof(ev));
  ev.events = conn->events_;
  ev.data.ptr = conn;
  int r = epoll_ctl(efd, EPOLL_CTL_ADD, conn->fd_, &ev);
  if (r != 0) {
    log_error << "epoll_ctl add failed. errno:" << errno << " " << strerror(errno) << endl;
  }
}

void epoll_mod(int efd, Connection* conn) {
  struct epoll_event ev;
  memset(&ev, 0, sizeof(ev));
  ev.events = conn->events_;
  ev.data.ptr = conn;
  int r = epoll_ctl(efd, EPOLL_CTL_MOD, conn->fd_, &ev);
  if (r != 0) {
    log_error << "epoll_ctl mod failed. errno:" << errno << " " << strerror(errno) << endl;
  }
}

void epoll_del(int efd, Connection* conn) {
  struct epoll_event ev;
  memset(&ev, 0, sizeof(ev));
  ev.events = conn->events_;
  ev.data.ptr = conn;
  int r = epoll_ctl(efd, EPOLL_CTL_DEL, conn->fd_, &ev);
  if (r != 0) {
    log_error << "epoll_ctl del failed. errno:" << errno << " " << strerror(errno) << endl;
  }
}
} // namespace

int TCPServer::create_server(int port) {
  int ret = -1;
  int fd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
  if (fd < 0) {
    log_error << "create socket failed. errno:" << errno << " " << strerror(errno) << endl;
    return -1;
  }

  set_nonblocking(fd, 1);

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof addr);
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;

  // set options
  ret = set_reuseaddr(fd, option_reuseaddr() ? 1 : 0);
  ret = set_reuseport(fd, option_reuseport() ? 1 : 0);

  set_sendbuf(fd, default_buffer_size());
  set_recvbuf(fd, default_buffer_size());

  set_linger(fd);
  set_nodelay(fd, 1);

  ret = ::bind(fd, (struct sockaddr*)&addr, sizeof(struct sockaddr));
  if (ret != 0) {
    log_error << "bind to 0.0.0.0:" << port << " failed. errno:" << errno << " " << strerror(errno)
              << endl;
    return -1;
  }

  ret = ::listen(fd, 20);
  if (ret != 0) {
    log_error << "listen failed. errno:" << errno << " " << strerror(errno) << endl;
    return -1;
  }

  return fd;
}

void TCPServer::handle_accept(Connection* conn) {
  if (verbose_ > 1)
    cout << __FUNCTION__ << endl;

  struct sockaddr_in clt_addr;
  memset(&clt_addr, 0, sizeof(clt_addr));
  socklen_t clientlen = sizeof(clt_addr);
  int cfd = accept(listenfd_, (struct sockaddr*)&clt_addr, &clientlen);
  if (cfd == -1) {
    log_error << "accept failed. errno:" << errno << " " << strerror(errno) << endl;
    throw socket_exp("accept failed");
  }
  //cout << "accept cfd: " << cfd << endl;

  int tmpcid = -9999;
  // recv client id from client
  ssize_t ret = ::read(cfd, (char*)&tmpcid, 4);
  int cid = tmpcid ^ 0x1234;
  if (cid != 0 && cid != 1 && cid != 2) {
    log_warn << "not really client. cid:" << cid << endl;
    close(cfd);
    return;
  }

  if (ret != 4) {
    log_error << "recv cid[" << cid << "] tmpcid[" << tmpcid << "] from client failed. ret:" << ret
              << " errno:" << errno << " " << strerror(errno) << endl;
    throw socket_exp("server recv cid error");
  }
  log_info << "server receive from client cid:" << cid << " tmpcid:" << tmpcid << endl;

  set_sendbuf(cfd, default_buffer_size());
  set_recvbuf(cfd, default_buffer_size());
  set_nodelay(cfd, 1);

  Connection* tc = nullptr;
  if (is_ssl_socket_)
    tc = new SSLConnection(cfd, EPOLL_EVENTS, true);
  else
    tc = new Connection(cfd, EPOLL_EVENTS, true);

  tc->ctx_ = ctx_;

  set_nonblocking(cfd, true);
  {
    unique_lock<mutex> lck(connections_mtx_);
    connections_[cid] = tc;
  }
  epoll_add(epollfd_, tc);
  epoll_mod(epollfd_, listen_conn_);
}

void TCPServer::handle_error(Connection* conn) {
  log_info << __FUNCTION__ << " fd:" << conn->fd_ << " errno:" << errno << " " << strerror(errno)
           << endl;
  {
    //! double check
    struct tcp_info info;
    int len = sizeof(info);
    getsockopt(conn->fd_, IPPROTO_TCP, TCP_INFO, &info, (socklen_t*)&len);
    log_info << __FUNCTION__ << " fd:" << conn->fd_
             << " tcp_info.tcpi_state: " << to_string(info.tcpi_state) << endl;
    if (info.tcpi_state == TCP_CLOSE) {
      ssize_t len = conn->readImpl(conn->fd_, main_buffer_, 8192);
      if (len > 0) { // Normal
        conn->buffer_->write(main_buffer_, len);
      }

      epoll_del(epollfd_, conn);
      conn->close();
    }
  }
}

void TCPServer::handle_write(Connection* conn) { cout << __FUNCTION__ << endl; }

void TCPServer::handle_read(Connection* conn) {
  if (conn->fd_ == listenfd_) {
    handle_accept(conn);
    return;
  }
  if (verbose_ > 1)
    cout << __FUNCTION__ << endl;

  if ((conn->state_ == Connection::State::Closing) || (conn->state_ == Connection::State::Closed)) {
    log_debug << "Closing or Closed." << endl;
    return;
  }

  // handle data read
  if (!conn->handshake()) {
    log_error << "server hadshake error" << endl;
    return;
  }

  while (true) {
    ssize_t len = conn->readImpl(conn->fd_, main_buffer_, 8192);
    if (len > 0) { // Normal
      conn->buffer_->write(main_buffer_, len);
    } else if (len == 0) { // EOF
      if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
        continue;
      }

      if (verbose_ > 1)
        cout << "fd: " << conn->fd_ << " server read 0 , client close. errno: " << errno << endl;

      epoll_del(epollfd_, conn);
      conn->close();
      return;
    } else { // <0, Error or Interrupt
      if (errno == EINTR) {
        continue;
      }
      if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
        break;
      }
      break;
    }
  }
  if (verbose_ > 2) {
    cout << __FUNCTION__ << " END  buffer.size() " << conn->buffer_->size() << endl;
  }
}

int timeout_counter = 0;
void TCPServer::loop_once(int efd, int waitms) {
  const int kMaxEvents = 20;
  struct epoll_event activeEvs[kMaxEvents];
  int nfds = epoll_wait(efd, activeEvs, kMaxEvents, waitms);
  if (nfds == 0) {
    timeout_counter++;
    if (verbose_ > 1)
      cout << "epoll timeout" << endl;
    return;
  } else if (nfds < 0) {
    log_error << "epoll error!" << endl;
    return;
  }

  timeout_counter = 0;
  for (int i = 0; i < nfds; i++) {
    Connection* conn = (Connection*)activeEvs[i].data.ptr;
    int events = activeEvs[i].events;

    if (events & EPOLLERR) {
      handle_error(conn);
    } else if (events & EPOLLIN) {
      handle_read(conn);
    } else if (events & EPOLLOUT) {
      handle_write(conn);
    } else {
      log_error << "unknown events " << events << endl;
    }
  }
}
void TCPServer::loop_main() {
  running_ = true;
  int64_t timeout = -1;
  if (timeout < 0)
    timeout = 1000 * 1000000;
  timeout += wait_timeout_;
  int64_t elapsed = 0;
  auto beg = system_clock::now();

  /**
   * check if all clients have connected to this server
   * if timeout, break
   * if all has connected, break
   */
  bool all_has_connected_to_server = true;
  while (!stop_ && (elapsed <= timeout)) {
    all_has_connected_to_server = true;
    loop_once(epollfd_, 1000);
    {
      unique_lock<mutex> lck(connections_mtx_);
      for (int i = 0; i < expected_cids_.size(); i++) {
        auto iter = connections_.find(expected_cids_[i]);
        if (iter == connections_.end()) {
          all_has_connected_to_server = false;
          break;
        }
      }
    }
    if (all_has_connected_to_server)
      break;
    auto end = system_clock::now();
    elapsed = duration_cast<duration<int64_t, std::milli>>(end - beg).count();
    all_has_connected_to_server = false;
    log_debug << "server .... timeout:" << timeout << " elapsed:" << elapsed << endl;
  }

  ::close(listenfd_); // close listen fd

  if (all_has_connected_to_server) {
    while (!stop_) {
      loop_once(epollfd_, 1000);
    }
  } else {
    log_info << "client(s) connect to this server timeout, wait for closing..." << endl;
  }

  bool all_closed = false;
  int retries = 10;
  while (!all_closed && (--retries > 0)) {
    log_info << "the server will stop soon ... " << retries << endl;
    all_closed = true;
    for (auto& c : connections_) {
      if (c.second != nullptr) {
        log_debug << "the server will stop soon ... " << retries << ", c.first:" << c.first
                  << ",c.second->state_:" << c.second->state_ << endl;
        if (c.second->state_ != Connection::State::Closed) {
          all_closed = false;
          break;
        }
      }
    }
    if (retries <= 0)
      all_closed = true;
    if (!all_closed) {
      loop_once(epollfd_, 1000);
    }
  }

  running_ = false;
}

bool TCPServer::init() {
  // 0
  //signal(SIGINT, handleInterrupt);

  // 1
  if ((epollfd_ = epoll_create1(EPOLL_CLOEXEC)) < 0) {
    log_error << "epoll_create1 failed. errno:" << errno << " " << strerror(errno) << endl;
    return false;
  }

  // 2
  if ((listenfd_ = create_server(port_)) < 0) {
    return false;
  }

  // 3
  if (is_ssl_socket_)
    listen_conn_ = new SSLConnection(listenfd_, EPOLL_EVENTS, true);
  else
    listen_conn_ = new Connection(listenfd_, EPOLL_EVENTS, true);

  // 4
  epoll_add(epollfd_, listen_conn_);

  // 5
  loop_thread_ = thread(&TCPServer::loop_main, this);

  return true;
}

bool TCPServer::start(int port, int64_t timeout) {
  port_ = port;
  if (!init_ssl())
    return false;

  if (!init())
    return false;

  stoped_ = false;
  return true;
}
void TCPServer::as_server() {
  //! @todo not supported for now
  std::unique_lock<std::mutex> lock(init_mtx_);
  init_cv_.wait(lock, [this]() { return !running_; });
  init_cv_.wait_for(lock, std::chrono::seconds(1), []() { return false; });
}

bool TCPServer::stop() {
  if (stoped_)
    return true;
  stoped_ = true;

  stop_ = 1;
  loop_thread_.join();

  for (auto& c : connections_) {
    if (c.second != nullptr) {
      if (c.second->state_ != Connection::State::Closed) {
        c.second->close();
        //cout << "in fact, never enter here!" << endl;
      }
      delete c.second;
      c.second = nullptr;
    }
  }
  connections_.clear();

  // 6
  delete listen_conn_;
  listen_conn_ = nullptr;
  ::close(listenfd_);

  ::close(epollfd_);

  log_debug << "server stopped!" << endl;
  return true;
}

} // namespace io
} // namespace rosetta
#endif
