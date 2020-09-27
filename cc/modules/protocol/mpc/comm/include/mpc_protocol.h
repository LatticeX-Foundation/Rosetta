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
#include "cc/modules/protocol/mpc/comm/include/mpc_prg.h"
#include "cc/modules/common/include/utils/perf_stats.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

extern int FLOAT_PRECISION_M;

namespace rosetta {
class RosettaConfig;
class MpcProtocol : public ProtocolBase {
  using ProtocolBase::ProtocolBase;

 public:
  virtual ~MpcProtocol() = default;

 public:
  virtual int Init(std::string config_json_str = "");
  virtual int Uninit();

  //! @attention! internal use, for cpp test cases
  virtual int Init(int partyid, std::string config_json_str = "");
  virtual int Init(int partyid, std::string config_json_str, std::string logfile);

  PerfStats GetPerfStats();
  void StartPerfStats();

 public:
  virtual shared_ptr<ProtocolOps> GetOps(const string& op_token = "") = 0;
  virtual shared_ptr<NET_IO> GetNetHandler() { return _net_io; }

 protected:
  virtual int _init_config(int partyid, const std::string& config_json);
  virtual int _init_config(const std::string& config_json);
  virtual int _init_with_config();
  virtual int _init_aeskeys();

  //! @attention! now, only for snn, will remove in the future
  virtual void _initialize_mpc_enviroment() {}

 protected:
  std::shared_ptr<MpcPRG> gseed = nullptr; // for global random seed
  std::string seed_msg_id = "[MPC] This msg id for global RandomSeed.";
  std::string pri_input_msg_id = "[MPC] This msg id for global PrivateInput.";
  shared_ptr<RosettaConfig> config = nullptr;
};
} // namespace rosetta
