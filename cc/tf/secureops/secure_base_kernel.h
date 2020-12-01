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
#include <string>
#include <typeinfo>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <stdexcept>
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

#include "cc/modules/protocol/mpc/snn/include/mpc_tools.h"
#include "cc/modules/protocol/mpc/snn/include/snn_opsets.h"
#include "cc/modules/protocol/public/protocol_manager.h"
#include "cc/tf/secureops/mpc_exceptions.h"


// please set 1 for debuging
#define PRINT_REVEAL 0
#if PRINT_REVEAL
#define DEBUG_PRINT_AFTER(CTX) debug_print_after(CTX)
#define DEBUG_PRINT_BEFORE(CTX) debug_print_before(CTX)
#else
#define DEBUG_PRINT_AFTER(CTX)
#define DEBUG_PRINT_BEFORE(CTX)
#endif

using CPUDevice = Eigen::ThreadPoolDevice;
using GPUDevice = Eigen::GpuDevice;

// string only
#define REGISTER_STR_CPU_KERNEL(name, cls) REGISTER_KERNEL_BUILDER(Name(#name).Device(DEVICE_CPU), cls)

namespace tensorflow {
class SecureOpKernel : public OpKernel {
 protected:
  const msg_id_t& msg_id() const {
    return msg_id_;
  }

 protected:
  int verbose_ = 1;
  string op_;
  msg_id_t msg_id_;
  unordered_map<string, string>   attrs_;

 public:
  explicit SecureOpKernel(OpKernelConstruction* context) : OpKernel(context) {
    // about node
    const NodeDef& def = context->def();
    op_ = def.op();

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

    msg_id_ = msg_id_t(def.name());
    log_debug << "SecureOpKernel msgid:" << msg_id();

    //-----------------------------------------------
    //Deal with PrivateInput op in decode function
    auto func_lib = context->function_library();
    if (func_lib) {
      auto func_def = func_lib->GetFunctionLibraryDefinition();
      if (func_def) {
        std::vector<string> func_name_lists = func_def->ListFunctionNames();
        if (func_name_lists.size() == 1 && !strcmp(def.name().c_str(), "PrivateInput")) {
          string op_unique_name = func_name_lists[0] + "/" +  def.name();
          msg_id_ = msg_id_t(op_unique_name);
          log_debug << "New SecureOpKernel msgid:" << msg_id();
        }       
      }
    }
    //-----------------------------------------------

  }
  void Compute(OpKernelContext* context) override {
    DEBUG_PRINT_BEFORE(context);
    ComputeImpl(context);
    DEBUG_PRINT_AFTER(context);
  }

