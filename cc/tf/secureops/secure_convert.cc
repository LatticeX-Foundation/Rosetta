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
#include "tensorflow/core/framework/op.h"
#include "tensorflow/core/framework/op_kernel.h"
#include "tensorflow/core/framework/shape_inference.h"
#include <iostream>
#include "cc/modules/common/include/utils/str_type_convert.h"

using namespace std;
using namespace tensorflow;

/////////////////////////// FOR TfToSecureOp and  SecureToTfOp
#include <errno.h>
#include <string>
#include "tensorflow/core/framework/kernel_def_builder.h"
#include "tensorflow/core/framework/op_kernel.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/lib/core/errors.h"
#include "tensorflow/core/lib/core/status.h"
#include "tensorflow/core/lib/strings/numbers.h"

#include "tensorflow/core/lib/strings/stringprintf.h"
#include "cc/modules/common/include/utils/rtt_logger.h"
#include "cc/modules/protocol/public/include/protocol_manager.h"
#include "cc/modules/protocol/mpc/comm/include/mpc_helper.h"
#include "cc/tf/secureops/secure_base_kernel.h"

using rosetta::ProtocolManager;

namespace tensorflow {

template <typename T>
class TfToSecureOp : public SecureOpKernel {
 public:
  explicit TfToSecureOp(OpKernelConstruction* ctx) : SecureOpKernel(ctx) {
    /// to debug. For now we keep 10 decimals for float numbers
    OP_REQUIRES_OK(ctx, ctx->GetAttr("dtype", &dtype));
    /// Note: shortcut string!
    if (dtype == DT_STRING) {
      return;
    }
    log_debug << "input dtype: " << dtype ;
  }

  void ComputeImpl(OpKernelContext* context) {
    log_debug << "tf_to_secure OpKernel compute ..." ;
    const Tensor* input_tensor;
    OP_REQUIRES_OK(context, context->input("input", &input_tensor));
    const DataType& dtype = input_tensor->dtype();

    Tensor* output_tensor = nullptr;
    OP_REQUIRES_OK(
      context, context->allocate_output("output", input_tensor->shape(), &output_tensor));
    auto output_flat = output_tensor->flat<string>();

    const auto& input_flat = input_tensor->flat<T>();
    vector<string> inputs(input_flat.size());
    for (int i = 0; i < input_flat.size(); ++i) {
      inputs[i] = std::to_string(input_flat(i));
    }

    // convert to protocol type hex string
    vector<string> outputs(input_flat.size());
    // string isconst(1, 0);
    // attrs_["is_const"] = isconst;
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(TfToSecure);
    ProtocolManager::Instance()
      ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->TfToSecure(inputs, outputs, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(TfToSecure);

    log_debug << "tf_to_rtt out:" ;
    for (int i = 0; i < input_flat.size(); ++i) {
      log_debug << outputs[i] << ", ";
      output_flat(i) = outputs[i];
    }

    log_debug << "\ntf_to_secure ok." ;
  }

 private:
  DataType dtype;
};

template <>
class TfToSecureOp<string> : public SecureOpKernel {
 public:
  explicit TfToSecureOp(OpKernelConstruction* ctx) : SecureOpKernel(ctx) {
    /// to debug. For now we keep 10 decimals for float numbers
    OP_REQUIRES_OK(ctx, ctx->GetAttr("dtype", &dtype));
    /// Note: shortcut string!
    if (dtype == DT_STRING) {
      return;
    }
    log_debug << "input dtype: " << dtype ;
  }

  void ComputeImpl(OpKernelContext* context) {
    log_debug
      << "tf_to_secure OpKernel compute, string input, if not suffix with R, it is constant input ..."
      ;
    const Tensor* input_tensor;
    OP_REQUIRES_OK(context, context->input("input", &input_tensor));
    const DataType& dtype = input_tensor->dtype();

    Tensor* output_tensor = nullptr;
    OP_REQUIRES_OK(
      context, context->allocate_output("output", input_tensor->shape(), &output_tensor));
    auto output_flat = output_tensor->flat<string>();

    const auto& input_flat = input_tensor->flat<string>();
    vector<string> inputs(input_flat.size());
    for (int i = 0; i < input_flat.size(); ++i) {
      inputs[i] = input_flat(i);
    }

    // convert to protocol type hex string
    vector<string> outputs(input_flat.size());
    // string isconst(1, 1);
    // attrs_["is_const"] = isconst;
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(TfToSecure);
    ProtocolManager::Instance()
      ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->TfToSecure(inputs, outputs, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(TfToSecure);

    for (int i = 0; i < input_flat.size(); ++i) {
      output_flat(i) = outputs[i];
    }

    log_debug << "\ntf_to_secure ok." ;
  }

 private:
  DataType dtype;
};

static constexpr char kErrorMessage[] = "SecureToTfOp could not correctly convert string: ";

template <typename T>
class SecureToTfOp : public SecureOpKernel {
 public:
  explicit SecureToTfOp(OpKernelConstruction* context) : SecureOpKernel(context) {}

