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

template <typename Device>
class RttSigmoidCrossEntropyOp : public OpKernel {
 public:
  explicit RttSigmoidCrossEntropyOp(OpKernelConstruction* context) : OpKernel(context) {}
  
  void Compute(OpKernelContext* context) {
    //TODO
  }
};

template <typename Device>
class RttSigmoidOp : public OpKernel {
 public:
  explicit RttSigmoidOp(OpKernelConstruction* context) : OpKernel(context) {}
  void Compute(OpKernelContext* context) {
    //TODO
  }
}; // namespace tensorflow

template <typename Device>
class RttReluOp : public OpKernel {
 public:
  explicit RttReluOp(OpKernelConstruction* context) : OpKernel(context) {}
  void Compute(OpKernelContext* context) {
    //TODO
  }
}; // namespace tensorflow

typedef Eigen::ThreadPoolDevice CPUDevice;

REGISTER_KERNEL_BUILDER(Name("RttSigmoidCrossEntropy").Device(DEVICE_CPU), RttSigmoidCrossEntropyOp<CPUDevice>);
REGISTER_KERNEL_BUILDER(Name("RttSigmoid").Device(DEVICE_CPU), RttSigmoidOp<CPUDevice>);
REGISTER_KERNEL_BUILDER(Name("RttRelu").Device(DEVICE_CPU), RttReluOp<CPUDevice>);

} // namespace tensorflow