  virtual void ComputeImpl(OpKernelContext* context) {
    log_debug << "SecureOpKernel ComputeImpl... exception !";
    throw;
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

  void debug_print_after(OpKernelContext* context) {
    //log_debug << "======================================================= BEG AFTER\n";
    //log_debug << "======================================================= END AFTER\n";
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

  virtual void debug_print_reveal(const vector<string>& in, std::string msg = "") {
    if (op_ == "SecureReveal")
      return;
    log_debug << __FUNCTION__ << "================= " << msg << endl;
    
    size_t size = in.size();
    vector<string> outs(size);
    attrs_["receive_party"] = string("1");
    rosetta::ProtocolManager::Instance()->GetProtocol()->GetOps(msg_id().str())->Reveal(in, outs, &attrs_);
    print_vec(outs);

    log_debug << __FUNCTION__ << "================= " << endl;
  }
};

class SecureBinaryOp : public SecureOpKernel {
 protected:
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
             << in1.shape().DebugString() << endl;
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

      if (verbose_ > 1) {
        log_debug << "        output.shape:" << output_shape;
        log_debug << "          in0.dims():" << in0.dims();
        log_debug << "          in1.dims():" << in1.dims();
        log_debug << " output_shape.dims():" << output_shape.dims();
        log_debug << "               ndims:" << ndims;
        log_debug << "    in0_num_elements:" << in0_num_elements;
        log_debug << "    in1_num_elements:" << in1_num_elements;
        log_debug << "    out_num_elements:" << out_num_elements;
      }
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
  };

  bool lh_is_const_ = false;
  bool rh_is_const_ = false;

 public:
  explicit SecureBinaryOp(OpKernelConstruction* context) : SecureOpKernel(context) {
    context->GetAttr("lh_is_const", &lh_is_const_);
    context->GetAttr("rh_is_const", &rh_is_const_);
#if PRINT_REVEAL
    log_debug << "input0 is " << string(lh_is_const_ ? "const" : "not const") << ", input1 is "
         << string(rh_is_const_ ? "const" : "not const") << endl;
#endif
    if (lh_is_const_ && rh_is_const_) {
      throw mpc_not_supported_exp("MpcBinaryOp inputs are const");
    }
  }

  virtual int BinaryCompute(
    const vector<string>& in1, const vector<string>& in2, vector<string>& output) {
    throw std::runtime_error(string("please implements BinaryCompute"));
  }
  void ComputeImpl(OpKernelContext* context) {
    BinaryOpState state(context);
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
    if ((in0_dims > 2) || (in1_dims > 2)) {
      // not support
      throw mpc_not_supported_exp("dim error, ndims:" + std::to_string(ndims));
    }

    // assuming element-wise
    size_t size = state.out_num_elements;
    vector<string> input0;
    vector<string> input1;

    int rows = 0, cols = 0;
    int shape_case = 0;
    if (out_dims == 0) {
      rows = 1;
      cols = 1;
    } else if (out_dims == 1) {
      rows = 1;
      cols = out->dim_size(0);
    } else if (out_dims == 2) {
      rows = out->dim_size(0);
      cols = out->dim_size(1);
    }

    ///////////////////////////////////////
    vector<vector<string>> vec0(rows, vector<string>(cols));
    vector<vector<string>> vec1(rows, vector<string>(cols));
    auto f = [&](vector<vector<string>>& vec, int index) {
      const Tensor& inp = context->input(index);
      const auto& in_flat = inp.flat<string>();
      int inp_dims = inp.dims();

      // row first
      if (out_dims == 0) {
        vec[0][0] = in_flat(0);
      } else if (out_dims == 1) {
        if (inp_dims == 0) {
          for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
              vec[i][j] = in_flat(0); // ()
            }
          }
        } else if (inp_dims == 1) {
          int inp_cols = inp.dim_size(0);
          for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
              if (inp_cols == 1)
                vec[i][j] = in_flat(0); // (1,)
              else
                vec[i][j] = in_flat(j); // (1,)
            }
          }
        }

      } else if (out_dims == 2) {
        if (inp_dims == 0) {
          for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
              vec[i][j] = in_flat(0); // ()
            }
          }
        } else if (inp_dims == 1) {
          int inp_cols = inp.dim_size(0);
          for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
              if (inp_cols == 1)
                vec[i][j] = in_flat(0); // (1,)
              else
                vec[i][j] = in_flat(j); // (1,)
            }
          }
        } else if (inp_dims == 2) {
          int inp_rows = inp.dim_size(0);
          int inp_cols = inp.dim_size(1);
          if (inp_cols == 1) {
            for (int i = 0; i < rows; i++) {
              for (int j = 0; j < cols; j++) {
                if (rows == inp_rows)
                  vec[i][j] = in_flat(i); // (n, 1)
                else
                  vec[i][j] = in_flat(0); // (1, 1)
              }
            }
          } else {
            for (int i = 0; i < rows; i++) {
              for (int j = 0; j < cols; j++) {
                if (rows == inp_rows)
                  vec[i][j] = in_flat(i * cols + j); // (n, m)
                else {
                  vec[i][j] = in_flat(j); // (1, m)
                }
              }
            }
          }
        }
      }
    };
    f(vec0, 0);
    f(vec1, 1);
    ///////////////////////////////////////

    // output raw data
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
        int idx = i * cols + j;
        input0.push_back(vec0[i][j]);
        if (idx < 2) {
#if PRINT_REVEAL
          log_debug << "input0 (i,j,I)" << i << " " << j << " " << idx << " : " << input0[idx] << endl;
#endif
        }
      }
    }
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
        int idx = i * cols + j;
        input1.push_back(vec1[i][j]);
        if (idx < 2) {
#if PRINT_REVEAL
          log_debug << "input1 (i,j,I)" << i << " " << j << " " << idx << " : " << input1[idx] << endl;
#endif
        }
      }
    }

    vector<string> output(size);

    // fill attributes
    attrs_["lh_is_const"] = lh_is_const_ ? "1" : "0";
    attrs_["rh_is_const"] = rh_is_const_ ? "1" : "0";

    // compute with protocol
    BinaryCompute(input0, input1, output);

    // set output
    auto out_flat = out->flat<string>();
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
        int idx = i * cols + j;
        out_flat(idx) = output[idx];
      }
    }
#if PRINT_REVEAL
    debug_print_reveal(output, string(" mpc out").c_str());
#endif
  }
};

class StrUnaryOp : public SecureOpKernel {
 protected:
  vector<int> dims_;
  int elem_num_;
 public:
  StrUnaryOp(OpKernelConstruction* context) : SecureOpKernel(context) {}
  ~StrUnaryOp() {}

  virtual int UnaryCompute(const vector<string>& input, vector<string>& output) {
    throw;
  }

  virtual string name() {return string("StrUnaryOp");}

