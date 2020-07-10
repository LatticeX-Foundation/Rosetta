
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

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
namespace py = pybind11;

#include <cstdio>
#include <string>
#include <iostream>

#include "cc/modules/common/include/utils/msg_id_mgr.h"

class MsgIdHandle {
 public:
  MsgIdHandle() = default;

 public:
  void update_message_id_info(const std::string& msgid_infos) {
    if (!rosetta::MsgIdMgr::Instance()->UpdateMsgIdInfo(msgid_infos))
      cerr << "update message id info failure." << std::endl;
  }
};
