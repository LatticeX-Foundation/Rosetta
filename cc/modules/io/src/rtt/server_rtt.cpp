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
    cerr << "epoll_ctl add failed. errno:" << errno << " " << strerror(errno) << endl;
  }
}

void epoll_mod(int efd, Connection* conn) {
  struct epoll_event ev;
  memset(&ev, 0, sizeof(ev));
  ev.events = conn->events_;
  ev.data.ptr = conn;
  int r = epoll_ctl(efd, EPOLL_CTL_MOD, conn->fd_, &ev);
  if (r != 0) {
    cerr << "epoll_ctl mod failed. errno:" << errno << " " << strerror(errno) << endl;
  }
}

void epoll_del(int efd, Connection* conn) {
  struct epoll_event ev;
  memset(&ev, 0, sizeof(ev));
  ev.events = conn->events_;
  ev.data.ptr = conn;
  int r = epoll_ctl(efd, EPOLL_CTL_DEL, conn->fd_, &ev);
  if (r != 0) {
    cerr << "epoll_ctl del failed. errno:" << errno << " " << strerror(errno) << endl;
  }
}
} // namespace

int TCPServer::create_server(int port) {
  int ret = -1;
  int fd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
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
    cerr << "bind to 0.0.0.0:" << port << " failed. errno:" << errno << " " << strerror(errno)
         << endl;
  }

  ret = ::listen(fd, 20);
  if (ret != 0) {
    cerr << "listen failed. errno:" << errno << " " << strerror(errno) << endl;
  }
  log_debug << "fd " << fd << " listening at " << port << endl;

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
    perror("accept failed!");
    return;
  }
  //cout << "accept cfd: " << cfd << endl;

  set_sendbuf(cfd, default_buffer_size());
  set_recvbuf(cfd, default_buffer_size());
  set_nodelay(cfd, 1);

  Connection* tc = nullptr;
  if (is_ssl_socket_)
    tc = new SSLConnection(cfd, EPOLL_EVENTS, true);
  else
    tc = new Connection(cfd, EPOLL_EVENTS, true);

  tc->ctx_ = ctx_;

  // recv client id from client
  int cid = 0;
  //int ret = tc->tcpreadn(cfd, (char*)&cid, 4);
  int ret = ::read(cfd, (char*)&cid, 4);
  if (ret != 4) {
    cerr << "error conn_->tcpwriten(fd_, (const char*)&cid_, 4);" << endl;
    exit(0);
  }
  //cout << "receive from client cid:" << cid << endl;

  set_nonblocking(cfd, true);
  {
    unique_lock<mutex> lck(connections_mtx_);
    //cout << "ssid:" << tc->ssid_ << " register " << cid << endl;
    connections_[cid] = tc;
  }
  epoll_add(epollfd_, tc);
  epoll_mod(epollfd_, listen_conn_);
}
void TCPServer::handle_error(Connection* conn) {
  //cout << __FUNCTION__ << " errno:" << errno << endl;
  //epoll_del(epollfd_, conn);
  //close(conn->fd_);
}

void TCPServer::handle_write(Connection* conn) { cout << __FUNCTION__ << endl; }

void TCPServer::handle_read(Connection* conn) {
  if (conn->fd_ == listenfd_) {
    handle_accept(conn);
    return;
  }
  if (verbose_ > 1)
    cout << __FUNCTION__ << endl;

  // handle data read
  conn->handshake();

  int a = 0;
  while (true) {
    a++;
    ssize_t len = conn->readImpl(conn->fd_, main_buffer_, 8192);
    if (len > 0) { // Normal
      conn->buffer_->write(main_buffer_, len);
    } else if (len == 0) { // EOF
      if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
        // continue;
      }

      if (false) { // for double check
        struct tcp_info info;
        int len = sizeof(info);
        getsockopt(conn->fd_, IPPROTO_TCP, TCP_INFO, &info, (socklen_t*)&len);
        cout << "getsockopt info.tcpi_state: " << to_string(info.tcpi_state) << endl;
        if (info.tcpi_state == TCP_CLOSE) {
          cout << "info.tcpi_state == TCP_CLOSE" << endl;
        }
      }

      if (false) { // for double check
        sockaddr_in peer, local;
        socklen_t alen = sizeof(peer);
        int ret = getpeername(conn->fd_, (sockaddr*)&peer, &alen);
        cout << "getpeername ret:" << ret << ", errno:" << errno << endl;
        if (ret = -1 && errno == ENOTCONN) {
          cout << "ret = -1 && errno == ENOTCONN" << endl;
        }
      }

      if (verbose_ > 1)
        cout << "fd: " << conn->fd_ << " server read 0 , client close. errno: " << errno << endl;
      close(conn->fd_);
      return;
    } else { // Error or Interrupt
      if (errno == EINTR) {
        if (a % 1000 == 0) {
          cout << "if (errno == EINTR) {" << endl;
        }
        continue;
      }
      if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
        //continue;
        break;
      }
      cout << "fd: " << conn->fd_ << " server 2 read errno: " << errno << endl;
      break;
    }
  }
  if (verbose_ > 2) {
    cout << __FUNCTION__ << "      ........END " << a << " buffer.size() " << conn->buffer_->size()
         << endl;
  }
  epoll_mod(epollfd_, conn);
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
    perror("epoll error!");
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
      cerr << "unknown events " << events << endl;
    }
  }
}
void TCPServer::loop_main() {
  running_ = true;
  while (!stop_) {
    loop_once(epollfd_, 1000);
  }
  running_ = false;
}

bool TCPServer::init() {
  // -1
  signal(SIGINT, handleInterrupt);
  // 0

  // 1
  epollfd_ = epoll_create1(EPOLL_CLOEXEC);
  //cout << "server epollfd_:" << epollfd_ << endl;

  // 2
  listenfd_ = create_server(port_);
  //cout << "server listenfd:" << listenfd_ << endl;

  if (is_ssl_socket_)
    listen_conn_ = new SSLConnection(listenfd_, EPOLL_EVENTS, true);
  else
    listen_conn_ = new Connection(listenfd_, EPOLL_EVENTS, true);

  epoll_add(epollfd_, listen_conn_);

  // 5
  loop_thread_ = thread(&TCPServer::loop_main, this);

  return true;
}

bool TCPServer::start(int port, int64_t timeout) {
  port_ = port;
  init_ssl();
  init();
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

  //! @todo will delete this line later
  usleep(200000);

  stop_ = 1;
  loop_thread_.join();

  for (auto& c : connections_) {
    if (c.second != nullptr) {
      delete c.second;
      c.second = nullptr;
    }
  }

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
