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
#include "internal/server.h"

#if USE_LIBEVENT_AS_BACKEND
namespace rosetta {
namespace io {
bool TCPServer::start(int port, int64_t timeout) {
  init_ssl();
  if (timeout < 0) {
    timeout = 999999999999L;
  }
  std::string ipport = "0.0.0.0:" + std::to_string(port);
  cout << "begin init server: " << ipport << " with timeout(ms):" << timeout << endl;

  running_ = false;
  enable_threads();
  enable_event_log();

  // struct event_config* config = event_config_new();
  // event_config_require_features(config, EV_FEATURE_ET);

  tps_ = new thread_params;
  thread_ = new std::thread(
    [&](TCPServer* client, thread_params* tps_) {
      auto& base = tps_->base;
      auto& bev = tps_->bev;
      base = event_base_new();

      // timer
      if (ev_timer == nullptr) {
        ev_timer = evtimer_new(base, onTimer, this);
        struct timeval tv = {.tv_sec = 1, .tv_usec = 0};
        evtimer_add(ev_timer, &tv);
      }

      // set timeout
      struct event* timeout_event = event_new(base, -1, EV_TIMEOUT, onTimeout, this);
      struct timeval t = client->gettv(100000000);
      event_add(timeout_event, &t);

      // parse addr
      struct sockaddr_in sa;
      int olen = sizeof(struct sockaddr_in);
      int ret = evutil_parse_sockaddr_port(ipport.c_str(), (struct sockaddr*)&sa, &olen);
      if (ret != 0) {
        printf("parse ip:port %s failed\n", ipport.c_str());
        init_cv_.notify_one();
        event_base_free(base);
        return;
      }

      // listener
      int flags = LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE;
      int backlog = -1;
      struct evconnlistener* listener =
        evconnlistener_new_bind(base, onAccept, this, flags, backlog, (struct sockaddr*)&sa, olen);

      //cout << "listen ok 1" << endl;

      // signal
      struct event* signal_event = evsignal_new(base, SIGINT, onSignal, this);
      if (!signal_event || event_add(signal_event, NULL) < 0) {
        fprintf(stderr, "Could not create/add a signal event!\n");
        return;
      }

      // start loop
      running_ = true;
      init_cv_.notify_one();
      event_base_dispatch(base);
      // event_base_loop(base, 0);
      running_ = false;

      evconnlistener_free(listener);
      event_free(signal_event);
      event_free(ev_timer);
      event_free(timeout_event);
      event_base_free(base);

      cout << "end event base loop" << endl;
      init_cv_.notify_one();
    },
    this, tps_);

  // wait and return
  std::unique_lock<std::mutex> lock(init_mtx_);
  init_cv_.wait_for(lock, std::chrono::milliseconds(timeout), [this]() { return running_; });
  cout << "server init " << string(running_ ? "ok" : "not-ok") << endl;
  if (!running_)
    return false;

  return true;
}

void TCPServer::as_server() {
  std::unique_lock<std::mutex> lock(init_mtx_);
  init_cv_.wait(lock, [this]() { return !running_; });
  init_cv_.wait_for(lock, std::chrono::seconds(1), []() { return false; });
}

bool TCPServer::stop() {
  if (stoped_)
    return true;
  stoped_ = true;
  stop_ = 1;

  if (tps_ != nullptr) {
    event_base_loopbreak(tps_->base);
    thread_->join();
    delete thread_;
    delete tps_;
    tps_ = nullptr;
  }

  cout << "server stopped!" << endl;
  return true;
}

/**
 * Here are callbacks
 * ******************
 * ******************
 * ******************
 */
void TCPServer::onTimer(evutil_socket_t fd, short what, void* ctx) {
  TCPServer* server = static_cast<TCPServer*>(ctx);
  struct timeval tv = {.tv_sec = 0, .tv_usec = 50000};

  // a temp way
  evtimer_add(server->ev_timer, &tv);
}

void TCPServer::onTimeout(evutil_socket_t fd, short what, void* ctx) {
  TCPServer* server = static_cast<TCPServer*>(ctx);
  printf("onServerTimeout\n");

  if (!server->running_) {
    printf("timeout\n");
    event_base_loopbreak(server->tps_->base);
  }
}

void TCPServer::onSignal(evutil_socket_t sig, short what, void* ctx) {
  TCPServer* server = static_cast<TCPServer*>(ctx);
  if (server->running_) {
    printf("Caught an interrupt signal\n");
    event_base_loopbreak(server->tps_->base);
  }
}

void TCPServer::onRead(struct bufferevent* bev, void* ctx) {
  Connection* conn = static_cast<Connection*>(ctx);
  TCPServer* server = static_cast<TCPServer*>(conn->ref_ptr);

  struct evbuffer* input = bufferevent_get_input(bev);
  int len = evbuffer_get_length(input);

  // set client id
  if (!conn->has_set_client_id) {
    std::unique_lock<mutex> lck(server->set_client_id_mtx_);
    if (!conn->has_set_client_id) {
      if (len < 4) // client id has 4 bytes at present
        return;

      len -= 4;
      int cid = 0;
      evbuffer_remove(input, (char*)&cid, 4);
      mutex connections_mtx_;
      if (server->connections_.find(cid) == server->connections_.end()) {
        server->connections_[cid] = conn;
      } else {
        cerr << "(never enter here) fatal error!" << endl;
      }

      conn->has_set_client_id = true;
    }
  }

  if (len > 0) {
    const int NN = 4 * 1024;
    if (len > NN) {
      char* buf = new char[len];
      evbuffer_remove(input, buf, len);
      conn->buffer_->write(buf, len);
      delete[] buf;
    } else {
      char buf[NN] = {0};
      evbuffer_remove(input, buf, len);
      conn->buffer_->write(buf, len);
    }
  }

  return;
}

void TCPServer::onWrite(struct bufferevent* bev, void* ctx) {
  printf("onWrite\n");
}

void TCPServer::onEvent(struct bufferevent* bev, short what, void* ctx) {
  TCPServer* server = static_cast<TCPServer*>(ctx);
  printf("onEvent event: %0x %d\n", what, what);
  // evutil_socket_error_to_string()

  if (what & BEV_EVENT_ERROR) {
    printf("Error: Server connection error\n");
  }
  if (what & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
    // broken network reconnection

    if (what & BEV_EVENT_EOF) {
      cout << "BEV_EVENT_EOF" << endl;
    }
    if (what & BEV_EVENT_ERROR) {
      cout << "BEV_EVENT_ERROR" << endl;
      if (server->is_ssl_socket_) {
        cout << "000000000000>" << bufferevent_get_openssl_error(bev) << endl;
      }
    }
    // bufferevent_free(bev);
    // event_base_loopbreak(bufferevent_get_base(bev));

    // close client fd ???
  }
  if (what & BEV_EVENT_CONNECTED) {
    cout << "BEV_EVENT_CONNECTED" << endl;
  }
}

/*
ref: https://www.jianshu.com/p/8ea60a8d3ab..
*/
void TCPServer::onAccept(
  struct evconnlistener* listener, evutil_socket_t fd, struct sockaddr* address, int socklen,
  void* ctx) {
  TCPServer* server = static_cast<TCPServer*>(ctx);
  {
    // options
    server->set_sendbuf(fd, 1024 * 1024);
    server->set_recvbuf(fd, 1024 * 1024);
    server->set_nodelay(fd, 1);
  }

  sockaddr_in* sin = (sockaddr_in*)address;
  char* ip = inet_ntoa(sin->sin_addr);
  int port = sin->sin_port;
  if (server->verbose_ > 0) {
    cout << "onAccept begin client fd: " << fd << ", ip:" << ip << ", port:" << port << endl;
  }

  // Create a new socket buffer event
  int flags = BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE | BEV_OPT_DEFER_CALLBACKS;

  struct bufferevent* bev = nullptr;
  if (server->is_ssl_socket_) {
    SSL_CTX* server_ctx;
    SSL* client_ctx;
    server_ctx = (SSL_CTX*)server->ctx;
    client_ctx = SSL_new(server_ctx);
    bev = bufferevent_openssl_socket_new(
      evconnlistener_get_base(listener), fd, client_ctx, BUFFEREVENT_SSL_ACCEPTING, flags);
  } else {
    bev = bufferevent_socket_new(evconnlistener_get_base(listener), fd, flags);
  }

  if (bev == NULL) {
    printf("Failed to create libevent buffer event\n");
    return;
  }
  bufferevent_enable(bev, EV_READ);

  {
    // settings
    size_t size = 1024 * 1024 * 2;
    bufferevent_set_max_single_read(bev, size);
    bufferevent_set_max_single_write(bev, size);

    if (server->verbose_ > 2) {
      size_t lowmark, highmark;
      bufferevent_getwatermark(bev, EV_READ, &lowmark, &highmark);
      cout << "watermark EV_READ: " << lowmark << ", " << highmark << endl;

      bufferevent_getwatermark(bev, EV_WRITE, &lowmark, &highmark);
      cout << "watermark EV_WRITE: " << lowmark << ", " << highmark << endl;
    }
  }

  Connection* conn = new Connection(fd, 0, true);
  conn->ref_ptr = (void*)server;
  conn->bev = bev;
  conn->client_ip = string(ip);
  conn->client_port = port;

  // set callback
  bufferevent_setcb(bev, onRead, NULL, onEvent, conn);

  if (server->verbose_ > 0) {
    cout << "onAccept done. client fd: " << fd << ", ip:" << ip << ", port:" << port << endl;
  }
}
} // namespace io
} // namespace rosetta
#endif
