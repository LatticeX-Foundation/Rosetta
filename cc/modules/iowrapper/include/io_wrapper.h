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

#include "cc/modules/common/include/utils/msg_id.h"
#include "cc/modules/iowrapper/include/stat.h"
#include "cc/modules/common/include/utils/rtt_logger.h"
#include "io/channel.h"

#include <vector>
#include <string>
#include <list>
#include <map>
#include <set>
using namespace std;

namespace rosetta {
class IOWrapper;
class ChannelConfig;

class IOWrapper {
  public:
    IOWrapper(const string& task_id, IChannel* io);

    ~IOWrapper();

    void sync_with(const msg_id_t& msg_id);

    void sync() { sync_with(msg_id_t("0123456789")); }

    ssize_t recv(int party, char* data, size_t len, const msg_id_t& msg_id_t, int64_t timeout = -1);

    ssize_t send(int party, const char* data, size_t len, const msg_id_t& msg_id, int64_t timeout = -1);

    ssize_t recv(const string& node_id, char* data, size_t len, const msg_id_t& msg_id_t, int64_t timeout = -1);

    ssize_t send(const string& node_id, const char* data, size_t len, const msg_id_t& msg_id, int64_t timeout = -1);

    template <typename T>
    ssize_t recv(int party, vector<T>& data, size_t n, const msg_id_t& msg_id) {
      return recv(party, (char *)data.data(), n * sizeof(T), msg_id); 
    }

    template <typename T>
    ssize_t send(int party, const vector<T>& data, size_t n, const msg_id_t& msg_id) {
      return send(party, (const char *)data.data(), n * sizeof(T), msg_id);
    }

    template <typename T>
    ssize_t recv(const string& node_id, vector<T>& data, size_t n, const msg_id_t& msg_id) {
      return recv(node_id, (char *)data.data(), n * sizeof(T), msg_id); 
    }

    template <typename T>
    ssize_t send(const string& node_id, const vector<T>& data, size_t n, const msg_id_t& msg_id) {
      return send(node_id, (const char *)data.data(), n * sizeof(T), msg_id);
    }

    ssize_t recv(const string& node_id, vector<string>& data, size_t n, const msg_id_t& msg_id) {
      ssize_t ret = 0;
      for (size_t i = 0; i < n; i++) {
        ret += recv(node_id, &data[i][0], data[i].size(), msg_id);
      }
      return ret;
    }

    ssize_t send(const string& node_id, const vector<string>& data, size_t n, const msg_id_t& msg_id) {
      ssize_t ret = 0;
      for (size_t i = 0; i < n; i++) {
        ret += send(node_id, data[i].data(), data[i].size(), msg_id);
      }
      return ret;
    }

    string recv_msg(const string& node_id, const string& msg_id, int msg_len) {
      string str(msg_len, 0);
      msg_id_t msgid(msg_id.c_str());
      recv(node_id, &str[0], msg_len, msgid);
      return str;
    }

    void send_msg(const string& node_id, const string& msg_id, const string& msg) {
      msg_id_t msgid(msg_id.c_str());
      send(node_id, msg.data(), msg.size(), msgid);
    }

    static void process_error(const char* current_node_id, const char* node_id, int errorno, const char* errormsg, void*user_data);

    void statistics(string str = "") ;

    void clear_statistics();

    NetStat net_stat();

    IChannel* GetIO();

    int GetCurrentPartyId();

    int GetPartyId(const string& node_id);

    const string& GetNodeId(int party_id);

    const string& GetCurrentNodeId();

    const vector<string>& GetResultNodes();

    const vector<string>& GetParty2Node();

    const vector<string>& GetDataNodes();

    const map<string, int>& GetComputationNodes();

    const vector<string>& GetConnectedNodes();
    
    const vector<string>& GetNonComputationNodes();

  private:
    int party_ =  -1;
    int parties_ = 0;
    string node_id_ = "";
    int nodes_ = 0;
    string task_id_ = "";

#ifdef DEBUG
    std::mutex send_mutex_;
    std::mutex recv_mutex_;
    map<string, map<msg_id_t, int>> send_seq_;
    map<string, map<msg_id_t, int>> recv_seq_;
#endif

    NetStat_st net_stat_st_;
    IChannel* channel_ = nullptr;
    vector<string> party2node_;
    map<string, int> node2party_;
    vector<string> connected_nodes_;
    vector<string> data_nodes_;
    vector<string> result_nodes_;
    vector<string> non_computation_nodes_;
};
} // namespace rosetta

