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
#include "internal/connection.h"

#if !USE_LIBEVENT_AS_BACKEND
namespace rosetta {
namespace io {
void SSLConnection::handshake() {
  if (state_ == State::Connected) {
    return;
  }
  if (ssl_ == nullptr) {
    {
      // new ssl
      ssl_ = SSL_new(ctx_);
      if (ssl_ == nullptr) {
        cerr << "SSLConnection::handshake() SSL_new failed!" << endl;
        exit(0);
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

    do {
      // do handshake
      int r = SSL_do_handshake(ssl_);
      if (r == 1) {
        state_ = State::Connected;
        return;
      }

      int err = SSL_get_error(ssl_, r);
      if (err == SSL_ERROR_WANT_WRITE) {
        continue;
      } else if (err == SSL_ERROR_WANT_READ) {
        continue;
      } else {
        cout << "SSL_do_handshake error " << err << endl;
        usleep(50000);
      }
      usleep(50000);
    } while (true);
  }
}
} // namespace io
} // namespace rosetta
#endif
