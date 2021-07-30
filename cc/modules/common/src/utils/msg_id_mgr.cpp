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
#include "cc/modules/common/include/utils/msg_id.h"
#include "cc/modules/common/include/utils/msg_id_mgr.h"
#include "cc/modules/common/include/utils/rtt_logger.h"

#include <map>
#include <vector>
#include <string>
#include <atomic>
#include <mutex>
#include <deque>
#include <iostream>
#include <cassert>
using namespace std;

////////////////////////////////////////////////////////////
static atomic<id_type> min_idx{0};
static atomic<id_type> max_idx{65535};
static map<id_type, msg_id_t> mapmid;
static map<string, msg_id_t> mapsmid;
static map<string, id_type> mapid;
/////////////////////////////////////////////////////////////
namespace rosetta {
MsgIdMgr* MsgIdMgr::Instance() {
  static MsgIdMgr _MsgIdMgrInst;
  return &_MsgIdMgrInst;
}

bool MsgIdMgr::UpdateMsgIdInfo(const string& msg_infos) {
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
      id_type nid = strtoul(uid.c_str(), nullptr, 10);
      if (nid > _MaxId)
        _MaxId = nid;
      _msg_id_info[op_name] = msg_id_t(nid, op_name);
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

msg_id_t& MsgIdMgr::GetMsgIdFromOpName(const string& OpName) {
  assert(!OpName.empty());
  return _msg_id_info[OpName];
}

msg_id_t& MsgIdMgr::GetUniqueMsgId(const string& unique_name) {
  auto iter = _msg_id_info.find(unique_name);
  if (iter != _msg_id_info.end())
    return _msg_id_info[unique_name];
  else {
    _MaxId++;
    if (min_idx < _MaxId) {
      min_idx.store(_MaxId);
    }
    if (_MaxId >= max_idx - 1) {
      log_error << "error:uid exceeds maximum value " << max_idx;
      throw;
    }
    _msg_id_info[unique_name] = msg_id_t(_MaxId, unique_name);
    return _msg_id_info[unique_name];
  }
}

id_type MsgIdMgr::GetMaxMsgIdNum() { return _MaxId; }
} // namespace rosetta

////////////////////////////////////////////////////////////
std::mutex map_msgid_mtx_;
const msg_id_t& get_msgid(const std::string& str_msgid) {
#if USE_SHA256_ID
  std::unique_lock<std::mutex> lck(map_msgid_mtx_);
  if (mapsmid.find(str_msgid) == mapsmid.end()) {
    mapsmid[str_msgid] = msg_id_t(0, str_msgid);
  }
  return mapsmid[str_msgid];
#else
  std::unique_lock<std::mutex> lck(map_msgid_mtx_);
  id_type index = 0;
  if (mapid.find(str_msgid) == mapid.end()) {
    // new message id
    index = max_idx--;
    if (max_idx <= min_idx + 1) {
      log_error << "error: " << min_idx << " exceeds maximum value " << max_idx;
      throw;
    }
    mapid[str_msgid] = index;
    mapmid[index] = msg_id_t(index, str_msgid);
    log_info << "new msgid, index:" << index << ", src:" << str_msgid;
  } else {
    index = mapid[str_msgid];
  }
  return mapmid[index];
#endif
}
