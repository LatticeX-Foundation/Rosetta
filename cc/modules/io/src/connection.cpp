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
#include "cc/modules/io/include/internal/connection.h"

#include <thread>
#include <chrono>
using namespace std::chrono;

namespace rosetta {
namespace io {

Connection::Connection(int _fd, int _events, bool _is_server) {
  fd_ = _fd;
  events_ = _events;
  is_server_ = _is_server;
  buffer_ = make_shared<cycle_buffer>(1024 * 1024 * 10);
}
Connection::~Connection() { close(); }
void Connection::close() {
  if (state_ != Connection::State::Closed) {
    state_ = Connection::State::Closing;
    ::close(fd_);
    state_ = Connection::State::Closed;
  }
}

SSLConnection::~SSLConnection() { close(); }

ssize_t Connection::send(const char* data, size_t len, int64_t timeout) {
  if (is_server()) {
    log_error << "not supports server's send at present!" << endl;
    return -1;
  }

  if (len > 1024 * 1024 * 100) {
    log_warn << "client will send " << len << " B, >100M!" << endl;
  }

  ssize_t n = 0;
#if USE_LIBEVENT_AS_BACKEND
  struct evbuffer* output = bufferevent_get_output(bev_);
  n = evbuffer_add(output, data, len);
  if (n == 0)
    n = len;
#else
  n = writen(fd_, data, len);
#endif
  return n;
}

ssize_t Connection::recv(char* data, size_t len, int64_t timeout) {
  ssize_t n = 0;
  if (is_server()) {
    n = buffer_->read(data, len);
  } else {
#if USE_LIBEVENT_AS_BACKEND
    struct evbuffer* intput = bufferevent_get_input(bev_);
    n = evbuffer_remove(intput, data, len);
    if (n == 0)
      n = len;
#else
    n = readn(fd_, data, len);
#endif
  }
  return n;
}

ssize_t Connection::recv(const msg_id_t& msg_id, char* data, size_t len, int64_t timeout) {
  if (!is_server()) {
    log_error << "not supports client's recv at present!" << endl;
    return -1;
  }
  if (len > 1024 * 1024 * 100) {
    log_warn << "msg_id:" << msg_id.str() << " will recv " << len << " B, >100M!" << endl;
  }

  int64_t elapsed = 0;
  auto beg = system_clock::now();
  ssize_t ret = 0;
  bool retry = false;
  State tmpstate = state_;
  do {
    auto end = system_clock::now();
    elapsed = duration_cast<duration<int64_t, std::milli>>(end - beg).count();
    if ((tmpstate == State::Connected) && (state_ != State::Connected)) {
      return E_UNCONNECTED; // un connected
    }
    if (elapsed > timeout) {
      return E_TIMEOUT; // timeout
    }

    if (retry)
      std::this_thread::yield();

    unique_lock<mutex> lck(mapbuffer_mtx_);
    if (verbose_ > 2) {
      cout << "msg_id:" << msg_id << " ------------------------:" << buffer_->size() << endl;
      if (verbose_ > 3) {
        for (auto& mb : mapbuffer_) {
          cout << " mb.first :" << mb.first << ",size:" << mb.second->size() << endl;
        }
      }
    }

    auto iter = mapbuffer_.find(msg_id);
    if (iter != mapbuffer_.end()) { // got id
      if (iter->second->can_read(len)) { // got data
        ret = iter->second->read(data, len);
        return ret;
      }
      if (verbose_ > 1) {
        cout << "got id [" << msg_id << "], but can not read. expected:" << len
             << ", actual:" << iter->second->size() << ",main buffer size:" << buffer_->size()
             << endl;
      }
      //std::this_thread::yield();
    }

    if (false) {
      // remove `one` empty <msg_id, buffer>
      for (auto iter = mapbuffer_.begin(); iter != mapbuffer_.end(); ++iter) {
        if (iter->second->can_remove(50.0)) {
          if (verbose_ > 0) {
            log_info << "remove [" << iter->first << "] from mapbuffer, because it's empty."
                     << endl;
          }
          mapbuffer_.erase(iter);
          break;
        }
      }
    }

    // length of len
    size_t len1 = sizeof(int32_t);
    if (!buffer_->can_read(len1)) {
      if (verbose_ > 1) {
        cout << "(!buffer_->can_read(len1)): len1:" << len1 << endl;
      }
      //usleep(50);
      retry = true;
      continue;
    }

    int32_t alen = 0;
    buffer_->peek((char*)&alen, len1);
    if (!buffer_->can_read(alen)) {
      if (verbose_ > 1) {
        cout << "(!buffer_->can_read(alen)): alen:" << alen << endl;
      }
      //usleep(10);
      continue;
    }

    buffer_->read((char*)&alen, len1); // the length of the total msg

    char tmp_id[msg_id_t::Size() + 1] = {0};
    buffer_->read(tmp_id, msg_id_t::Size()); // msg id
    msg_id_t tmp(tmp_id, msg_id_t::Size());
    if (mapbuffer_.find(tmp) == mapbuffer_.end()) {
      if (verbose_ > 1) {
        cout << "server recv fd:" << fd_ << " enter msgid:" << tmp << endl;
      }
      mapbuffer_[tmp] = make_shared<cycle_buffer>(1024 * 8);
    }

    size_t data_len = alen - len1 - msg_id_t::Size();
    if ((tmp == msg_id) && (len == data_len) && (mapbuffer_[msg_id]->size() == 0)) {
      ssize_t ret = buffer_->read(data, len);
      return ret;
    }

    // write the real data
    const int NN = 4 * 1024;
    if (data_len > NN) {
      char* buff = new char[data_len];
      buffer_->read(buff, data_len);
      mapbuffer_[tmp]->write(buff, data_len);
      delete[] buff;
    } else {
      char buff[NN] = {0};
      buffer_->read(buff, data_len);
      mapbuffer_[tmp]->write(buff, data_len);
    }
  } while (true);

  log_error << "in fact, never enter here" << endl;
  return E_ERROR;
}

ssize_t Connection::peek(int sockfd, void* buf, size_t len) {
  cout << __FUNCTION__ << " len:" << len << endl;
  while (1) {
    ssize_t ret = ::recv(sockfd, buf, len, MSG_PEEK);
    if (ret == -1 && errno == EINTR)
      continue;
    return ret;
  }
}

ssize_t Connection::readn(int connfd, char* vptr, size_t n) {
  ssize_t nleft;
  ssize_t nread;
  char* ptr;

  ptr = vptr;
  nleft = n;

  while (nleft > 0) {
    if (verbose_ > 2)
      cout << __FUNCTION__ << " nleft:" << nleft << endl;
    if ((nread = readImpl(connfd, ptr, nleft)) < 0) {
      if (errno == EINTR) {
        nread = 0;
      } else {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
          //usleep(200);
          continue;
        }
        log_error << __FUNCTION__ << " errno:" << errno << " " << strerror(errno) << endl;
        return -1;
      }
    } else if (nread == 0) {
      if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
        continue;
      }
      break;
    }
    nleft -= nread;
    ptr += nread;
  }
  return n - nleft;
}

ssize_t Connection::writen(int connfd, const char* vptr, size_t n) {
  ssize_t nleft, nwritten;
  const char* ptr;

  std::unique_lock<mutex> lck(mtx_send_);

  ptr = vptr;
  nleft = n;

  while (nleft > 0) {
    if (verbose_ > 2)
      cout << __FUNCTION__ << " nleft:" << nleft << endl;
    if ((nwritten = writeImpl(connfd, ptr, nleft)) < 0) {
      if (errno == EINTR) {
        nwritten = 0;
      } else if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
        continue;
      }
      log_error << __FUNCTION__ << " errno:" << errno << " " << strerror(errno) << endl;
      return -1;
    } else if (nwritten == 0) {
      if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
        continue;
      }
      break;
    }
    nleft -= nwritten;
    ptr += nwritten;
  }

  return n - nleft;
}

} // namespace io
} // namespace rosetta
