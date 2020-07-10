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

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
using namespace std;

#if USE_LIBEVENT_AS_BACKEND
namespace rosetta {
namespace io {
int TCPClient::tcp_connect_to_server(const string& ip, int port) {
  int ret = -1;
  int status = -1;
  int sockfd = -1;

  struct sockaddr_in sa;
  sa.sin_family = AF_INET;
  sa.sin_port = htons(port);
  status = inet_aton(ip.c_str(), &sa.sin_addr);

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1)
    return sockfd;

  while (true && !pre_exit) {
    status = ::connect(sockfd, (struct sockaddr*)&sa, sizeof(sa));
    if (status == -1) {
      int errn = errno;
      if (errn == ECONNREFUSED) {
        sleep(1); // reconnect later
        continue;
      } else if (errn == EISCONN) {
        evutil_make_socket_nonblocking(sockfd);
        break;
      } else {
        cout << "tcp_connect_to_server errno: " << errn << endl;
      }
      sleep(1); // reconnect later
      continue;
    }
  }

  evutil_make_socket_nonblocking(sockfd);
  cout << "tcp_connect_to_server ok" << endl;
  return sockfd;
}

bool TCPClient::connect(int64_t timeout) {
  if (timeout < 0) {
    timeout = 999999999999L;
  }

  std::string ipport = ip_ + ":" + std::to_string(port_);
  cout << "client[" << cid_ << "] begin connect to: " << ipport << " with timeout:" << timeout
       << endl;

  enable_threads();
  enable_event_log();

  tps_ = new thread_params;

  thread_ = new std::thread(
    [&](TCPClient* client, thread_params* tps_) {
      auto& base = tps_->base;
      auto& bev = tps_->bev;

      int iccc = 0;
      while (client->reconnect_ && !pre_exit) {
        //cout << "BEGIN CONNECT TO SERVER 0... " << iccc << endl;

        base = event_base_new();

        // set timeout
        struct event* timeout_event = event_new(base, -1, EV_TIMEOUT, onTimeout, this);
        struct timeval t = client->gettv(100000000); // 10s
        event_add(timeout_event, &t);
        //cout << "BEGIN CONNECT TO SERVER 1... " << iccc << endl;

        // parse addr
        int ret = -1;
        int flags = BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE;

        //! Basic socket connection beg
        fd_ = tcp_connect_to_server(ip_, port_);
        //cout << "fd:" << fd_ << " BEGIN CONNECT TO SERVER 2... " << iccc << endl;
        if (client->is_ssl_socket_) {
          auto& ctx = client->ctx_;
          auto& ssl = client->ssl_;
#if 1
          bev = bufferevent_openssl_socket_new(base, fd_, ssl, BUFFEREVENT_SSL_CONNECTING, flags);
#else
          struct bufferevent* tmp = bufferevent_socket_new(base, fd_, flags);
          bev = bufferevent_openssl_filter_new(base, tmp, ssl, BUFFEREVENT_SSL_CONNECTING, flags);
#endif
        } else {
          bev = bufferevent_socket_new(base, fd_, flags);
        }
        //! Basic socket connection end

        conn_ = new Connection(fd_, 0, false);
        conn_->bev_ = tps_->bev;
        conn_->obj_ptr_ = (void*)this;
        conn_->thread_ptr_ = (void*)tps_;

        //cout << "BEGIN CONNECT TO SERVER 3... " << iccc << endl;
        // set read/write max size
        bufferevent_set_max_single_read(bev, client->default_buffer_size());
        bufferevent_set_max_single_write(bev, client->default_buffer_size());

        // set callback
        bufferevent_setcb(bev, onRead, NULL, onEvent, this);
        //cout << "client 1 " << endl;

        bufferevent_enable(bev, EV_READ | EV_WRITE);

        //cout << "client 2 " << endl;
        running_ = true;
        init_cv_.notify_one();
        event_base_loop(base, 0);

        //if (fd_ != -1) {
        //  evutil_closesocket(fd_);
        //  fd_ = -1;
        //}

        event_free(timeout_event);
        bufferevent_free(bev);
        event_base_free(base);

        //cout << "client 3 " << endl;
        running_ = false;
        init_cv_.notify_one();
      }
    },
    this, tps_);

  // wait and return
  std::unique_lock<std::mutex> lock(init_mtx_);
  init_cv_.wait_for(lock, std::chrono::milliseconds(timeout), [this]() { return running_; });
  cout << "client init " << string(running_ ? "ok" : "not-ok") << endl;
  if (!running_)
    return false;

  connected_ = true;
  send_client_id();
  return true;
}

void TCPClient::close() {
  if (connected_) {
    cout << "client closing." << endl;
    connected_ = false;

    pre_exit = true;
    if (tps_ != nullptr) {
      event_base_loopbreak(tps_->base);
      thread_->join();
      delete thread_;
      delete tps_;
      tps_ = nullptr;
    }
    cout << "client closed." << endl;
  }
}

/**
 * Here are callbacks
 * ******************
 * ******************
 * ******************
 */
void TCPClient::onTimeout(evutil_socket_t fd, short what, void* ctx) {
  TCPClient* client = static_cast<TCPClient*>(ctx);

  if (!client->running_) {
    printf("TCPClient timeout\n");
    event_base_loopbreak(client->tps_->base);
  }
}

void TCPClient::onWrite(struct bufferevent* bev, void* ctx) {
  TCPClient* client = static_cast<TCPClient*>(ctx);
}

void TCPClient::onRead(struct bufferevent* bev, void* ctx) {}

void TCPClient::onEvent(struct bufferevent* bev, short what, void* ctx) {
  TCPClient* client = static_cast<TCPClient*>(ctx);
  printf("onClientEvent events: %02x %d\n", what, what);

  if (what & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
    client->running_ = false;

    cout << " BEV_EVENT_ERROR " << endl;
    bufferevent_free(bev);
    event_base_loopbreak(bufferevent_get_base(bev));
  }

  if (what & BEV_EVENT_READING) {
  } else if (what & BEV_EVENT_WRITING) {
  } else if (what & BEV_EVENT_TIMEOUT) {
  } else if (what & BEV_EVENT_CONNECTED) {
    client->running_ = true;
    //client->setTcpNoDelay(bufferevent_getfd(bev));
    client->init_cv_.notify_one();
  }
}
} // namespace io
} // namespace rosetta
#endif
