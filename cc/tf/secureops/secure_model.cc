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
#include "cc/tf/secureops/secure_base_kernel.h"
#include "cc/modules/protocol/public/include/protocol_manager.h"
#include "cc/modules/common/include/utils/rtt_logger.h"
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

using rosetta::ProtocolManager;

#include "tensorflow/core/lib/strings/stringprintf.h"
#include "tensorflow/core/platform/logging.h"

#include "tensorflow/core/framework/bounds_check.h"
#include "tensorflow/core/framework/op_kernel.h"
#include "tensorflow/core/framework/register_types.h"
#include "tensorflow/core/framework/types.h"
#include "tensorflow/core/lib/core/threadpool.h"
#include "tensorflow/core/lib/gtl/array_slice.h"
#include "tensorflow/core/lib/strings/str_util.h"
#include "tensorflow/core/lib/strings/strcat.h"
#include "tensorflow/core/lib/strings/stringprintf.h"
#include "tensorflow/core/platform/logging.h"
#include "tensorflow/core/platform/types.h"
#include "tensorflow/core/util/tensor_slice_reader.h"
#include "tensorflow/core/util/tensor_slice_reader_cache.h"
#include "tensorflow/core/util/tensor_slice_writer.h"

namespace tensorflow {

string MetaFilename(StringPiece prefix) {
  return strings::Printf("%.*s.index", static_cast<int>(prefix.size()), prefix.data());
}

string DataFilename(StringPiece prefix, int32 shard_id, int32 num_shards) {
  DCHECK_GT(num_shards, 0);
  DCHECK_LT(shard_id, num_shards);
  return strings::Printf(
    "%.*s.data-%05d-of-%05d", static_cast<int>(prefix.size()), prefix.data(), shard_id, num_shards);
}

void RestoreTensor(
  OpKernelContext* context,
  checkpoint::TensorSliceReader::OpenTableFunction open_func,
  int preferred_shard,
  bool restore_slice,
  int restore_index) {
  const Tensor& file_pattern_t = context->input(0);
  {
    const int64 size = file_pattern_t.NumElements();
    OP_REQUIRES(
      context, size == 1,
      errors::InvalidArgument(
        "Input 0 (file_pattern) must be a string scalar; got a tensor of ", size, "elements"));
  }
  const string& file_pattern = file_pattern_t.flat<string>()(0);

  const Tensor& tensor_name_t = context->input(1);
  const string& tensor_name = tensor_name_t.flat<string>()(restore_index);

  // If we cannot find a cached reader we will allocate our own.
  std::unique_ptr<checkpoint::TensorSliceReader> allocated_reader;

  const checkpoint::TensorSliceReader* reader = nullptr;

  if (context->slice_reader_cache()) {
    reader = context->slice_reader_cache()->GetReader(file_pattern, open_func, preferred_shard);
  }
  if (!reader) {
    allocated_reader.reset(
      new checkpoint::TensorSliceReader(file_pattern, open_func, preferred_shard));
    reader = allocated_reader.get();
  }
  OP_REQUIRES_OK(context, CHECK_NOTNULL(reader)->status());

  // Get the shape and type from the save file.
  DataType type;
  TensorShape saved_shape;
  OP_REQUIRES(
    context, reader->HasTensor(tensor_name, &saved_shape, &type),
    errors::NotFound(
      "Tensor name \"", tensor_name, "\" not found in checkpoint files ", file_pattern));
  OP_REQUIRES(
    context, type == context->expected_output_dtype(restore_index),
    errors::InvalidArgument(
      "Expected to restore a tensor of type ", DataTypeString(context->expected_output_dtype(0)),
      ", got a tensor of type ", DataTypeString(type), " instead: tensor_name = ", tensor_name));

  // Shape of the output and slice to load.
  TensorShape output_shape(saved_shape);
  TensorSlice slice_to_load(saved_shape.dims());
  if (restore_slice) {
    const string& shape_spec = context->input(2).flat<string>()(restore_index);
    if (!shape_spec.empty()) {
      TensorShape parsed_shape;
      OP_REQUIRES_OK(
        context,
        checkpoint::ParseShapeAndSlice(shape_spec, &parsed_shape, &slice_to_load, &output_shape));
      OP_REQUIRES(
        context, parsed_shape.IsSameSize(saved_shape),
        errors::InvalidArgument(
          "Shape in shape_and_slice spec does not match the shape in the "
          "save file: ",
          parsed_shape.DebugString(), ", save file shape: ", saved_shape.DebugString()));
    }
  }

  Tensor* t = nullptr;
  OP_REQUIRES_OK(context, context->allocate_output(restore_index, output_shape, &t));

  if (output_shape.num_elements() == 0)
    return;

#define READER_COPY(T)                                                                 \
  case DataTypeToEnum<T>::value:                                                       \
    OP_REQUIRES(                                                                       \
      context, reader->CopySliceData(tensor_name, slice_to_load, t->flat<T>().data()), \
      errors::InvalidArgument("Error copying slice data"));                            \
    break;

  switch (type) {
    TF_CALL_SAVE_RESTORE_TYPES(READER_COPY)
    default:
      context->SetStatus(
        errors::Unimplemented("Restoring data type ", DataTypeString(type), " not yet supported"));
  }
#undef READER_COPY
}

namespace {

struct StoredValue {
  int64_t idx;
  string shape_and_slice; /*now not use*/
  TensorShape shape;
  vector<double> value;
  DataType original_dtype;
};

// Tensors larger than this threshold will be restored from a thread-pool.
const int64 kLargeShapeThreshold = 16 << 20; // 16M

// A restore operation for a single tensor.  Small tensors may be restored
// directly from the op thread to improve read locality.  Large tensors can be
// restored from a thread pool: this requires creating a separate BundleReader
// for each restore.
struct RestoreOp {
  RestoreOp& operator=(const RestoreOp&) = delete;

