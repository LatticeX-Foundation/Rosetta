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
#include "cc/modules/iowrapper/include/io_manager.h"
#include "io/internal_channel.h"
#include <iostream>
#include "cc/modules/common/include/utils/rtt_exceptions.h"

using namespace std;

namespace rosetta {

void IOManager::process_error(const char* current_node_id, const char* node_id, int errorno, const char* errormsg, void*user_data) {
  log_error << "the connection to party " << node_id << " is broken, errorno:" << errorno << " errormsg:" << errormsg ;
}

bool IOManager::CreateChannel(const string& task_id, const string& node_id, const string& io_config_json_str) {
  log_debug << "DEBUG: IM ActivateIO " << "task id:" << task_id << " node id:" << node_id << " config: " << io_config_json_str;
  std::unique_lock<std::mutex> lck(ios_mutex_);
  auto iter = ios_.find(task_id);
  if (iter != ios_.end()) {
    return false;
  }
  IChannel* channel = CreateInternalChannel(task_id.c_str(), node_id.c_str(), io_config_json_str.c_str(), process_error);
  shared_ptr<IOWrapper> io = make_shared<IOWrapper>(task_id, channel);

  ios_.insert(std::pair<string, shared_ptr<IOWrapper>>(task_id, io));
  internal_map_.insert(std::pair<string, bool>(task_id, true));
  log_debug << "task id:" << task_id <<  " IOManager::ActivateIO Done!" ;
  return true;
}

void IOManager::SetChannel(const string& task_id, IChannel* channel) {
  std::unique_lock<std::mutex> lck(ios_mutex_);
  auto iter = ios_.find(task_id);
  if (iter == ios_.end()) {
    shared_ptr<IOWrapper> io = make_shared<IOWrapper>(task_id, channel);
    ios_.insert(std::pair<string, shared_ptr<IOWrapper>>(task_id, io));
    internal_map_.insert(std::pair<string, bool>(task_id, false));
  }
}

shared_ptr<IOWrapper> IOManager::GetIOWrapper(const string& task_id) {
  std::unique_lock<std::mutex> lck(ios_mutex_);
  auto iter = ios_.find(task_id);
  if (iter != ios_.end()) {
    return iter->second;
  }
  log_error << "task id:" << task_id << " get null iowrapper";
  return nullptr;
}

bool IOManager::HasIOWrapper(const string& task_id) {
  std::unique_lock<std::mutex> lck(ios_mutex_);
  auto iter = ios_.find(task_id);
  if (iter != ios_.end()) {
    return true;
  }
  return false;
}

void IOManager::DestroyChannel(const string& task_id) {
  IChannel *channel = nullptr;
  {
    std::unique_lock<std::mutex> lck(ios_mutex_);
    auto iter = internal_map_.find(task_id);
    if (iter != internal_map_.end() && iter->second) {
      auto iter2 = ios_.find(task_id);
      channel = iter2->second->GetIO();
      ios_.erase(iter2);
      internal_map_.erase(iter);
    }
  }
  if (channel != nullptr) {
    DestroyInternalChannel(channel);
  }
}

} // namespace rosetta
