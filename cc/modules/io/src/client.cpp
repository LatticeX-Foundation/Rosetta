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
#include "internal/client.h"

namespace rosetta {
namespace io {
size_t TCPClient::send(const char* data, size_t len, int64_t timeout) {
  if (timeout < 0) {
    timeout = 999999999999L;
  }

  if (conn_ == nullptr) {
    cerr << "client fatal error !" << endl;
    throw;
  }
  int n = conn_->send(data, len, timeout);
  if (n != len) {
    cerr << "client n != len (" << n << " != " << len << ")" << endl;
    throw;
  }
  return n;
}

/**
 * @todo not completed supports client's recv at present
 */
size_t TCPClient::recv(char* data, size_t len, int64_t timeout) {
  int n = conn_->recv(data, len);
  return n;
}
} // namespace io
} // namespace rosetta