  bool should_run_in_pool(BundleReader* reader) const {
    TensorShape restored_full_shape;

    // Ignore status here; we'll catch the error later.
    if (!reader->LookupTensorShape(tensor_name, &restored_full_shape).ok()) {
      return false;
    }

    return restored_full_shape.num_elements() > kLargeShapeThreshold;
  }

  // Run this restore operation using a new BundleReader.
  void run_with_new_reader() {
    BundleReader reader(Env::Default(), reader_prefix);
    if (!reader.status().ok()) {
      status = reader.status();
      return;
    }

    status = run(&reader);
  }

  Status run(BundleReader* reader) {
    TensorShape restored_full_shape;
    TF_RETURN_IF_ERROR(reader->LookupTensorShape(tensor_name, &restored_full_shape));

    VLOG(1) << "Restoring tensor " << idx << " : " << tensor_name << " : "
            << restored_full_shape.num_elements();
    Tensor* restored_tensor;
    if (shape_and_slice.empty()) {
      if (restore_mode.empty()) {
        // Lookup the full tensor.
        TF_RETURN_IF_ERROR(context->allocate_output(idx, restored_full_shape, &restored_tensor));
        TF_RETURN_IF_ERROR(reader->Lookup(tensor_name, restored_tensor));
      } else {
        Tensor temp_tensor;

        TF_RETURN_IF_ERROR(
          context->allocate_temp(stored_value->original_dtype, restored_full_shape, &temp_tensor));
        TF_RETURN_IF_ERROR(reader->Lookup(tensor_name, &temp_tensor));
        vector<double> tempvd(temp_tensor.NumElements());
        if (stored_value->original_dtype == DataType::DT_FLOAT) {
          const auto& temp_flat = temp_tensor.flat<float>();
          for (int i = 0; i < temp_tensor.NumElements(); i++) {
            tempvd[i] = temp_flat(i);
          }
        } else if (stored_value->original_dtype == DataType::DT_DOUBLE) {
          const auto& temp_flat = temp_tensor.flat<double>();
          for (int i = 0; i < temp_tensor.NumElements(); i++) {
            tempvd[i] = temp_flat(i);
          }
        } else {
          log_error << "not supported1" ;
        }
        stored_value->idx = idx;
        stored_value->shape_and_slice = shape_and_slice;
        stored_value->shape = restored_full_shape;
        stored_value->value = tempvd;
      }
    } else {
      //! @todo [ujnss]
      log_warn << "[NOT GOOD TEST] RestoreOp::run shape_and_slice:" << shape_and_slice ;
      // Lookup the slice.
      TensorShape parsed_full_shape;
      TensorSlice parsed_slice;
      TensorShape parsed_slice_shape;

      TF_RETURN_IF_ERROR(checkpoint::ParseShapeAndSlice(
        shape_and_slice, &parsed_full_shape, &parsed_slice, &parsed_slice_shape));

      if (!restored_full_shape.IsSameSize(parsed_full_shape)) {
        return errors::InvalidArgument(
          "tensor_name = ", tensor_name, "; shape in shape_and_slice spec ",
          parsed_full_shape.DebugString(),
          " does not match the shape stored in checkpoint: ", restored_full_shape.DebugString());
      }

      if (restore_mode.empty()) {
        TF_RETURN_IF_ERROR(context->allocate_output(idx, parsed_slice_shape, &restored_tensor));
        TF_RETURN_IF_ERROR(reader->LookupSlice(tensor_name, parsed_slice, restored_tensor));
      } else {
        Tensor temp_tensor;

        TF_RETURN_IF_ERROR(
          context->allocate_temp(stored_value->original_dtype, parsed_slice_shape, &temp_tensor));
        TF_RETURN_IF_ERROR(reader->LookupSlice(tensor_name, parsed_slice, &temp_tensor));

        vector<double> tempvd(temp_tensor.NumElements());
        if (stored_value->original_dtype == DataType::DT_FLOAT) {
          const auto& temp_flat = temp_tensor.flat<float>();
          for (int i = 0; i < temp_tensor.NumElements(); i++) {
            tempvd[i] = temp_flat(i);
          }
        } else if (stored_value->original_dtype == DataType::DT_DOUBLE) {
          const auto& temp_flat = temp_tensor.flat<double>();
          for (int i = 0; i < temp_tensor.NumElements(); i++) {
            tempvd[i] = temp_flat(i);
          }
        } else {
          log_error << "not supported2" ;
        }
        stored_value->idx = idx;
        stored_value->shape_and_slice = shape_and_slice;
        stored_value->shape = restored_full_shape;
        stored_value->value = tempvd;
      }
    }
    return Status::OK();
  }

