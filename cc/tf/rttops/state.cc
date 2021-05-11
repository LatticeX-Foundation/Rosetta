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
#include <iostream>
#include <memory>
#include <assert.h>
#include <vector>
#include <string>

#include "Eigen/Core"
#include "Eigen/Dense"
#include "tensorflow/core/framework/op.h"
#include "tensorflow/core/framework/op_kernel.h"
#include "tensorflow/core/framework/shape_inference.h"
#include "tensorflow/core/util/bcast.h"

namespace tensorflow {

class RttAssignOp : public OpKernel {
 public:
  explicit RttAssignOp(OpKernelConstruction* ctx) : OpKernel(ctx) {
    OP_REQUIRES_OK(ctx, ctx->GetAttr("use_locking", &use_locking_));
  }

  void Compute(OpKernelContext* ctx) override {
    Tensor var = ctx->mutable_input(0, use_locking_);
    const Tensor& value = ctx->input(1);
    auto var_flat = var.flat<string>();
    auto value_flat = value.flat<string>();

    auto ele_nums = value.NumElements();
    for (int i = 0; i < ele_nums; ++i) {
      var_flat(i) = value_flat(i);
    }

    if (ctx->input_dtype(0) != DT_RESOURCE) {
      ctx->forward_ref_input_to_ref_output(0, 0);
    }

  }

 private:
  bool use_locking_ = true;
};


template <typename Device>
class RttAssignSubOp : public OpKernel {
 public:
  explicit RttAssignSubOp(OpKernelConstruction* context) : OpKernel(context) {}
  void Compute(OpKernelContext* context) {
    //TODO
  }
};



typedef Eigen::ThreadPoolDevice CPUDevice;

REGISTER_KERNEL_BUILDER(Name("RttAssign").Device(DEVICE_CPU), RttAssignOp);
REGISTER_KERNEL_BUILDER(Name("RttAssignSub").Device(DEVICE_CPU), RttAssignSubOp<CPUDevice>);

} // namespace tensorflow
