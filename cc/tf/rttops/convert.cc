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

using namespace std;
using namespace tensorflow;

/////////////////////////// FOR TfToRttOp and  RttToTfOp
#include <errno.h>
#include <string>
#include "tensorflow/core/framework/kernel_def_builder.h"
#include "tensorflow/core/framework/op_kernel.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/lib/core/errors.h"
#include "tensorflow/core/lib/core/status.h"
#include "tensorflow/core/lib/strings/numbers.h"

#include "tensorflow/core/lib/strings/stringprintf.h"

template <typename T>
class TfToRttOp : public OpKernel {
 public:
  explicit TfToRttOp(OpKernelConstruction* ctx) : OpKernel(ctx) {
    /// to debug. For now we keep 10 decimals for float numbers
    int32 precision = -1;
    bool scientific = false;
    bool shortest = false;
    int32 width = -1;
    string fill_string = "";
    DataType dtype;
    OP_REQUIRES_OK(ctx, ctx->GetAttr("dtype", &dtype));
    /// Note: shortcut string!
    if (dtype == DT_STRING) {
      return;
    }

    switch (dtype) {
      case DT_FLOAT:
      case DT_DOUBLE:
      case DT_COMPLEX64:
      case DT_COMPLEX128:
        break;
      default:
        OP_REQUIRES(
          ctx, !(scientific || shortest),
          errors::InvalidArgument(
            "scientific and shortest format "
            "not supported for datatype ",
            DataTypeString(dtype)));
        OP_REQUIRES(
          ctx, precision < 0,
          errors::InvalidArgument(
            "precision not supported "
            "for datatype ",
            DataTypeString(dtype)));
    }
    OP_REQUIRES(
      ctx, fill_string.size() <= 1,
      errors::InvalidArgument("Fill string must be one or fewer characters"));
    OP_REQUIRES(
      ctx, !(scientific && shortest),
      errors::InvalidArgument("Cannot select both scientific and shortest notation"));
    format_ = "%";
    if (width > -1) {
      strings::Appendf(&format_, "%s%d", fill_string.c_str(), width);
    }
    if (precision > -1) {
      strings::Appendf(&format_, ".%d", precision);
    }
    switch (dtype) {
      case DT_INT8:
      case DT_INT16:
      case DT_INT32:
        strings::Appendf(&format_, "d");
        break;
      case DT_INT64:
        strings::Appendf(&format_, "lld");
        break;
      case DT_FLOAT:
      case DT_DOUBLE:
      case DT_COMPLEX64:
      case DT_COMPLEX128:
        if (shortest) {
          strings::Appendf(&format_, "g");
        } else if (scientific) {
          strings::Appendf(&format_, "e");
        } else {
          strings::Appendf(&format_, "f");
        }
        break;
      case DT_BOOL:
        break;
      default:
        bool type_not_supported = true;
        OP_REQUIRES(
          ctx, !type_not_supported,
          errors::InvalidArgument("Type not supported: ", DataTypeString(dtype)));
    }

    if (dtype == DT_COMPLEX64 || dtype == DT_COMPLEX128) {
      format_ = strings::Printf("(%s,%s)", format_.c_str(), format_.c_str());
    }
  }

  void Compute(OpKernelContext* context) override {
    // GeorgeShi: this is just for prototyping the feasibility
    //            Actually, this is just like a simplified 'AsString' OP in native TF

    const Tensor* input_tensor;
    OP_REQUIRES_OK(context, context->input("input", &input_tensor));
    const DataType& dtype = input_tensor->dtype();

    Tensor* output_tensor = nullptr;
    OP_REQUIRES_OK(
      context, context->allocate_output("output", input_tensor->shape(), &output_tensor));
    auto output_flat = output_tensor->flat<string>();

#define ENCODE_TYPE(type, T, enc_str)                                     \
  case (type): {                                                          \
    const auto& input_flat = input_tensor->flat<T>();                     \
    for (int i = 0; i < input_flat.size(); ++i) {                         \
      output_flat(i) = strings::Printf((enc_str.c_str()), input_flat(i)); \
    }                                                                     \
  } break

    /// short-cut string!!
    if (dtype == DT_STRING) {
      const auto& input_flat = input_tensor->flat<string>();
      for (int i = 0; i < input_flat.size(); ++i) {
        output_flat(i) = input_flat(i);
        //cout << "convert:" << input_flat(i) << " to " << output_flat(i) << endl;
      }
      return;
    }

    switch (dtype) {
      ENCODE_TYPE(DT_INT32, int32, format_);
      ENCODE_TYPE(DT_INT64, int64, format_);
      ENCODE_TYPE(DT_FLOAT, float, format_);
      ENCODE_TYPE(DT_DOUBLE, double, format_);
      ENCODE_TYPE(DT_INT8, int8, format_);
      ENCODE_TYPE(DT_INT16, int16, format_);
      case (DT_BOOL): {
        const auto& input_flat = input_tensor->flat<bool>();
        for (int i = 0; i < input_flat.size(); ++i) {
          output_flat(i) = (input_flat(i)) ? "true" : "false";
        }
      } break;
      case (DT_COMPLEX64): {
        const auto& input_flat = input_tensor->flat<complex64>();
        for (int i = 0; i < input_flat.size(); ++i) {
          output_flat(i) =
            strings::Printf(format_.c_str(), input_flat(i).real(), input_flat(i).imag());
        }
      } break;
      case (DT_COMPLEX128): {
        const auto& input_flat = input_tensor->flat<complex128>();
        for (int i = 0; i < input_flat.size(); ++i) {
          output_flat(i) =
            strings::Printf(format_.c_str(), input_flat(i).real(), input_flat(i).imag());
        }
      } break;
      default:
        bool can_encode_type = false;
        OP_REQUIRES(
          context, can_encode_type,
          errors::InvalidArgument("Cannot encode input of type ", DataTypeString(dtype)));
    }
#undef ENCODE_TYPE
  }

