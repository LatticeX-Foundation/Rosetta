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
#include "cc/modules/protocol/utility/include/prg.h"
#include "cc/modules/common/include/utils/perf_stats.h"
#include "cc/modules/protocol/mpc/comm/include/mpc_prg_controller.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>


namespace rosetta {

class MpcProtocol : public ProtocolBase {
  using ProtocolBase::ProtocolBase;

 public:
  MpcProtocol(const string& protocol, int parties=3, const string& task_id="");
  virtual ~MpcProtocol() = default;

 public:
  virtual int Init();
  virtual int Uninit();

  //! @attention! internal use, for cpp test cases
  virtual int Init(std::string logfile);

  virtual PerfStats GetPerfStats();
  virtual void StartPerfStats();

 public:
  // virtual shared_ptr<ProtocolOps> GetOps(const msg_id_t& msgid) = 0;
  virtual shared_ptr<NET_IO> GetNetHandler() { return net_io_; }

 protected:
  //virtual int _init_config(int partyid, const std::string& config_json);
  //virtual int _init_config(const std::string& config_json);
  //virtual int _init_with_config();
  virtual int InitAesKeys();

  virtual int OfflinePreprocess() {
    tlog_debug << "MPCProtocol: do nothing during offline preprocess.";
    return 0; 
  }

  //! @attention! now, only for snn, will remove in the future
  virtual void InitMpcEnvironment() {}

 protected:
  std::shared_ptr<RttPRG> gseed_ = nullptr; // for global random seed
  std::shared_ptr<MpcKeyPrgController> key_prg_controller_ = nullptr;
  std::string seed_msg_id_ = "[MPC] This msg id for global RandomSeed.";
};
} // namespace rosetta
