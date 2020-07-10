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
#include "cc/modules/common/include/utils/logger.h"
#include "cc/modules/protocol/public/protocol_manager.h"
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
    log_debug << "input dtype: " << dtype << endl;
  }

  void Compute(OpKernelContext* context) override {
    log_debug << "tf_to_secure OpKernel compute ..." << endl;
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
    ProtocolManager::Instance()->GetProtocol()->GetOps(msg_id().str())->TfToSecure(inputs, outputs, &attrs_);

    log_debug << "tf_to_rtt out:" << endl;
    for (int i = 0; i < input_flat.size(); ++i) {
      log_debug << outputs[i] << ", ";
      output_flat(i) = outputs[i];
    }

    log_debug << "\ntf_to_secure ok." << endl;
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
    log_debug << "input dtype: " << dtype << endl;
  }

  void Compute(OpKernelContext* context) override {
    log_debug << "tf_to_secure OpKernel compute, string input, if not suffix with R, it is constant input ..." << endl;
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
    ProtocolManager::Instance()->GetProtocol()->GetOps(msg_id().str())->TfToSecure(inputs, outputs, &attrs_);

    for (int i = 0; i < input_flat.size(); ++i) {
      output_flat(i) = outputs[i];
    }

    log_debug << "\ntf_to_secure ok." << endl;
  }

 private:
  DataType dtype;
};

static constexpr char kErrorMessage[] = "SecureToTfOp could not correctly convert string: ";

template <typename T>
class SecureToTfOp : public SecureOpKernel {
 public:
  explicit SecureToTfOp(OpKernelConstruction* context) : SecureOpKernel(context) {}

  void Compute(OpKernelContext* context) override {
    log_debug << "--> SecureToTfOp OpKernel compute.";

    const Tensor* input_tensor;
    OP_REQUIRES_OK(context, context->input("input", &input_tensor));
    const auto& input_flat = input_tensor->flat<string>();

    Tensor* output_tensor = nullptr;
    OP_REQUIRES_OK(context, context->allocate_output("output", input_tensor->shape(), &output_tensor));
    auto output_flat = output_tensor->flat<T>();

    vector<string> inputs(input_flat.size());
    for (int i = 0; i < input_flat.size(); ++i) {
      inputs[i] = input_flat(i);
    }

    vector<string> outputs(input_flat.size());
    // convert from protocol type hex string to native double string
    ProtocolManager::Instance()->GetProtocol()->GetOps(msg_id().str())->SecureToTf(inputs, outputs);

    for (int i = 0; i < input_flat.size(); ++i) {
      output_flat(i) = static_cast<T>(rosetta::convert::from_hex_str<double>(outputs[i]));
    }

    log_debug << "SecureToTf OpKernel compute ok. <--";
  }
};

template <>
class SecureToTfOp<string> : public SecureOpKernel {
 public:
  explicit SecureToTfOp(OpKernelConstruction* context) : SecureOpKernel(context) {}

  void Compute(OpKernelContext* context) override {
    log_debug << "--> SecureToTfOp OpKernel compute. string input";

    const Tensor* input_tensor;
    OP_REQUIRES_OK(context, context->input("input", &input_tensor));
    const auto& input_flat = input_tensor->flat<string>();

    const DataType& dtype = input_tensor->dtype();
    
    Tensor* output_tensor = nullptr;
    OP_REQUIRES_OK(context, context->allocate_output("output", input_tensor->shape(), &output_tensor));
    auto output_flat = output_tensor->flat<string>();

    vector<string> inputs(input_flat.size());
    for (int i = 0; i < input_flat.size(); ++i) {
      inputs[i] = input_flat(i);
    }

    vector<string> outputs(input_flat.size());
    // convert from protocol type hex string to native double string
    ProtocolManager::Instance()->GetProtocol()->GetOps(msg_id().str())->SecureToTf(inputs, outputs);

    for (int i = 0; i < input_flat.size(); ++i) {
      output_flat(i) = std::to_string(rosetta::convert::from_hex_str<double>(outputs[i]));
    }

    log_debug << "SecureToTf OpKernel compute ok. <--";
  }
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

REGISTER_KERNEL_BUILDER(
  Name("TfToSecure").Device(DEVICE_CPU).TypeConstraint<string>("dtype"), TfToSecureOp<string>);
REGISTER_KERNEL_BUILDER(
  Name("TfToSecure").Device(DEVICE_CPU).TypeConstraint<int32>("dtype"), TfToSecureOp<int32>);
REGISTER_KERNEL_BUILDER(
  Name("TfToSecure").Device(DEVICE_CPU).TypeConstraint<int64>("dtype"), TfToSecureOp<int64>);
REGISTER_KERNEL_BUILDER(
  Name("TfToSecure").Device(DEVICE_CPU).TypeConstraint<double>("dtype"), TfToSecureOp<double>);

// REGISTER_KERNEL_BUILDER(
//   Name("SecureToTf").Device(DEVICE_CPU).TypeConstraint<string>("dtype"), SecureToTfOp<string>);
// REGISTER_KERNEL_BUILDER(
//   Name("SecureToTf").Device(DEVICE_CPU).TypeConstraint<int32>("dtype"), SecureToTfOp<int32>);

}