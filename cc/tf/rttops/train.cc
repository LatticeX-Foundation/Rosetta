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
    cout << "begin debuging RttApplyGradientDescentOp!" << endl;
    // // Step 1: check the lock level and validity of inputs the same as the native one.
    // //  Note: For now, we do NOT support the exclusice_lock feature.
    // OP_REQUIRES(
    //   ctx, use_exclusive_lock_ == false,
    //   errors::FailedPrecondition(
    //     "In this MPC version, we do NOT support the 'use_locking' attr be true"));
    // // TMP: only non-resource variable
    // OP_REQUIRES(
    //   ctx, ctx->input_dtype(0) != DT_RESOURCE,
    //   errors::FailedPrecondition(
    //     "In this MPC version, we do NOT support the 'ResourceVarible'-type variable."));
    // Tensor var;
    // //Tensor* var_p;
    // var = ctx->mutable_input(0, use_exclusive_lock_);

    // OP_REQUIRES(
    //   ctx, var.IsInitialized(),
    //   errors::FailedPrecondition(
    //     "Attempting to use uninitialized variables: ", requested_input(0)));
    // const Tensor& alpha = ctx->input(1);
    // OP_REQUIRES(
    //   ctx, IsLegacyScalar(alpha.shape()),
    //   errors::InvalidArgument("alpha is not a scalar: ", alpha.shape().DebugString()));
    // const Tensor& delta = ctx->input(2);
    // OP_REQUIRES(
    //   ctx, var.shape().IsSameSize(delta.shape()),
    //   errors::InvalidArgument(
    //     "var and delta do not have the same shape", var.shape().DebugString(), " ",
    //     delta.shape().DebugString()));
    // // Step 2:
    // //cout << "======DEBUG: AGD MPC part =======" <<endl;
    // // Attention!：In this version, we still need to convert the nums to MpcType
    // auto var_flat = var.flat<string>();
    // auto alpha_flat = double(alpha.scalar<T>()());
    // auto delta_flat = delta.flat<string>();
    // cout << " ALPHA: " << alpha_flat << endl;
    // auto ele_nums = delta.NumElements();
    // vector<double> var_double(ele_nums);
    // vector<double> delta_double(ele_nums);
    // vector<double> new_var_double(ele_nums);

    // for (int i = 0; i < ele_nums; ++i) {
    //   OP_REQUIRES(
    //     ctx, strings::SafeStringToNumeric<double>(var_flat(i), &var_double[i]),
    //     errors::InvalidArgument(kErrorMessage, var_flat(i).c_str()));

    //   OP_REQUIRES(
    //     ctx, strings::SafeStringToNumeric<double>(delta_flat(i), &delta_double[i]),
    //     errors::InvalidArgument(kErrorMessage, delta_flat(i).c_str()));

    //   new_var_double[i] = var_double[i] - alpha_flat * delta_double[i];
    // }

    // // TODO[georgeshi]: why we canot and need not use this?? due to string?
    // //var_flat.setZero();

    // for (int i = 0; i < ele_nums; ++i) {
    //   // Note: in this demo, we fix it tobe float64, so we use %f
    //   //       in the next official version for MPC, we should use %lld to have int64!
    //   cout << i << "-th v:" << new_var_double[i] << endl;
    //   var_flat(i) = strings::Printf("%f", new_var_double[i]);
    // }
    // if (ctx->input_dtype(0) != DT_RESOURCE) {
    //   ctx->forward_ref_input_to_ref_output(0, 0);
    // }
  }

 private:
  bool use_exclusive_lock_;
};

//REGISTER_KERNEL_BUILDER(Name("RttApplyGradientDescent").Device(DEVICE_CPU), RttApplyGradientDescentOp);
REGISTER_KERNEL_BUILDER(
  Name("RttApplyGradientDescent").Device(DEVICE_CPU).TypeConstraint<double>("T"),
  RttApplyGradientDescentOp<double>);
REGISTER_KERNEL_BUILDER(
  Name("RttApplyGradientDescent").Device(DEVICE_CPU).TypeConstraint<int>("T"),
  RttApplyGradientDescentOp<int>);

/// we do not support resourceAGD in Demo for now
// REGISTER_KERNEL_BUILDER(
//   Name("RttApplyGradientDescent").Device(DEVICE_CPU).HostMemory("var").TypeConstraint<float>("T"), cls<Eigen::ThreadPoolDevice, type>);

