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

namespace rosetta {
namespace io {
ssize_t TCPClient::send(const char* data, size_t len, int64_t timeout) {
  if (timeout < 0)
    timeout = 1000 * 1000000;

  if (conn_ == nullptr) {
    log_error << "TCPClient conn_ is nullptr!" << endl;
    return -1;
  }

  ssize_t ret = conn_->send(data, len, timeout);
  if (ret != len) {
    string errmsg =
      "TCPClient send error. expected:" + to_string(len) + " but got:" + to_string(ret);
    log_error << errmsg << endl;
    return -1;
  }

  return ret;
}

/**
 * @todo not completed supports client's recv at present
 */
ssize_t TCPClient::recv(char* data, size_t len, int64_t timeout) {
  ssize_t n = conn_->recv(data, len);
  return n;
}
} // namespace io
} // namespace rosetta
