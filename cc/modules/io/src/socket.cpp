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
#include "cc/modules/io/include/internal/socket.h"

namespace rosetta {
namespace io {

Socket::Socket() { default_buffer_size_ = 1024 * 1024 * 10; }

int Socket::set_reuseaddr(int fd, int optval) {
  int ret = ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(optval));
  return ret;
}
int Socket::set_reuseport(int fd, int optval) {
  int ret = ::setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, (const void*)&optval, sizeof(optval));
  return ret;
}
int Socket::set_sendbuf(int fd, int size) {
  int ret = ::setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (const void*)&size, sizeof(size));
  return ret;
}
int Socket::set_recvbuf(int fd, int size) {
  int ret = ::setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (const void*)&size, sizeof(size));
  return ret;
}
int Socket::set_linger(int fd) {
  struct linger l;
  l.l_onoff = 1;
  l.l_linger = 0;
  int ret = ::setsockopt(fd, SOL_SOCKET, SO_LINGER, (const void*)&l, sizeof(l));
  return ret;
}
int Socket::set_nodelay(int fd, int optval) {
  int ret = ::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const void*)&optval, sizeof(optval));
  return ret;
}

} // namespace io
} // namespace rosetta