  void ComputeImpl(OpKernelContext* context) {
    log_debug << "--> SecureToTfOp OpKernel compute.";

    const Tensor* input_tensor;
    OP_REQUIRES_OK(context, context->input("input", &input_tensor));
    const auto& input_flat = input_tensor->flat<string>();

    Tensor* output_tensor = nullptr;
    OP_REQUIRES_OK(
      context, context->allocate_output("output", input_tensor->shape(), &output_tensor));
    auto output_flat = output_tensor->flat<T>();

    vector<string> inputs(input_flat.size());
    for (int i = 0; i < input_flat.size(); ++i) {
      inputs[i] = input_flat(i);
    }

    vector<string> outputs(input_flat.size());
    // convert from protocol type hex string to native double string
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(SecureToTf);
    ProtocolManager::Instance()
      ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->SecureToTf(inputs, outputs);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(SecureToTf);
    for (int i = 0; i < input_flat.size(); ++i) {
      output_flat(i) = static_cast<T>(rosetta::convert::from_binary_str<double>(outputs[i]));
    }

    log_debug << "SecureToTf OpKernel compute ok. <--";
  }
};

template <>
class SecureToTfOp<string> : public SecureOpKernel {
 public:
  explicit SecureToTfOp(OpKernelConstruction* context) : SecureOpKernel(context) {}

  void ComputeImpl(OpKernelContext* context) {
    log_debug << "--> SecureToTfOp OpKernel compute. string input";

    const Tensor* input_tensor;
    OP_REQUIRES_OK(context, context->input("input", &input_tensor));
    const auto& input_flat = input_tensor->flat<string>();

    const DataType& dtype = input_tensor->dtype();

    Tensor* output_tensor = nullptr;
    OP_REQUIRES_OK(
      context, context->allocate_output("output", input_tensor->shape(), &output_tensor));
    auto output_flat = output_tensor->flat<string>();

    vector<string> inputs(input_flat.size());
    for (int i = 0; i < input_flat.size(); ++i) {
      inputs[i] = input_flat(i);
    }

    vector<string> outputs(input_flat.size());
    // convert from protocol type hex string to native double string
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(SecureToTf);
    ProtocolManager::Instance()
      ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->SecureToTf(inputs, outputs);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(SecureToTf);

    for (int i = 0; i < input_flat.size(); ++i) {
      output_flat(i) = std::to_string(rosetta::convert::from_binary_str<double>(outputs[i]));
    }

    log_debug << "SecureToTf OpKernel compute ok. <--";
  }
};

template <typename T>
class PrivateInputOp : public SecureOpKernel {
 public:
  explicit PrivateInputOp(OpKernelConstruction* ctx) : SecureOpKernel(ctx) {
    OP_REQUIRES_OK(ctx, ctx->GetAttr("data_owner_", &data_owner_));
    log_debug << "construct private input T op, data_owner_: " << data_owner_ ;
  }

  void ComputeImpl(OpKernelContext* context) {
    log_debug << "private_input OpKernel compute ..." ;
    const Tensor *input_tensor, *data_owner;
    OP_REQUIRES_OK(context, context->input("input", &input_tensor));
    OP_REQUIRES_OK(context, context->input("data_owner", &data_owner));

    Tensor* output_tensor = nullptr;
    OP_REQUIRES_OK(
      context, context->allocate_output("output", input_tensor->shape(), &output_tensor));
    auto output_flat = output_tensor->flat<string>();

    const auto& input_flat = input_tensor->flat<T>();
    vector<double> inputs(input_flat.size());
    for (int i = 0; i < input_flat.size(); ++i) {
      inputs[i] = double(input_flat(i));
    }

    const auto& data_owner_flat = data_owner->flat<string>();
    data_owner_ = data_owner_flat(0);

    string task_id = ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation());
    shared_ptr<NET_IO> netio = ProtocolManager::Instance()->GetProtocol(task_id)->GetNetHandler();
    const vector<string>& party2nodes = netio->GetParty2Node();
    const vector<string>& result_nodes = netio->GetResultNodes();
    vector<string> nodes = decode_reveal_nodes(data_owner_, party2nodes, result_nodes);
    OP_REQUIRES(context, nodes.size() == 1, errors::InvalidArgument("Unsupported node."));
    data_owner_ = nodes[0];

