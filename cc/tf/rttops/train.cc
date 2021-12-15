#include "tensorflow/core/framework/op.h"
#include "tensorflow/core/framework/op_kernel.h"
#include "tensorflow/core/framework/shape_inference.h"
#include "tensorflow/core/util/bcast.h"

#include <iostream>
#include <vector>
#include <string>

using namespace std;
using namespace tensorflow;

template <typename T>
class RttApplyGradientDescentOp : public OpKernel {
 public:
  explicit RttApplyGradientDescentOp(OpKernelConstruction* ctx) : OpKernel(ctx) {
    OP_REQUIRES_OK(ctx, ctx->GetAttr("use_locking", &use_exclusive_lock_));
  }

  void Compute(OpKernelContext* ctx) override {
    cout << "error: can't exec RttApplyGradientDescentOp compute function!" << endl;
  }

 private:
  bool use_exclusive_lock_;
};


template <typename T>
class RttApplyAdamOp : public OpKernel {
 public:
  explicit RttApplyAdamOp(OpKernelConstruction* ctx) : OpKernel(ctx) {
    OP_REQUIRES_OK(ctx, ctx->GetAttr("use_locking", &use_exclusive_lock_));
    OP_REQUIRES_OK(ctx, ctx->GetAttr("use_nesterov", &use_nesterov_));
  }

  void Compute(OpKernelContext* ctx) override {
    cout << "error: can't exec RttApplyAdamOp compute function!" << endl;
  }

  private:
  bool use_exclusive_lock_;
  bool use_nesterov_;
};




REGISTER_KERNEL_BUILDER(
  Name("RttApplyGradientDescent").Device(DEVICE_CPU).TypeConstraint<double>("T"),
  RttApplyGradientDescentOp<double>);
REGISTER_KERNEL_BUILDER(
  Name("RttApplyGradientDescent").Device(DEVICE_CPU).TypeConstraint<int>("T"),
  RttApplyGradientDescentOp<int>);

  REGISTER_KERNEL_BUILDER(
  Name("RttApplyAdamOp").Device(DEVICE_CPU).TypeConstraint<double>("T"),
  RttApplyAdamOp<double>);
REGISTER_KERNEL_BUILDER(
  Name("RttApplyAdamOp").Device(DEVICE_CPU).TypeConstraint<int>("T"),
  RttApplyAdamOp<int>);

/// we do not support resourceAGD in Demo for now
// REGISTER_KERNEL_BUILDER(
//   Name("RttApplyGradientDescent").Device(DEVICE_CPU).HostMemory("var").TypeConstraint<float>("T"), cls<Eigen::ThreadPoolDevice, type>);