 private:
  string format_;
};

static constexpr char kErrorMessage[] = "RttToTfOp could not correctly convert string: ";

template <typename T>
class RttToTfOp : public OpKernel {
 public:
  explicit RttToTfOp(OpKernelConstruction* context) : OpKernel(context) {}

  void Compute(OpKernelContext* context) override {
    // GeorgeShi: this is just for prototyping the feasibility
    //            Actually, this is just like a simplified 'StringToNumber' OP in native TF

    ///// Note[GeorgeShi: modified from 'StringToNumber' OP in native TF]
    // This is not a deep copy of the input tensor; they will share the same
    // underlying storage.
    const Tensor* input_tensor;
    OP_REQUIRES_OK(context, context->input("val", &input_tensor));
    const auto& input_flat = input_tensor->flat<string>();

    Tensor* output_tensor = nullptr;
    OP_REQUIRES_OK(context, context->allocate_output("out", input_tensor->shape(), &output_tensor));
    auto output_flat = output_tensor->flat<T>();

    for (int i = 0; i < input_flat.size(); ++i) {
      OP_REQUIRES(
        context, strings::SafeStringToNumeric<T>(input_flat(i), &output_flat(i)),
        errors::InvalidArgument(kErrorMessage, input_flat(i).c_str()));
    }
  }
};

// The native TF does not support string type
template <>
class RttToTfOp<string> : public OpKernel {
 public:
  explicit RttToTfOp(OpKernelConstruction* context) : OpKernel(context) {}

  void Compute(OpKernelContext* context) override {
    // GeorgeShi: this is just for prototyping the feasibility
    //            Actually, this is just like a simplified 'StringToNumber' OP in native TF

    ///// Note[GeorgeShi: modified from 'StringToNumber' OP in native TF]
    // This is not a deep copy of the input tensor; they will share the same
    // underlying storage.
    const Tensor* input_tensor;
    OP_REQUIRES_OK(context, context->input("val", &input_tensor));
    const auto& input_flat = input_tensor->flat<string>();

    Tensor* output_tensor = nullptr;
    OP_REQUIRES_OK(context, context->allocate_output("out", input_tensor->shape(), &output_tensor));
    auto output_flat = output_tensor->flat<string>();

    for (int i = 0; i < input_flat.size(); ++i) {
      output_flat(i) = input_flat(i);
    }
  }
};
// Registers the currently supported output types.
#define REGISTER(type)     \
  REGISTER_KERNEL_BUILDER( \
    Name("RttToTf").Device(DEVICE_CPU).TypeConstraint<type>("dtype"), RttToTfOp<type>)
REGISTER(float);
REGISTER(double);
REGISTER(int32);
REGISTER(int64);
REGISTER(string);
#undef REGISTER

REGISTER_KERNEL_BUILDER(
  Name("TfToRtt").Device(DEVICE_CPU).TypeConstraint<string>("dtype"), TfToRttOp<string>);
REGISTER_KERNEL_BUILDER(
  Name("TfToRtt").Device(DEVICE_CPU).TypeConstraint<int32>("dtype"), TfToRttOp<int32>);
REGISTER_KERNEL_BUILDER(
  Name("TfToRtt").Device(DEVICE_CPU).TypeConstraint<int64>("dtype"), TfToRttOp<int64>);
REGISTER_KERNEL_BUILDER(
  Name("TfToRtt").Device(DEVICE_CPU).TypeConstraint<double>("dtype"), TfToRttOp<double>);

// REGISTER_KERNEL_BUILDER(
//   Name("RttToTf").Device(DEVICE_CPU).TypeConstraint<string>("dtype"), RttToTfOp<string>);
// REGISTER_KERNEL_BUILDER(
//   Name("RttToTf").Device(DEVICE_CPU).TypeConstraint<int32>("dtype"), RttToTfOp<int32>);

