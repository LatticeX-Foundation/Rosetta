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
  MsgIdMgr(){};
  MsgIdMgr(const MsgIdMgr&) = delete;
  MsgIdMgr(MsgIdMgr&&) = delete;
  MsgIdMgr& operator=(const MsgIdMgr&) = delete;
  MsgIdMgr& operator=(MsgIdMgr&&) = delete;

 public:
  static MsgIdMgr* Instance();

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
  bool UpdateMsgIdInfo(const string& msg_infos);

  /**
   * @desc: Get the message id from the operation name
   * @param:
   * 	OpName, the operation name
   * @returns:
   * 	messsage id info
   */
  msg_id_t& GetMsgIdFromOpName(const string& OpName);

  /**
   * @desc: Get the unique message id(for rand_seed ...)
   * @param:unique_name
   * 	unique_name, the unique name
   * @returns:
   * 	unique messsage id info
   */
  msg_id_t& GetUniqueMsgId(const string& unique_name);

  /**
   * @desc: Get current max message id numerical value
   * @param: None
   * @returns: max messsage id value
   */
  id_type GetMaxMsgIdNum();

 private:
  // message id maping to message id info
  unordered_map<string, msg_id_t> _msg_id_info;

  // default graph max id value
  std::atomic<id_type> _MaxId{0};

  // delimiter
  const char _delim = '\n';
  const char _sub_delim = '\t';
};
} // namespace rosetta