  OpKernelContext* context;
  size_t idx;
  string tensor_name;
  string shape_and_slice;
  string reader_prefix;
  StoredValue* stored_value;
  vector<string> restore_mode;

  ::tensorflow::Status status;
};

} // namespace

/**
 * can only cope dtypes are string and original dtype is float/double.
 */
Status RestoreTensorsV2(
  OpKernelContext* context,
  const Tensor& prefix,
  const Tensor& tensor_names,
  const Tensor& shape_and_slices,
  gtl::ArraySlice<DataType> dtypes,
  vector<StoredValue>& stored_values,
  const vector<string>& restore_mode) {
  const string& prefix_string = prefix.scalar<string>()();
  const auto& tensor_names_flat = tensor_names.flat<string>();
  const auto& shape_and_slices_flat = shape_and_slices.flat<string>();

  // Sort lookup keys to improve locality when reading multiple tensors.
  std::vector<size_t> sorted_name_idx(tensor_names_flat.size());
  std::iota(sorted_name_idx.begin(), sorted_name_idx.end(), 0);
  std::sort(
    sorted_name_idx.begin(), sorted_name_idx.end(), [&tensor_names_flat](size_t a, size_t b) {
      return tensor_names_flat(a) < tensor_names_flat(b);
    });
  stored_values.resize(sorted_name_idx.size());

  std::vector<std::unique_ptr<RestoreOp>> pool_restore_ops;
  std::vector<std::unique_ptr<RestoreOp>> direct_restore_ops;

  BundleReader default_reader(Env::Default(), prefix_string);
  TF_RETURN_IF_ERROR(default_reader.status());

  std::vector<string> mismatched_errors;
  for (const size_t i : sorted_name_idx) {
    TensorShape restored_full_shape;
    DataType original_dtype;
    const string& tensor_name = tensor_names_flat(i);
    TF_RETURN_IF_ERROR(
      default_reader.LookupDtypeAndShape(tensor_name, &original_dtype, &restored_full_shape));
    // log_info << "tensor_name:" << tensor_name << ",dtypes[i]:" << DataTypeString(dtypes[i])
    //          << ",original_dtype:" << DataTypeString(original_dtype) ;
    stored_values[i].original_dtype = original_dtype;
    if (restore_mode.empty()) {
      if (dtypes[i] != original_dtype) {
        string error_msg = strings::StrCat(
          "tensor_name = ", tensor_name, "; expected dtype ", DataTypeString(dtypes[i]),
          " does not equal original dtype ", DataTypeString(original_dtype));
        mismatched_errors.emplace_back(error_msg);
      }
    } else {
      //! ==== modify beg
      if (dtypes[i] == original_dtype) {
        // do nothing
      } else {
        if ((original_dtype == DataType::DT_FLOAT) || (original_dtype == DataType::DT_DOUBLE)) {
          // If original dtype is float/double, will to convert to string
        } else {
          string error_msg = strings::StrCat(
            "tensor_name = ", tensor_name, "; expected dtype ", DataTypeString(dtypes[i]),
            " does not equal original dtype ", DataTypeString(original_dtype));
          mismatched_errors.emplace_back(error_msg);
        }
      }
      //! ==== modify end
    }
  }
  if (!mismatched_errors.empty()) {
    const string error_msg = str_util::Join(mismatched_errors, "\n");
    return errors::InvalidArgument(error_msg);
  }

  for (auto i : sorted_name_idx) {
    const string& tensor_name = tensor_names_flat(i);
    const string& shape_and_slice = shape_and_slices_flat(i);
    auto op = new RestoreOp{
      context, i, tensor_name, shape_and_slice, prefix_string, &stored_values[i], restore_mode};
    if (op->should_run_in_pool(&default_reader)) {
      pool_restore_ops.emplace_back(op);
    } else {
      direct_restore_ops.emplace_back(op);
    }
  }

  {
    // Schedule any threaded operations first, skipping thread pool creation if
    // we don't have any expensive operations.
    std::unique_ptr<thread::ThreadPool> reader_pool;
    if (!pool_restore_ops.empty()) {
      reader_pool.reset(new thread::ThreadPool(Env::Default(), "restore_tensors", 8));
      for (auto& op : pool_restore_ops) {
        reader_pool->Schedule([&op]() { op->run_with_new_reader(); });
      }
    }

    // Read small tensors from the op thread
    for (auto& op : direct_restore_ops) {
      TF_RETURN_IF_ERROR(op->run(&default_reader));
    }
  }

  // Check status of pool ops; this must come after the pool shuts down.
  for (auto& op : pool_restore_ops) {
    TF_RETURN_IF_ERROR(op->status);
  }

  if (!restore_mode.empty()) {
    return Status::OK();
  }

  for (auto i : sorted_name_idx) {
    const string& tensor_name = tensor_names_flat(i);
    if (dtypes[i] != context->mutable_output(i)->dtype()) {
      return errors::InvalidArgument(
        "tensor_name = ", tensor_name, "; expected dtype ", DataTypeString(dtypes[i]),
        " does not equal restored dtype ", DataTypeString(context->mutable_output(i)->dtype()));
    }
  }

  return Status::OK();
}

} // namespace tensorflow

