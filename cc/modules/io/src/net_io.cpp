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
#include "cc/modules/io/include/net_io.h"

namespace rosetta {
namespace io {

void BasicIO::close() {
  for (int i = 0; i < client.size(); i++) { // parties_
    for (int j = 0; j < client[i].size(); j++) { // thread_nums_
      if (party_ != i) {
        client[i][j]->close();
      }
    }
    client[i].clear();
  }
  client.clear();

  if (server != nullptr)
    server->stop();
}

BasicIO::~BasicIO() {
  close();
  if (verbose_ > 2)
    cout << "End" << endl;
}

BasicIO::BasicIO(int parties, int party, int thread_nums, int base_port, const vector<string>& ips)
    : parties_(parties),
      party_(party),
      thread_nums_(thread_nums),
      base_port_(base_port),
      ips_(ips) {
  // init ports, each party has one port, which is `base_port + party_id`
  ports_.resize(parties_, 0);
  for (int i = 0; i < parties_; i++) {
    ports_[i] = base_port_ + i; // for server listen on and client connect to
  }
  if (verbose_ > 1)
    cout << "init ports end" << endl;
}

BasicIO::BasicIO(
  int parties,
  int party,
  int thread_nums,
  const vector<int>& ports,
  const vector<string>& ips)
    : parties_(parties), party_(party), thread_nums_(thread_nums), ports_(ports), ips_(ips) {}

bool BasicIO::init() {
  if (verbose_ > 0)
    cout << "init all beg" << endl;

  init_inner();

  // init client id, which is `party_id * thread_nums + thread_id`
  vector<int> expected_cids;
  for (int i = 0; i < parties_; i++) {
    for (int j = 0; j < thread_nums_; j++) {
      int cid = thread_nums_ * i + j;
      if (party_ != i) {
        expected_cids.push_back(cid);
      }
      party_cids_[i].insert({j, cid});
      if (verbose_ > 1) {
        cout << "party " << i << " in tid " << j << " with cid " << party_cids_[i][j] << endl;
      }
    }
  }
  if (verbose_ > 1)
    cout << "init client ids end" << endl;

  // init server with each party's port
  if (is_ssl_io_)
    server = make_shared<SSLServer>();
  else
    server = make_shared<TCPServer>();
  server->set_server_cert(server_cert_);
  server->set_server_prikey(server_prikey_, server_prikey_password_);
  server->set_expected_cids(expected_cids);
  server->setsid(party_);

  if (!server->start(ports_[party_]))
    return false;
  if (verbose_ > 1)
    cout << "init server end" << endl;

  // init clients, each have parties's clients. one which i==party_id is not use
  /////////////////////////////////////////////////////
  client.resize(parties_);
  vector<thread> client_threads(parties_ * thread_nums_);
  for (int i = 0; i < parties_; i++) {
    for (int j = 0; j < thread_nums_; j++) {
      client[i].push_back(nullptr);
    }
  }

  bool init_client_ok = true;
  auto conn_f = [&](int i, int j) -> bool {
    if (party_ != i) {
      if (is_ssl_io_)
        client[i][j] = make_shared<SSLClient>(ips_[i], ports_[i]);
      else
        client[i][j] = make_shared<TCPClient>(ips_[i], ports_[i]);

      client[i][j]->setcid(party_cids_[party_][j]);
      client[i][j]->setsid(i);
      client[i][j]->setsslid(party_);
      if (!client[i][j]->connect()) {
        init_client_ok = false;
        return false;
      }
    }
    return true;
  };

  for (int i = 0; i < parties_; i++) {
    for (int j = 0; j < thread_nums_; j++) {
      client_threads[i * thread_nums_ + j] = thread(conn_f, i, j);
    }
  }

  for (int i = 0; i < parties_; i++) {
    for (int j = 0; j < thread_nums_; j++) {
      client_threads[i * thread_nums_ + j].join();
    }
  }

  if (!init_client_ok)
    return false;
  /////////////////////////////////////////////////////

  if (verbose_ > 1)
    cout << "init clients end" << endl;

  //cout << "init all network connections succeed!" << endl;
  return true;
}

#if 0
void BasicIO::sync_with(const msg_id_t& msg_id) {
  string msg("1");
  char buf[2] = {0};
  for (int i = 0; i < parties_; i++) {
    for (int j = 0; j < thread_nums_; j++) {
      if (party_ != i) {
        if (verbose_ > 1) {
          cout << "sync parties:" << parties_ << ", party:" << party_ << ", i:" << i << ", j:" << j
               << " with msg id:" << msg_id << "," << is_parallel_io_ << endl;
        }
        if (is_parallel_io_) {
          send(i, msg.data(), 1, msg_id);
          recv(i, buf, 1, msg_id);
        } else {
          send(i, msg.data(), 1, j);
          recv(i, buf, 1, j);
        }
      }
    }
  }
  if (verbose_ > 0)
    cout << "sync ok" << endl;
}
#else
// NOTE: send all first, then recv
void BasicIO::sync_with(const msg_id_t& msg_id) {
  string msg("1");
  char buf[2] = {0};
  for (int i = 0; i < parties_; i++) {
    for (int j = 0; j < thread_nums_; j++) {
      if (party_ != i) {
        if (verbose_ > 1) {
          log_info << "sync parties:" << parties_ << ", party:" << party_ << ", i:" << i
                   << ", j:" << j << " with msg id:" << msg_id << "," << is_parallel_io_ << endl;
        }
        if (is_parallel_io_) {
          send(i, msg.data(), 1, msg_id);
        } else {
          send(i, msg.data(), 1, j);
        }
      }
    }
  }

  for (int i = 0; i < parties_; i++) {
    for (int j = 0; j < thread_nums_; j++) {
      if (party_ != i) {
        if (verbose_ > 1) {
          log_info << "sync recv beg parties:" << parties_ << ", party:" << party_ << ", i:" << i
                   << ", j:" << j << " with msg id:" << msg_id << "," << is_parallel_io_ << endl;
        }
        if (is_parallel_io_) {
          recv(i, buf, 1, msg_id);
        } else {
          recv(i, buf, 1, j);
        }
        if (verbose_ > 1) {
          log_info << "sync recv end parties:" << parties_ << ", party:" << party_ << ", i:" << i
                   << ", j:" << j << " with msg id:" << msg_id << "," << is_parallel_io_ << endl;
        }
      }
    }
  }
  if (verbose_ > 0)
    cout << "sync ok" << endl;
}
#endif

void BasicIO::sync() {
  msg_id_t msg_id("0000000000000000000000000000000000000");
  sync_with(msg_id);
}

void BasicIO::clear_statistics() { net_stat_st_.reset(); }

void BasicIO::statistics(string str) {
  auto stat = net_stat();
  log_debug << setw(15) << str << " communications P(" << party_ << "/" << parties_ << ") " << stat
            << endl;
}

/*
** thread version
*/

ssize_t BasicIO::recv(int party, char* data, size_t len, int tid) {
  if (server->stoped())
    throw socket_recv_exp("t server->stoped().");
  ssize_t ret = server->recv(party_cids_[party][tid], data, len);
  if (ret != len)
    throw socket_recv_exp("netio recv failed. Please see log for more details.");

  {
    net_stat_st_.message_received++;
    net_stat_st_.bytes_received += len;
  }
  return ret;
}

ssize_t BasicIO::send(int party, const char* data, size_t len, int tid) {
  if (client[party][tid]->closed())
    throw socket_send_exp("t client[party][tid]->closed()");
  ssize_t ret = client[party][tid]->send(data, len);
  if (ret != len)
    throw socket_send_exp("netio send failed. Please see log for more details.");

  {
    net_stat_st_.message_sent++;
    net_stat_st_.bytes_sent += len;
  }
  return ret;
}

ssize_t BasicIO::broadcast(const char* data, size_t len, int tid) {
  // send to other parties (by thread)
  for (int i = 0; i < parties_; i++) {
    if (party_ != i)
      send(i, data, len, tid);
  }
  return len;
}

/*
** mesasge key version
*/
ssize_t BasicIO::recv(int party, char* data, size_t len, const msg_id_t& msg_id) {
  if (server->stoped())
    throw socket_exp("m server->stoped()");
  int tid = (*((uint8_t*)msg_id.data()) & 0xF) % thread_nums_;
  ssize_t ret = server->recv(party_cids_[party][tid], msg_id, data, len);
  if (ret != len)
    throw socket_recv_exp("netio recv failed. Please see log for more details.");

  {
    net_stat_st_.message_received++;
    net_stat_st_.bytes_received += sizeof(int32_t);
    net_stat_st_.bytes_received += msg_id_t::Size();
    net_stat_st_.bytes_received += len;
  }
  return ret;
}

ssize_t BasicIO::send(int party, const char* data, size_t len, const msg_id_t& msg_id) {
  simple_buffer buffer(msg_id, data, len);
  int tid = (*((uint8_t*)msg_id.data()) & 0xF) % thread_nums_;
  ssize_t ret = send(party, buffer.data(), buffer.len(), tid);
  return ret;
}

ssize_t BasicIO::broadcast(const char* data, size_t len, const msg_id_t& msg_id) {
  for (int i = 0; i < parties_; i++) {
    if (party_ != i)
      send(i, data, len, msg_id);
  }
  return len;
}

} // namespace io
} // namespace rosetta
