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
#include "cc/modules/iowrapper/include/io_wrapper.h"

#include <string>
#include <vector>
#include <mutex>

namespace rosetta {
using std::shared_ptr;
using std::string;
using std::vector;

class IOManager {
  private:
    IOManager() = default;
    IOManager(const IOManager&) = delete;
    IOManager(IOManager&&) = delete;
    IOManager& operator=(const IOManager&) = delete;
    IOManager& operator=(IOManager&&) = delete;

  public:
    static IOManager* Instance() {
      static IOManager ioMgr;
      return &ioMgr;
    }


    static void process_error(const char* current_node_id, const char* node_id, int errorno, const char* errormsg, void*user_data);
    
    bool HasIOWrapper(const string& task_id);
    
    shared_ptr<IOWrapper> GetIOWrapper(const string& task_id);

    void SetChannel(const string& task_id, IChannel* channel);

    bool CreateChannel(const string& task_id, const string& node_id, const string& io_config_json_str);
    
    void DestroyChannel(const string& task_id);
 private:
    std::mutex ios_mutex_;
    map<string, shared_ptr<IOWrapper>> ios_;
    map<string, bool> internal_map_;
};

} // namespace rosetta
