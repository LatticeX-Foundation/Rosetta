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
#include "cc/modules/iowrapper/include/io_manager.h"
#include "cc/modules/protocol/zk/mystique/include/mystique_impl.h"
#include "cc/modules/protocol/zk/mystique/include/mystique_ops_impl.h"
#include "cc/modules/protocol/utility/include/util.h"
#include "cc/modules/protocol/zk/mystique/include/zk_int_fp.h"
#include <iostream>

#include <stdexcept>
#include <string>
#include <vector>
#include <sys/stat.h>

// using zk::ZkIntFp;

std::atomic<int64_t> mul_gate_counter{0};
std::atomic<int64_t> mul_const_gate_counter{0};
int64_t matmul_convert_alloc_time = 0; //us
int64_t matmul_proof_time = 0; //us
std::atomic<int64_t> convert_2_str_time{0};
std::atomic<int64_t> convert_2_mac_time{0};

// float2int/int2float
std::atomic<int64_t> fxf_counter{0};
std::atomic<int64_t> fxf_elapsed{0};
std::atomic<int64_t> float2int_counter{0};
std::atomic<int64_t> float2int_elapsed{0};
std::atomic<int64_t> int2float_counter{0};
std::atomic<int64_t> int2float_elapsed{0};

#if 1
void print_global_static_elasped() {
  log_info << "      mul_gate_counter:" << mul_gate_counter;
  log_info << "mul_const_gate_counter:" << mul_const_gate_counter;
  log_info << "matmul_convert_alloc_time:" << matmul_convert_alloc_time / 1000.0 << " us";
  log_info << "   matmul: proof_time:" << matmul_proof_time / 1000.0 << " us";
  log_info << "convert_mac_to_string:" << convert_2_str_time / 1000.0 << " us";
  log_info << "   convert_str_to_mac:" << convert_2_mac_time / 1000.0 << " us";

  // float2int/int2float
  log_info << "      float2int_counter:" << float2int_counter;
  log_info << "   float2int_elapsed(s):" << float2int_elapsed / 1e9
           << ",avg:" << float2int_elapsed / 1e9 / float2int_counter;

  log_info << "      int2float_counter:" << int2float_counter;
  log_info << "   int2float_elapsed(s):" << int2float_elapsed / 1e9
           << ",avg:" << int2float_elapsed / 1e9 / int2float_counter;

  log_info << "      fxf_counter:" << fxf_counter;
  log_info << "   fxf_elapsed(s):" << fxf_elapsed / 1e9
           << ",avg:" << fxf_elapsed / 1e9 / fxf_counter;
}
#else
#define print_global_static_elasped (void)0
#endif

