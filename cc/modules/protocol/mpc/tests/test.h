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
#include <cassert>
#include <cstdlib>

#if PROTOCOL_MPC_TEST_SNN
#include "cc/modules/protocol/mpc/snn/tests/snn__test.h"

#define GET_PROTOCOL(proto)                                 \
  SnnProtoType* proto = new SnnProtoType(); \
  std::string protocol_name("snn")
using namespace rosetta;
using namespace rosetta::snn;
using namespace rosetta::convert;
#elif PROTOCOL_MPC_TEST_HELIX
#include "cc/modules/protocol/mpc/helix/include/helix_internal.h"
#include "cc/modules/protocol/mpc/helix/include/helix_impl.h"
#define GET_PROTOCOL(proto)                               \
  rosetta::HelixImpl* proto = new rosetta::HelixImpl(); \
  std::string protocol_name("helix")
using namespace rosetta;
#elif PROTOCOL_MPC_TEST_NAIVE
#include "cc/modules/protocol/mpc/naive/include/naive_impl.h"
#include "cc/modules/protocol/mpc/naive/include/naive_ops_impl.h"
#define GET_PROTOCOL(proto) \
  rosetta::NaiveProtocol* proto = new rosetta::NaiveProtocol();\
  std::string protocol_name("naive")
#else
#error "unsupported protocol!"
#endif

#define PROTOCOL_MPC_TEST_INIT(partyid)                                                           \
  GET_PROTOCOL(mpc_proto);                                                                        \
  string logfile = "log/mpc_tests_" + protocol_name + "_" + get_file_name(__FILENAME__) + "-" + \
    to_string(partyid);                                                                           \
  Logger::Get().log_to_stdout(false);                                                             \
  Logger::Get().set_filename(logfile + "-backend.log");                                           \
  Logger::Get().set_level(0);                                           \
  string node_id;                                                                     \
  string task_id = "";\
  string config_json;                                                                 \
  rosetta_old_conf_parse(node_id, config_json, partyid, "CONFIG.json");               \
  IOManager::Instance()->CreateChannel(task_id, node_id, config_json);                         \
  mpc_proto->Init(logfile + "-console.log");                              \
  shared_ptr<NET_IO> net_io = mpc_proto->GetNetHandler();    \
  string node_id_0 = net_io->GetNodeId(0);                                            \
  string node_id_1 = net_io->GetNodeId(1);                                            \
  string node_id_2 = net_io->GetNodeId(2);                                            \
  vector<string> reveal_receivers = {"P0", "P1", "P2"};                                      \
  rosetta::attr_type reveal_attr;                                                     \
  reveal_attr["receive_parties"] = receiver_parties_pack(reveal_receivers);


#define PROTOCOL_MPC_INTERNAL_TEST_INIT(partyid) \
  PROTOCOL_MPC_TEST_INIT(partyid)                \
  msg_id_t msg__FILE(string(__FILE__));            \
  auto hi = mpc_proto->GetInternal(msg__FILE);

#define PROTOCOL_MPC_TEST_UNINIT(partyid)                                                          \
  mpc_proto->Uninit();                                                                             \
  delete mpc_proto

// around equal for two sets
template<class InputIt1, class InputIt2>
bool around_equal(InputIt1 first1, InputIt1 last1,
            InputIt2 first2, InputIt2 last2,
            typename InputIt1::value_type delta, const std::string& tag="") {
  assert(last1-first1 <= last2-first2);

  size_t step = 0;
  auto size = last1 - first1;
  bool result = true;
#if 0
  std::string bold_red("");
  std::string light_red("");
  std::string bold_green("");
  std::string light_green("");
  std::string reset_color("");
#else
  std::string bold_red("\033[1;31m");
  std::string light_red("\033[21;31m");
  std::string bold_green("\033[0;32m");
  std::string light_green("\033[0;32m");
  std::string reset_color("\033[0m");
#endif
  for (size_t pos = 0; pos < size; pos++)
  {
    if (!(std::abs(*(first1+pos) - *(first2+pos)) <= delta)) {
      std::cout << light_red << "[" << tag << "]" << reset_color << " : " 
                << bold_red << "***Error***  index: " << pos << ", value: " << *(first1+pos) << " != " << *(first2+pos) << reset_color << std::endl;
      result = false;
    }
  }
  
  if (result) {
    std::cout << light_green << "[" << tag << "]" << reset_color 
              << " => " << bold_green << "Pass." << reset_color << std::endl;
  }
  return result;
}

#define DEFINE_TEST_VARIABLES(context, attr) \
  string task_id = mpc_proto->GetMpcContext()->TASK_ID;\
  string node_id = mpc_proto->GetMpcContext()->NODE_ID;\
  string node_id_0 = attr->at("P0");\
  string node_id_1 = attr->at("P1");\
  string node_id_2 = attr->at("P2")\

#define HD_AROUND_EQUAL(VAL1, VAL2) \
    if (node_id == node_id_0) around_equal(VAL1.begin(), VAL1.end(), VAL2.begin(), VAL2.end(), 0.001)

#define SIMPLE_AROUND_EQUAL(VAL1, VAL2) \
    if (node_id == node_id_0) around_equal(VAL1.begin(), VAL1.end(), VAL2.begin(), VAL2.end(), 0.1)

#define AROUND_EQUAL(VAL1, VAL2, ERROR) \
    if (node_id == node_id_0) around_equal(VAL1.begin(), VAL1.end(), VAL2.begin(), VAL2.end(), ERROR)

#define HD_AROUND_EQUAL_T(VAL1, VAL2, TAG) \
    if (node_id == node_id_0) around_equal(VAL1.begin(), VAL1.end(), VAL2.begin(), VAL2.end(), 0.01, TAG)

#define SIMPLE_AROUND_EQUAL_T(VAL1, VAL2, TAG) \
    if (node_id == node_id_0) around_equal(VAL1.begin(), VAL1.end(), VAL2.begin(), VAL2.end(), 0.1, TAG)

#define AROUND_EQUAL_T(VAL1, VAL2, ERROR, TAG) \
    if (node_id == node_id_0) around_equal(VAL1.begin(), VAL1.end(), VAL2.begin(), VAL2.end(), ERROR, TAG)
