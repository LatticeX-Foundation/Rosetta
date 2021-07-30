#include "tensorflow/core/framework/op.h"
#include "tensorflow/core/framework/op_kernel.h"
#include "tensorflow/core/framework/shape_inference.h"
#include "tensorflow/core/lib/strings/stringprintf.h"
#include "cc/tf/secureops/secure_base_kernel.h"
#include <stdexcept>

#include <iostream>

using namespace std;
using namespace tensorflow;


namespace tensorflow {

class SecureAssignOp : public SecureOpKernel {
 public:
  explicit SecureAssignOp(OpKernelConstruction* ctx) : SecureOpKernel(ctx) {
    OP_REQUIRES_OK(ctx, ctx->GetAttr("use_locking", &use_locking_));
  }

  void ComputeImpl(OpKernelContext* ctx) {
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

class SecureAssignSubOp : public SecureOpKernel {
 public:
  explicit SecureAssignSubOp(OpKernelConstruction* ctx) : SecureOpKernel(ctx) {
    OP_REQUIRES_OK(ctx, ctx->GetAttr("use_locking", &use_locking_));
  }

  void ComputeImpl(OpKernelContext* ctx) {
    log_debug << "--> SecureAssignSubOp OpKernel compute.";
    Tensor var = ctx->mutable_input(0, use_locking_);
    const Tensor& value = ctx->input(1);
    auto var_flat = var.flat<string>();
    auto value_flat = value.flat<string>();

    auto ele_nums = value.NumElements();
    vector<string> in1(ele_nums), in2(ele_nums);
    for (int i = 0; i < ele_nums; ++i) {
      in1[i] = var_flat(i);
      in2[i] = value_flat(i);
    }

    // secure protocol calling
    vector<string> output;
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Sub);
    int ret = ProtocolManager::Instance()
                ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(ctx->device()->attributes().incarnation()))
                ->GetOps(msg_id())
                ->Sub(in1, in2, output, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Sub);
    if (ret != 0) {
      log_error << "call Secure Sub failed, ret: " << ret;
      throw std::runtime_error(string("Secure Sub runtime error!"));
    }

    if (ctx->input_dtype(0) != DT_RESOURCE) {
      ctx->forward_ref_input_to_ref_output(0, 0);
    }

    // store output
    for (int i = 0; i < ele_nums; ++i) {
      var_flat(i) = output[i];
    }
    log_debug << "SecureAssignSubOp OpKernel compute ok. <--";
  }

 private:
  bool use_locking_ = true;
};

REGISTER_STR_CPU_KERNEL(SecureAssign, SecureAssignOp);
REGISTER_STR_CPU_KERNEL(SecureAssignSub, SecureAssignSubOp);

} // namespace tensorflow
