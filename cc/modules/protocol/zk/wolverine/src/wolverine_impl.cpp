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
#include "cc/modules/protocol/zk/wolverine/include/wolverine_impl.h"
#include "cc/modules/protocol/zk/wolverine/include/wolverine_ops_impl.h"
#include "cc/modules/protocol/utility/include/util.h"
#include "cc/modules/protocol/zk/wolverine/include/zk_int_fp.h"
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

int WolverineProtocol::Init(std::string config_json_str) { return Init(-1, config_json_str, ""); }

int WolverineProtocol::Init(int partyid, std::string config_json_str, std::string logfile) {
  /**
   * if partyid has been setted ([0,N]), means internel calls.
   */
  log_debug << "WolverineProtocol::Init with config:" << config_json_str << endl;
  if (logfile != "")
    rosetta::redirect_stdout(logfile);

  if (!_is_inited) {
    std::unique_lock<std::mutex> lck(_status_mtx);
    if (!_is_inited) {
      if (partyid < 0) {
        _init_config(config_json_str);
      } else {
        _init_config(partyid, config_json_str);
      }
      my_party_id = config->PARTY;
      config->fmt_print();

      // Note that in wolverine, we use the P0's port in config file as zk port.
      host = config->zk.P[0].HOST;
      port = config->zk.P[0].PORT;
      log_info << "ZK host: " << host << ",base port: " << port;
      if (my_party_id == PARTY_A) {
        // ALICE in Wolverine
        party = 1;
        log_info << "I'm the prover, Alice!";
      } else if (my_party_id == PARTY_B) {
        // BOB in Wolverine
        party = 2;
        log_info << "I'm the verifier, Bob!";
      } else {
        log_info << "This is 2PC protocol, there is no seat for you!";
        exit(1);
      }
      zk_party_id = party;

      ::mkdir("./data", 0755);
      config_map["restore_mode"] = to_string(config->zk.RESTORE_MODE);
      parties = 2;

      for (int i = 0; i < THREAD_NUM + 1; ++i)
        zk_ios[i] = new ZK_NET_IO(new NetIO(party == ALICE ? nullptr : host.c_str(), port + i), party == ALICE);

      auto start = clock_start();
      // setup_boolean_zk<ZK_NET_IO>(zk_ios, THREAD_NUM, party);
      // setup_fp_zk<ZK_NET_IO>(zk_ios, THREAD_NUM, party);
      setup_zk_bool<ZK_NET_IO>(zk_ios, THREAD_NUM, party);
      setup_zk_arith<ZK_NET_IO>(zk_ios, THREAD_NUM, party, true);

      auto timesetup = time_from(start);
      log_info << "time for setup: " << timesetup / 1000 << " " << party << " " << endl;

      // zk_ios[0]->sync();

      _is_inited = true;
      log_info << "------------ circuit zero-knowledge proof BEGIN ------------" << std::endl;
      StartPerfStats();
    }
  }
  log_debug << "WolverineProtocol::Init done!" << endl;
  log_info << "Rosetta: Protocol [" << _protocol_name << "] backend initialization succeeded!"
           << endl;
  return 0;
}

int WolverineProtocol::_init_config(int partyid, const std::string& config_json) {
  config = make_shared<RosettaConfig>(partyid, config_json);
  return 0;
}
int WolverineProtocol::_init_config(const std::string& config_json) {
  config = make_shared<RosettaConfig>(config_json);
  return 0;
}

int WolverineProtocol::Uninit() {
  log_debug << "WolverineProtocol::Uninit()" << endl;
  std::unique_lock<std::mutex> lck(_status_mtx);

  if (_is_inited) {
    // finalize_fp_zk();
    // finalize_boolean_zk<ZK_NET_IO>(my_party_id);
    finalize_zk_bool<BoolIO<NetIO>>();
	  finalize_zk_arith<BoolIO<NetIO>>();

    // for (int i = 0; i < (THREAD_NUM + 1); ++i) {
    //   if (zk_ios[i] != nullptr) {
    //     delete zk_ios[i];
    //   }
    // }

    log_info << "Rosetta: Protocol [" << _protocol_name << "] backend has been released." << endl;
    rosetta::restore_stdout();
    log_info << "------------ circuit zero-knowledge proof END ------------" << std::endl;
    print_global_static_elasped();
    _is_inited = false;
  }
  return 0;
}

shared_ptr<ProtocolOps> WolverineProtocol::GetOps(const msg_id_t& msgid) {
  auto wvr_ops_ptr = make_shared<WolverineOpsImpl>(msgid);
  wvr_ops_ptr->op_config_map["PID"] = "P" + to_string(my_party_id);
  wvr_ops_ptr->io = GetNetHandler();
  for (int i = 0; i < THREAD_NUM; ++i) {
    wvr_ops_ptr->zk_ios[i] = zk_ios[i];
  }
  wvr_ops_ptr->port = port;
  wvr_ops_ptr->party = party;

  return std::dynamic_pointer_cast<ProtocolOps>(wvr_ops_ptr);
}

PerfStats WolverineProtocol::GetPerfStats() {
  PerfStats perf_stats;
  if (!_is_inited) {
    return perf_stats;
  }

  //! Time/Mem/Cpu
  perf_stats.s = perf_stats_.get_perf_stats();

  //! Name
  perf_stats.name = Name() + " P" + to_string(GetPartyId());

  //! Network
  ZKBoolCircExec<BoolIO<NetIO>>* env = (ZKBoolCircExec<BoolIO<NetIO>>*)(CircuitExecution::circ_exec);
  perf_stats.s.bytes_sent = env->communication(); // including NetIO and BoolIO
  perf_stats.s.bytes_recv = 0;
  perf_stats.s.msg_sent = 0;
  perf_stats.s.msg_recv = 0;

  return perf_stats;
}

void WolverineProtocol::StartPerfStats() {
  if (!_is_inited) {
    return;
  }

  //! Time/Mem/Cpu
  perf_stats_.start_perf_stats(); // true false

  //! Network
}

} // namespace rosetta
