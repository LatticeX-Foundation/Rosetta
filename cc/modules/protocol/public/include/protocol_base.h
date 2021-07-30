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

#include "cc/modules/common/include/utils/perf_stats.h"
#include "cc/modules/common/include/utils/msg_id.h"
#include "cc/modules/protocol/public/include/protocol_ops.h"
#include "cc/modules/iowrapper/include/io_wrapper.h"

#include <memory>
#include <vector>
#include <string>
#include <iostream>
#include <mutex>

namespace rosetta {

//! the unique NET_IO defined here!
using NET_IO = IOWrapper;

/**
 * This is the base interface class for all secure cryptographic protocols
 */
class ProtocolBase {
 public:
  /**
   * @brief: constructor, and you can name the protocol.
   */
  ProtocolBase(const string& protocol_name, int parties=3, const string& task_id="") : 
            protocol_name_(protocol_name),
            parties_(parties),
            net_io_(nullptr), 
            context_(std::make_shared<ProtocolContext>()) {
    context_->TASK_ID = task_id;
  };
  
  virtual ~ProtocolBase() = default;

  /**
   * @desc: to init and activate this protocol. 
   *         Start the underlying network and prepare resources.
   * @return:
   *     0 if success, otherwise some errcode
   * @note:
   *   The partyID for MPC protocol is also included in the config_json_str,
   *   you may need extract it.
   * @changelog:
   *    The preprocessing/offline phase for some protocols is also carried out in this stage
   */
  virtual int Init();

  /**
   * @desc: to uninit and deactivate this protocol.
   * @return:
   *     0 if success, otherwise some errcode
   */
  virtual int Uninit();

  /**
   * @desc: Check if protocol is initialized.
   * @return:
   *    True if it has been initialized, otherwise false
   */
  virtual bool IsInit() {return is_inited_; }
  
  /**
   * @desc: after initialization, get the actual operation interface of this protocol
   * @param:
   *     msgid: the message id passed by caller
   * @return:
   *     the Operations interface, the Ops whithin have the same token
   */
  virtual shared_ptr<ProtocolOps> GetOps(const msg_id_t& msgid) { THROW_NOT_IMPL_FN(__func__); }

  /**
   * @desc: after initialization, get the network channel for this protocol
   * that can be used to send and receive data in Ops.
   */
  virtual shared_ptr<NET_IO> GetNetHandler() { THROW_NOT_IMPL_FN(__func__); }

  // // Set/Get io channel
  // virtual void SetIOChannel(shared_ptr<IChannel> channel) { io_channel_ = channel; }
  // virtual shared_ptr<IChannel> GetIOChannel() { return io_channel_; }

  /**
   *  @desc: get the name of this cryptographical protocol
   */
  virtual string Name() const { return protocol_name_; }

  int GetParties() const { return parties_; }

  /**
   * @brief Set float precision for fixpoint of Rosetta.
   * @param
   *    float_precision, precision bit count.
   *    task_id, task id the current task.
   */
  void SetFloatPrecision(int float_precision);

  /**
   * @brief Set up the way of saving Rosetta training model.
   * @param 
   *    mode, it is a 3-bit bitmap:[P2 P1 P0]
              0: none, all private model
              1: P0 owns plain model, others got nothing
              2: P1 owns plain model, others got nothing
              4: P2 owns plain model, others got nothing
              3: P0 and P1 owns plain model, others got nothing
              5: P0 and P2 owns plain model, others got nothing
              6: P1 and P2 owns plain model, others got nothing
              7: P0, P1 and P2 owns plain model.
   * @note
        By default, the local ciphertext values are saved in model(mode=0)
  */
  void SetSaverModel(const vector<string>& mode);

  /**
   * @brief Set up the way of restoring Rosetta training model.
   * @param 
   *    mode, it is a 3-bit bitmap:[P2 P1 P0]
              0: none, all local ciphertext
              1: P0 owns plain model, others got nothing
              2: P1 owns plain model, others got nothing
              4: P2 owns plain model, others got nothing
              3: P0 and P1 owns plain model, others got nothing
              5: P0 and P2 owns plain model, others got nothing
              6: P1 and P2 owns plain model, others got nothing
              7: P0, P1 and P2 owns plain model.
   * @note
        By default, the local ciphertext values are saved in model (mode=0).
  */
  void SetRestoreModel(const vector<string>& mode);
  
  /** 
   * @desc: get current performance statistic info
   * @code
   * auto ps0 = GetPerfStats();
   * // some code
   * auto ps1 = GetPerfStats();
   * auto ps = ps1 - ps0;
   * @endcode
   */
  virtual PerfStats GetPerfStats() { return PerfStats(); }
  virtual void StartPerfStats() {}

  shared_ptr<ProtocolContext> GetMpcContext() { return context_; }

protected:
  
  /**
   * @desc: during initialization, perform some pre-processing, such as generating beaver triples offline.
   * 
   * @return:
   *      0 if success, otherwise some errcode
   * @note:
   *      For now, some configuration can be protocol-depedent, so specific protocol can parse config files
   *      to get its addtional hyper-perameters. 
   */
  virtual int OfflinePreprocess() {
    log_debug << "Protocol Base do nothing during offline preprocess.";
    return 0; 
  }

 protected:
  string protocol_name_ = "";
  int parties_ = 3;
  bool is_inited_ = false;
  
  std::mutex status_mtx_;
  shared_ptr<NET_IO> net_io_ = nullptr;

  PerfStats perf_stats_;
  shared_ptr<ProtocolContext> context_;
};


class IProtocolFactory
{
public:
  virtual shared_ptr<ProtocolBase> Create(const string& task_id="") = 0;
};//IProtocolFactory


} // namespace rosetta
