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

#include <stdlib.h>
#include <functional>
#include "cc/modules/protocol/mpc/tests/test.h"
#include "cc/modules/protocol/utility/include/version_compat_utils.h"
#include "cc/modules/iowrapper/include/io_manager.h"
#include "cc/modules/protocol/mpc/comm/include/mpc_protocol.h"

using namespace rosetta;

/**
 * multiple-task test, now only support SecureNN, Helix  
*/

#if PROTOCOL_MPC_TEST_SNN
// SecureNN
#include "cc/modules/protocol/mpc/snn/include/snn_protocol.h"
using SnnProtoType = rosetta::snn::SnnProtocol;
using namespace rosetta;
using namespace rosetta::snn;
using namespace rosetta::convert;
#define MPC_TEST_PROTOCOL_NAME "snn"

#elif PROTOCOL_MPC_TEST_HELIX
// Helix
#include "cc/modules/protocol/mpc/helix/include/helix_internal.h"
#include "cc/modules/protocol/mpc/helix/include/helix_impl.h"
using namespace rosetta;
#define MPC_TEST_PROTOCOL_NAME "helix"
#else
#error "unsupported protocol!"
#endif


using MpcTaskCodeFunc = std::function<void(shared_ptr<MpcProtocol>, shared_ptr<attr_type>)>;

static inline shared_ptr<MpcProtocol> mpc_task_test_create_protocol(const string& task_id) {
#if PROTOCOL_MPC_TEST_SNN
  return std::make_shared<SnnProtoType>(task_id);                                        
#elif PROTOCOL_MPC_TEST_HELIX
  return std::make_shared<HelixImpl>(task_id);
#else
  #error "create not support protocol"
  throw std::runtime_error("bad protocol !");
#endif
}

// for snn test
static inline shared_ptr<MpcProtocol> mpc_task_test_create_init_protocol(int partyid, const string& task_id, const string& logfile, 
                                                        attr_type* reveal_attr)  {                      
  shared_ptr<MpcProtocol> protocol_inst = mpc_task_test_create_protocol(task_id);
  return protocol_inst;
}

static inline void mpc_task_test_uninit_protocol(int partyid, shared_ptr<MpcProtocol> protocol_inst) {
  protocol_inst->Uninit();
}


static void run_task_threads(vector<shared_ptr<MpcProtocol>>& protos, vector<shared_ptr<attr_type>>& attrs, 
                    vector<MpcTaskCodeFunc> task_code_funcs) {
  vector<std::thread*> threads(protos.size());
  for (int i = 0; i < threads.size(); ++i) {
    threads[i] = new std::thread(task_code_funcs[i], protos[i], attrs[i]);
  }
  
  for (int i = 0; i < threads.size(); ++i) {
    threads[i]->join();
    delete threads[i];
  }
}

static void run_task_threads_main(int partyid, const string& logfile, vector<MpcTaskCodeFunc> task_codes) {
  int thread_num = task_codes.size();
  /***********    load rosetta inner configure   **********/
  Logger::Get().log_to_stdout(false);
  Logger::Get().set_filename(logfile);                             
  Logger::Get().set_level(0);
  string node_id;
  string config_json;
  rosetta_old_conf_parse(node_id, config_json, partyid, "CONFIG.json");
  log_info << "load and parse rosetta inner configure ok \n";

  /***********    create channel, protocol instances   **********/
  vector<shared_ptr<MpcProtocol>> protocol_ins;
  vector<shared_ptr<attr_type>> reveal_attrs;
  vector<string> task_ids;
  vector<string> reveal_receivers = {"P0", "P1", "P2"};
  for (size_t i = 0 ; i < thread_num; ++i) {
    attr_type* reveal_attr = new attr_type;
    string task_id = "rosetta-task-"+std::to_string(i+1);

    IOManager::Instance()->CreateChannel(task_id, node_id, config_json);
    task_ids.push_back(task_id);

    (*reveal_attr)["receive_parties"] = receiver_parties_pack(reveal_receivers);
    auto net_io = IOManager::Instance()->GetIOWrapper(task_id);
    (*reveal_attr)["P0"] = net_io->GetNodeId(0);
    (*reveal_attr)["P1"] = net_io->GetNodeId(1);
    (*reveal_attr)["P2"] = net_io->GetNodeId(2); ////////////////////  P2 cannot find !!!!!!
    (*reveal_attr)["logfile"] = logfile;
    reveal_attrs.push_back(std::shared_ptr<attr_type>(reveal_attr));
  }

  for (size_t i = 0 ; i < thread_num; ++i) {
    protocol_ins.push_back(mpc_task_test_create_init_protocol(partyid, task_ids[i], logfile, reveal_attrs[i].get()));
  }
  
  run_task_threads(protocol_ins, reveal_attrs, task_codes);
  
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  std::thread *threads = new std::thread[thread_num];
  for (size_t i = 0 ; i < thread_num; ++i) {
    threads[i] = thread(&mpc_task_test_uninit_protocol, partyid, protocol_ins[i]);
  }
  for (size_t i = 0; i < thread_num; i++)
    threads[i].join();
  protocol_ins.clear();
  reveal_attrs.clear();
}
