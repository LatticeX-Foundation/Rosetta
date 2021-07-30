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
#include "cc/modules/protocol/utility/include/version_compat_utils.h"
// #include "cc/modules/common/include/utils/secure_encoder.h"
#include "cc/modules/iowrapper/include/io_manager.h"
#include "cc/modules/protocol/mpc/snn/include/snn_protocol.h"
#include "cc/modules/protocol/mpc/snn/include/snn_ops.h"


using SnnProtoType = rosetta::snn::SnnProtocol;
using namespace rosetta;
// using namespace rosetta::snn;
// using namespace rosetta::convert;

// for snn test
#define SNN_PROTOCOL_TEST_INIT(partyid)                                               \
  string logfile = "log/" + get_file_name(__FILENAME__) + "-" + std::to_string(partyid) + ".log"; \
  Logger::Get().log_to_stdout(false);                                                 \
  Logger::Get().set_filename(logfile);                                                \
  Logger::Get().set_level(0);                                                         \
  string node_id;                                                                     \
  string config_json;                                                                 \
  rosetta_old_conf_parse(node_id, config_json, partyid, "CONFIG.json");               \
  IOManager::Instance()->CreateChannel("", node_id, config_json);                     \
  SnnProtoType snn0;                                                                  \
  snn0.Init(logfile);                                                                 \
  shared_ptr<NET_IO> net_io = snn0.GetNetHandler();                                   \
  string node_id_0 = net_io->GetNodeId(0);                                            \
  string node_id_1 = net_io->GetNodeId(1);                                            \
  string node_id_2 = net_io->GetNodeId(2);                                            \
  vector<string> reveal_receivers = {"P0", "P1", "P2"};                               \
  rosetta::attr_type reveal_attr;                                                     \
  reveal_attr["receive_parties"] = receiver_parties_pack(reveal_receivers);           \

#define SNN_PROTOCOL_INTERNAL_TEST_INIT(partyid) throw

#define SNN_PROTOCOL_TEST_UNINIT(partyid)                                             \
  snn0.Uninit()
