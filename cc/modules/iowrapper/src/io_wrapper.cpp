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
#include "cc/modules/iowrapper/include/io_wrapper.h"
#include "cc/modules/common/include/utils/rtt_exceptions.h"
#include "io/channel_decode.h"
#include <stdexcept>
#include <string>
#include <iostream>
#include <string.h>
#include <algorithm>
#include <iomanip>
using namespace std;

namespace rosetta {

IOWrapper::IOWrapper(const string& task_id, IChannel* channel) {
  task_id_ = task_id;
  channel_ = channel;
  node_id_ = decode_string(channel_->GetCurrentNodeID());
  log_debug << "node id:" << node_id_;
  node2party_ = decode_map(channel_->GetComputationNodeIDs());
  party2node_.resize(node2party_.size());
  for (auto iter = node2party_.begin(); iter != node2party_.end(); iter++) {
    party2node_[iter->second] = iter->first;
    log_debug << "computation node:" << iter->first << " " << iter->second;
  }
  party_ = GetPartyId(node_id_);
  log_debug << "party id:" << party_ ;
  parties_ = node2party_.size();
  connected_nodes_ = decode_vector(channel_->GetConnectedNodeIDs());
  std::sort(connected_nodes_.begin(), connected_nodes_.end());
  for (int i = 0; i < connected_nodes_.size(); i++) {
    log_debug << "connected node:" << connected_nodes_[i] ;
  }
  data_nodes_ = decode_vector(channel_->GetDataNodeIDs());
  std::sort(data_nodes_.begin(), data_nodes_.end());
  for (int i = 0; i < data_nodes_.size(); i++) {
    log_debug << "data node:" << data_nodes_[i] ;
  }
  result_nodes_ = decode_vector(channel_->GetResultNodeIDs());
  std::sort(result_nodes_.begin(), result_nodes_.end());
  for (int i = 0; i < result_nodes_.size(); i++) {
    log_debug << "result node:" << result_nodes_[i] ;
  }

  set<string> non_computation_nodes(data_nodes_.begin(), data_nodes_.end());
  for (int i = 0; i < result_nodes_.size(); i++) {
    if (non_computation_nodes.find(result_nodes_[i]) == non_computation_nodes.end()) {
      non_computation_nodes.insert(result_nodes_[i]);
    }
  }
  for (auto iter = non_computation_nodes.begin(); iter != non_computation_nodes.end();) {
    if (node2party_.find(*iter) != node2party_.end()) {
      non_computation_nodes.erase(iter++);
    } else {
      iter++;
    }
  }
  non_computation_nodes_.clear();
  if (!non_computation_nodes.empty()) {
    non_computation_nodes_.insert(non_computation_nodes_.end(), non_computation_nodes.begin(), non_computation_nodes.end());
    std::sort(non_computation_nodes_.begin(), non_computation_nodes_.end());
  }
  nodes_ = node2party_.size() + non_computation_nodes_.size();
  log_debug << "node id:" << node_id_ << " party id:" << party_ ;
}

IOWrapper::~IOWrapper() {
  node2party_.clear();
  party2node_.clear();
  data_nodes_.clear();
  result_nodes_.clear();
  connected_nodes_.clear();
#ifdef DEBUG
  send_seq_.clear();
  recv_seq_.clear();
#endif
}

ssize_t IOWrapper::recv(int party, char* data, size_t len, const msg_id_t& msg_id, int64_t timeout) {
  string node_id = GetNodeId(party);
  return recv(node_id, data, len, msg_id, timeout);
}

ssize_t IOWrapper::recv(const string& node_id, char* data, size_t len, const msg_id_t& msg_id, int64_t timeout) {

#ifdef DEBUG
  int recv_seq = 0;
  {
    std::unique_lock<std::mutex> lck(recv_mutex_);
    auto iter = recv_seq_.find(node_id);
    if (iter == recv_seq_.end()) {
      iter = recv_seq_.insert(std::pair<string, map<msg_id_t, int>>(node_id, map<msg_id_t, int>())).first;
    }
    auto iter2 = iter->second.find(msg_id);
    if (iter2 != iter->second.end()) {
      ++iter2->second;
      recv_seq = iter2->second;
    } else {
      recv_seq = 1;
       iter->second.insert(std::pair<msg_id_t, int>(msg_id, recv_seq));
    }
  }
  //msg_id_t id(task_id_ + "|" + std::to_string(recv_seq) + "|" + msg_id.str());
  msg_id_t id(task_id_ + "|" + msg_id.str());
#else
  msg_id_t id(task_id_ + "|" + msg_id.str());
#endif

  log_debug << "begin recv data from " << node_id << " id:" << id << " len:" << len;
  if (timeout < 0)
    timeout = 10 * 1000000;
  ssize_t ret = channel_->Recv(node_id.c_str(), id.str().c_str(), data, len, timeout);
  if (ret != len) {
    log_error << "recv len:" << ret << " expect: " << len;
  }
  {
    net_stat_st_.message_received++;
    net_stat_st_.bytes_received += sizeof(int32_t);
    net_stat_st_.bytes_received += msg_id_t::Size();
    net_stat_st_.bytes_received += len;
  }
  log_debug << "end recv data from " << node_id << " id:" << id << " len:" << len ;
  return ret;
}

ssize_t IOWrapper::send(int party, const char* data, size_t len, const msg_id_t& msg_id, int64_t timeout) {
  string node_id = GetNodeId(party);
  return send(node_id, data, len, msg_id, timeout);
}

ssize_t IOWrapper::send(const string& node_id, const char* data, size_t len, const msg_id_t& msg_id, int64_t timeout) {
#ifdef DEBUG
  int send_seq = 0;
  {
    std::unique_lock<std::mutex> send_mutex_;
    auto iter = send_seq_.find(node_id);
    if (iter == send_seq_.end()) {
      iter = send_seq_.insert(std::pair<string, map<msg_id_t, int>>(node_id, map<msg_id_t, int>())).first;
    }
    auto iter2 = iter->second.find(msg_id);
    if (iter2 != iter->second.end()) {
      ++iter2->second;
      send_seq = iter2->second;
    } else {
      send_seq = 1;
      iter->second.insert(std::pair<msg_id_t, int>(msg_id, send_seq));
    }
  }
  //msg_id_t id(task_id_ + "|" + std::to_string(send_seq) + "|" + msg_id.str());
  msg_id_t id(task_id_ + "|" + msg_id.str());
#else
  msg_id_t id(task_id_ + "|" + msg_id.str());
#endif

  log_debug << "begin send data to " << node_id << " id:" << id << " len:" << len;
  if (timeout < 0)
    timeout = 10 * 1000000;
  ssize_t ret = channel_->Send(node_id.c_str(), id.str().c_str(), data, len, timeout);
  {
    net_stat_st_.message_sent++;
    net_stat_st_.bytes_sent += sizeof(int32_t);
    net_stat_st_.bytes_sent += msg_id_t::Size();
    net_stat_st_.bytes_sent += len;
  }
  log_debug << "end send data to " << node_id << " id:" << id << " len:" << len;
  return ret;
}

void IOWrapper::sync_with(const msg_id_t& msg_id) {
  string msg("1");
  char buf[2] = {0};
  vector<string> peers = connected_nodes_;
  for (int i = 0; i < peers.size(); i++) {
    send(peers[i], msg.data(), 1, msg_id);
  }

  for (int i = 0; i < peers.size(); i++) {
    recv(peers[i], buf, 1, msg_id);
  }
}

void IOWrapper::statistics(string str) {
  auto stat = net_stat();
  if (!task_id_.empty()) {
    log_debug << setw(15) << str << " communications Task ID(" << task_id_ << ") Node(" << node_id_ << "/" << nodes_ << ") " << stat;
  } else {
    log_debug << setw(15) << str << " communications Node(" << node_id_ << "/" << nodes_ << ") " << stat;
  }
}

void IOWrapper::clear_statistics() {
  net_stat_st_.bytes_received = 0;
  net_stat_st_.bytes_sent = 0;
  net_stat_st_.message_received = 0;
  net_stat_st_.message_sent = 0;
}

NetStat IOWrapper::net_stat() {
  return NetStat(net_stat_st_); 
}

IChannel* IOWrapper::GetIO() {
  return channel_; 
}

int IOWrapper::GetCurrentPartyId() {
  return party_; 
}

int IOWrapper::GetPartyId(const string& node_id) {
  int party_id = -1;
  auto iter = node2party_.find(node_id);
  if (iter != node2party_.end()) {
    party_id = iter->second;
    return party_id;
  }
  log_warn << "get party id error, node id:" << node_id ;
  return party_id;
}

const string& IOWrapper::GetNodeId(int party_id) {
  static string nullstr = "";
  if (party_id < 0 || party_id >= party2node_.size()) {
    log_error << "get node id error, party id:" << party_id ;
    return nullstr;
  }
  return party2node_[party_id];
}

const string& IOWrapper::GetCurrentNodeId() {
  return node_id_; 
}

const vector<string>& IOWrapper::GetResultNodes() {
  return result_nodes_;
}

const vector<string>& IOWrapper::GetParty2Node() {
 return party2node_; 
}

const vector<string>& IOWrapper::GetDataNodes() {
  return data_nodes_;
}

const map<string, int>& IOWrapper::GetComputationNodes() {
  return node2party_;
}

const vector<string>& IOWrapper::GetConnectedNodes() {
  return connected_nodes_;
}

const vector<string>& IOWrapper::GetNonComputationNodes() {
   return non_computation_nodes_; 
}
} // namespace rosetta