namespace tensorflow {
// Shared validations of the inputs to the SaveV2 and RestoreV2 ops.
void ValidateInputs(
  bool is_save_op,
  OpKernelContext* context,
  const Tensor& prefix,
  const Tensor& tensor_names,
  const Tensor& shape_and_slices) {
  const int kFixedInputs = 3; // Prefix, tensor names, shape_and_slices.
  const int num_tensors = static_cast<int>(tensor_names.NumElements());
  OP_REQUIRES(
    context, prefix.NumElements() == 1,
    errors::InvalidArgument(
      "Input prefix should have a single element, got ", prefix.NumElements(), " instead."));
  OP_REQUIRES(
    context,
    TensorShapeUtils::IsVector(tensor_names.shape()) &&
      TensorShapeUtils::IsVector(shape_and_slices.shape()),
    errors::InvalidArgument(
      "Input tensor_names and shape_and_slices "
      "should be an 1-D tensors, got ",
      tensor_names.shape().DebugString(), " and ", shape_and_slices.shape().DebugString(),
      " instead."));
  OP_REQUIRES(
    context, tensor_names.NumElements() == shape_and_slices.NumElements(),
    errors::InvalidArgument(
      "tensor_names and shape_and_slices "
      "have different number of elements: ",
      tensor_names.NumElements(), " vs. ", shape_and_slices.NumElements()));
  OP_REQUIRES(
    context,
    FastBoundsCheck(tensor_names.NumElements() + kFixedInputs, std::numeric_limits<int>::max()),
    errors::InvalidArgument("Too many inputs to the op"));
  OP_REQUIRES(
    context, shape_and_slices.NumElements() == num_tensors,
    errors::InvalidArgument(
      "Expected ", num_tensors, " elements in shapes_and_slices, but got ",
      context->input(2).NumElements()));
  if (is_save_op) {
    OP_REQUIRES(
      context, context->num_inputs() == num_tensors + kFixedInputs,
      errors::InvalidArgument(
        "Got ", num_tensors, " tensor names but ", context->num_inputs() - kFixedInputs,
        " tensors."));
    OP_REQUIRES(
      context, context->num_inputs() == num_tensors + kFixedInputs,
      errors::InvalidArgument(
        "Expected a total of ", num_tensors + kFixedInputs,
        " inputs as input #1 (which is a string "
        "tensor of saved names) contains ",
        num_tensors, " names, but received ", context->num_inputs(), " inputs"));
  }
}

} // namespace tensorflow

namespace tensorflow {

static constexpr char kErrorMessage[] = "SecureSaveV2Op could not correctly convert string: ";

// Saves a list of named tensors using the tensor bundle library.
//template <typename Device>
class SecureSaveV2Op : public SecureOpKernel {
 public:
  explicit SecureSaveV2Op(OpKernelConstruction* context) : SecureOpKernel(context) {}

