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

int Socket::readn(int connfd, char* vptr, int n) {
  int nleft;
  int nread;
  char* ptr;

  ptr = vptr;
  nleft = n;

  while (nleft > 0) {
    if ((nread = ::read(connfd, ptr, nleft)) < 0) {
      if (errno == EINTR) {
        nread = 0;
      } else {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
          usleep(10000);
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

int Socket::writen(int connfd, const char* vptr, size_t n) {
  int nleft = 0, nwritten = 0;
  const char* ptr;

  ptr = vptr;
  nleft = n;

  while (nleft > 0) {
    if ((nwritten = ::write(connfd, ptr, nleft)) <= 0) {
      if (nwritten < 0) {
        if (errno == EINTR) {
          nwritten = 0;
        } else if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
          usleep(10000);
          continue;
        }
        cout << __FUNCTION__ << " errno:" << errno << endl;
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