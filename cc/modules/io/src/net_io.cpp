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
  for (int i = 0; i < parties_; i++) {
    for (int j = 0; j < thread_nums_; j++) {
      if (party_ != i) {
        client[i][j]->close();
      }
    }
  }
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

  // init server with each party's port
  if (is_ssl_io_)
    server = make_shared<SSLServer>();
  else
    server = make_shared<TCPServer>();

  server->set_server_cert(server_cert_);
  server->set_server_prikey(server_prikey_, server_prikey_password_);
  server->start(ports_[party_]);
  if (verbose_ > 1)
    cout << "init server end" << endl;

  // init client id, which is `party_id * thread_nums + thread_id`
  for (int i = 0; i < parties_; i++) {
    for (int j = 0; j < thread_nums_; j++) {
      int cid = thread_nums_ * i + j;
      party_cids_[i].insert({j, cid});
      if (verbose_ > 1) {
        cout << "party " << i << " in tid " << j << " with cid " << party_cids_[i][j] << endl;
      }
    }
  }
  if (verbose_ > 1)
    cout << "init client ids end" << endl;

  // init clients, each have parties's clients. one which i==party_id is not use
  client.resize(parties_);
  for (int i = 0; i < parties_; i++) {
    for (int j = 0; j < thread_nums_; j++) {
      if (party_ != i) {
        if (is_ssl_io_)
          client[i].push_back(make_shared<SSLClient>(ips_[i], ports_[i]));
        else
          client[i].push_back(make_shared<TCPClient>(ips_[i], ports_[i]));

        client[i][j]->setcid(party_cids_[party_][j]);
        client[i][j]->setsid(i);
        if (verbose_ > 1) {
          cout << "party " << i << " in tid " << j << " with cid " << party_cids_[i][j] << endl;
        }
        if (!client[i][j]->connect()) {
          cout << "error ......" << endl;
          return false;
        }
      } else {
        client[i].push_back(nullptr);
      }
    }
  }
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
          cout << "sync parties:" << parties_ << ", party:" << party_ << ", i:" << i << ", j:" << j
               << " with msg id:" << msg_id << "," << is_parallel_io_ << endl;
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
          cout << "sync parties:" << parties_ << ", party:" << party_ << ", i:" << i << ", j:" << j
               << " with msg id:" << msg_id << "," << is_parallel_io_ << endl;
        }
        if (is_parallel_io_) {
          recv(i, buf, 1, msg_id);
        } else {
          recv(i, buf, 1, j);
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
  cout << setw(15) << str << " communications P(" << party_ << "/" << parties_ << ") " << stat
       << endl;
}

/*
** thread version
*/

int BasicIO::recv(int party, char* data, size_t len, int tid) {
  int ret = server->recv(party_cids_[party][tid], data, len);
  {
    net_stat_st_.message_received++;
    net_stat_st_.bytes_received += len;
  }
  return ret;
}

int BasicIO::send(int party, const char* data, size_t len, int tid) {
  int ret = client[party][tid]->send(data, len);
  {
    net_stat_st_.message_sent++;
    net_stat_st_.bytes_sent += len;
  }
  return ret;
}

int BasicIO::broadcast(const char* data, size_t len, int tid) {
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
int BasicIO::recv(int party, char* data, size_t len, const msg_id_t& msg_id) {
  int ret = server->recv(party_cids_[party][0], msg_id, data, len);
  {
    net_stat_st_.message_received++;
    net_stat_st_.bytes_received += sizeof(int32_t);
    net_stat_st_.bytes_received += msg_id_t::Size();
    net_stat_st_.bytes_received += len;
  }
  return ret;
}

int BasicIO::send(int party, const char* data, size_t len, const msg_id_t& msg_id) {
  simple_buffer buffer(msg_id, data, len);
  int ret = send(party, buffer.data(), buffer.len(), 0);
  return ret;
}

int BasicIO::broadcast(const char* data, size_t len, const msg_id_t& msg_id) {
  for (int i = 0; i < parties_; i++) {
    if (party_ != i)
      send(i, data, len, msg_id);
  }
  return len;
}

} // namespace io
} // namespace rosetta
