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
#include <fstream>
#include <typeinfo>
using namespace std;

#include "tensorflow/core/framework/op.h"
#include "tensorflow/core/framework/op_kernel.h"
#include "tensorflow/core/framework/common_shape_fns.h"
#include "tensorflow/core/framework/tensor_shape.h"
#include "tensorflow/core/framework/kernel_def.pb.h"
#include "tensorflow/core/framework/kernel_def_builder.h"
#include "tensorflow/core/framework/node_def.pb.h"
#include "tensorflow/core/framework/node_def_util.h"
#include "tensorflow/core/util/bcast.h"
#include "tensorflow/core/framework/rendezvous.h"
#include "tensorflow/core/framework/session_state.h"

#include "model_tool.h"
#include "mpc.h"
#include "mpc_exceptions.h"

#define USE_MPC_OP 1

// please set 1 for debuging
#define PRINT_REVEAL 0

namespace tensorflow {

typedef Eigen::ThreadPoolDevice CPUDevice;
typedef Eigen::GpuDevice GPUDevice;

#define REGISTER_MPCOP_KERNELS(name, cls, type) \
  REGISTER_KERNEL_BUILDER(                      \
    Name(#name).Device(DEVICE_CPU).TypeConstraint<type>("T"), cls<Eigen::ThreadPoolDevice, type>);

// Added by SJJ: for reosurcevariable on ApplyGradientDescent only, for now.
#define VAR_REGISTER_MPCOP_KERNELS(name, cls, type) \
  REGISTER_KERNEL_BUILDER(                      \
    Name(#name).Device(DEVICE_CPU).HostMemory("var").TypeConstraint<type>("T"), cls<Eigen::ThreadPoolDevice, type>);


// now, only support float64 for input
#define REGISTER_MPCOP_KERNELS_ALL_TYPES(name, cls) \
  REGISTER_MPCOP_KERNELS(name, cls, float)          \
  REGISTER_MPCOP_KERNELS(name, cls, double)         \
  REGISTER_MPCOP_KERNELS(name, cls, int)

#define VAR_REGISTER_MPCOP_KERNELS_ALL_TYPES(name, cls) \
  VAR_REGISTER_MPCOP_KERNELS(name, cls, float)          \
  VAR_REGISTER_MPCOP_KERNELS(name, cls, double)         \
  VAR_REGISTER_MPCOP_KERNELS(name, cls, int)


#define REGISTER_MPCOP_KERNELS_BINARYOP(name, cls, type, bop) \
  REGISTER_KERNEL_BUILDER(                                    \
    Name(#name).Device(DEVICE_CPU).TypeConstraint<type>("T"), \
    cls<Eigen::ThreadPoolDevice, type, bop>);


// now, only support float64 for input
#define REGISTER_MPCOP_KERNELS_ALL_TYPES_BINARYOP(name, cls, bop) \
  REGISTER_MPCOP_KERNELS_BINARYOP(name, cls, float, bop)          \
  REGISTER_MPCOP_KERNELS_BINARYOP(name, cls, double, bop)         \
  REGISTER_MPCOP_KERNELS_BINARYOP(name, cls, int, bop)

// the fellowing macro used by binary compare ops
#define REGISTER_MPCOP_KERNELS_CMP_BINARYOP(name, cls, type, bop) \
  REGISTER_KERNEL_BUILDER(                                        \
    Name(#name).Device(DEVICE_CPU).TypeConstraint<type>("T"),     \
    cls<Eigen::ThreadPoolDevice, type, bop, uint64, true>);

// now, only support float64 for input
#define REGISTER_MPCOP_KERNELS_ALL_TYPES_CMP_BINARYOP(name, cls, bop) \
  REGISTER_MPCOP_KERNELS_CMP_BINARYOP(name, cls, float, bop)          \
  REGISTER_MPCOP_KERNELS_CMP_BINARYOP(name, cls, double, bop)         \
  REGISTER_MPCOP_KERNELS_CMP_BINARYOP(name, cls, int, bop)

} // namespace tensorflow

class BaseFunctor {
 public:
  virtual ~BaseFunctor() = default;
  BaseFunctor(const shared_ptr<rosetta::mpc::Empty>& baseop) : baseop_(baseop) {}

 protected:
  shared_ptr<rosetta::mpc::Empty> baseop_ = nullptr;
  const msg_id_t& msg_id() const {
    return baseop_->msg_id();
  }
};
#define tfGetMpcOp(opname) std::make_shared<rosetta::mpc::opname>(baseop_)
//#define tfGetMpcOp(opname) GetMpcOpWithKey(opname, baseop_->msg_id());

namespace tensorflow {
/*
0. first implement add/sub/mul/matmul/...

todo:s
1. setShapeFn(). Now only support the input shape by default
  eg.
  1) for add, input1.shape() == input2.shape
  2) for matmul, m x k x n, input1.shape() = (m, k), input2.shape() = (k, n)
2. how to reveal
3. only support 1d,2d
*/
class MpcOpKernel : public OpKernel {
  int verbose_ = 1;

  msg_id_t msg_id_;

 protected:
  const msg_id_t& msg_id() const {
    return msg_id_;
  }

 protected:
  // in construct, init key and so on
  // in deconstruct, del the key and so on
  shared_ptr<rosetta::mpc::Empty> baseop_ = nullptr;
  const shared_ptr<rosetta::mpc::Empty>& baseop() const {
    return baseop_;
  }

 public:
  explicit MpcOpKernel(OpKernelConstruction* context) : OpKernel(context) {
    // about node
    const NodeDef& def = context->def();
#if PRINT_REVEAL
    {
      cout << "======================================================= BEG NODEDEF\n";

      cout << "  name:" << def.name() << endl;
      cout << "    op:" << def.op() << endl;
      cout << "device:" << def.device() << endl;
      cout << "inputs:" << endl;
      for (auto& input : def.input()) {
        cout << "       " << input << endl;
      }
      cout << " attrs:" << endl;
      for (auto& attr : def.attr()) {
        cout << "       " << attr.first << endl;
      }
      cout << "type_string:" << type_string() << endl;
      cout << "======================================================= END NODEDEF\n";
    }
#endif

    msg_id_ = msg_id_t(def.name());
    cout << "MpcOpKernel msgid:" << msg_id() << endl;

    // init base op, do some init.
    // in this op, will set the same message key for each party
    baseop_ = GetMpcOpWithKey(Empty, msg_id());
  }
  void Compute(OpKernelContext* context) override {
    /*
    1. convert input(s) to vector<uint64_t>
    2. execute mpc operations
    3. convert output(s) to tensor (?? something problems here: loss the precision)
    4. extras: exception control, such as network broken and reconnection
    */

    // do something before mpc-op

    // for test, print some debug infomations
#if PRINT_REVEAL
    cout << endl;
    debug_print_before(context);
#endif

    // a temp way to cope exceptions thrown by op
    try {
      MpcCompute(context);
    } catch (exception& e) {
      cerr << "MpcCompute(context) got an exception " << e.what() << endl;
    }

    // do something after mpc-op
    // for test, print some debug infomations
#if PRINT_REVEAL
    debug_print_after(context);
#endif
  }

  virtual void MpcCompute(OpKernelContext* context) {
    throw;
  }

  /*
  **
  ** below functions, just for test or debug
  **
  */
  void debug_init();
  void debug_print_before(OpKernelContext* context) {
    cout << "======================================================= BEG BEFORE\n";
    // about input(s)
    cout << "Debug MpcOpKernel num_inputs: " << context->num_inputs() << endl;
    for (int i = 0; i < context->num_inputs(); i++) {
      if (IsRefType(context->input_dtype(i))) {
        Tensor var = context->mutable_input(i, false);
        cout << "input var(" << i << ").shape(): " << var.shape() << endl;
        cout << "Debug MpcOpKernel input(" << i << "): " << var.DebugString(9) << endl;
        continue;
      }
      const Tensor& x = context->input(i);
      cout << "input(" << i << ").shape(): " << x.shape() << endl;
      if (x.NumElements() > 0) {
        cout << "Debug MpcOpKernel input(" << i << "): " << x.DebugString(9) << endl;
      }
      // else x.shape() == TensorShape({0})
    }
    cout << "-------------------------------------------------------\n";

    // about output(s)
    cout << "Debug MpcOpKernel num_outputs: " << context->num_outputs() << endl;

    cout << "======================================================= END BEFORE\n";
  }

  void debug_print_after(OpKernelContext* context) {
    //cout << "======================================================= BEG AFTER\n";
    //cout << "======================================================= END AFTER\n";
  }

  void debug_print_tensor(const Tensor* z, const char* msg = "") {
    string s = "";
    if (z != nullptr) {
      s = z->DebugString(16);
    }
    cout << msg << " " << s;
  }

  void debug_print_reveal(const vector<mpc_t>& in, vector<double>& out, std::string msg = "") {
    cout << __FUNCTION__ << "================= " << msg << endl;

    size_t size = in.size();
    vector<mpc_t> out2(size);

    tfGetMpcOp(Reconstruct2PC)->Run(in, size, out2);
    print_vec(out2, 7);

    out.clear();
    out.resize(size);
    convert_mpctype_to_double(out2, out);
    print_vec(out, 7);

    cout << __FUNCTION__ << "================= " << endl;
  }

  void debug_print_reveal(const vector<double>& in, std::string msg = "") {
    cout << __FUNCTION__ << "================= " << msg << endl;
    print_vec(in, 7);
    cout << __FUNCTION__ << "================= " << endl;
  }
};

} // namespace tensorflow