  void ComputeImpl(OpKernelContext* context) {
    const Tensor& prefix = context->input(0);
    const Tensor& tensor_names = context->input(1);
    const Tensor& shape_and_slices = context->input(2);
    ValidateInputs(true /* is save op */, context, prefix, tensor_names, shape_and_slices);

    const int kFixedInputs = 3; // Prefix, tensor names, shape_and_slices.
    const int num_tensors = static_cast<int>(tensor_names.NumElements());
    const string& prefix_string = prefix.scalar<string>()();
    const auto& tensor_names_flat = tensor_names.flat<string>();
    const auto& shape_and_slices_flat = shape_and_slices.flat<string>();

    // BundleWriter writer(Env::Default(), prefix_string);
    BundleWriter* writer_p = nullptr;
    bool need_writer = false;
    //OP_REQUIRES_OK(context, writer_p->status());
    log_debug << "DEBUG SecureSaveV2!:" ;

    for (int i = 0; i < num_tensors; ++i) {
      const string& tensor_name = tensor_names_flat(i);
      const Tensor& curr_tensor = context->input(i + kFixedInputs);
      DataType curr_type = curr_tensor.dtype();
      auto ele_nums = curr_tensor.NumElements();

      OP_REQUIRES(
        context, curr_type == DT_STRING,
        errors::InvalidArgument(
          "The ", i, "-th tensor is not in DT_STRING type!", curr_tensor.DebugString(5)));
      auto const curr_tensor_flat = curr_tensor.flat<string>();

      Tensor plain_tensor;
      OP_REQUIRES_OK(
        context, context->allocate_temp(DT_DOUBLE, curr_tensor.shape(), &plain_tensor));
      Tensor cipher_tensor;
      OP_REQUIRES_OK(
        context, context->allocate_temp(DT_STRING, curr_tensor.shape(), &cipher_tensor));

      auto plain_tensor_flat = plain_tensor.flat<double>();
      plain_tensor_flat.setZero();
      auto cipher_tensor_flat = cipher_tensor.flat<string>();

      // Note that we call the Save interface of specific protocol because
      // different protocol may have different protocol config style.
      vector<double> potential_plain_res;
      vector<string> potential_cipher_res;
      vector<string> input_tensor_vec(ele_nums);
      for (int i = 0; i < ele_nums; ++i) {
        input_tensor_vec[i] = curr_tensor_flat(i);
      }
      SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(ConditionalReveal);
      ProtocolManager::Instance()
        ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
        ->GetOps(msg_id())
        ->ConditionalReveal(input_tensor_vec, potential_cipher_res, potential_plain_res);
      SECURE_OP_CALL_PROTOCOL_OP_STATS_END(ConditionalReveal);

      if (potential_cipher_res.empty() && potential_plain_res.empty()) {
        log_warn << "No need to save anything!" ;
        // to help handle next input tensor
        continue;
      } else {
        need_writer = true;
        if (writer_p == NULL) {
          writer_p = new BundleWriter(Env::Default(), prefix_string);
          OP_REQUIRES_OK(context, writer_p->status());
        }
      }

      log_debug << "plain size:" << potential_plain_res.size()
                << " VS cipher size:" << potential_cipher_res.size() ;

      bool is_plain = false;
      if (!potential_plain_res.empty()) {
        if (!potential_cipher_res.empty()) {
          // TODO: throw exception.
          log_error << "ERROR! we can not save the tensors both in plaintext and ciphertext!"
                    ;
          return;
        }
        is_plain = true;
        for (int i = 0; i < ele_nums; ++i) {
          plain_tensor_flat(i) = potential_plain_res[i];
        }
      } else {
        for (int i = 0; i < ele_nums; ++i) {
          cipher_tensor_flat(i) = potential_cipher_res[i];
        }
      }

      // [TODO]: in this version, we should validate that the user does not config this input.
      if (!shape_and_slices_flat(i).empty()) {
        const string& shape_spec = shape_and_slices_flat(i);
        TensorShape shape;
        TensorSlice slice(curr_tensor.dims());
        TensorShape slice_shape;
        OP_REQUIRES_OK(
          context, checkpoint::ParseShapeAndSlice(shape_spec, &shape, &slice, &slice_shape));
        OP_REQUIRES(
          context, slice_shape.IsSameSize(curr_tensor.shape()),
          errors::InvalidArgument(
            "Slice in shape_and_slice "
            "specification does not match the "
            "shape of the tensor to  save: ",
            shape_spec, ", tensor: ", curr_tensor.shape().DebugString()));
        if (is_plain) {
          OP_REQUIRES_OK(context, writer_p->AddSlice(tensor_name, shape, slice, plain_tensor));
        } else {
          OP_REQUIRES_OK(context, writer_p->AddSlice(tensor_name, shape, slice, cipher_tensor));
        }
      } else {
        if (is_plain) {
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

class SecureRestoreV2Op : public SecureOpKernel {
 public:
  explicit SecureRestoreV2Op(OpKernelConstruction* context) : SecureOpKernel(context) {
    OP_REQUIRES_OK(context, context->GetAttr("dtypes", &dtypes_));
  }

  /**
   * SecureRestoreV2Op
   * 
   * If the model is ciphertext (restore_party_id == -1), the behavior of this OP is no different from Tensorflow RestoreV2.
   *    all parties each have the secret sharing value of the model.
   * If the model is plaintext,
   *    If load model as private (restore_party_id >= 0),
   *        only model-owning party have model.
   *        load the value in the model by calling PrivateInput().
   *    If load model as public (restore_party_id == -2),
   *        each party has the same plain model.
   *        load the value in the model as a public-constant string. e.g. 1.2345 --> "1.2345"
   * 
   * RESTORE_MODE 
   *  for general 3PC, e.g. MPC protocol
   *  b000(0), cipher model, each party has the secret sharing value of the model.
   *  b001/b010/b100, plain model, only P0(b001) or P1(b010) or P2(b100) owns the plain model, load as private;
   *  b111(-1), plain model, all parties have the plain model, load as public-constant;
   *  others, not supported
   * 
   *  for general 2PC, e.g. ZK protocol
   *  b00(0), cipher model, each party has the secret sharing value of the model. (not supported in ZK.)
   *  b01/b10, plain model, only P0(b01) or P1(b10) owns the plain model, load as private; (only P0 owns the plain model in ZK.)
   *  b11(-1), plain model, all parties have the plain model, load as public-constant;
   *  others, not supported
   * 
   * **NOTE**: The legitimacy of `RESTORE_MODE` will be verified during initialization according to the different protocols.
   * 
   */
  void ComputeImpl(OpKernelContext* context) {
    SimpleTimer timer;
    int parties = ProtocolManager::Instance()
                    ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
                    ->GetParties();
    node_id_ = ProtocolManager::Instance()
                  ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
                  ->GetNetHandler()->GetCurrentNodeId();
    restore_mode_ = ProtocolManager::Instance()
                  ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
                  ->GetMpcContext()->RESTORE_MODE;
    int restore_party_id = -1;
    bool is_public_model = false;
    bool is_model_owner = false;

    // party id start 0
    std::string restore_desc = "unsupported restore_mode";
    auto prtc = ProtocolManager::Instance()->GetProtocol(ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()));
    vector<string> data_nodes = prtc->GetNetHandler()->GetDataNodes();
    map<string, int> computation_nodes = prtc->GetNetHandler()->GetComputationNodes(); 
    if (restore_mode_.empty()) {
      restore_desc = "all parties each have the secret sharing value of the model";
    } else if (restore_mode_.size() == 1) {
      if (std::find(data_nodes.begin(), data_nodes.end(), restore_mode_[0]) == data_nodes.end()) {
        log_error << "restore node is not a valid data node!" ;
        return;
      }
      if (node_id_ == restore_mode_[0]) {
        is_model_owner = true;
      }
      restore_desc = restore_mode_[0] + " owns the plain model, load model as private";
    } else {
      for (auto iter = computation_nodes.begin(); iter != computation_nodes.end(); iter++) {
        if (std::find(restore_mode_.begin(), restore_mode_.end(), iter->first) == restore_mode_.end()) {
          log_error << "more than one computation node own model, but " + iter->first + " does not own model!";
          return;
        }
      }
      restore_desc = "each party has the same plain model, load model as public";
      is_public_model = true;
    }

    //log_info << "partyid: " << partyid_ << "/" << parties << ", restore_mode:" << restore_mode_
    //         << ", restore type:" << restore_party_id << " - " << restore_desc ;
    log_info << restore_desc;

    const Tensor& prefix = context->input(0);
    const Tensor& tensor_names = context->input(1);
    const Tensor& shape_and_slices = context->input(2);
    OP_REQUIRES(
      context, tensor_names.NumElements() == dtypes_.size(),
      errors::InvalidArgument(
        "Got ", tensor_names.NumElements(), " tensor names, but ", dtypes_.size(),
        " expected dtypes."));
    ValidateInputs(false /* not save op */, context, prefix, tensor_names, shape_and_slices);

    const string& prefix_string = prefix.scalar<string>()();
    const auto& tensor_names_flat = tensor_names.flat<string>();
    const auto& shape_and_slices_flat = shape_and_slices.flat<string>();

    if ((restore_mode_.empty()) || (is_public_model) || (is_model_owner)) {
      // Intention: we plan to use the RestoreV2 op as a backward-compatible
      // reader as we upgrade to the V2 format.  This allows transparent upgrade.
      // We here attempt to read a V1 checkpoint, if "prefix_string" does not
      // refer to a V2 checkpoint.
      Env* env = Env::Default();
      std::vector<string> paths;
      if (!env->GetMatchingPaths(MetaFilename(prefix_string), &paths).ok() || paths.empty()) {
        // Cannot find V2's metadata file, so "prefix_string" does not point to a
        // V2 checkpoint.  Invokes the V1 read path instead.
        for (size_t i = 0; i < tensor_names.NumElements(); ++i) {
          RestoreTensor(
            context, &checkpoint::OpenTableTensorSliceReader,
            /* preferred_shard */ -1, /* restore_slice */ true,
            /* restore_index */ i);
          if (!context->status().ok()) {
            return;
          }
        }
        return;
      }

      // If found, invokes the V2 reader.
      OP_REQUIRES_OK(
        context,
        RestoreTensorsV2(
          context, prefix, tensor_names, shape_and_slices, dtypes_, stored_values_, restore_mode_));
    }
    auto print_model_parameters_info = [&]() {
      int64_t total_size = 0;
      for (auto& stored_values : stored_values_) {
        total_size += stored_values.value.size();
      }
      log_info << "the variable number of the model:" << stored_values_.size()
               << ", total parameters:" << total_size ;
    };

    if (restore_mode_.empty()) {
      print_model_parameters_info();
      log_info << "restore model done:" << timer.elapse() ;
      return;
    }

    if (is_public_model) {
      print_model_parameters_info();
      for (auto& stored_values : stored_values_) {
        int64_t idx = stored_values.idx;
        TensorShape restored_full_shape = stored_values.shape;
        Tensor* restored_tensor = nullptr;

        auto& tempvd = stored_values.value;
        OP_REQUIRES_OK(
          context, context->allocate_output(idx, restored_full_shape, &restored_tensor));
        auto out_flat = restored_tensor->flat<string>();
        for (int64_t i = 0; i < tempvd.size(); ++i) {
#if !use_literal_value_binary_version
          std::string stmp(sizeof(double) + 1, '$');
          memcpy((char*)stmp.data(), (char*)&tempvd[i], sizeof(double));
          out_flat(i) = stmp;
#else
          out_flat(i) = std::to_string(tempvd[i]);
#endif
        }
      }
      log_info << "restore model done:" << timer.elapse() ;
      return;
    }

    auto protocol = ProtocolManager::Instance()
                ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()));
    auto ops = protocol->GetOps(msg_id());
    shared_ptr<NET_IO> net_io = protocol->GetNetHandler();
    if (node_id_ == restore_mode_[0]) {
      print_model_parameters_info();
      vector<int64_t> shapes;
      shapes.push_back(stored_values_.size());
      for (auto& stored_values : stored_values_) {
        auto dims = stored_values.shape.dims();
        shapes.push_back(dims);
        for (int i = 0; i < dims; i++) {
          shapes.push_back(stored_values.shape.dim_size(i));
        }
      }

      // sync shapes to other parties
      ///////////////////////////////////////////////////////////////////////////
      int64_t n = shapes.size();
      SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Broadcast);
      ops->Broadcast(restore_mode_[0], (const char*)&n, (char*)&n, sizeof(n));
      ops->Broadcast(restore_mode_[0], (const char*)shapes.data(), (char*)shapes.data(),
        sizeof(int64_t) * shapes.size());
      SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Broadcast);
      log_info << "send n:" << n;
      for (int i = 0; i < shapes.size(); i++) {
        log_info << "send shapes:" << shapes[i];
      }
    } else {
      int64_t n = 0;
      SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Broadcast);
      ops->Broadcast(restore_mode_[0], (const char*)&n, (char*)&n, sizeof(n));
      vector<int64_t> shapes(n);
      ops->Broadcast(restore_mode_[0], (const char*)shapes.data(), (char*)shapes.data(), sizeof(int64_t) * shapes.size());
      SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Broadcast);
      log_info << "recv n:" << n ;
      for (int i = 0; i < shapes.size(); i++) {
        log_info << "recv shape:" << shapes[i] ;
      }
      ///////////////////////////////////////////////////////////////////////////

