#include "tensorflow/core/framework/op.h"
#include "tensorflow/core/framework/op_kernel.h"
#include "tensorflow/core/framework/shape_inference.h"
#include "tensorflow/core/lib/strings/stringprintf.h"

#include "cc/tf/secureops/secure_base_kernel.h"
#include "cc/modules/protocol/public/include/protocol_manager.h"

#include <iostream>

using namespace std;
using namespace tensorflow;
using rosetta::ProtocolManager;

static constexpr char kErrorMessage[] =
  "SecureApplyGradientDescentOp could not correctly convert string: ";

namespace tensorflow {

template <typename T>
class SecureApplyGradientDescentOp : public SecureOpKernel {
 public:
  explicit SecureApplyGradientDescentOp(OpKernelConstruction* ctx) : SecureOpKernel(ctx) {
    OP_REQUIRES_OK(ctx, ctx->GetAttr("use_locking", &use_exclusive_lock_));
  }

  void ComputeImpl(OpKernelContext* ctx) {
    log_debug << "begin debuging SecureApplyGradientDescentOp!" ;
    // Step 1: check the lock level and validity of inputs the same as the native one.
    //  Note: For now, we do NOT support the exclusive_lock feature.
    OP_REQUIRES(
      ctx, use_exclusive_lock_ == false,
      errors::FailedPrecondition(
        "In this MPC version, we do NOT support the 'use_locking' attr be true"));
    // TMP: only non-resource variable
    OP_REQUIRES(
      ctx, ctx->input_dtype(0) != DT_RESOURCE,
      errors::FailedPrecondition(
        "In this MPC version, we do NOT support the 'ResourceVariable'-type variable."));
    Tensor var;
    //Tensor* var_p;
    var = ctx->mutable_input(0, use_exclusive_lock_);

    OP_REQUIRES(
      ctx, var.IsInitialized(),
      errors::FailedPrecondition(
        "Attempting to use uninitialized variables: ", requested_input(0)));
    const Tensor& alpha = ctx->input(1);
    OP_REQUIRES(
      ctx, IsLegacyScalar(alpha.shape()),
      errors::InvalidArgument("alpha is not a scalar: ", alpha.shape().DebugString()));
    const Tensor& delta = ctx->input(2);
    OP_REQUIRES(
      ctx, var.shape().IsSameSize(delta.shape()),
      errors::InvalidArgument(
        "var and delta do not have the same shape", var.shape().DebugString(), " ",
        delta.shape().DebugString()));
    // Step 2: use internal Ops to compute
    auto ele_nums = delta.NumElements();
    auto new_var = var.flat<string>();
    // Note: simple way to parse alpha, make sure it is normal
    auto alpha_flat = double(alpha.scalar<T>()());
    string alpha_str = strings::Printf("%f", alpha_flat);
    auto delta_flat = delta.flat<string>();
    log_debug << " DEBUG ALPHA: " << alpha_flat ;

    vector<string> input_alpha(ele_nums);
    vector<string> input_delta(ele_nums);
    vector<string> input_var(ele_nums);

    for (int i = 0; i < ele_nums; ++i) {
      input_alpha[i] = alpha_str;
      input_delta[i] = delta_flat(i);
      input_var[i] = new_var(i);
    }

    vector<string> out_var(ele_nums);
    attrs_["lh_is_const"] = "1";
    attrs_["rh_is_const"] = "0";

    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Mul);
    ProtocolManager::Instance()
      ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(ctx->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->Mul(input_alpha, input_delta, out_var, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Mul);
    
    attrs_["lh_is_const"] = "0";
    attrs_["rh_is_const"] = "0";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Sub);
    ProtocolManager::Instance()
      ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(ctx->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->Sub(input_var, out_var, out_var, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Sub);

    // TODO[GeorgeShi]: why we cannot and need not use this?? due to string?
    //new_var.setZero();

    for (int i = 0; i < ele_nums; ++i) {
      new_var(i) = out_var[i];
    }

    if (ctx->input_dtype(0) != DT_RESOURCE) {
      ctx->forward_ref_input_to_ref_output(0, 0);
    }
  }

 private:
  bool use_exclusive_lock_;
};

//REGISTER_KERNEL_BUILDER(Name("RttApplyGradientDescent").Device(DEVICE_CPU), SecureApplyGradientDescentOp);
REGISTER_KERNEL_BUILDER(
  Name("SecureApplyGradientDescent").Device(DEVICE_CPU).TypeConstraint<double>("T"),
  SecureApplyGradientDescentOp<double>);
REGISTER_KERNEL_BUILDER(
  Name("SecureApplyGradientDescent").Device(DEVICE_CPU).TypeConstraint<int>("T"),
  SecureApplyGradientDescentOp<int>);

/// we do not support resourceAGD for now
// REGISTER_KERNEL_BUILDER(
//   Name("RttApplyGradientDescent").Device(DEVICE_CPU).HostMemory("var").TypeConstraint<float>("T"), cls<Eigen::ThreadPoolDevice, type>);
} // namespace tensorflow

