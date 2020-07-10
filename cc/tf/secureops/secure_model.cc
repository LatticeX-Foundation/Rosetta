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

#include "cc/tf/secureops/secure_base_kernel.h"
#include "cc/modules/protocol/public/protocol_manager.h"
#include "cc/modules/common/include/utils/logger.h"

using rosetta::ProtocolManager;

namespace tensorflow {
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

namespace tensorflow{
  
static constexpr char kErrorMessage[] = "SecureSaveV2Op could not correctly convert string: ";

// Saves a list of named tensors using the tensor bundle library.
//template <typename Device>
class SecureSaveV2Op : public SecureOpKernel{ 
 public:
  explicit SecureSaveV2Op(OpKernelConstruction* context) : SecureOpKernel(context) {}

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

    // BundleWriter writer(Env::Default(), prefix_string);
    BundleWriter* writer_p = nullptr;
    bool need_writer = false;
    //OP_REQUIRES_OK(context, writer_p->status());
    log_debug << "DEBUG SecureSaveV2!:" << endl;

    for (int i = 0; i < num_tensors; ++i) {
      const string& tensor_name = tensor_names_flat(i);
      const Tensor& curr_tensor = context->input(i + kFixedInputs);
      DataType curr_type = curr_tensor.dtype();
      auto ele_nums = curr_tensor.NumElements();

      OP_REQUIRES(context, 
                    curr_type == DT_STRING,
                    errors::InvalidArgument(
                      "The ", i, "-th tensor is not in DT_STRING type!",
                      curr_tensor.DebugString(5)));
      auto const curr_tensor_flat = curr_tensor.flat<string>();

      Tensor plain_tensor;
      OP_REQUIRES_OK(context, context->allocate_temp(DT_DOUBLE, 
                          curr_tensor.shape(), &plain_tensor));
      Tensor cipher_tensor;
      OP_REQUIRES_OK(context, context->allocate_temp(DT_STRING, 
                          curr_tensor.shape(), &cipher_tensor));
            
      auto plain_tensor_flat = plain_tensor.flat<double>();
      plain_tensor_flat.setZero();
      auto cipher_tensor_flat = cipher_tensor.flat<string>();

      // Note that we call the Save interface of specific protocol because
      // different protocol may have different protocol config style.
      vector<double> potential_plain_res;
      vector<string> potential_cipher_res;
      vector<string> input_tensor_vec(ele_nums);
      for(int i = 0; i < ele_nums; ++i) {
        input_tensor_vec[i] = curr_tensor_flat(i);
      }
      ProtocolManager::Instance()->GetProtocol()->GetOps(msg_id().str())->ConditionalReveal(input_tensor_vec, potential_cipher_res, potential_plain_res);

      if(potential_cipher_res.empty() && potential_plain_res.empty()) {
        log_warn << "No need to save anything!" << endl;
        // to help handle next input tensor
        continue;
      } else {
        need_writer = true;
        if (writer_p == NULL) {
          writer_p = new BundleWriter(Env::Default(), prefix_string);
          OP_REQUIRES_OK(context, writer_p->status());
        }
      }

      cout << "plain size:" << potential_plain_res.size() << " VS cipher size:" << potential_cipher_res.size() << endl;

      bool is_plain = false;
      if(!potential_plain_res.empty()) {
        if(!potential_cipher_res.empty()) {
          // TODO: throw exception.
          log_error << "ERROR! we can not save the tensors both in plaintext and ciphertext!" << endl;
          return;
        }
        is_plain = true;
        for(int i = 0; i < ele_nums; ++i) {
          plain_tensor_flat(i) = potential_plain_res[i];
        }
      } else {
        for(int i = 0; i < ele_nums; ++i) {
          cipher_tensor_flat(i) = potential_cipher_res[i];
        }
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
        if(is_plain) {
          OP_REQUIRES_OK(context,
                       writer_p->AddSlice(tensor_name, shape, slice, plain_tensor));
          } else {
            OP_REQUIRES_OK(context,
                       writer_p->AddSlice(tensor_name, shape, slice, cipher_tensor));
          }
        } else {
          if(is_plain) {
            OP_REQUIRES_OK(context, writer_p->Add(tensor_name, plain_tensor));
          } else {
            OP_REQUIRES_OK(context, writer_p->Add(tensor_name, cipher_tensor));
          }

      }
    }
    if (need_writer && writer_p != NULL) {
      OP_REQUIRES_OK(context, writer_p->Finish());
      delete writer_p;
    }
  }
};

REGISTER_KERNEL_BUILDER(Name("SecureSaveV2").Device(DEVICE_CPU), SecureSaveV2Op);

}// namespace tensorflow
