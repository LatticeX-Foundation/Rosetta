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

#include <regex>
#include <resolv.h>

namespace rosetta {
namespace io {

Socket::Socket() { default_buffer_size_ = 1024 * 1024 * 10; }

std::string Socket::gethostip(std::string hostname) {
  string ipaddr("");
  int retval;

  struct addrinfo* cur_info = NULL;
  struct addrinfo* res_info = NULL;
  struct addrinfo hints = {0};
  struct sockaddr_in* addr_in;

  hints.ai_family = AF_INET; //ipv4
  hints.ai_socktype = SOCK_STREAM; //tcp + udp
  hints.ai_protocol = IPPROTO_IP; //0

  // for tests
  // vector<string> host_names = {
  //   "localhost", "127.0.0.1", "www.baidu.com", "www.zhihu.com", "www.ping.cn"};
  vector<string> host_names = {hostname};
  for (int i = 0; i < host_names.size(); i++) {
    auto host_name = host_names[i];
    retval = getaddrinfo(host_name.c_str(), NULL, &hints, &res_info);
    if (retval != 0) {
      log_error << "getaddrinfo for host: " << host_name << ", error:" << gai_strerror(retval)
                << endl;
      continue;
    }
    log_info << " gethostbyname: " << host_name << endl;
    cur_info = res_info;
    while (cur_info != NULL) {
#if 1
      if (!ipaddr.empty())
        break;
#endif
      switch (cur_info->ai_family) {
        case AF_INET: {
          char buf[128] = {0};
          addr_in = (struct sockaddr_in*)cur_info->ai_addr;
          inet_ntop(cur_info->ai_family, &addr_in->sin_addr, buf, sizeof(buf));
          log_info << "   got address: " << buf << endl;
          if (ipaddr.empty()) {
            ipaddr.assign(buf);
          }
          break;
        }
        case AF_INET6: {
          break;
        }
        default:
          log_warn << "Unknown address type." << endl;
          break;
      }
      cur_info = cur_info->ai_next;
    }
    freeaddrinfo(res_info);
  }

  if (ipaddr.empty()) {
    // sometimes, gethostbyname("xxx.xxx.xxx.xxx") cannot get the IP, so
    // IP4: ^((2(5[0-5]|[0-4]\d))|[0-1]?\d{1,2})(\.((2(5[0-5]|[0-4]\d))|[0-1]?\d{1,2})){3}$
    std::regex ip4pattern(
      "^((2(5[0-5]|[0-4]\\d))|[0-1]?\\d{1,2})(\\.((2(5[0-5]|[0-4]\\d))|[0-1]?\\d{1,2})){3}$");
    if (std::regex_match(hostname, ip4pattern))
      ipaddr = hostname;
    else if (hostname == "localhost")
      ipaddr = "127.0.0.1";
  }

  if (ipaddr.empty()) {
    log_error << "cannot get any ip address." << endl;
  } else {
    log_info << "use ip address: " << ipaddr << endl;
  }

  return ipaddr;
}

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
int Socket::set_send_timeout(int fd, int64_t timeout) {
  struct timeval tv = {timeout / 1000L, (timeout % 1000L) * 1000L}; // s, us
  return ::setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char*)&tv, sizeof(tv));
}
int Socket::set_recv_timeout(int fd, int64_t timeout) {
  struct timeval tv = {timeout / 1000L, (timeout % 1000L) * 1000L}; // s, us
  return ::setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv));
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