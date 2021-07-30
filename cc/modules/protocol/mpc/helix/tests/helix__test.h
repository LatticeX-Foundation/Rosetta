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
#include "cc/modules/protocol/mpc/helix/include/helix_internal.h"
#include "cc/modules/protocol/mpc/helix/include/helix_impl.h"
#include "cc/third_party/rapidjson/include/rapidjson/document.h"
#include "cc/third_party/rapidjson/include/rapidjson/writer.h"
#include "cc/third_party/rapidjson/include/rapidjson/prettywriter.h"
#include "cc/third_party/rapidjson/include/rapidjson/stringbuffer.h"
#include "cc/modules/iowrapper/include/io_manager.h"

using namespace rosetta;
using namespace rosetta::helix;


// for helix test
#define HELIX_PROTOCOL_TEST_INIT(partyid)                                             \
  string logfile = "log/" + get_file_name(__FILENAME__) + "-" + to_string(partyid) + ".log"; \
    Logger::Get().log_to_stdout(false);                                                             \
  Logger::Get().set_filename(logfile);                                           \
  Logger::Get().set_level(3);                                           \
  string node_id;                                                                     \
  string config_json;                                                                 \
  rosetta_old_conf_parse(node_id, config_json, partyid, "CONFIG.json");               \
  IOManager::Instance()->CreateChannel("", node_id, config_json);                         \
  rosetta::HelixImpl helix0;                                                          \
  helix0.Init(logfile);                                                               \
  shared_ptr<NET_IO> net_io = helix0.GetNetHandler();                                 \
  string node_id_0 = net_io->GetNodeId(0);                                            \
  string node_id_1 = net_io->GetNodeId(1);                                            \
  string node_id_2 = net_io->GetNodeId(2);                                            \
  vector<string> receivers = {"P0", "P1", "P2"};                                      \
  rosetta::attr_type reveal_attr;                                                     \
  reveal_attr["receive_parties"] = receiver_parties_pack(receivers);

#define HELIX_PROTOCOL_INTERNAL_TEST_INIT(partyid) \
  HELIX_PROTOCOL_TEST_INIT(partyid)                \
  msg_id_t msg__FILE(string(__FILE__));            \
  auto hi = helix0.GetInternal(msg__FILE);

#define HELIX_PROTOCOL_TEST_UNINIT(partyid)                                                        \
  helix0.Uninit()
