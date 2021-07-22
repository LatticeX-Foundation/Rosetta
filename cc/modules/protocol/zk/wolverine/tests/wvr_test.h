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
#include "cc/modules/protocol/utility/include/_test_common.h"

#include "cc/modules/protocol/zk/wolverine/include/wolverine_impl.h"
#include "cc/modules/protocol/zk/wolverine/include/wolverine_ops_impl.h"

#include "cc/modules/common/include/utils/helper.h"
#include "cc/modules/common/include/utils/model_tool.h"
#include "cc/modules/common/include/utils/simple_timer.h"
using namespace rosetta;
using namespace rosetta::zk;

// for ZK test
#define WVR_PROTOCOL_TEST_INIT(partyid)                                                            \
  string logfile = "log/" + string(__FILENAME__) + "-" + to_string(partyid); \
  Logger::Get().log_to_stdout(false);                                                              \
  Logger::Get().set_filename(logfile + "-backend.log");                                            \
  Logger::Get().set_level(1);                                                                      \
  rosetta::WolverineProtocol WVR0;                                                                 \
  WVR0.Init(partyid, "CONFIG.json", logfile + "-console.log");                                     \
  cout << "partyid:" << partyid << " ppid:" << getppid() << " pid:" << getpid() << " beg" << endl

#define WVR_PROTOCOL_TEST_UNINIT(partyid)                                                          \
  cout << "partyid:" << partyid << " ppid:" << getppid() << " pid:" << getpid() << " end" << endl; \
  WVR0.Uninit()
