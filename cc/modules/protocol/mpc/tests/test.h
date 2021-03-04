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

#if PROTOCOL_MPC_TEST_SNN
#include "cc/modules/protocol/mpc/snn/src/snn_protocol.h"
#include "cc/modules/protocol/mpc/snn/src/snn_protocol_ops.h"
#include "cc/modules/common/include/utils/secure_encoder.h"
#define GET_PROTOCOL(proto)                                 \
  rosetta::MpcProtocol* proto = new rosetta::SnnProtocol(); \
  std::string protocol_name("SecureNN")
using namespace rosetta;
using namespace rosetta::snn;
using namespace rosetta::convert;
#elif PROTOCOL_MPC_TEST_HELIX
#include "cc/modules/protocol/mpc/helix/include/helix_internal.h"
#include "cc/modules/protocol/mpc/helix/include/helix_impl.h"
#define GET_PROTOCOL(proto)                               \
  rosetta::MpcProtocol* proto = new rosetta::HelixImpl(); \
  std::string protocol_name("Helix")
using namespace rosetta;
#else
#error "unsupported protocol!"
#endif

#define PROTOCOL_MPC_TEST_INIT(partyid)                                                           \
  GET_PROTOCOL(mpc_proto);                                                                        \
  string logfile = "log/protocol_mpc_tests_" + protocol_name + "_" + string(__FILENAME__) + "-" + \
    to_string(partyid);                                                                           \
  Logger::Get().log_to_stdout(false);                                                             \
  Logger::Get().set_filename(logfile + "-backend.log");                                           \
  mpc_proto->Init(partyid, "CONFIG.json", logfile + "-console.log");                              \
  cout << "partyid:" << partyid << " ppid:" << getppid() << " pid:" << getpid() << " beg" << endl

#define PROTOCOL_MPC_TEST_UNINIT(partyid)                                                          \
  cout << "partyid:" << partyid << " ppid:" << getppid() << " pid:" << getpid() << " end" << endl; \
  mpc_proto->Uninit();                                                                             \
  delete mpc_proto
