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
#include "cc/modules/protocol/public/include/protocol_base.h"
#include "cc/modules/protocol/zk/mystique/include/zk_int_fp.h"
#include "cc/modules/protocol/zk/mystique/include/zk_int_fp_eigen.h"
#include <iostream>
#include "emp-tool/emp-tool.h"
#if defined(__linux__)
#include <sys/time.h>
#include <sys/resource.h>
#elif defined(__APPLE__)
#include <unistd.h>
#include <sys/resource.h>
#include <mach/mach.h>
#endif

using namespace emp;
using namespace std;

#include "cc/modules/protocol/zk/mystique/include/wvr_util.h"

namespace rosetta {
class RosettaConfig;
class MystiqueProtocol : public ProtocolBase {
 public:
  MystiqueProtocol(const string task_id="") : ProtocolBase("Mystique", 2, task_id) {}

 public:
  virtual ~MystiqueProtocol() = default;

 public:
  virtual int Init();

  //! @attention! internal use, for cpp test cases
  virtual int Init(std::string logfile);

  virtual int Uninit();

  virtual PerfStats GetPerfStats();
  virtual void StartPerfStats();

 public:
  virtual shared_ptr<ProtocolOps> GetOps(const msg_id_t& msgid);
  virtual shared_ptr<NET_IO> GetNetHandler() { return net_io_; }
  

  ZK_NET_IO* zk_ios[THREAD_NUM + 1];
  std::string host = "127.0.0.1";
  int port=11224, party;
  shared_ptr<RosettaConfig> config = nullptr;
};

class MystiqueProtocolFactory : public IProtocolFactory {
 public:
  MystiqueProtocolFactory() {}

 public:
  shared_ptr<ProtocolBase> Create(const string& task_id) { return std::make_shared<MystiqueProtocol>(task_id); }
};

} // namespace rosetta
