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
#pragma once

#include "third_party/eigen3/unsupported/Eigen/CXX11/Tensor"
#include "tensorflow/core/framework/register_types.h"
#include "tensorflow/core/framework/tensor_types.h"
#include "tensorflow/core/util/tensor_format.h"
#include "tensorflow/core/util/padding.h"
#include "tensorflow/core/framework/tensor_shape.h"
#include "tensorflow/core/framework/op_kernel.h"
#include "tensorflow/core/platform/macros.h"
#include "tensorflow/core/lib/core/errors.h"
#include "tensorflow/core/lib/core/threadpool.h"
#include "tensorflow/core/framework/common_shape_fns.h"

#include <cstring>
#include <vector>

namespace tensorflow {
typedef std::vector<int32> TShape;

// Convolution parameters specified by Op attributes.
struct Conv2DParameters {
  std::vector<int32> dilations;
  std::vector<int32> strides;
  Padding padding;
  TensorFormat data_format;
  std::vector<int64> explicit_paddings;
};

// Convolution dimensions inferred from parameters, input and filter tensors.
struct Conv2DDimensions {
  int batch;
  int input_rows;
  int input_cols;
  int in_depth;

  int filter_rows;
  int filter_cols;
  int patch_depth;
  int out_depth;

  int stride_rows;
  int stride_cols;

  int dilation_rows;
  int dilation_cols;

