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
#include "../include/_test_common.h"

#include "cc/modules/protocol/mpc/snn/src/snn_protocol.h"
#include "cc/modules/protocol/mpc/snn/src/snn_protocol_ops.h"
#include "cc/modules/common/include/utils/secure_encoder.h"
using namespace rosetta;
using namespace rosetta::snn;
using namespace rosetta::convert;

// for helix test
#define SNN_PROTOCOL_TEST_INIT(partyid)                                               \
  string logfile = "log/" + string(__FILENAME__) + "-" + to_string(partyid) + ".log"; \
  rosetta::SnnProtocol snn0;                                                          \
  snn0.Init(partyid, "CONFIG.json", logfile);                                         \
  cout << "partyid:" << partyid << " ppid:" << getppid() << " pid:" << getpid() << " beg" << endl

#define SNN_PROTOCOL_INTERNAL_TEST_INIT(partyid) throw

#define SNN_PROTOCOL_TEST_UNINIT(partyid)                                                          \
  cout << "partyid:" << partyid << " ppid:" << getppid() << " pid:" << getpid() << " end" << endl; \
  snn0.Uninit()
