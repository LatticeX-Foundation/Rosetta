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
#include "cc/modules/protocol/utility/include/util.h"
#include "cc/modules/common/include/utils/rtt_exceptions.h"
#include "cc/modules/protocol/mpc/comm/include/mpc_helper.h"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include <chrono>
#include <random>
using namespace std;

void convert_mpctype_to_double(const vector<mpc_t>& a, vector<double>& b, int precision) {
  b.resize(a.size());
  for (int i = 0; i < a.size(); i++)
    b[i] = MpcTypeToFloat(a[i], precision);
}
void convert_double_to_mpctype(const vector<double>& a, vector<mpc_t>& b, int precision) {
  b.resize(a.size());
  for (int i = 0; i < a.size(); i++)
    b[i] = FloatToMpcType(a[i], precision);
}

void convert_double_to_literal_str(const vector<double>& a, vector<string>& b, int precision) {
  b.resize(a.size());
  for (int i = 0; i < a.size(); i++)
    b[i] = std::to_string(a[i]);
}

void convert_string_to_mpctype(const vector<std::string>& a, vector<mpc_t>& b, bool human) {
  size_t size = a.size();
  b.resize(size);
  if (human) {
    for (int i = 0; i < size; i++) {
      b[i] = from_hex_str<mpc_t>(a[i]);
    }
  } else {
    for (int i = 0; i < size; i++) {
      char* p = (char*)&b[i];
      memcpy(p, a[i].data(), sizeof(mpc_t));
    }
  }
}

/**
 * convert_mpctype_to_string
 * 
 * if human == true:
 * a[i].size() == sizeof(mpc_t)
 * a[i] = 0x1234567887654321UL
 * b[i] = "1234567887654321"
 * b[i].size() == 2*sizeof(mpc_t)
 * 
 * if human == false:
 * a[i].size() == sizeof(mpc_t)
 * a[i] = 0x1234567887654321UL
 * b[i].size() == sizeof(mpc_t)
 */
void convert_mpctype_to_string(const vector<mpc_t>& a, vector<std::string>& b, bool human) {
  size_t size = a.size();
  b.resize(size);
  if (human) {
    for (int i = 0; i < size; i++) {
      b[i] = get_hex_str(a[i]);
    }
  } else {
    for (int i = 0; i < size; i++) {
      char* p = (char*)&a[i];
      b[i].assign(p, p + sizeof(mpc_t));
    }
  }
}

string encode_reveal_nodes(const vector<string>& node_ids, const vector<int>& party_ids) {
  int byte_size = 0;
  int offset = 0;
  string res;
  if (!node_ids.empty() || !party_ids.empty()) {
    byte_size += sizeof(int);    // node id and party id count
    for (int i = 0; i < node_ids.size(); i++) {
      byte_size += sizeof(char);   // node id flag
      byte_size += sizeof(int);      // node id length
      byte_size += node_ids[i].size();  // node id
    }
    for (int i = 0; i < party_ids.size(); i++) {
      byte_size += sizeof(char);  // party id flag
      byte_size += sizeof(int);     //  party id
    }

    int id_count = node_ids.size() + party_ids.size();
    res.resize(byte_size);
    memcpy(&res[offset], &id_count, sizeof(int));
    offset += sizeof(int);
    char node_id_flag = 'N';
    char party_id_flag = 'P';
    for (int i = 0; i < node_ids.size(); i++) {
      memcpy(&res[offset], &node_id_flag, sizeof(char));
      offset += sizeof(char);
      int node_id_length = node_ids[i].size();
      memcpy(&res[offset], &node_id_length, sizeof(int));
      offset += sizeof(int);
      memcpy(&res[offset], node_ids[i].data(), node_id_length);
      offset += node_id_length;
    }

    for (int i = 0; i < party_ids.size(); i++) {
      memcpy(&res[offset], &party_id_flag, sizeof(char));
      offset += sizeof(char);
      memcpy(&res[offset], &party_ids[i], sizeof(int));
      offset += sizeof(int);
    }
    return res;
  }
  int id_count = 0;
  res.resize(id_count, 0);
  memcpy(&res[0], &id_count, sizeof(int));
  return res;
}

string encode_reveal_mask(int mask) {
  int party_mask = -mask;
  string res(sizeof(int), 0);
  memcpy(&res[0], &party_mask, sizeof(int));
  return res;
}
string encode_reveal_multi_party(const vector<int> &party_ids) {
  return encode_reveal_nodes(vector<string>(), party_ids);
}
string encode_reveal_party(int party_id) {
  vector<int> party_ids = { party_id };
  return encode_reveal_multi_party(party_ids);
}
string encode_reveal_multi_node(const vector<string>& node_ids) {
  return encode_reveal_nodes(node_ids, vector<int>());
}
string encode_reveal_node(const string& node_id) {
  vector<string> node_ids = { node_id };
  return encode_reveal_multi_node(node_ids);
}

vector<string> decode_reveal_nodes(const string& bytes, const vector<string>& party2node, const vector<string>& result_nodes) {
  const char* recv_buf = &bytes[0];
  int party_size = *(const int*)recv_buf;
  vector<string> nodes;
  if (party_size > 0) {
    nodes.resize(party_size);
    const char* p = recv_buf + sizeof(int);
    for (int i = 0; i < party_size; i++) {
      char data_type = *p;
      p += sizeof(char);
      if (data_type == 'N') {
        int len = *(const int*)p;
        p += sizeof(int);
        nodes[i].assign(string(p, len));
        p += len;
      }
      else if (data_type == 'P') {
        int party_id = *(const int*)p;
        if (party_id < 0 || party_id >= party2node.size()) {
          throw other_exp("unknown party id " + to_string(party_id));
        }
        nodes[i].assign(party2node[party_id]);
        p += sizeof(int);
      }
      else {
        throw other_exp("unknown data type!") ;
      }
    }
  }
  else if (party_size < 0) {
    int party_mask = -party_size;
    nodes.reserve(party2node.size());
    for (int i = 0; i < party2node.size(); i++) {
      if (party_mask & (1 << i)) {
        nodes.push_back(party2node[i]);
      }
    }
  }
  else {
    nodes.insert(nodes.end(), result_nodes.begin(), result_nodes.end());
  }
  return nodes;
}


/**
 * get_mpc_peers
 */
vector<int> get_mpc_peers() {
  static vector<int> mpc_peers{PARTY_A, PARTY_B, PARTY_C};
  return mpc_peers;
}