namespace rosetta {

int MystiqueProtocol::Init() { return Init(""); }
int MystiqueProtocol::Init(std::string logfile) {
  tlog_debug << "MystiqueProtocol Init...";
  if (!is_inited_) {
    std::unique_lock<std::mutex> lck(status_mtx_);
    if (!is_inited_) {
      net_io_ = IOManager::Instance()->GetIOWrapper(context_->TASK_ID);
      // init mpc context
      context_->FLOAT_PRECISION = FLOAT_PRECISION_DEFAULT;
      context_->NODE_ID = net_io_->GetCurrentNodeId();
      context_->ROLE_ID = net_io_->GetPartyId(context_->NODE_ID);
      context_->NODE_ROLE_MAPPING = net_io_->GetComputationNodes();
      context_->SAVER_MODEL.set_local_ciphertext_mode();
      context_->RESTORE_MODEL.set_local_ciphertext_mode();

      int my_party_id = net_io_->GetCurrentPartyId();

      // Note that in mystique, we use the P0's port in config file as zk port.
      if (my_party_id == PARTY_A) {
        // ALICE in Mystique
        party = 1;
        tlog_info << "I'm the prover, Alice!";
      } else if (my_party_id == PARTY_B) {
        // BOB in Mystique
        party = 2;
        tlog_info << "I'm the verifier, Bob!";
      } else {
        tlog_info << "This is 2PC protocol, there is no seat for you!";
        exit(1);
      }
      zk_party_id = party;

      ::mkdir("./data", 0755);
      parties_ = 2;

      //! @todo yyyyyyyyyyyl
      // for (int i = 0; i < THREAD_NUM + 1; ++i)
      //   zk_ios[i] = new ZK_NET_IO(
      //     new ZKNetIO(party == ALICE ? nullptr : host.c_str(), port + i), party == ALICE);

      for (int i = 0; i < THREAD_NUM + 1; ++i)
        zk_ios[i] = new ZK_NET_IO(new ZKNetIO(net_io_, to_string(i)), party == ALICE);

      // Setup
      auto start = clock_start();
      // setup_boolean_zk<ZK_NET_IO>(zk_ios, THREAD_NUM, party);
      // setup_fp_zk<ZK_NET_IO>(zk_ios, THREAD_NUM, party);
      setup_zk_bool<ZK_NET_IO>(zk_ios, THREAD_NUM, party);
      setup_zk_arith<ZK_NET_IO>(zk_ios, THREAD_NUM, party, true);
      auto timesetup = time_from(start);
      tlog_info << "time for setup: " << timesetup / 1000 << " " << party;

      is_inited_ = true;
      StartPerfStats();
    }
  }

  tlog_info << "Rosetta: Protocol [" << protocol_name_
            << "] backend initialization succeeded! task: " << context_->TASK_ID
            << ", node id: " << context_->NODE_ID;
  tlog_debug << "MystiqueProtocol Init ok.";

  return 0;
}

int MystiqueProtocol::Uninit() {
  tlog_debug << "MystiqueProtocol Uninit...";
  std::unique_lock<std::mutex> lck(status_mtx_);

  if (is_inited_) {
    net_io_->statistics();

    // finalize_fp_zk();
    // finalize_boolean_zk<ZK_NET_IO>(my_party_id);
    finalize_zk_bool<BoolIO<ZKNetIO>>();
    finalize_zk_arith<BoolIO<ZKNetIO>>();
    // for (int i = 0; i < (THREAD_NUM + 1); ++i) {
    //   if (zk_ios[i] != nullptr) {
    //     delete zk_ios[i];
    //   }
    // }

    msg_id_t msgid(context_->TASK_ID + "_this message id for synchronize P0/P1/P2 uninit");

    // the following time(0) will show the sync beg/end
    tlog_debug << __FUNCTION__ << " beg sync :" << time(0);
    net_io_->sync_with(msgid);
    tlog_debug << __FUNCTION__ << " end sync :" << time(0);

    //IOManager::Instance()->DestroyIO();
    net_io_.reset();

    rosetta::restore_stdout();
    tlog_info << "Rosetta: Protocol [" << protocol_name_ << "] backend has been released.";
    print_global_static_elasped();
    is_inited_ = false;
  }

  IOManager::Instance()->DestroyChannel(
    context_
      ->TASK_ID); // [HGF] why should we destroy channel here ? channel creation and destory should be outside
  tlog_debug << "MystiqueProtocol Uninit ok.";
  return 0;
}

shared_ptr<ProtocolOps> MystiqueProtocol::GetOps(const msg_id_t& msgid) {
  auto wvr_ops_ptr = make_shared<MystiqueOpsImpl>(msgid, context_);
  // wvr_ops_ptr->op_config_map["PID"] = "P" + to_string(my_party_id);
  wvr_ops_ptr->io = GetNetHandler();
  for (int i = 0; i < THREAD_NUM; ++i) {
    wvr_ops_ptr->zk_ios[i] = zk_ios[i];
  }
  wvr_ops_ptr->port = port;
  wvr_ops_ptr->party = party;

  return std::dynamic_pointer_cast<ProtocolOps>(wvr_ops_ptr);
}

PerfStats MystiqueProtocol::GetPerfStats() {
  PerfStats perf_stats;
  if (!is_inited_) {
    return perf_stats;
  }

  //! Time/Mem/Cpu
  perf_stats.s = perf_stats_.get_perf_stats();

  //! Name
  perf_stats.name = Name() + " " + net_io_->GetCurrentNodeId();

  //! Network
  ZKBoolCircExec<BoolIO<ZKNetIO>>* env =
    (ZKBoolCircExec<BoolIO<ZKNetIO>>*)(CircuitExecution::circ_exec);
  perf_stats.s.bytes_sent = env->communication(); // including ZKNetIO and BoolIO
  perf_stats.s.bytes_recv = 0;
  perf_stats.s.msg_sent = 0;
  perf_stats.s.msg_recv = 0;

  return perf_stats;
}

void MystiqueProtocol::StartPerfStats() {
  if (!is_inited_) {
    return;
  }

  //! Time/Mem/Cpu
  perf_stats_.start_perf_stats(); // true false

  //! Network
}

} // namespace rosetta
