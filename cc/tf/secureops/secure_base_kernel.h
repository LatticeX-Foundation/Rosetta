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

#include <cstdio>
#include <vector>
#include <iostream>
#include <sstream>
#include <string>
#include <typeinfo>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <stdexcept>
#include <map>
#include <atomic>
#include <chrono>
using namespace std;

#include "tensorflow/core/framework/types.h"
#include "tensorflow/core/framework/op.h"
#include "tensorflow/core/framework/op_kernel.h"
#include "tensorflow/core/framework/function.h"
#include "tensorflow/core/framework/common_shape_fns.h"
#include "tensorflow/core/framework/tensor_shape.h"
#include "tensorflow/core/framework/kernel_def.pb.h"
#include "tensorflow/core/framework/kernel_def_builder.h"
#include "tensorflow/core/framework/node_def.pb.h"
#include "tensorflow/core/framework/node_def_util.h"
#include "tensorflow/core/util/bcast.h"
#include "tensorflow/core/framework/rendezvous.h"
#include "tensorflow/core/framework/session_state.h"
#include "tensorflow/core/framework/variant_op_registry.h"
#include "tensorflow/core/platform/logging.h"
#include "third_party/eigen3/unsupported/Eigen/CXX11/Tensor"
#include "tensorflow/core/framework/bounds_check.h"
#include "tensorflow/core/framework/numeric_types.h"
#include "tensorflow/core/framework/tensor_types.h"
#include "tensorflow/core/framework/device_attributes.pb.h"

#include "cc/modules/common/include/utils/rtt_logger.h"
#include "cc/modules/common/include/utils/msg_id.h"
#include "cc/modules/common/include/utils/msg_id_mgr.h"
#include "cc/modules/iowrapper/include/io_wrapper.h"
#include "cc/modules/protocol/public/include/protocol_manager.h"
#include "cc/tf/secureops/mpc_exceptions.h"
#include "cc/modules/common/include/utils/helper.h"
#include "cc/modules/protocol/mpc/comm/include/mpc_helper.h"

#ifndef TYPENAME
#if defined(__GNUC__)
#include <cxxabi.h> // abi::__cxa_demangle
#define TYPENAME(typeid_name) abi::__cxa_demangle(typeid_name, nullptr, nullptr, nullptr)
#else
#define TYPENAME(typeid_name) typeid_name
#endif
#endif

// please set 1 for debuging
#define PRINT_REVEAL 0
#if PRINT_REVEAL
#define DEBUG_PRINT_AFTER(CTX) debug_print_after(CTX)
#define DEBUG_PRINT_BEFORE(CTX) debug_print_before(CTX)
#else
#define DEBUG_PRINT_AFTER(CTX)
#define DEBUG_PRINT_BEFORE(CTX)
#endif

