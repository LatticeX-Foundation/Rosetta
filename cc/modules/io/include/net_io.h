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
#pragma once

#include "internal/comm.h"
#include "internal/msg_id.h"
#include "internal/simple_buffer.h"
#include "internal/server.h"
#include "internal/client.h"
#include "internal/stat.h"

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <iomanip>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
using namespace std;

/**
 * Users only need to include this one header file.
 * 
 * Provides NetIO/SSLNetIO/ParallelNetIO/SSLParallelNetIO
 * 
 * Note, the supports of SSL have not totally completed.
 * 
 * The Client send and the Server receive. [supported]
 * The Client receive and the Server send. [unsupported]
 */
namespace rosetta {
namespace io {

/**
 * This is the basic class of Network IO.
 */
template <typename Server, typename Client>
class BasicIO {
 public:
  virtual ~BasicIO();
  BasicIO() = default;
  BasicIO(int parties, int party, int thread_nums, int base_port, const vector<string>& ips);

 public:
  void close();
  bool init();
  void sync();
  void sync_with(const msg_id_t& msg_id);
  void statistics(string str = "");
  void clear_statistics();

  /**
   * get the statistic \n
   * eg. \n
   * @code
   * auto ns1 = net_stat();
   * // ns1.print();
   * // do something
   * auto ns2 = net_stat();
   * // ns2.print();
   * auto ns = ns2 - ns1;
   * ns.print();
   * @endcode
   * \see internal/stat.h
   */
  NetStat net_stat() {
    return NetStat(net_stat_st_);
  }

  /**
   * about certifications
   */
  void set_server_cert(string server_cert) {
    server_cert_ = server_cert;
  }
  void set_server_prikey(string server_prikey, string password = "") {
    server_prikey_ = server_prikey;
    server_prikey_password_ = password;
  }

 public:
  /**
   * thread version \n
   * party: receive from or send to \n
   * data: will send or recveive buffer, for recveive, must be allocated first \n
   * len: byte \n
   * n: item size \n
   * tid: thread id \n
   * connid: connection id (not supported now) \n
   */
  int recv(int party, char* data, size_t len, int tid = 0);
  int send(int party, const char* data, size_t len, int tid = 0);
  int broadcast(const char* data, size_t len, int tid = 0);

  template <typename T>
  int recv(int party, vector<T>& data, size_t n, int tid = 0);
  template <typename T>
  int send(int party, const vector<T>& data, size_t n, int tid = 0);
  template <typename T>
  int broadcast(const vector<T>& data, size_t n, int tid = 0);

 public:
  /**
   * message-id version \n
   * choice 1. in the future, will combine 'thread version' to this version
   */
  int recv(int party, char* data, size_t len, const msg_id_t& msg_id);
  int send(int party, const char* data, size_t len, const msg_id_t& msg_id);
  int broadcast(const char* data, size_t len, const msg_id_t& msg_id);

  template <typename T>
  int recv(int party, vector<T>& data, size_t n, const msg_id_t& msg_id);
  template <typename T>
  int send(int party, const vector<T>& data, size_t n, const msg_id_t& msg_id);
  template <typename T>
  int broadcast(const vector<T>& data, size_t n, const msg_id_t& msg_id);

 protected:
  int verbose_ = 0;
  int parties_ = -1;
  int party_ = -1;
  int thread_nums_ = 1;
  int base_port_ = -1;
  vector<string> ips_;
  bool parallel_ = false;

  NetStat_st net_stat_st_;

  string server_cert_;
  string server_prikey_;
  string server_prikey_password_;

 protected:
  vector<int> ports_;
  map<int, map<int, int>> party_cids_; // party id --> client ids <tid --> cid>
  shared_ptr<Server> server = nullptr;
  vector<vector<shared_ptr<Client>>> client; // [party_id][thread_id]
};

#include "internal/net_io.hpp"

class NetIO : public BasicIO<TCPServer, TCPClient> {
 public:
  virtual ~NetIO() = default;
  NetIO(int parties, int party, int thread_nums, int base_port, const vector<string>& ips)
      : BasicIO(parties, party, thread_nums, base_port, ips) {}
};

class SSLNetIO : public BasicIO<SSLServer, SSLClient> {
 public:
  virtual ~SSLNetIO() = default;
  SSLNetIO(int parties, int party, int thread_nums, int base_port, const vector<string>& ips)
      : BasicIO(parties, party, thread_nums, base_port, ips) {}
};

class ParallelNetIO : public BasicIO<TCPServer, TCPClient> {
 public:
  virtual ~ParallelNetIO() = default;
  ParallelNetIO(int parties, int party, int thread_nums, int base_port, const vector<string>& ips)
      : BasicIO(parties, party, thread_nums, base_port, ips) {
    parallel_ = true;
  }
};

class SSLParallelNetIO : public BasicIO<SSLServer, SSLClient> {
 public:
  virtual ~SSLParallelNetIO() = default;
  SSLParallelNetIO(
    int parties, int party, int thread_nums, int base_port, const vector<string>& ips)
      : BasicIO(parties, party, thread_nums, base_port, ips) {
    parallel_ = true;
  }
};

} // namespace io
} // namespace rosetta