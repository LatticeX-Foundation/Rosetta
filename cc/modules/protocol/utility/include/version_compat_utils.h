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

#include <string>
#include <vector>
#include <fstream>
#include "cc/modules/common/include/utils/rtt_logger.h"
#include "cc/third_party/rapidjson/include/rapidjson/document.h"
#include "cc/third_party/rapidjson/include/rapidjson/writer.h"
#include "cc/third_party/rapidjson/include/rapidjson/prettywriter.h"
#include "cc/third_party/rapidjson/include/rapidjson/stringbuffer.h"
#include "cc/modules/iowrapper/include/io_manager.h"


using std::string;
using std::vector;
using std::ifstream;
using std::cout;
using std::endl;

static void rosetta_old_conf_parse(string& node_id, string& config_json, int party_id, const string& config_file) {
  string sjson(config_file);
  ifstream ifile(config_file);
  if (!ifile.is_open()) {
    //log_warn << "open " << config_file << " error!\n";
    log_debug << "try to load as json string" ;
  } else {
    sjson = "";
    while (!ifile.eof()) {
      string s;
      getline(ifile, s);
      sjson += s;
    }
    ifile.close();
  }
  config_json = sjson;

  rapidjson::Document doc;
  if (doc.Parse(sjson.data()).HasParseError()) {
    log_error << "parser " << config_file << " error!\n";
    return;
  }
  if (doc.HasMember("MPC")) {
    node_id = "P" + std::to_string(party_id);
    rapidjson::Document new_doc;
    new_doc.SetObject();
    rapidjson::Value& mpc = doc["MPC"];
    rapidjson::Value node_info;
    node_info.SetArray();
    for (int i = 0; i < 3; i++) {
      rapidjson::Value mpc_node;
      mpc_node.SetObject();
      rapidjson::Value nodeid;
      const char* node_str = string("P" + std::to_string(i)).c_str();
      rapidjson::Value& old_node = mpc[node_str];
      nodeid.SetString(node_str, new_doc.GetAllocator());
      mpc_node.AddMember("NODE_ID", nodeid, new_doc.GetAllocator());
      mpc_node.AddMember("NAME", old_node["NAME"], new_doc.GetAllocator());
      mpc_node.AddMember("HOST", old_node["HOST"], new_doc.GetAllocator());
      mpc_node.AddMember("PORT", old_node["PORT"], new_doc.GetAllocator());
      node_info.PushBack(mpc_node, new_doc.GetAllocator());
    }
    new_doc.AddMember("NODE_INFO", node_info, new_doc.GetAllocator());

    rapidjson::Value data_nodes;
    data_nodes.SetArray();
    for (int i = 0; i < 3; i++) {
      rapidjson::Value node;
      node.SetString(string("P" + std::to_string(i)).c_str(), new_doc.GetAllocator());
      data_nodes.PushBack(node, new_doc.GetAllocator());
    }
    new_doc.AddMember("DATA_NODES", data_nodes, new_doc.GetAllocator());

    rapidjson::Value computation_nodes;
    computation_nodes.SetObject();
    for (int i = 0; i < 3; i++) {
      rapidjson::Value name;
      rapidjson::Value value;
      name.SetString(string("P" + std::to_string(i)).c_str(), new_doc.GetAllocator());
      value.SetInt(i);
      computation_nodes.AddMember(name, value, new_doc.GetAllocator());
    }
    new_doc.AddMember("COMPUTATION_NODES", computation_nodes, new_doc.GetAllocator());

    rapidjson::Value result_nodes;
    result_nodes.SetArray();
    for (int i = 0; i < 3; i++) {
      rapidjson::Value node;
      node.SetString(string("P" + std::to_string(i)).c_str(), new_doc.GetAllocator());
      result_nodes.PushBack(node, new_doc.GetAllocator());
    }
    new_doc.AddMember("RESULT_NODES", result_nodes, new_doc.GetAllocator());
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    new_doc.Accept(writer);
    log_info << "config json:" << buffer.GetString() ;
    config_json = buffer.GetString();
  } else {
    config_json = sjson;
    rapidjson::Value& computation_nodes = doc["COMPUTATION_NODES"];
    for (auto iter = computation_nodes.MemberBegin(); iter != computation_nodes.MemberEnd();
         iter++) {
      if (iter->value.GetInt() == party_id) {
        node_id = iter->name.GetString();
        break;
      }
    }
  }
}


static string receiver_parties_pack(const vector<string>& parities) {
  string data(1024, 0);
  int size = (int)parities.size();
  int data_size = sizeof(int);
  memcpy(&data[0], &size, sizeof(size));
  for (auto &elem : parities) {
    data[data_size] = 'N';
    data_size += sizeof(char);
    int size = elem.size();
    memcpy(&data[data_size], &size, sizeof(size));
    data_size += sizeof(size);

    memcpy(&data[data_size], elem.data(), elem.size());
    data_size += size;
  }

  data.resize(data_size);
  // cout << "data size: " << data_size << endl;
  return data;
}