  int64 out_rows;
  int64 out_cols;
  int64 pad_rows_before;
  int64 pad_rows_after;
  int64 pad_cols_before;
  int64 pad_cols_after;
};

#define TF_REQUIRES(EXP, STATUS) \
  do {                           \
    if (!TF_PREDICT_TRUE(EXP))   \
      return (STATUS);           \
  } while (false)

Status InitConv2DParameters(const OpKernelConstruction* context, Conv2DParameters* params) {
  TF_RETURN_IF_ERROR(context->GetAttr("dilations", &params->dilations));
  TF_RETURN_IF_ERROR(context->GetAttr("strides", &params->strides));
  TF_RETURN_IF_ERROR(context->GetAttr("padding", &params->padding));
  if (context->HasAttr("explicit_paddings")) {
    TF_RETURN_IF_ERROR(context->GetAttr("explicit_paddings", &params->explicit_paddings));
  }
  string data_format_string;
  TF_RETURN_IF_ERROR(context->GetAttr("data_format", &data_format_string));
  TF_REQUIRES(
    FormatFromString(data_format_string, &params->data_format),
    errors::InvalidArgument("Invalid data format"));

  const auto& strides = params->strides;
  const auto& dilations = params->dilations;
  const auto& data_format = params->data_format;

  TF_REQUIRES(
    dilations.size() == 4,
    errors::InvalidArgument("Sliding window dilations field must "
                            "specify 4 dimensions"));
  TF_REQUIRES(
    strides.size() == 4,
    errors::InvalidArgument("Sliding window strides field must "
                            "specify 4 dimensions"));
  const int64 stride_n = GetTensorDim(strides, data_format, 'N');
  const int64 stride_c = GetTensorDim(strides, data_format, 'C');
  const int64 stride_h = GetTensorDim(strides, data_format, 'H');
  const int64 stride_w = GetTensorDim(strides, data_format, 'W');
  TF_REQUIRES(
    stride_n == 1 && stride_c == 1,
    errors::InvalidArgument("Current implementation does not yet support "
                            "strides in the batch and depth dimensions."));
  TF_REQUIRES(
    stride_h > 0 && stride_w > 0,
    errors::InvalidArgument("Row and column strides should be larger than 0."));

  const int64 dilation_n = GetTensorDim(dilations, data_format, 'N');
  const int64 dilation_c = GetTensorDim(dilations, data_format, 'C');
  const int64 dilation_h = GetTensorDim(dilations, data_format, 'H');
  const int64 dilation_w = GetTensorDim(dilations, data_format, 'W');
  TF_REQUIRES(
    dilation_n == 1 && dilation_c == 1,
    errors::InvalidArgument("Current implementation does not yet support "
                            "dilations in the batch and depth dimensions."));
  TF_REQUIRES(
    dilation_h > 0 && dilation_w > 0,
    errors::InvalidArgument("Dilated rates should be larger than 0."));

  TF_RETURN_IF_ERROR(CheckValidPadding(
    params->padding, params->explicit_paddings,
    /*num_dims=*/4, data_format));

  return Status::OK();
}

Status ComputeConv2DDimension(
  const Conv2DParameters& params,
  const Tensor& input,
  const Tensor& filter,
  Conv2DDimensions* dimensions) {
  // Check that 2D convolution input and filter have exactly 4 dimensions.
  TF_REQUIRES(
    input.dims() == 4,
    errors::InvalidArgument("input must be 4-dimensional", input.shape().DebugString()));
  TF_REQUIRES(
    filter.dims() == 4,
    errors::InvalidArgument("filter must be 4-dimensional: ", filter.shape().DebugString()));
  for (int i = 0; i < 3; i++) {
    TF_REQUIRES(
      FastBoundsCheck(filter.dim_size(i), std::numeric_limits<int>::max()),
      errors::InvalidArgument("filter too large"));
  }

  // The last dimension for input is in_depth. Check that it is the same as the
  // filter's in_depth or it is evenly divisible by filter's in_depth.
  const int64 in_depth_raw = GetTensorDim(input, params.data_format, 'C');
  const int64 patch_depth_raw = filter.dim_size(2);
  TF_REQUIRES(
    FastBoundsCheck(in_depth_raw, std::numeric_limits<int>::max()),
    errors::InvalidArgument("Input depth too large"));
  TF_REQUIRES(
    FastBoundsCheck(patch_depth_raw, std::numeric_limits<int>::max()),
    errors::InvalidArgument("Patch depth too large"));
  const int in_depth = static_cast<int>(in_depth_raw);
  const int patch_depth = static_cast<int>(patch_depth_raw);
  TF_REQUIRES(
    in_depth % patch_depth == 0,
    errors::InvalidArgument(
      "input depth must be evenly divisible by filter depth: ", in_depth, " vs ", patch_depth));

  // The last dimension for filter is out_depth.
  const int out_depth = static_cast<int>(filter.dim_size(3));

  // The second dimension for input is rows/height.
  // The first dimension for filter is rows/height.
  const int64 input_rows_raw = GetTensorDim(input, params.data_format, 'H');
  TF_REQUIRES(
    FastBoundsCheck(input_rows_raw, std::numeric_limits<int>::max()),
    errors::InvalidArgument("Input rows too large"));
  const int input_rows = static_cast<int>(input_rows_raw);
  const int filter_rows = static_cast<int>(filter.dim_size(0));

  // The third dimension for input is columns/width.
  // The second dimension for filter is columns/width.
  const int64 input_cols_raw = GetTensorDim(input, params.data_format, 'W');
  TF_REQUIRES(
    FastBoundsCheck(input_cols_raw, std::numeric_limits<int>::max()),
    errors::InvalidArgument("Input cols too large"));
  const int input_cols = static_cast<int>(input_cols_raw);
  const int filter_cols = static_cast<int>(filter.dim_size(1));

  // The first dimension for input is batch.
  const int64 batch_raw = GetTensorDim(input, params.data_format, 'N');
  TF_REQUIRES(
    FastBoundsCheck(batch_raw, std::numeric_limits<int>::max()),
    errors::InvalidArgument("batch is too large"));
  const int batch = static_cast<int>(batch_raw);

  // Take the stride and dilation from the second and third dimensions only (we
  // do not support striding or dilation on the batch or depth dimension).
  const int stride_rows = GetTensorDim(params.strides, params.data_format, 'H');
  const int stride_cols = GetTensorDim(params.strides, params.data_format, 'W');
  const int dilation_rows = GetTensorDim(params.dilations, params.data_format, 'H');
  const int dilation_cols = GetTensorDim(params.dilations, params.data_format, 'W');

  int64 pad_rows_before, pad_rows_after, pad_cols_before, pad_cols_after;
  if (params.padding == Padding::EXPLICIT) {
    GetExplicitPaddingForDim(
      params.explicit_paddings, params.data_format, 'H', &pad_rows_before, &pad_rows_after);
    GetExplicitPaddingForDim(
      params.explicit_paddings, params.data_format, 'W', &pad_cols_before, &pad_cols_after);
  }

  // Compute windowed output sizes for rows and columns.
  int64 out_rows = 0, out_cols = 0;
  TF_RETURN_IF_ERROR(GetWindowedOutputSizeVerboseV2(
    input_rows, filter_rows, dilation_rows, stride_rows, params.padding, &out_rows,
    &pad_rows_before, &pad_rows_after));
  TF_RETURN_IF_ERROR(GetWindowedOutputSizeVerboseV2(
    input_cols, filter_cols, dilation_cols, stride_cols, params.padding, &out_cols,
    &pad_cols_before, &pad_cols_after));

  dimensions->batch = batch;
  dimensions->input_rows = input_rows;
  dimensions->input_cols = input_cols;
  dimensions->in_depth = in_depth;
  dimensions->filter_rows = filter_rows;
  dimensions->filter_cols = filter_cols;
  dimensions->patch_depth = patch_depth;
  dimensions->out_depth = out_depth;
  dimensions->stride_rows = stride_rows;
  dimensions->stride_cols = stride_cols;
  dimensions->dilation_rows = dilation_rows;
  dimensions->dilation_cols = dilation_cols;
  dimensions->out_rows = out_rows;
  dimensions->out_cols = out_cols;
  dimensions->pad_rows_before = pad_rows_before;
  dimensions->pad_rows_after = pad_rows_after;
  dimensions->pad_cols_before = pad_cols_before;
  dimensions->pad_cols_after = pad_cols_after;

  return Status::OK();
}

} // namespace tensorflow