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

#if !USE_LIBEVENT_AS_BACKEND
namespace rosetta {
namespace io {
void SSLConnection::close() {
  state_ = Connection::State::Closing;
  if (ssl_ != nullptr) {
    SSL_shutdown(ssl_);
    SSL_free(ssl_);
    ssl_ = nullptr;
  }
  ::close(fd_);
  state_ = Connection::State::Closed;
}

bool SSLConnection::handshake() {
  if (state_ == State::Connected) {
    return true;
  }
  if (ssl_ == nullptr) {
    {
      // new ssl
      ssl_ = SSL_new(ctx_);
      if (ssl_ == nullptr) {
        ERR_print_errors_fp(stderr);
        log_error << "SSLConnection::handshake() SSL_new failed!" << endl;
        return false;
      }
    }
    {
      // set fd to ssl
      int r = SSL_set_fd(ssl_, fd_);
      if (is_server()) {
        SSL_set_accept_state(ssl_);
      } else {
        SSL_set_connect_state(ssl_);
      }
    }

    // do handshake
    int fail_retries = 0;
    int r = -2;
    do {
#if 0
      if (is_server()) {
        r = SSL_accept(ssl_);
      } else {
        r = SSL_connect(ssl_);
      }
#else
      r = SSL_do_handshake(ssl_);
#endif
      if (r == 1) {
        state_ = State::Connected;
        // DEBUG INFO
        // const SSL_CIPHER* sc = SSL_get_current_cipher(ssl_);
        // log_info << "SSL VERSION INFO - CIPHER  VERSION:" << SSL_CIPHER_get_version(sc);
        // log_info << "SSL VERSION INFO - CIPHER     NAME:" << SSL_CIPHER_get_name(sc);
        // log_info << "SSL VERSION INFO - CIPHER STD NAME:" << SSL_CIPHER_standard_name(sc);
        // int alg_bits;
        // log_info << "SSL VERSION INFO - CIPHER ALG BITS:" << SSL_CIPHER_get_bits(sc, &alg_bits);
        return netutil::show_certs(ssl_, "");
      }

      int err = SSL_get_error(ssl_, r);
      if ((err == SSL_ERROR_WANT_WRITE) || (err == SSL_ERROR_WANT_READ)) {
        std::this_thread::sleep_for(chrono::milliseconds(100));
        continue;
      } else {
        log_warn << "is server:" << is_server() << " SSL_do_handshake retries:" << fail_retries
                 << " sslerr " << err << ":" << errno << " " << strerror(errno) << endl;
        if (fail_retries-- <= 0) {
          return false;
        }
      }
      std::this_thread::sleep_for(chrono::milliseconds(1000));
    } while (true);
  }

  if (SSL_is_init_finished(ssl_)) {
    return true;
  }
  return false;
} // namespace io

ssize_t SSLConnection::readImpl(int fd, char* data, size_t len) {
  size_t rd = 0;
#if 1
  {
    unique_lock<std::mutex> lck(ssl_rw_mtx_);
    int ret = SSL_read(ssl_, data, len);
    rd = ret;
    int e = SSL_get_error(ssl_, ret);
    //cout << "readImpl ssl ret:" << ret << ",r:" << rd << ", errno:" << errno << ", e:" << e << endl;
    if (ret < 0) {
      if ((e == SSL_ERROR_WANT_READ) || (e == SSL_ERROR_WANT_WRITE)) {
        return ret;
      }
      ERR_print_errors_fp(stdout);
      log_error << "SSLConnection::readImpl sslerr:" << e << ", errno:" << errno << " "
                << strerror(errno) << endl;
      return ret;
    }
  }
#else
  {
    //usleep(10000);
    //cout << "......................readImpl" << endl;
    unique_lock<std::mutex> lck(ssl_rw_mtx_);
    int ret = SSL_read_ex(ssl_, data, len, &rd);
    int e = SSL_get_error(ssl_, ret);
    //cout << "readImpl ssl ret:" << ret << ",r:" << rd << ", errno:" << errno << ", e:" << e << endl;
    if (ret < 0 && e != SSL_ERROR_WANT_READ) {
      ERR_print_errors_fp(stdout);
      log_error << "SSLConnection::readImpl errno:" << errno << " " << strerror(errno) << ",e:" << e
                << endl;
      throw socket_exp("SSLConnection::readImpl failed!");
    }
  }
#endif
  return rd;
}
ssize_t SSLConnection::writeImpl(int fd, const char* data, size_t len) {
  size_t wr = 0;
#if 1
  {
    unique_lock<std::mutex> lck(ssl_rw_mtx_);
    int ret = SSL_write(ssl_, data, len);
    wr = ret;
    int e = SSL_get_error(ssl_, ret);
    //cout << "wriImpl ssl ret:" << ret << ",r:" << wr << ", errno:" << errno << ", e:" << e << endl;
    if (ret < 0) {
      if ((e == SSL_ERROR_WANT_READ) || (e == SSL_ERROR_WANT_WRITE)) {
        return ret;
      }
      ERR_print_errors_fp(stdout);
      log_error << "SSLConnection::writeImpl sslerr:" << e << ", errno:" << errno << " "
                << strerror(errno) << endl;
      return ret;
    }
  }
#else
  {
    //usleep(100);
    //cout << "......................writeImpl" << endl;
    unique_lock<std::mutex> lck(ssl_rw_mtx_);
    int ret = SSL_write_ex(ssl_, data, len, &wr);
    int e = SSL_get_error(ssl_, ret);
    //cout << "wriImpl ssl ret:" << ret << ",r:" << wr << ", errno:" << errno << ", e:" << e << endl;
    if (ret < 0 && e != SSL_ERROR_WANT_WRITE) {
      ERR_print_errors_fp(stdout);
      log_error << "SSLConnection::writeImpl errno:" << errno << ",e:" << e << endl;
      throw socket_exp("SSLConnection::writeImpl failed!");
    }
  }
#endif
  return wr;
}
} // namespace io
} // namespace rosetta
#endif