  void ComputeImpl(OpKernelContext* context) {
    const Tensor& x = context->input(0);
    int dims = x.dims();
    for (int i = 0; i < dims; ++i)
    {
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
    UnaryCompute(input, out);

    // set output
    Tensor* output;
    const TensorShape output_shape = x.shape();
    OP_REQUIRES_OK(
      context, context->forward_input_or_allocate_output({0}, 0, output_shape, &output));
    auto out_flat = output->flat<string>();
    for (int i = 0; i < size; i++) {
      out_flat(i) = out[i];
    }
#if PRINT_REVEAL
    debug_print_reveal(out, string(" mpc out").c_str());
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

  virtual int ReduceCompute(const vector<string>& input, vector<string>& output, int rows, int cols) {
    throw;
  }
  void ComputeImpl(OpKernelContext* context) {
    const Tensor& x = context->input(0);
    const Tensor& y = context->input(1);
    const auto& x_flat = x.flat<string>();
    assert(y.NumElements() <= 2);       //Only 2-dims are supported for now

    ///////////////////////////////////////////////////////////
#if PRINT_REVEAL
    log_debug << "-------------   ReduceOp-" << name() << " , keep_dims: " << keep_dims_
         << " ------------\n";
    log_debug << "          in0.dims():" << x.dims() << endl;
    log_debug << "          in1.dims():" << y.dims() << endl;
    log_debug << "    in0_num_elements:" << x.NumElements() << endl;
    log_debug << "    in1_num_elements:" << y.NumElements() << endl;
    log_debug << "          axes      :" << y.SummarizeValue(10) << endl;
    log_debug << "--------------------------------------------------------------------------\n";
    log_debug << endl << endl << std::flush;
#endif
    ///////////////////////////////////////////////////////////

    // TODO: HANDLE REDUCE DIMS AND RESHAPE
    int dims = x.dims();
    int axis = 0;
    
    if (dims == 0) {
      axis = -1;
    } else if (y.NumElements() > 0 && y.NumElements() < 2) {
      auto rindices = y.flat<int32_t>();
      axis = rindices(0);
    } else if (y.NumElements() >= 2) {   
      axis = -1;
    } else if (y.NumElements() == 0) {
      // not do reshape, just copy
      Tensor out;
      if (!out.CopyFrom(x, x.shape())) {
        context->SetStatus(errors::Internal("Error during reduction copy."));
      }
      context->set_output(0, out);
      return;
    }

    int rows, cols;
    if (dims == 0) {
      rows = 1;
      cols = 1;
    } else if (dims == 1) {
      rows = 1;
      cols = x.dim_size(0);
    } else if (dims == 2) {
      rows = x.dim_size(0);
      cols = x.dim_size(1);
    } else {
      // OP_REQUIRES_OK(context, context->allocate_output(0, x.shape(), &z));
      log_debug << "only supported 0 <= dim <= 2, not supported dims:" << dims << endl;
      return;
    }

    int fix_rows = rows;
    int fix_cols = cols;
    int shape_size = rows;
    if (axis == 0) {
      shape_size = cols;
      std::swap(fix_rows, fix_cols);
    } else if (axis == 1) {
      shape_size = rows;
    } else if (axis == -1) {
      shape_size = 1;
      fix_cols = rows*cols;
      fix_rows = 1;
    } else {
      log_error << "not support axis: " << axis << endl;
      throw;
    }

    if (dims == 1) {
      shape_size = 1;
      fix_cols = rows*cols;
      fix_rows = 1;
    }

    Tensor* z = nullptr;
    if ((dims <= 1) || (axis == -1)) {
      TensorShape zshape;
      OP_REQUIRES_OK(context, context->allocate_output(0, zshape, &z));
    } else {
      TensorShape zshape({shape_size});
      OP_REQUIRES_OK(context, context->allocate_output(0, zshape, &z));
    }

    // assuming element-wise
    vector<string> inputx;
    if (axis == 0) {// col first
      for (int j = 0; j < cols; j++) {
        for (int i = 0; i < rows; i++) {
          int idx = i * cols + j;
          inputx.push_back(x_flat(idx));
        }
      }
    } else {// row first
      for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
          int idx = i * cols + j;
          inputx.push_back(x_flat(idx));
        }
      }
    }

    // fill reduce op attributes
    // adjust rows, cols, because mpc process reduce with reduce_indices=1
    attrs_["rows"] = std::to_string(fix_rows);
    attrs_["cols"] = std::to_string(fix_cols);

    int size = z->NumElements();
    log_debug << "**Reduce-" << name() << "Op, rows: " << rows  << ", cols: " << cols 
          << ", axis: " << axis << ", z-size:" << size << ", shape_size: " << shape_size << ", dims: " << dims << endl;

    // compute with protocol
    vector<string> out(shape_size);
    ReduceCompute(inputx, out, rows, cols);

    // set output
    auto out_flat = z->flat<string>();
    for (int i = 0; i < size; i++) {
      out_flat(i) = out[i];
    }
#if PRINT_REVEAL
    debug_print_reveal(out, string(" mpc out").c_str());
#endif
    return;
  }
};

} // namespace tensorflow