      stored_values_.resize(shapes[0]);

      int64_t k = 1;
      for (int64_t i = 0; i < stored_values_.size(); i++) {
        auto& stored_values = stored_values_[i];
        stored_values.idx = i;

        vector<int64_t> ds;
        int64_t dims = shapes[k++];
        for (int d = 0; d < dims; d++) {
          ds.push_back(shapes[k++]);
        }

        if (dims == 1)
          stored_values.shape = TensorShape({ds[0]});
        else if (dims == 2)
          stored_values.shape = TensorShape({ds[0], ds[1]});
        else if (dims == 3)
          stored_values.shape = TensorShape({ds[0], ds[1], ds[2]});
        else if (dims == 4)
          stored_values.shape = TensorShape({ds[0], ds[1], ds[2], ds[3]});
        else if (dims == 5)
          stored_values.shape = TensorShape({ds[0], ds[1], ds[2], ds[3], ds[4]});
        else
          throw;

        stored_values.value.resize(stored_values.shape.num_elements(), 0);
      }
    }

    //! @note only supported shape_and_slice is empty()
    // if (shape_and_slice.empty())

#define use_vectorize 0
#if use_vectorize
    int64_t EEEEE = 1024 * 1024 * 2; // 2M
    int64_t stored_values_size = stored_values_.size();
    int64_t next_index = 0;
    do {
      int64_t j = next_index;
      vector<double> tempvd;
      for (int64_t k = next_index; k < stored_values_size; k++) {
        tempvd.insert(tempvd.end(), stored_values_[k].value.begin(), stored_values_[k].value.end());
        if ((tempvd.size() >= EEEEE) || (k == stored_values_size - 1)) {
          log_info << "-- restore call PrivateInput(). index [" << next_index << "," << k << "]/"
                   << stored_values_size << ", elements:" << tempvd.size() << "." ;
          next_index = k + 1;
          break;
        }
      }

      vector<string> tempvs; //(tempvd.size());
      SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(PrivateInput);
      ops->PrivateInput(restore_mode_[0], tempvd, tempvs);
      SECURE_OP_CALL_PROTOCOL_OP_STATS_END(PrivateInput);

      int64_t temp_params_size = 0;
      for (; j < next_index; j++) {
        auto& stored_values = stored_values_[j];
        int64_t idx = stored_values.idx;
        TensorShape restored_full_shape = stored_values.shape;
        Tensor* restored_tensor = nullptr;
        OP_REQUIRES_OK(
          context, context->allocate_output(idx, restored_full_shape, &restored_tensor));
        auto out_flat = restored_tensor->flat<string>();
        for (int64_t i = 0; i < stored_values.value.size(); ++i) {
          out_flat(i) = std::move(tempvs[temp_params_size + i]);
        }
        temp_params_size += stored_values.value.size();
      }

    } while (next_index < stored_values_size);
#else
    int ii = 0;
    for (auto& stored_values : stored_values_) {
      if (stored_values.value.size() > 500 * 1000) {
        log_debug << "-- print the size of those variables with more than 500,000 elements ("
                  << ii + 1 << "/" << stored_values_.size() << "):" << stored_values.value.size()
                  ;
      }

      int64_t idx = stored_values.idx;
      TensorShape restored_full_shape = stored_values.shape;
      Tensor* restored_tensor = nullptr;

      auto& tempvd = stored_values.value;
      vector<string> tempvs; //(stored_values.shape.num_elements());

      //print_vector(tempvd, "restore index " + to_string(ii), 5, 8);
      SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(PrivateInput);
      ops->PrivateInput(restore_mode_[0], tempvd, tempvs);
      SECURE_OP_CALL_PROTOCOL_OP_STATS_END(PrivateInput);

      OP_REQUIRES_OK(context, context->allocate_output(idx, restored_full_shape, &restored_tensor));
      auto out_flat = restored_tensor->flat<string>();
      for (int64_t i = 0; i < tempvs.size(); ++i) {
        out_flat(i) = std::move(tempvs[i]);
      }
      ii++;
    }
#endif
    log_info << "restore model done:" << timer.elapse() ;

    //! @todo add check
    // for (auto i : sorted_name_idx) {
    //   const string& tensor_name = tensor_names_flat(i);
    //   if (dtypes[i] != context->mutable_output(i)->dtype()) {
    //     return errors::InvalidArgument(
    //       "tensor_name = ", tensor_name, "; expected dtype ", DataTypeString(dtypes[i]),
    //       " does not equal restored dtype ", DataTypeString(context->mutable_output(i)->dtype()));
    //   }
    // }
  }

 private:
  std::vector<DataType> dtypes_;
  vector<StoredValue> stored_values_;
  string node_id_;
  vector<string> restore_mode_; // ref config
}; // namespace tensorflow

REGISTER_KERNEL_BUILDER(Name("SecureSaveV2").Device(DEVICE_CPU), SecureSaveV2Op);
REGISTER_KERNEL_BUILDER(Name("SecureRestoreV2").Device(DEVICE_CPU), SecureRestoreV2Op);

} // namespace tensorflow
