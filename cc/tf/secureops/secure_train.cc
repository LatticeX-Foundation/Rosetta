#include "tensorflow/core/framework/op.h"
#include "tensorflow/core/framework/op_kernel.h"
#include "tensorflow/core/framework/shape_inference.h"
#include "tensorflow/core/lib/strings/stringprintf.h"

#include "cc/tf/secureops/secure_base_kernel.h"
#include "cc/modules/protocol/public/include/protocol_manager.h"
#include "cc/modules/common/include/utils/secure_misc.h"

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

template <typename T>
class SecureApplyAdamOp : public SecureOpKernel {
public:
  explicit SecureApplyAdamOp(OpKernelConstruction* ctx) : SecureOpKernel(ctx) {
    OP_REQUIRES_OK(ctx, ctx->GetAttr("use_locking", &use_exclusive_lock_));
    OP_REQUIRES_OK(ctx, ctx->GetAttr("use_nesterov", &use_nesterov_));
  }

  void ComputeImpl(OpKernelContext* ctx) {
    log_debug << "begin debuging SecureApplyAdamOp!" ;
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

    Tensor var = ctx->mutable_input(0, use_exclusive_lock_);
    OP_REQUIRES(
      ctx, var.IsInitialized(),
      errors::FailedPrecondition(
        "Attempting to use uninitialized variables: ", requested_input(0)));

    Tensor m = ctx->mutable_input(1, use_exclusive_lock_);
    OP_REQUIRES(
      ctx, m.IsInitialized(),
      errors::FailedPrecondition(
        "Attempting to use uninitialized variables: ", requested_input(1)));

    Tensor v = ctx->mutable_input(2, use_exclusive_lock_);
    OP_REQUIRES(
      ctx, v.IsInitialized(),
      errors::FailedPrecondition(
        "Attempting to use uninitialized variables: ", requested_input(2)));

    const Tensor& beta1_power = ctx->input(3);
    OP_REQUIRES(
      ctx, IsLegacyScalar(beta1_power.shape()),
      errors::InvalidArgument("beta1_power is not a scalar: ", beta1_power.shape().DebugString()));

    const Tensor& beta2_power = ctx->input(4);
    OP_REQUIRES(
      ctx, IsLegacyScalar(beta2_power.shape()),
      errors::InvalidArgument("beta2_power is not a scalar: ", beta2_power.shape().DebugString()));

    const Tensor& lr = ctx->input(5);
    OP_REQUIRES(
      ctx, IsLegacyScalar(lr.shape()),
      errors::InvalidArgument("lr is not a scalar: ", lr.shape().DebugString()));

    const Tensor& beta1 = ctx->input(6);
    OP_REQUIRES(
      ctx, IsLegacyScalar(beta1.shape()),
      errors::InvalidArgument("beta1 is not a scalar: ", beta1.shape().DebugString()));

    const Tensor& beta2 = ctx->input(7);
    OP_REQUIRES(
      ctx, IsLegacyScalar(beta2.shape()),
      errors::InvalidArgument("beta2 is not a scalar: ", beta2.shape().DebugString()));

    const Tensor& epsilon = ctx->input(8);
    OP_REQUIRES(
      ctx, IsLegacyScalar(epsilon.shape()),
      errors::InvalidArgument("epsilon is not a scalar: ", epsilon.shape().DebugString()));

    const Tensor& grad = ctx->input(9);
    OP_REQUIRES(
      ctx, var.shape().IsSameSize(grad.shape()),
      errors::InvalidArgument(
        "var and grad do not have the same shape", var.shape().DebugString(), " ",
        grad.shape().DebugString()));

    // Step 2: use internal Ops to compute
    auto ele_nums = grad.NumElements();
    auto new_var = var.flat<string>();
    auto new_m = m.flat<string>();
    auto new_v = v.flat<string>();
    auto beta1_power_str = beta1_power.scalar<string>()();
    auto beta2_power_str = beta2_power.scalar<string>()();
    auto grad_flat = grad.flat<string>();
    // Note: simple way to parse alpha, make sure it is normal
    auto lr_flat = double(lr.scalar<T>()());
    string lr_str = strings::Printf("%f", lr_flat);

    auto beta1_flat = double(beta1.scalar<T>()());
    string beta1_str = strings::Printf("%f", beta1_flat);

    auto beta2_flat = double(beta2.scalar<T>()());
    string beta2_str = strings::Printf("%f", beta2_flat);

    auto epsilon_flat = double(epsilon.scalar<T>()());
    string epsilon_str = strings::Printf("%f", epsilon_flat);

    string one_str = strings::Printf("%f", 1.0);

    log_debug <<"adam parameter " << "lr:" << lr_str << ", beta1:" << beta1_str << ", beta2:" << beta2_str << ", epsilon:" << epsilon_str << ", beta1 power:" << beta1_power_str << ", beta2 power:" << beta2_power_str;
    vector<string> input_lr(ele_nums, lr_str);
    vector<string> input_beta1(ele_nums, beta1_str);
    vector<string> input_beta2(ele_nums, beta2_str);
    vector<string> input_epsilon(ele_nums, epsilon_str);
    vector<string> input_beta1_power(ele_nums, beta1_power_str);
    vector<string> input_beta2_power(ele_nums, beta2_power_str);
    vector<string> input_grad(ele_nums);
    vector<string> input_var(ele_nums);
    vector<string> input_m(ele_nums);
    vector<string> input_v(ele_nums);

    for (int i = 0; i < ele_nums; ++i) {
      input_grad[i] = grad_flat(i);
      input_var[i] = new_var(i);
      input_m[i] = new_m(i);
      input_v[i] = new_v(i);
    }

    vector<string> out_var, out_var2, out_var3, out_m, out_v, out_alpha;
    // calculate m
    auto fn_calc_m = [&]() {
      Calc_m(out_m, input_m, input_grad, input_beta1, ctx);
    };

    auto fn_calc_v = [&]() {
      Calc_v(out_v, input_v, input_grad, input_beta2, ctx);
    };

    std::thread threads[] = {std::thread(fn_calc_m), std::thread(fn_calc_v)};
    for (int i = 0; i < sizeof(threads) / sizeof(std::thread); i++) {
      threads[i].join();
    }
    //std::thread threads_m = std::thread(fn_calc_m);
    //threads_m.join();
    //std::thread threads_v = std::thread(fn_calc_v);
    //threads_v.join();

    double d_lr = atof(lr_str.c_str());
    double d_beta1_power = atof(beta1_power_str.c_str());
    double d_beta2_power = atof(beta2_power_str.c_str());
    double d_alpha = (d_lr * sqrt(1 - d_beta2_power)) / (1 - d_beta1_power);
    string str_alpha = strings::Printf("%f", d_alpha);
    out_alpha.resize(ele_nums, str_alpha);
    

    // calculate var
    Calc_var(out_var, input_var, out_alpha, out_m, out_v, input_epsilon, ctx);

    // TODO[GeorgeShi]: why we cannot and need not use this?? due to string?
    //new_var.setZero();

    for (int i = 0; i < ele_nums; ++i) {
      new_var(i) = out_var[i];
      new_m(i) = out_m[i];
      new_v(i) = out_v[i];
    }

    if (ctx->input_dtype(0) != DT_RESOURCE) {
      ctx->forward_ref_input_to_ref_output(0, 0);
    }
  }

private:
  void Calc_m(vector<string>& m, const vector<string>& prev_m, const vector<string>& g, const vector<string>& beta1, OpKernelContext* ctx) {
    vector<string> res(g.size());

    string msg_id_str(msg_id().str() + "calc_m");
    msg_id_t msg_id(msg_id_str);
    attr_type attrs;

    attrs["lh_is_const"] = "0";
    attrs["rh_is_const"] = "0";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Sub);
    ProtocolManager::Instance()
      ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(ctx->device()->attributes().incarnation()))
      ->GetOps(msg_id)
      ->Sub(prev_m, g, res, &attrs);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Sub);

    attrs["lh_is_const"] = "1";
    attrs["rh_is_const"] = "0";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Mul);
    ProtocolManager::Instance()
      ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(ctx->device()->attributes().incarnation()))
      ->GetOps(msg_id)
      ->Mul(beta1, res, res, &attrs);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Mul);
    
    attrs["lh_is_const"] = "0";
    attrs["rh_is_const"] = "0";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Add);
    ProtocolManager::Instance()
      ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(ctx->device()->attributes().incarnation()))
      ->GetOps(msg_id)
      ->Add(g, res, m, &attrs);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Add);
    SECURE_OP_REVEAL(m, 1, m.size(), msg_id, "reveal m");
  }

 void Calc_v(vector<string>& v, const vector<string>& prev_v, const vector<string>& g, const vector<string>& beta2, OpKernelContext* ctx) {
    vector<string> res(g.size()), res2(g.size()), g_square(g.size());

    string msg_id_str(msg_id().str() + "calc_v");
    msg_id_t msg_id(msg_id_str);
    attr_type attrs;

    attrs["lh_is_const"] = "0";
    attrs["rh_is_const"] = "0";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Square);
    ProtocolManager::Instance()
      ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(ctx->device()->attributes().incarnation()))
      ->GetOps(msg_id)
      ->Square(g, g_square);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Square);

    attrs["lh_is_const"] = "0";
    attrs["rh_is_const"] = "0";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Sub);
    ProtocolManager::Instance()
      ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(ctx->device()->attributes().incarnation()))
      ->GetOps(msg_id)
      ->Sub(prev_v, g_square, res, &attrs);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Sub);

    attrs["lh_is_const"] = "1";
    attrs["rh_is_const"] = "0";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Mul);
    ProtocolManager::Instance()
      ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(ctx->device()->attributes().incarnation()))
      ->GetOps(msg_id)
      ->Mul(beta2, res, res2, &attrs);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Mul);
    
    attrs["lh_is_const"] = "0";
    attrs["rh_is_const"] = "0";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Add);
    ProtocolManager::Instance()
      ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(ctx->device()->attributes().incarnation()))
      ->GetOps(msg_id)
      ->Add(g_square, res2, v, &attrs);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Add);
    SECURE_OP_REVEAL(v, 1, v.size(), msg_id, "reveal v");
  }

 void Calc_var(vector<string>& var, const vector<string>& prev_var, const vector<string>& alpha, const vector<string>& m, const vector<string>& v, const vector<string>& epsilon, OpKernelContext* ctx) {
    assert(prev_var.size() == alpha.size());
    assert(prev_var.size() == m.size());
    assert(prev_var.size() == v.size());
    size_t size = prev_var.size();
    vector<string> res1(size), res2(size), res3(size);
    string msg_id_str(msg_id().str() + "calc_var");
    msg_id_t msg_id(msg_id_str);
    attr_type attrs;
#if 0
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Sqrt);
    ProtocolManager::Instance()
      ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(ctx->device()->attributes().incarnation()))
      ->GetOps(msg_id)
      ->Sqrt(v, res1, &attrs);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Sqrt);
    
    attrs["lh_is_const"] = "0";
    attrs["rh_is_const"] = "1";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Add);
    ProtocolManager::Instance()
      ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(ctx->device()->attributes().incarnation()))
      ->GetOps(msg_id)
      ->Add(res1, epsilon, res2, &attrs);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Add);
    
    attrs["lh_is_const"] = "1";
    attrs["rh_is_const"] = "0";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Mul);
    ProtocolManager::Instance()
      ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(ctx->device()->attributes().incarnation()))
      ->GetOps(msg_id)
      ->Mul(alpha, m, res1, &attrs);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Mul);

    attrs["lh_is_const"] = "0";
    attrs["rh_is_const"] = "0";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Reciprocaldiv);
    ProtocolManager::Instance()
      ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(ctx->device()->attributes().incarnation()))
      ->GetOps(msg_id)
      ->Reciprocaldiv(res1, res2, res3, &attrs);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Reciprocaldiv);