// string only
#define REGISTER_STR_CPU_KERNEL(name, cls) \
  REGISTER_KERNEL_BUILDER(Name(#name).Device(DEVICE_CPU), cls)

namespace tensorflow {

typedef Eigen::ThreadPoolDevice CPUDevice;
typedef Eigen::GpuDevice GPUDevice;
#ifdef TENSORFLOW_USE_SYCL
typedef Eigen::SyclDevice SYCLDevice;
#endif // TENSORFLOW_USE_SYCL

#define DO_SECURE_OP_PERFORMANCE_STATISTICS 1
#if DO_SECURE_OP_PERFORMANCE_STATISTICS
// ////////////////////// PERFORMANCE STATISTICS
struct secure_op_stats {
  secure_op_stats(const string& _op) : op(_op) {}
  string op;
  atomic<int64_t> calls{0};
  atomic<int64_t> elapse{0};
  map<string, atomic<int64_t>> protocol_op_elapse;
};
extern mutex secure_op_stats_mtx;
extern map<string, secure_op_stats*> map_op_stats;
extern atomic<int64_t> init_exit_counter;
void secure_op_stats_exit_func();
// ////////////////////// PERFORMANCE STATISTICS
#define SECURE_OP_KERNEL_BASE_CLASS_COMPUTE_STATS_BEG() \
  map_op_stats[op_]->calls++;                           \
  auto ___begin = std::chrono::steady_clock::now()
#define SECURE_OP_KERNEL_BASE_CLASS_COMPUTE_STATS_END()                    \
  map_op_stats[op_]->elapse +=                                             \
    std::chrono::duration_cast<std::chrono::duration<int64_t, std::nano>>( \
      std::chrono::steady_clock::now() - ___begin)                         \
      .count()

#define SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(opname) \
  auto __##opname##_begin = std::chrono::steady_clock::now()

#define SECURE_OP_CALL_PROTOCOL_OP_STATS_END(opname)                                           \
  auto __##opname##_end = std::chrono::steady_clock::now();                                    \
  auto __##opname##__ = std::chrono::duration_cast<std::chrono::duration<int64_t, std::nano>>( \
                          __##opname##_end - __##opname##_begin)                               \
                          .count();                                                            \
  map_op_stats[op_]->protocol_op_elapse[#opname] += __##opname##__

//! Usage
//! SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(XXX);
//! ProtocolManager::Instance()->GetProtocol(task_id)->GetOps(...)->YYYY(...);
//! SECURE_OP_CALL_PROTOCOL_OP_STATS_END(XXX);
// ////////////////////// PERFORMANCE STATISTICS
#else
#define SECURE_OP_KERNEL_BASE_CLASS_COMPUTE_STATS_BEG() (void)0
#define SECURE_OP_KERNEL_BASE_CLASS_COMPUTE_STATS_END() (void)0
#define SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(opname) (void)0
#define SECURE_OP_CALL_PROTOCOL_OP_STATS_END(opname) (void)0
#endif
// ////////////////////// PERFORMANCE STATISTICS

class SecureOpKernel : public OpKernel {
 protected:
  const msg_id_t& msg_id() const { return msg_id_; }

 protected:
  int verbose_ = 1;
  string op_;
  string op_name_;
  msg_id_t msg_id_;
  unordered_map<string, string> attrs_;

 public:
  explicit SecureOpKernel(OpKernelConstruction* context) : OpKernel(context) {
    // about node
    const NodeDef& def = context->def();
    op_ = def.op();

#if DO_SECURE_OP_PERFORMANCE_STATISTICS
    {
      if (init_exit_counter++ == 0) {
        atexit(secure_op_stats_exit_func);
      }
      unique_lock<mutex> lck(secure_op_stats_mtx);
      auto iter = map_op_stats.find(op_);
      if (iter == map_op_stats.end()) {
        map_op_stats[op_] = new secure_op_stats(op_);
      }
    }
#endif

#if PRINT_REVEAL
    {
      log_debug << op_ << "======================================================= BEG NODEDEF\n";

      log_debug << "  name:" << def.name();
      log_debug << "    op:" << def.op();
      log_debug << "device:" << def.device();
      log_debug << "inputs:";
      for (auto& input : def.input()) {
        log_debug << "       " << input;
      }
      log_debug << " attrs:";
      for (auto& attr : def.attr()) {
        log_debug << "       " << attr.first;
      }
      log_debug << "type_string:" << type_string();
      log_debug << "======================================================= END NODEDEF\n";
    }
#endif
    
    string task_id = ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation());
    op_name_ = def.name() + "#T" + task_id;//def.name();
    msg_id_ = msg_id_t(def.name() + "#T" + task_id);
    log_debug << "SecureOpKernel msgid:" << msg_id();

    //-----------------------------------------------
    //Deal with PrivateInput op in decode function
    auto func_lib = context->function_library();
    if (func_lib) {
      auto func_def = func_lib->GetFunctionLibraryDefinition();
      if (func_def) {
        std::vector<string> func_name_lists = func_def->ListFunctionNames();
        if (func_name_lists.size() == 1 && !strcmp(def.name().c_str(), "PrivateInput")) {
          string op_unique_name = func_name_lists[0] + "/" + def.name()+ "#T" + task_id;
          msg_id_ = msg_id_t(op_unique_name);
          op_name_ = op_unique_name;
          log_debug << "New SecureOpKernel msgid:" << msg_id();
        }
      }
    }
    //-----------------------------------------------
  }
  void Compute(OpKernelContext* context) override {
#if !USE_SHA256_ID
    msg_id_t& iid = MsgIdMgr::Instance()->GetMsgIdFromOpName(op_name_);
    if (!iid.str().empty()) {
      msg_id_ = iid;
    }
    log_debug << "SecureOpKernel Compute msgid:" << msg_id();
#endif

    SECURE_OP_KERNEL_BASE_CLASS_COMPUTE_STATS_BEG();
    DEBUG_PRINT_BEFORE(context);
    // log_info << "OpKernel Compute:" << this_thread::get_id() << " " << msg_id_;
    //debug_print_before_v2(context);
    ComputeImpl(context);
    //debug_print_after_v2(context);
    DEBUG_PRINT_AFTER(context);
    SECURE_OP_KERNEL_BASE_CLASS_COMPUTE_STATS_END();
  }

  virtual void ComputeImpl(OpKernelContext* context) {
    log_debug << "SecureOpKernel ComputeImpl... exception !";
    throw;
  }

  /**
   * \param input_0 if input[0] is literal value, return true[constant]
   */
  bool is_public_or_constant_input_by_input0(const std::string& input_0) {
    //! @todo
    return false;
  }
  bool is_public_or_constant_input_by_restore_mode(OpKernelContext* context) {
    string task_id = ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation());
    int parties = ProtocolManager::Instance()->GetProtocol(task_id) ->GetParties();
    auto proto_context = ProtocolManager::Instance()->GetProtocol(task_id)->GetMpcContext();
#ifndef ENABLE_ZK_TASK
#define ENABLE_ZK_TASK 1
#if (ENABLE_ZK_TASK == 0)
    int restore_mode = std::stoi(proto_context->PAYLOAD);// int restore_mode = atoi(cfg.at("restore_mode").c_str());
    int a = (1 << parties) - 1;
    if ((restore_mode & a) == a) {
      // is a public-constant value
      return true;
    }
    return false;
#else
    int party_counter = 0;
    for (const auto& node : proto_context->RESTORE_MODE) {
      if (-1 == proto_context->GetRole(node)) {
        // not compute node
        return false;
      } else {
        ++party_counter;
      }
    }
    if (party_counter == parties)
      return true;
    
    return false;
#endif

#endif
#undef ENABLE_ZK_TASK
  }

  /*
  **
  ** below functions, just for test or debug
  **
  */
  //void debug_init();
  void debug_print_before(OpKernelContext* context) {
    log_debug << "======================================================= BEG BEFORE\n";
    // about input(s)
    log_debug << "Debug StrOpKernel num_inputs: " << context->num_inputs();
    for (int i = 0; i < context->num_inputs(); i++) {
      if (IsRefType(context->input_dtype(i))) {
        Tensor var = context->mutable_input(i, false);
        log_debug << "input var(" << i << ").shape(): " << var.shape();
        log_debug << "Debug StrOpKernel input(" << i << "): " << var.DebugString(9);
        continue;
      }
      const Tensor& x = context->input(i);
      log_debug << "input(" << i << ").shape(): " << x.shape();
      if (x.NumElements() > 0) {
        log_debug << "Debug StrOpKernel input(" << i << "): " << x.DebugString(9);
      }
    }
    log_debug << "-------------------------------------------------------\n";

    // about output(s)
    log_debug << "Debug StrOpKernel num_outputs: " << context->num_outputs();

    log_debug << "======================================================= END BEFORE\n";
  }

  void debug_print_v2(const Tensor& x, int j, string prefix, OpKernelContext* context) {
    if ((x.dtype() != DataType::DT_STRING) && (x.dtype() != DataType::DT_STRING_REF)) {
      return;
    }
    return;
    auto x_flat = x.flat<string>();
    vector<string> vs(x.NumElements());
    vector<double> vd(x.NumElements());
    for (int i = 0; i < x.NumElements(); i++) {
      vs[i] = x_flat(i);
    }
    if (vs.size() > 0) {
      auto ops = ProtocolManager::Instance()
                  ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
                  ->GetOps(msg_id());
      ops->Reveal(vs, vd);
      print_vector(vd, prefix + " index " + to_string(j), 5, 8);
    }
  }

  void debug_print_before_v2(OpKernelContext* context) {
    log_info << "======================================================= BEG BEFORE\n";
    // about input(s)
    log_info << "OpKernel num_inputs: " << context->num_inputs();
    for (int i = 0; i < context->num_inputs(); i++) {
      if (IsRefType(context->input_dtype(i))) {
        Tensor var = context->mutable_input(i, false);
        log_info << "op shape:" << op_ << " input var(" << i << ").shape(): " << var.shape();
        log_debug << "op value:" << op_ << " OpKernel input(" << i << "): " << var.DebugString(9);
        debug_print_v2(var, i, "input", context);
        continue;
      }
      const Tensor& x = context->input(i);
      log_info << "op shape:" << op_ << " input(" << i << ").shape(): " << x.shape();
      if (x.NumElements() > 0) {
        log_debug << "op value:" << op_ << " OpKernel input(" << i << "): " << x.DebugString(9);
        debug_print_v2(x, i, "input", context);
      }
    }
    log_info << "======================================================= END BEFORE\n";
  }

  void debug_print_after(OpKernelContext* context) {
    //log_debug << "======================================================= BEG AFTER\n";
    //log_debug << "======================================================= END AFTER\n";
  }

  void debug_print_after_v2(OpKernelContext* context) {
    log_info << "======================================================= BEG AFTER\n";
    log_info << "OpKernel num_outputs: " << context->num_outputs();
    for (int i = 0; i < context->num_outputs(); i++) {
      Tensor* x = context->mutable_output(i);
      log_info << "op shape:" << op_ << " output(" << i << ").shape(): " << x->shape();
      debug_print_v2(*x, i, "output", context);
    }
    log_info << "======================================================= END AFTER\n";
  }

  void debug_print_tensor(const Tensor* z, const char* msg = "") {
    string s = "";
    if (z != nullptr) {
      s = z->DebugString(16);
    }

    log_debug << msg << " " << s;
    auto flat_z = z->flat<string>();
    int size = z->NumElements();
    for (int i = 0; i < size; ++i)
      log_debug << flat_z(i) << ", ";
  }

  virtual void debug_print_reveal(const vector<string>& in, std::string msg, OpKernelContext* context) {
    if (op_ == "SecureReveal")
      return;
    log_debug << __FUNCTION__ << "================= " << msg ;

    size_t size = in.size();
    vector<string> outs(size);
    //char by_recv_pts[] = "\x01\x00\x00\x00N\x02\x00\x00\x00P0";
    string recv_nodes = encode_reveal_node("P0");
    attrs_["receive_parties"] = recv_nodes;
    // attrs_["receive_party"] = string("1");
    rosetta::ProtocolManager::Instance()
      ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->Reveal(in, outs, &attrs_);
    print_vec(outs);

    log_debug << __FUNCTION__ << "================= " ;
  }
};

struct BinaryOpState {
  // Sets up bcast with the shape of in0 and in1, ensures that the bcast
  // is valid, and if so, set out, either by allocating a new buffer using
  // ctx->output(...) or by creating an alias for an owned input buffer for
  // in-place computation.
  // Caller must check ctx->status() upon return for non-ok status.
  // If ctx->status().ok() is true, then out is guaranteed to be allocated.
  BinaryOpState(OpKernelContext* ctx)
      : in0(ctx->input(0)),
        in1(ctx->input(1)),
        bcast(BCast::FromShape(in0.shape()), BCast::FromShape(in1.shape())) {
    if (!bcast.IsValid()) {
      log_debug << "Incompatible shapes: " << in0.shape().DebugString() << " vs. "
                << in1.shape().DebugString() ;
      ctx->SetStatus(errors::InvalidArgument(
        "Incompatible shapes: ", in0.shape().DebugString(), " vs. ", in1.shape().DebugString()));
      return;
    }
    const TensorShape output_shape = BCast::ToShape(bcast.output_shape());
    out_num_elements = output_shape.num_elements();
    in0_num_elements = in0.NumElements();
    in1_num_elements = in1.NumElements();
    OP_REQUIRES_OK(ctx, ctx->forward_input_or_allocate_output({0, 1}, 0, output_shape, &out));

    ndims = static_cast<int>(bcast.x_reshape().size());
    in0_dims = in0.dims();
    in1_dims = in1.dims();
    out_dims = output_shape.dims();

    log_debug << "    ndims:" << ndims << ", x_shape.size(): " << bcast.x_reshape().size() << ", y_shape.size(): " << bcast.y_reshape().size();
    log_debug << "      in0 dims shape:" << in0.dims() << " " << in0.shape();
    log_debug << "      in1 dims shape:" << in1.dims() << " " << in1.shape();
    log_debug << "      out dims shape:" << output_shape.dims() << " " << output_shape;
    log_debug << "    in0_num_elements:" << in0_num_elements;
    log_debug << "    in1_num_elements:" << in1_num_elements;
    log_debug << "    out_num_elements:" << out_num_elements;
 }

  int verbose_ = 1;

  const Tensor& in0;
  const Tensor& in1;

  BCast bcast;
  Tensor* out = nullptr;

  int64 in0_num_elements;
  int64 in1_num_elements;
  int64 out_num_elements;

  int in0_dims;
  int in1_dims;
  int out_dims;

  int ndims;
}; //BinaryOpState

template <int NDIMS>
void ASSIGN_TENSOR(
  const Tensor& in0,
  const Tensor& in1,
  const Tensor* out,
  int out_dims,
  Tensor& in0_tensor,
  Tensor& in1_tensor,
  const CPUDevice& eigen_device,
  const BCast* bcast) {
  do {
    auto _in0 = in0.template shaped<string, NDIMS>(bcast->x_reshape());
    auto _bcast0 = BCast::ToIndexArray<NDIMS>(bcast->x_bcast());
    auto _in1 = in1.template shaped<string, NDIMS>(bcast->y_reshape());
    auto _bcast1 = BCast::ToIndexArray<NDIMS>(bcast->y_bcast());
    auto _lhs = _in0.broadcast(_bcast0);//BroadcastReshapeOp
    auto _rhs = _in1.broadcast(_bcast1);//BroadcastReshapeOp

    // get a bigger shape
    bool use_in0 = true;
    for (auto i = 0; i < bcast->x_bcast().size(); ++i) {
      if (2 <= bcast->x_bcast()[i]) { // not 1
        use_in0 = false;
        break;
      }
    }
    Eigen::DSizes<int64_t, NDIMS> _out_shape(use_in0 ? BCast::ToIndexArray<NDIMS>(bcast->x_reshape()) : BCast::ToIndexArray<NDIMS>(bcast->y_reshape()));

    // log_debug << "dims: " << NDIMS << ", out_dims: " << out_dims ;
    // log_debug << "_bcast0.size():" << _bcast0.size() ;
    // log_debug << "_bcast1.size():" << _bcast1.size() ;
    // log_debug << "type:\n" << typeid(_rhs).name() ;
    // log_debug << "_lhs type:\n" << TYPENAME(typeid(_lhs).name()) ;
    // log_debug << "_rhs type:\n" << TYPENAME(typeid(_rhs).name()) ;
    if (out_dims == 0) {
      log_debug << "ASSIGN_TENSOR hit out_dims == 0, which means both inputs are scale values.";
      auto _out = out->tensor<string, 0>();
      Eigen::DSizes<int64_t, 1> _out_shape(_out.size());
      in0_tensor.tensor<string, 0>().reshape(_out_shape).device(eigen_device) = _lhs;
      in1_tensor.tensor<string, 0>().reshape(_out_shape).device(eigen_device) = _rhs;  
    } else if (out_dims == 1) {
      // auto _out = out->tensor<string, 1>();
      // Eigen::DSizes<int64_t, 1> _out_shape(_out.size());
      in0_tensor.tensor<string, 1>().reshape(_out_shape).device(eigen_device) = _lhs;
      in1_tensor.tensor<string, 1>().reshape(_out_shape).device(eigen_device) = _rhs;
    } else if (out_dims == 2) {
      // auto _out = out->tensor<string, 2>();
      // Eigen::DSizes<int64_t, 1> _out_shape(_out.size());
      in0_tensor.tensor<string, 2>().reshape(_out_shape).device(eigen_device) = _lhs;
      in1_tensor.tensor<string, 2>().reshape(_out_shape).device(eigen_device) = _rhs;
    } else if (out_dims == 3) {
      // auto _out = out->tensor<string, 3>();
      // Eigen::DSizes<int64_t, 1> _out_shape(_out.size());
      in0_tensor.tensor<string, 3>().reshape(_out_shape).device(eigen_device) = _lhs;
      in1_tensor.tensor<string, 3>().reshape(_out_shape).device(eigen_device) = _rhs;
    } else if (out_dims == 4) {
      // auto _out = out->tensor<string, 4>();
      // Eigen::DSizes<int64_t, 1> _out_shape(_out.size());
      // in0_tensor.tensor<string, 4>().reshape(_out_shape).device(eigen_device) = _lhs;
      // in1_tensor.tensor<string, 4>().reshape(_out_shape).device(eigen_device) = _rhs;
      in0_tensor.tensor<string, 4>().reshape(_out_shape).device(eigen_device) = _lhs;
      in1_tensor.tensor<string, 4>().reshape(_out_shape).device(eigen_device) = _rhs;
    } else if (out_dims == 5) {
      // auto _out = out->tensor<string, 5>();
      // Eigen::DSizes<int64_t, 1> _out_shape(_out.size());
      in0_tensor.tensor<string, 5>().reshape(_out_shape).device(eigen_device) = _lhs;
      in1_tensor.tensor<string, 5>().reshape(_out_shape).device(eigen_device) = _rhs;
    } else {
      throw mpc_not_supported_exp("out_dims:" + std::to_string(out_dims));
    }
  } while (0);
} // assign_tensor

template <typename BinaryStateType = BinaryOpState>
class SecureBinaryOp : public SecureOpKernel {
 protected:
  bool lh_is_const_ = false;
  bool rh_is_const_ = false;

 public:
  explicit SecureBinaryOp(OpKernelConstruction* context) : SecureOpKernel(context) {
    context->GetAttr("lh_is_const", &lh_is_const_);
    context->GetAttr("rh_is_const", &rh_is_const_);
#if PRINT_REVEAL
    log_debug << "input0 is " << string(lh_is_const_ ? "const" : "not const") << ", input1 is "
              << string(rh_is_const_ ? "const" : "not const") ;
#endif
    if (lh_is_const_ && rh_is_const_) {
      throw mpc_not_supported_exp("MpcBinaryOp inputs are const");
    }
  }

  virtual int BinaryCompute(
    const vector<string>& in1,
    const vector<string>& in2,
    vector<string>& output,
    OpKernelContext* context) {
    throw std::runtime_error(string("please implements BinaryCompute"));
  }

  virtual void ComputeImpl(OpKernelContext* context) {
    BinaryStateType state(context);
    if (!context->status().ok())
      return;
    if (state.out_num_elements == 0) {
      // no any inputs or outputs
      return;
    }
    Tensor* out = state.out;
    BCast* bcast = &state.bcast;
    auto& in0 = state.in0;
    auto& in1 = state.in1;
    const Tensor& x0 = context->input(0);
    const Tensor& x1 = context->input(1);
    const auto& in0_flat = x0.flat<string>();
    const auto& in1_flat = x1.flat<string>();

    const int ndims = state.ndims;
    int in0_dims = state.in0_dims;
    int in1_dims = state.in1_dims;
    int out_dims = state.out_dims;

    log_debug << "dim info::: " << "N:" << ndims << ", in0:" << in0_dims << ", in1:" << in1_dims << ", out_dims:" << out_dims;
    const CPUDevice& eigen_device = context->eigen_device<CPUDevice>();
    auto out_shape = out->shape();
    Tensor in0_tensor;
    Tensor in1_tensor;
    OP_REQUIRES_OK(context, context->allocate_temp(DT_STRING, out_shape, &in0_tensor));
    OP_REQUIRES_OK(context, context->allocate_temp(DT_STRING, out_shape, &in1_tensor));

    size_t size = state.out_num_elements;
    vector<string> input0(size);
    vector<string> input1(size);
    vector<string> output(size);
    
    // we need to align the input tensor by broadcasting the low-dim tensor in missing dim.
    if (ndims == 1) {
      ASSIGN_TENSOR<1>(in0, in1, out, out_dims, in0_tensor, in1_tensor, eigen_device, bcast);
    } else if (ndims == 2) {
      ASSIGN_TENSOR<2>(in0, in1, out, out_dims, in0_tensor, in1_tensor, eigen_device, bcast);
    } else if (ndims == 3) {
      ASSIGN_TENSOR<3>(in0, in1, out, out_dims, in0_tensor, in1_tensor, eigen_device, bcast);
    } else if (ndims == 4) {
      ASSIGN_TENSOR<4>(in0, in1, out, out_dims, in0_tensor, in1_tensor, eigen_device, bcast);
    } else if (ndims == 5) {
      ASSIGN_TENSOR<5>(in0, in1, out, out_dims, in0_tensor, in1_tensor, eigen_device, bcast);
    } else {
      if (out_dims == 0) {
        input0[0] = in0_flat(0);
        input1[0] = in1_flat(0);
      } else {
        throw mpc_not_supported_exp(
          "dim error, ndims:" + std::to_string(ndims) + ",out_dims:" + std::to_string(out_dims));
      }
    }
    
    log_debug << "DEBUG raw in0:" << x0.shape() << ", and in1:" << x1.shape();
    log_debug << "DEBUG in0: " << in0_tensor.shape() << ", and in1:" << in1_tensor.shape();
    const auto& expanded_in0_flat = in0_tensor.flat<string>();
    const auto& expanded_in1_flat = in1_tensor.flat<string>();
    for (int64_t i = 0; i < size; i++) {
      input0[i] = expanded_in0_flat(i);
      input1[i] = expanded_in1_flat(i);
    }

    // fill attributes
    attrs_["lh_is_const"] = lh_is_const_ ? "1" : "0";
    attrs_["rh_is_const"] = rh_is_const_ ? "1" : "0";

    // compute with protocol
    BinaryCompute(input0, input1, output, context);

    // set output
    auto out_flat = out->flat<string>();
    for (int64_t i = 0; i < size; i++) {
      out_flat(i) = std::move(output[i]);
    }

#if PRINT_REVEAL
    debug_print_reveal(output, string(" mpc out").c_str(), context);
#endif
  }
};

class SecureUnaryOp : public SecureOpKernel {
 protected:
  vector<int> dims_;
  int elem_num_;

 public:
  SecureUnaryOp(OpKernelConstruction* context) : SecureOpKernel(context) {}
  ~SecureUnaryOp() {}

  virtual int UnaryCompute(const vector<string>& input, vector<string>& output, OpKernelContext* context) { throw; }

  virtual string name() { return string("SecureUnaryOp"); }

  virtual void ComputeImpl(OpKernelContext* context) {
    const Tensor& x = context->input(0);
    int dims = x.dims();
    for (int i = 0; i < dims; ++i) {
      dims_.push_back(x.dim_size(i));
    }
    elem_num_ = x.NumElements();

    const auto& x_flat = x.flat<string>();

    size_t size = x.NumElements();
    vector<string> input(size);
    for (auto i = 0; i < size; ++i)
      input[i] = x_flat(i);

    // compute with protocol
    vector<string> out(size);
    UnaryCompute(input, out, context);

    // set output
    Tensor* output;
    const TensorShape output_shape = x.shape();
    OP_REQUIRES_OK(
      context, context->forward_input_or_allocate_output({0}, 0, output_shape, &output));
    auto out_flat = output->flat<string>();
    for (int i = 0; i < size; i++) {
      out_flat(i) = std::move(out[i]);
    }
#if PRINT_REVEAL
    debug_print_reveal(out, string(" mpc out").c_str(), context);
#endif

    return;
  }
};

class SecureReduceOp : public SecureOpKernel {
 protected:
  bool keep_dims_ = false;

 public:
  SecureReduceOp(OpKernelConstruction* context) : SecureOpKernel(context) {
    OP_REQUIRES_OK(context, context->GetAttr("keep_dims", &keep_dims_));
  }
  ~SecureReduceOp() {}

  virtual int ReduceCompute(
    const vector<string>& input,
    vector<string>& output,
    int rows,
    int cols,
    OpKernelContext* context) {
    throw;
  }

  virtual void ComputeImpl(OpKernelContext* context);
};

} // namespace tensorflow