    // PrivateInput input
    vector<string> outputs(input_flat.size());
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(PrivateInput);
    ProtocolManager::Instance()
      ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->PrivateInput(data_owner_, inputs, outputs);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(PrivateInput);

    for (int i = 0; i < input_flat.size(); ++i) {
      output_flat(i) = outputs[i];
    }

    log_debug << "run PrivateInput op ok." ;
  }

 private:
  string data_owner_;
};

template <>
class PrivateInputOp<string> : public SecureOpKernel {
 public:
  explicit PrivateInputOp(OpKernelConstruction* ctx) : SecureOpKernel(ctx) {
    // OP_REQUIRES_OK(ctx, ctx->GetAttr("data_owner_", &data_owner_));
    // log_debug << "construct private input string op, data_owner_: " << data_owner_ ;
  }

  void ComputeImpl(OpKernelContext* context) {
    log_debug << "private_input OpKernel compute ...";
    const Tensor *input_tensor, *data_owner;
    OP_REQUIRES_OK(context, context->input("input", &input_tensor));
    OP_REQUIRES_OK(context, context->input("data_owner", &data_owner));

    Tensor* output_tensor = nullptr;
    OP_REQUIRES_OK(
      context, context->allocate_output("output", input_tensor->shape(), &output_tensor));
    auto output_flat = output_tensor->flat<string>();

    const auto& input_flat = input_tensor->flat<string>();
    vector<double> inputs(input_flat.size());
    for (int i = 0; i < input_flat.size(); ++i) {
      inputs[i] = std::stod(input_flat(i));
    }

    const auto& data_owner_flat = data_owner->flat<string>();
    data_owner_ = data_owner_flat(0);

    string task_id = ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation());
    shared_ptr<NET_IO> netio = ProtocolManager::Instance()->GetProtocol(task_id)->GetNetHandler();
    const vector<string>& party2nodes = netio->GetParty2Node();
    const vector<string>& result_nodes = netio->GetResultNodes();
    vector<string> nodes = decode_reveal_nodes(data_owner_, party2nodes, result_nodes);
    OP_REQUIRES(context, nodes.size() == 1, errors::InvalidArgument("Unsupported node."));
    data_owner_ = nodes[0];

    // PrivateInput input
    vector<string> outputs(input_flat.size());
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(PrivateInput);
    log_info << "private input:" << data_owner_ << "data owner:" << data_owner_ << " size:" << inputs.size();
    ProtocolManager::Instance()
      ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->PrivateInput(data_owner_, inputs, outputs);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(PrivateInput);

    for (int i = 0; i < input_flat.size(); ++i) {
      output_flat(i) = outputs[i];
    }

    log_debug << "run PrivateInput op ok." ;
  }

 private:
  string data_owner_;
};

// Registers the currently supported output types.
#define REGISTER(type)     \
  REGISTER_KERNEL_BUILDER( \
    Name("SecureToTf").Device(DEVICE_CPU).TypeConstraint<type>("dtype"), SecureToTfOp<type>)
REGISTER(float);
REGISTER(double);
REGISTER(int32);
REGISTER(int64);
REGISTER(string);
#undef REGISTER

// tf_to_secure kernel
REGISTER_KERNEL_BUILDER(
  Name("TfToSecure").Device(DEVICE_CPU).TypeConstraint<string>("dtype"),
  TfToSecureOp<string>);
REGISTER_KERNEL_BUILDER(
  Name("TfToSecure").Device(DEVICE_CPU).TypeConstraint<int32>("dtype"),
  TfToSecureOp<int32>);
REGISTER_KERNEL_BUILDER(
  Name("TfToSecure").Device(DEVICE_CPU).TypeConstraint<int64>("dtype"),
  TfToSecureOp<int64>);
REGISTER_KERNEL_BUILDER(
  Name("TfToSecure").Device(DEVICE_CPU).TypeConstraint<double>("dtype"),
  TfToSecureOp<double>);

// private_input kernel
REGISTER_KERNEL_BUILDER(
  Name("PrivateInput").Device(DEVICE_CPU).TypeConstraint<string>("dtype"),
  PrivateInputOp<string>);
REGISTER_KERNEL_BUILDER(
  Name("PrivateInput").Device(DEVICE_CPU).TypeConstraint<int32>("dtype"),
  PrivateInputOp<int32>);
REGISTER_KERNEL_BUILDER(
  Name("PrivateInput").Device(DEVICE_CPU).TypeConstraint<int64>("dtype"),
  PrivateInputOp<int64>);
REGISTER_KERNEL_BUILDER(
  Name("PrivateInput").Device(DEVICE_CPU).TypeConstraint<double>("dtype"),
  PrivateInputOp<double>);

} // namespace tensorflow