#else
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Rsqrt);
    ProtocolManager::Instance()
      ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(ctx->device()->attributes().incarnation()))
      ->GetOps(msg_id)
      ->Rsqrt(v, res1, &attrs);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Rsqrt);
    
    attrs["lh_is_const"] = "1";
    attrs["rh_is_const"] = "0";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Mul);
    ProtocolManager::Instance()
      ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(ctx->device()->attributes().incarnation()))
      ->GetOps(msg_id)
      ->Mul(alpha, m, res2, &attrs);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Mul);

    attrs["lh_is_const"] = "0";
    attrs["rh_is_const"] = "0";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Mul);
    ProtocolManager::Instance()
      ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(ctx->device()->attributes().incarnation()))
      ->GetOps(msg_id)
      ->Mul(res1, res2, res3, &attrs);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Mul);
#endif
    
    attrs["lh_is_const"] = "0";
    attrs["rh_is_const"] = "0";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Sub);
    ProtocolManager::Instance()
      ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(ctx->device()->attributes().incarnation()))
      ->GetOps(msg_id)
      ->Sub(prev_var, res3, var, &attrs);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Sub);
    SECURE_OP_REVEAL(var, 1, var.size(), msg_id, "reveal var");
 }

private:
  bool use_exclusive_lock_;
  bool use_nesterov_;
};



REGISTER_KERNEL_BUILDER(
  Name("SecureApplyGradientDescent").Device(DEVICE_CPU).TypeConstraint<double>("T"),
  SecureApplyGradientDescentOp<double>);
REGISTER_KERNEL_BUILDER(
  Name("SecureApplyGradientDescent").Device(DEVICE_CPU).TypeConstraint<int>("T"),
  SecureApplyGradientDescentOp<int>);
REGISTER_KERNEL_BUILDER(
  Name("SecureApplyAdam").Device(DEVICE_CPU).TypeConstraint<double>("T"),
  SecureApplyAdamOp<double>);
REGISTER_KERNEL_BUILDER(
  Name("SecureApplyAdam").Device(DEVICE_CPU).TypeConstraint<int>("T"),
  SecureApplyAdamOp<int>);



/// we do not support resourceAGD for now
// REGISTER_KERNEL_BUILDER(
//   Name("RttApplyGradientDescent").Device(DEVICE_CPU).HostMemory("var").TypeConstraint<float>("T"), cls<Eigen::ThreadPoolDevice, type>);

} // namespace tensorflow

