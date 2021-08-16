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
#include "cc/modules/protocol/mpc/comm/include/mpc_protocol.h"
#include "cc/modules/protocol/utility/include/util.h"
#include "cc/modules/iowrapper/include/io_manager.h"
#include "cc/modules/common/include/utils/rtt_logger.h"

#include <stdexcept>
#include <string>
#include <vector>
using namespace std;

namespace rosetta {

MpcProtocol::MpcProtocol(const string& protocol, int parties, const string& task_id) : ProtocolBase(protocol, parties, task_id) {
}

int MpcProtocol::Init() { return Init(""); }

int MpcProtocol::Init(string logfile) {
#if NDEBUG
  if (logfile != "")
    rosetta::redirect_stdout(logfile);
#endif

  //! @todo optimized
  if (!is_inited_) {
    std::unique_lock<std::mutex> lck(status_mtx_);
    if (!is_inited_) {
      net_io_ = IOManager::Instance()->GetIOWrapper(context_->TASK_ID);
      // init mpc context
      context_->FLOAT_PRECISION = FLOAT_PRECISION_DEFAULT;
      context_->NODE_ID = net_io_->GetCurrentNodeId();
      context_->ROLE_ID = net_io_->GetPartyId(context_->NODE_ID);
      context_->NODE_ROLE_MAPPING = net_io_->GetComputationNodes();
      context_->SAVER_MODEL.set_computation_mode();
      context_->RESTORE_MODEL.set_computation_mode();

      InitMpcEnvironment();
      InitAesKeys();
      
      // todo: for offline triple generation.
      OfflinePreprocess();

      is_inited_ = true;
      StartPerfStats();
    }
  }

  tlog_info << "Rosetta: Protocol [" << protocol_name_ << "] backend initialization succeeded! task: " 
           << context_->TASK_ID << ", node id: " << context_->NODE_ID;
  return 0;
}

int MpcProtocol::InitAesKeys() {
  // please implements in subclass !!!
  return 0;
}

int MpcProtocol::Uninit() {
  tlog_debug << "MpcProtocol Uninit..." ;
  std::unique_lock<std::mutex> lck(status_mtx_);
  if (is_inited_) {
    net_io_->statistics();

    msg_id_t msgid(context_->TASK_ID + "_this message id for synchronize P0/P1/P2 uninit");

    // the following time(0) will show the sync beg/end
    tlog_debug << __FUNCTION__ << " beg sync :" << time(0) ;
    net_io_->sync_with(msgid);
    tlog_debug << __FUNCTION__ << " end sync :" << time(0) ;
    //IOManager::Instance()->DestroyIO();
    net_io_.reset();
    rosetta::restore_stdout();
    tlog_info << "Rosetta: Protocol [" << protocol_name_ << "] backend has been released." ;
    is_inited_ = false;
  }
   IOManager::Instance()->DestroyChannel(context_->TASK_ID);// [HGF] why should we destroy channel here ? channel creation and destory should be outside 
  tlog_debug << "MpcProtocol Uninit ok." ;
  return 0;
}

PerfStats MpcProtocol::GetPerfStats() {
  PerfStats perf_stats;
  if (!is_inited_) {
    return perf_stats;
  }

  //! Time/Mem/Cpu
  perf_stats.s = perf_stats_.get_perf_stats();

  //! Name
  perf_stats.name = Name() + " " + net_io_->GetCurrentNodeId();

  //! Network
  auto net_stat = net_io_->net_stat();
  perf_stats.s.bytes_sent = net_stat.bytes_sent();
  perf_stats.s.bytes_recv = net_stat.bytes_received();
  perf_stats.s.msg_sent = net_stat.message_sent();
  perf_stats.s.msg_recv = net_stat.message_received();

  return perf_stats;
}
void MpcProtocol::StartPerfStats() {
  if (!is_inited_) {
    return;
  }

  //! Time/Mem/Cpu
  perf_stats_.start_perf_stats(); // true false

  //! Network
}

} // namespace rosetta
