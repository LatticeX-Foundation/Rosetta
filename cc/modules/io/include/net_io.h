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

#include "cc/modules/io/include/internal/comm.h"
#include "cc/modules/io/include/internal/msg_id.h"
#include "cc/modules/io/include/internal/simple_buffer.h"
#include "cc/modules/io/include/internal/server.h"
#include "cc/modules/io/include/internal/client.h"
#include "cc/modules/io/include/internal/stat.h"

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
 * Note:
 * 
 * Only the receiving of the server and the sending of the client are totally supported. 
 * 
 * The receiving of the client and the sending of the server are not totally supported.
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
  /**
   * Constructor for create an IO.
   * 
   * \param parties how many parties are involved  \n
   * \param party id of the current party \n
   * \param thread_nums tow many threds will open in each connection \n
   * \param base_port the base port for server \n
   * \param ips the ips for all servers, in order \n
   */
  BasicIO(int parties, int party, int thread_nums, int base_port, const vector<string>& ips);
  /**
   * Constructor for create an IO.
   * 
   * \param ports the ports for servers, respectively \n
   * 
   * ips.size() == ports.size() >= 2
   */
  BasicIO(
    int parties,
    int party,
    int thread_nums,
    const vector<int>& ports,
    const vector<string>& ips);

 protected:
  virtual bool init_inner() { return true; }

 public:
  /**
   * init the server and clients.
   */
  bool init();
  /**
   * close the connections.
   */
  void close();
  /**
   * sync each party, used in non-parallel io.
   */
  void sync();
  /**
   * sync each party, used in parallel io.
   * 
   * \param msg_id the id user for parallel io
   */
  void sync_with(const msg_id_t& msg_id);
  /**
   * get the statistics (counts and elpased of recv/send).
   * 
   * \param str a helper message for print
   */
  void statistics(string str = "");
  /**
   * reset all statistics to initialized value.
   */
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
  NetStat net_stat() { return NetStat(net_stat_st_); }

  /**
   * set server certification
   * 
   * \param server_cert the file path of server certification
   */
  void set_server_cert(string server_cert) { server_cert_ = server_cert; }
  /**
   * set server private key
   * 
   * \param server_cert the file path of server private key \n
   * \param password optional. the password for server private key \n
   */
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
  /**
   * thread version \n
   * receive len size data from the client which id is party. \n
   * 
   * \param party receive from \n
   * \param data the buffer for receiving, must be allocated first \n
   * \param len legth size bytes will be received \n
   * \param tid thread id \n
   */
  int recv(int party, char* data, size_t len, int tid = 0);
  /**
   * thread version \n
   * send len size data to the server which id is party. \n
   * 
   * \param party send to \n
   * \param data the buffer will be sent \n
   * \param len legth size bytes will be sent \n
   * \param tid thread id \n
   */
  int send(int party, const char* data, size_t len, int tid = 0);
  /**
   * thread version \n
   * current party send len size data to the server which id is not current party. \n
   */
  int broadcast(const char* data, size_t len, int tid = 0);

  /**
   * thread version \n
   * 
   * \see recv
   */
  template <typename T>
  int recv(int party, vector<T>& data, size_t n, int tid = 0);
  /**
   * thread version \n
   */
  template <typename T>
  int send(int party, const vector<T>& data, size_t n, int tid = 0);
  /**
   * thread version \n
   */
  template <typename T>
  int broadcast(const vector<T>& data, size_t n, int tid = 0);

 public:
  /**
   * message-id version \n
   */
  int recv(int party, char* data, size_t len, const msg_id_t& msg_id);
  /**
   * message-id version \n
   */
  int send(int party, const char* data, size_t len, const msg_id_t& msg_id);
  /**
   * message-id version \n
   */
  int broadcast(const char* data, size_t len, const msg_id_t& msg_id);

  /**
   * message-id version \n
   */
  template <typename T>
  int recv(int party, vector<T>& data, size_t n, const msg_id_t& msg_id);
  /**
   * message-id version \n
   */
  template <typename T>
  int send(int party, const vector<T>& data, size_t n, const msg_id_t& msg_id);
  /**
   * message-id version \n
   */
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

/**
 * General Net IO.
 */
class NetIO : public BasicIO<TCPServer, TCPClient> {
 public:
  using BasicIO<TCPServer, TCPClient>::BasicIO;
  virtual ~NetIO() = default;
};

/**
 * General Net IO with SSL.
 */
class SSLNetIO : public BasicIO<SSLServer, SSLClient> {
 public:
  using BasicIO<SSLServer, SSLClient>::BasicIO;
  virtual ~SSLNetIO() = default;
};

/**
 * Parallel Net IO.
 */
class ParallelNetIO : public BasicIO<TCPServer, TCPClient> {
 public:
  using BasicIO<TCPServer, TCPClient>::BasicIO;
  virtual ~ParallelNetIO() = default;

 protected:
  virtual bool init_inner() {
    parallel_ = true;
    return true;
  }
};

/**
 * Parallel Net IO with SSL.
 */
class SSLParallelNetIO : public BasicIO<SSLServer, SSLClient> {
 public:
  using BasicIO<SSLServer, SSLClient>::BasicIO;
  virtual ~SSLParallelNetIO() = default;

 protected:
  virtual bool init_inner() {
    parallel_ = true;
    return true;
  }
};

} // namespace io
} // namespace rosetta