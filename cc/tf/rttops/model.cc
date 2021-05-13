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
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/lib/core/errors.h"
#include "tensorflow/core/lib/core/status.h"
#include "tensorflow/core/lib/strings/numbers.h"

#include "tensorflow/core/lib/strings/stringprintf.h"
#include "tensorflow/core/util/tensor_bundle/tensor_bundle.h"

#include "tensorflow/core/framework/bounds_check.h"
#include "tensorflow/core/util/saved_tensor_slice_util.h"
#include "tensorflow/core/util/tensor_slice_reader.h"
#include "tensorflow/core/util/tensor_slice_writer.h"


namespace {
// Shared validations of the inputs to the SaveV2 and RestoreV2 ops.
void ValidateInputs(bool is_save_op, OpKernelContext* context,
                    const Tensor& prefix, const Tensor& tensor_names,
                    const Tensor& shape_and_slices) {
  const int kFixedInputs = 3;  // Prefix, tensor names, shape_and_slices.
  const int num_tensors = static_cast<int>(tensor_names.NumElements());
  OP_REQUIRES(
      context, prefix.NumElements() == 1,
      errors::InvalidArgument("Input prefix should have a single element, got ",
                              prefix.NumElements(), " instead."));
  OP_REQUIRES(context,
              TensorShapeUtils::IsVector(tensor_names.shape()) &&
                  TensorShapeUtils::IsVector(shape_and_slices.shape()),
              errors::InvalidArgument(
                  "Input tensor_names and shape_and_slices "
                  "should be an 1-D tensors, got ",
                  tensor_names.shape().DebugString(), " and ",
                  shape_and_slices.shape().DebugString(), " instead."));
  OP_REQUIRES(context,
              tensor_names.NumElements() == shape_and_slices.NumElements(),
              errors::InvalidArgument("tensor_names and shape_and_slices "
                                      "have different number of elements: ",
                                      tensor_names.NumElements(), " vs. ",
                                      shape_and_slices.NumElements()));
  OP_REQUIRES(context,
              FastBoundsCheck(tensor_names.NumElements() + kFixedInputs,
                              std::numeric_limits<int>::max()),
              errors::InvalidArgument("Too many inputs to the op"));
  OP_REQUIRES(
      context, shape_and_slices.NumElements() == num_tensors,
      errors::InvalidArgument("Expected ", num_tensors,
                              " elements in shapes_and_slices, but got ",
                              context->input(2).NumElements()));
  if (is_save_op) {
    OP_REQUIRES(context, context->num_inputs() == num_tensors + kFixedInputs,
                errors::InvalidArgument(
                    "Got ", num_tensors, " tensor names but ",
                    context->num_inputs() - kFixedInputs, " tensors."));
    OP_REQUIRES(context, context->num_inputs() == num_tensors + kFixedInputs,
                errors::InvalidArgument(
                    "Expected a total of ", num_tensors + kFixedInputs,
                    " inputs as input #1 (which is a string "
                    "tensor of saved names) contains ",
                    num_tensors, " names, but received ", context->num_inputs(),
                    " inputs"));
  }
}

} // end of namespace

static constexpr char kErrorMessage[] = "RttSaveV2Op could not correctly convert string: ";

// Saves a list of named tensors using the tensor bundle library.
//template <typename Device>
class RttSaveV2Op : public OpKernel{ 
 public:
  explicit RttSaveV2Op(OpKernelConstruction* context) : OpKernel(context) {}

  void Compute(OpKernelContext* context) override {
    const Tensor& prefix = context->input(0);
    const Tensor& tensor_names = context->input(1);
    const Tensor& shape_and_slices = context->input(2);
    ValidateInputs(true /* is save op */, context, prefix, tensor_names,
                   shape_and_slices);


    const int kFixedInputs = 3;  // Prefix, tensor names, shape_and_slices.
    const int num_tensors = static_cast<int>(tensor_names.NumElements());
    const string& prefix_string = prefix.scalar<string>()();
    const auto& tensor_names_flat = tensor_names.flat<string>();
    const auto& shape_and_slices_flat = shape_and_slices.flat<string>();

    BundleWriter writer(Env::Default(), prefix_string);
    OP_REQUIRES_OK(context, writer.status());
    cout << "DEBUG RTTsaveV2!:" <<endl;

    for (int i = 0; i < num_tensors; ++i) {
      const string& tensor_name = tensor_names_flat(i);
      const Tensor& curr_tensor = context->input(i + kFixedInputs);
      DataType curr_type = curr_tensor.dtype();
      auto ele_nums = curr_tensor.NumElements();

      // in this demo, ALL the input tensors should be as string type
      OP_REQUIRES(context, 
                    curr_type == DT_STRING,
                    errors::InvalidArgument(
                      "The ", i, "-th tensor is not in DT_STRING type!",
                      curr_tensor.DebugString(5)));
      auto const curr_flat = curr_tensor.flat<string>();

    // in this demo, we will concvert the string to 'float64' plaintext.
      Tensor out_tensor;
      OP_REQUIRES_OK(context, context->allocate_temp(DT_DOUBLE, 
                          curr_tensor.shape(), &out_tensor));
      
      auto out_tensor_double = out_tensor.flat<double>();
      out_tensor_double.setZero();

      vector<double> out_flat_vec(ele_nums);
      cout << "DEBUG: convert string to float64!" <<endl;
      for(int j = 0; j < ele_nums; ++j) {
        OP_REQUIRES(
        context, strings::SafeStringToNumeric<double>(curr_flat(j), &out_tensor_double(j)),
        errors::InvalidArgument(kErrorMessage, curr_flat(i).c_str()));
        cout << j << "-th: save " << curr_flat(j) << " to " << out_tensor_double(j) << endl; 
      }

     // [TODO]: in this version, we should validate that the user does not config this input.
      if (!shape_and_slices_flat(i).empty()) {
        const string& shape_spec = shape_and_slices_flat(i);
        TensorShape shape;
        TensorSlice slice(curr_tensor.dims());
        TensorShape slice_shape;
        OP_REQUIRES_OK(context, checkpoint::ParseShapeAndSlice(
                                    shape_spec, &shape, &slice, &slice_shape));
        OP_REQUIRES(context, slice_shape.IsSameSize(curr_tensor.shape()),
                    errors::InvalidArgument("Slice in shape_and_slice "
                                            "specification does not match the "
                                            "shape of the tensor to  save: ",
                                            shape_spec, ", tensor: ",
                                            curr_tensor.shape().DebugString()));

        OP_REQUIRES_OK(context,
                       writer.AddSlice(tensor_name, shape, slice, curr_tensor));
      } else {
        // TODO: add the feature to config which party can save the plain value.
        OP_REQUIRES_OK(context, writer.Add(tensor_name, out_tensor));
      }
    }
    OP_REQUIRES_OK(context, writer.Finish());
  }
};

class RttRestoreV2Op : public OpKernel{ 
 public:
  explicit RttRestoreV2Op(OpKernelConstruction* context) : OpKernel(context) {}

  void Compute(OpKernelContext* context) override {
  }
};

REGISTER_KERNEL_BUILDER(Name("RttSaveV2").Device(DEVICE_CPU), RttSaveV2Op);
REGISTER_KERNEL_BUILDER(Name("RttRestoreV2").Device(DEVICE_CPU), RttRestoreV2Op);