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
#include <memory>
#include <string>
#include <unordered_map>
#include <iostream>
#include <assert.h>
#include <limits>
#include <atomic>

#include "msg_id.h"

namespace rosetta {
using std::shared_ptr;
using std::string;
using std::unordered_map;

class MsgIdMgr {
 private:
  MsgIdMgr() : _MaxId(0){};
  MsgIdMgr(const MsgIdMgr&) = delete;
  MsgIdMgr(MsgIdMgr&&) = delete;
  MsgIdMgr& operator=(const MsgIdMgr&) = delete;
  MsgIdMgr& operator=(MsgIdMgr&&) = delete;

 public:
  static MsgIdMgr* Instance() {
    static MsgIdMgr _MsgIdMgrInst;
    return &_MsgIdMgrInst;
  }

 public:
  /**
   * @desc: Update message id info, generate message ids at python level
   * @param:
   * 	msg_infos, the message id informations
   * 		message id format:
   * 		"rosetta op name" +  "\t" + message id index  + "\n".
   * @returns:
   * 	True if success, otherwise errcode.
   */
  bool UpdateMsgIdInfo(const string& msg_infos) {
    if (msg_infos.empty())
      return false;

    bool ret = true;
    size_t start_pos = 0;
    size_t len = msg_infos.find_first_of(_delim);
    string unit_info;
    while (len != string::npos) {
      unit_info = msg_infos.substr(start_pos, (len - start_pos));
      size_t pos = unit_info.find_first_of(_sub_delim);
      if (pos != string::npos) {
        string op_name = unit_info.substr(0, pos);
        string uid = unit_info.substr(pos + 1);
        unsigned nid = strtoul(uid.c_str(), nullptr, 10);
        if (nid > _MaxId)
          _MaxId = nid;
        _msg_id_info[op_name] = MsgId(uid);
      } else {
        std::cout << "message id format incorret!(" << unit_info << ")" << std::endl;
        ret = false;
        break;
      }

      start_pos = len + 1;
      len = msg_infos.find_first_of(_delim, start_pos);
    }

    return ret;
  }

  /**
   * @desc: Get the message id from the operation name
   * @param:
   * 	OpName, the operation name
   * @returns:
   * 	messsage id info
   */
  MsgId& GetMsgIdFromOpName(const string& OpName) {
    assert(!OpName.empty());
    return _msg_id_info[OpName];
  }

  /**
   * @desc: Get the unique message id(for rand_seed ...)
   * @param:unique_name
   * 	unique_name, the unique name
   * @returns:
   * 	unique messsage id info
   */
  MsgId& GetUniqueMsgId(const string& unique_name) {
    auto iter = _msg_id_info.find(unique_name);
    if (iter != _msg_id_info.end())
      return _msg_id_info[unique_name];
    else {
      _MaxId++;
      if (_MaxId > std::numeric_limits<unsigned short>::max()) {
        std::cerr << "error:uid exceeds maximum value "
                  << std::numeric_limits<unsigned short>::max() << std::endl;
        throw;
      }
      _msg_id_info[unique_name] = MsgId(_MaxId);
      return _msg_id_info[unique_name];
    }
  }

  /**
   * @desc: Get current max message id numerical value
   * @param: None
   * @returns: max messsage id value
   */
  unsigned short GetMaxMsgIdNum() { return _MaxId; }

 private:
  // message id maping to message id info
  unordered_map<string, MsgId> _msg_id_info;

  // default graph max id value
  std::atomic<unsigned int> _MaxId;

  // delimiter
  const char _delim = '\n';
  const char _sub_delim = '\t';
};
} // namespace rosetta
