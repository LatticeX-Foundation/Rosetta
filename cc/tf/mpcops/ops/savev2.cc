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
// this is a derived file based on 
// tensorflow/tensorflow/core/kernels/save_restore_v2_ops.cc.
// Thanks to the Tensorflow team.

#include "mpcop_kernel.h"

#include <string>
#include <vector>

#include "thirdparty/tensorflow/core/kernels/save_restore_tensor.h"
#include "thirdparty/tensorflow/core/util/tensor_bundle/tensor_bundle.h"

#include "tensorflow/core/framework/bounds_check.h"
#include "tensorflow/core/framework/op_kernel.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/framework/types.h"
#include "tensorflow/core/framework/types.pb.h"
//#include "tensorflow/core/kernels/save_restore_tensor.h"
#include "tensorflow/core/lib/core/status.h"
#include "tensorflow/core/lib/io/path.h"
#include "tensorflow/core/platform/env.h"
#include "tensorflow/core/platform/logging.h"
#include "tensorflow/core/platform/types.h"
#include "tensorflow/core/util/saved_tensor_slice_util.h"
//#include "tensorflow/core/util/tensor_bundle/tensor_bundle.h"
#include "tensorflow/core/util/tensor_slice_reader.h"

namespace tensorflow {

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

// Saves a list of named tensors using the tensor bundle library.
//template <typename Device>
class MpcSaveV2Op : public MpcOpKernel{ 
 public:
  explicit MpcSaveV2Op(OpKernelConstruction* context) : MpcOpKernel(context) {}

  void MpcCompute(OpKernelContext* context) override {
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
    // std::cout << "BundleWriter, prefix_string: " << prefix_string << endl;;

    for (int i = 0; i < num_tensors; ++i) {
      const string& tensor_name = tensor_names_flat(i);
      const Tensor& curr_tensor = context->input(i + kFixedInputs);
      DataType curr_type = curr_tensor.dtype();
      auto ele_nums = curr_tensor.NumElements();
      // ALL the input tensors should be as double type
      OP_REQUIRES(context, 
                    curr_type == DT_DOUBLE,
                    errors::InvalidArgument(
                      "The ", i, "-th tensor is not in DT_DOUBLE type!",
                      curr_tensor.DebugString(5)));
      auto const curr_flat = curr_tensor.flat<double>();

      Tensor curr_tensor_plain;
      OP_REQUIRES_OK(context, context->allocate_temp(DT_DOUBLE, 
                          curr_tensor.shape(), &curr_tensor_plain));
      
      auto inner_tensor_plain = curr_tensor_plain.flat<double>();
      inner_tensor_plain.setZero();

      vector<double> curr_flat_vec(ele_nums);
      for(int j = 0; j < ele_nums; ++j) {
        curr_flat_vec[j] = curr_flat(j);
      }

      vector<mpc_t> inner_tensor_cipher(ele_nums, 0);
      //convert_double_to_mytype(curr_flat_vec, inner_tensor_cipher);
      tf_convert_double_to_mpctype(curr_flat_vec, inner_tensor_cipher);
      //=================MPC========================
      vector<mpc_t> inner_tensor_cipher_recons = inner_tensor_cipher;

      // By default, the local shred values are saved in model.
      // Currently, only support it as 3-bit bitmap:[P2 P1 P0]
      //  0: none, all local shared value. 
      //  1: P0,
      //  2: P1,
      //  4: P2,
      //  3: P0 and P1
      //  5: P0 and P2
      //  6: P1 and P2
      //  7: P0, P1 and P2
      if (SAVER_MODE & 1) {
        tfGetMpcOp(Reconstruct2PC)->RunV2(inner_tensor_cipher, ele_nums, inner_tensor_cipher_recons, PARTY_A);
      }
      if (SAVER_MODE & 2) {
        tfGetMpcOp(Reconstruct2PC)->RunV2(inner_tensor_cipher, ele_nums, inner_tensor_cipher_recons, PARTY_B);
      }
      if (SAVER_MODE & 4) {
        tfGetMpcOp(Reconstruct2PC)->RunV2(inner_tensor_cipher, ele_nums, inner_tensor_cipher_recons, PARTY_C);
      }     
      vector<double> curr_plain_flat_vec(ele_nums,0);
      convert_mytype_to_double(inner_tensor_cipher_recons, curr_plain_flat_vec);
      for(int j = 0; j < ele_nums; ++j) {
        inner_tensor_plain(j) = curr_plain_flat_vec[j];
      }

      #if PRINT_REVEAL
      print_vec(curr_flat_vec, ele_nums, "LOCAL raw input values");
      print_vec(inner_tensor_cipher, ele_nums, "LOCAL input as ring value");
      print_vec(inner_tensor_cipher_recons, ele_nums, "REAL result as ring value");
      print_vec(curr_plain_flat_vec, ele_nums, "REAL result as output value");
      #endif

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
        OP_REQUIRES_OK(context, writer.Add(tensor_name, curr_tensor_plain));
      }
    }
    OP_REQUIRES_OK(context, writer.Finish());
  }
};

REGISTER_OP("MpcSaveV2")
    .Input("prefix: string")
    .Input("tensor_names: string")
    .Input("shape_and_slices: string")
    .Input("tensors: dtypes")
    .Attr("dtypes: list(type)")
    .Doc(R"doc(
MpcSaveV2Op
)doc");

/*
    * Attention: 
    * The SaveV2 OP is not like other arithmatic OPs 
    * because its last input is a list of tensors 
    * and these tensors can have different types.
    * In MpcSaveV2 scenarios, altough all the tensors 
    * should be the in 'shared' state, we do not restaint 
    * its dtype, and we check it in our 'compute' function. 
*/
// REGISTER_MPCOP_KERNELS_ALL_TYPES(MpcSaveV2, MpcSaveV2Op)
REGISTER_KERNEL_BUILDER(Name("MpcSaveV2").Device(DEVICE_CPU), MpcSaveV2Op);

} // namespace tensorflow
