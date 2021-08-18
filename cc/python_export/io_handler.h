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

#include <cstdio>
#include <string>
#include <vector>
#include <iostream>
#include <mutex>
#include <fstream>
using namespace std;

#include "cc/third_party/io/include/io/channel.h"
#include "cc/modules/iowrapper/include/io_manager.h"
#include "cc/modules/iowrapper/include/io_wrapper.h"

class IOHandler {
  public:
    IOHandler() {}
    bool create_io(const string& task_id, const string& node_id, const std::string &config_str) {
      return IOManager::Instance()->CreateChannel(task_id, node_id, config_str);
    }
    bool has_io_wrapper(const string& task_id) {
      return IOManager::Instance()->HasIOWrapper(task_id);
    }
    shared_ptr<IOWrapper> get_io_wrapper(const string& task_id) {
      return IOManager::Instance()->GetIOWrapper(task_id);
    }
    void set_channel(const string& task_id, IChannel* channel) {
      IOManager::Instance()->SetChannel(task_id, channel);
    }
};
