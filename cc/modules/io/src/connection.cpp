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

namespace rosetta {
namespace io {

Connection::Connection(int _fd, int _events, bool _is_server) {
  fd_ = _fd;
  events_ = _events;
  is_server_ = _is_server;
  buffer_ = make_shared<cycle_buffer>(1024 * 1024 * 10);
}

SSLConnection::~SSLConnection() {
  if (ssl_ != nullptr) {
    SSL_free(ssl_);
    ssl_ = nullptr;
  }
}

size_t Connection::send(const char* data, size_t len, int64_t timeout) {
  if (is_server()) {
    cerr << "not supports server's send at present!" << endl;
    throw;
  }

  int n = 0;
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

size_t Connection::recv(char* data, size_t len, int64_t timeout) {
  int n = 0;
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
size_t Connection::recv(const msg_id_t& msg_id, char* data, size_t len, int64_t timeout) {
  if (!is_server()) {
    cerr << "not supports client's recv at present!" << endl;
    throw;
  }

  int ret = 0;
  bool retry = false;
  do {
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

    {
      // remove `one` empty <msg_id, buffer>
      for (auto iter = mapbuffer_.begin(); iter != mapbuffer_.end(); ++iter) {
        if (iter->second->can_remove(20.0)) {
          if (verbose_ > 0) {
            cout << "remove [" << iter->first << "] from mapbuffer, because it's empty" << endl;
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

    int alen = 0;
    buffer_->peek((char*)&alen, len1);
    if (!buffer_->can_read(alen)) {
      if (verbose_ > 1) {
        cout << "(!buffer_->can_read(alen)): alen:" << alen << endl;
      }
      //usleep(10);
      continue;
    }

    buffer_->read((char*)&alen, len1); // the length of the total msg

    msg_id_t tmp;
    buffer_->read(tmp.data(), msg_id_t::Size()); // msg id
    if (mapbuffer_.find(tmp) == mapbuffer_.end()) {
      if (verbose_ > 1) {
        cout << "server recv fd:" << fd_ << " enter msgid:" << tmp << endl;
      }
      mapbuffer_[tmp] = make_shared<cycle_buffer>(1024 * 1024 * 10);
    }

    // write the real data
    size_t data_len = alen - len1 - msg_id_t::Size();
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

    // re-fetch
    {
      auto iter = mapbuffer_.find(msg_id);
      if (iter != mapbuffer_.end()) { // got id
        if (iter->second->can_read(len)) { // got data
          ret = iter->second->read(data, len);
          return ret;
        }
      }
    }
  } while (true);

  cerr << "in fact, never enter here" << endl;
  return 0;
}

ssize_t Connection::peek(int sockfd, void* buf, size_t len) {
  cout << __FUNCTION__ << " len:" << len << endl;
  while (1) {
    int ret = ::recv(sockfd, buf, len, MSG_PEEK);
    if (ret == -1 && errno == EINTR)
      continue;
    return ret;
  }
}

int Connection::readn(int connfd, char* vptr, int n) {
  int nleft;
  int nread;
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
        cout << __FUNCTION__ << " errno:" << errno << endl;
        return -1;
      }
    } else if (nread == 0) {
      break;
    }
    nleft -= nread;
    ptr += nread;
  }
  return n - nleft;
}

int Connection::writen(int connfd, const char* vptr, size_t n) {
  int nleft, nwritten;
  const char* ptr;

  ptr = vptr;
  nleft = n;

  while (nleft > 0) {
    if (verbose_ > 2)
      cout << __FUNCTION__ << " nleft:" << nleft << endl;
    if ((nwritten = writeImpl(connfd, ptr, nleft)) <= 0) {
      if (nwritten < 0) {
        if (errno == EINTR) {
          nwritten = 0;
        } else if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
          //usleep(200);
          continue;
        }
        nwritten = 0;
        usleep(100000);
        cout << __FUNCTION__ << " errno:" << errno << endl;
        return -1;
      } else {
        return -1;
      }
    }
    nleft -= nwritten;
    ptr += nwritten;
  }

  return n - nleft;
}

} // namespace io
} // namespace rosetta
