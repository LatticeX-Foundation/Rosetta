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
#include "tensorflow/core/common_runtime/device.h"
#include "cc/modules/common/include/utils/rtt_logger.h"
#include "cc/modules/protocol/public/include/protocol_manager.h"
#include "tensorflow/core/util/work_sharder.h"

using rosetta::ProtocolManager;

namespace tensorflow {

// const static int SECURE_NUM_SPATIAL_DIMS = 2;

// A helper class to manage sizes and shapes for secure pooling operations.
struct SecurePoolParameter {
  SecurePoolParameter(OpKernelContext* context, const std::vector<int32>& ksize,
                      std::vector<int32>& stride, Padding padding,
                      TensorFormat data_format, const TensorShape& tensor_in_shape) {
    // At present only support NHWC (maybe support NCHW soon)
    OP_REQUIRES(context,
                GetTensorSpatialDims(tensor_in_shape.dims(), data_format) == 2,
                errors::InvalidArgument(
                    "tensor_in_shape must have 2 spatial dimensions. ",
                    tensor_in_shape.dims(), " ", data_format));

    this->data_format = data_format;
    depth = GetTensorDim(tensor_in_shape, data_format, 'C') *
            (data_format == FORMAT_NCHW_VECT_C ? 4 : 1);
    tensor_in_cols = GetTensorDim(tensor_in_shape, data_format, 'W');
    tensor_in_rows = GetTensorDim(tensor_in_shape, data_format, 'H');
    tensor_in_batch = GetTensorDim(tensor_in_shape, data_format, 'N');
    window_rows = GetTensorDim(ksize, data_format, 'H');
    window_cols = GetTensorDim(ksize, data_format, 'W');
    depth_window = GetTensorDim(ksize, data_format, 'C');
    row_stride = GetTensorDim(stride, data_format, 'H');
    col_stride = GetTensorDim(stride, data_format, 'W');
    depth_stride = GetTensorDim(stride, data_format, 'C');
    this->padding = padding;
    // We only support 2D pooling across width/height and depthwise
    // pooling, not a combination.
    OP_REQUIRES(context,
                (depth_window == 1 || (window_rows == 1 && window_cols == 1)),
                errors::Unimplemented(
                    "MaxPooling supports exactly one of pooling across depth "
                    "or pooling across width/height."));

    if (depth_window == 1) {
      OP_REQUIRES_OK(
          context, GetWindowedOutputSize(tensor_in_rows, window_rows, row_stride,
                                        padding, &out_height, &pad_rows));
      OP_REQUIRES_OK(
          context, GetWindowedOutputSize(tensor_in_cols, window_cols, col_stride,
                                        padding, &out_width, &pad_cols));
      pad_depth = 0;
      out_depth = depth;
    } else {
      // Our current version of depthwise max pooling does not support
      // any padding, and expects the depth_window to equal the
      // depth_stride (no overlapping).
      OP_REQUIRES(
          context, depth % depth_window == 0,
          errors::Unimplemented("Depthwise max pooling requires the depth "
                                "window to evenly divide the input depth"));
      OP_REQUIRES(
          context, depth_stride == depth_window,
          errors::Unimplemented("Depthwise max pooling requires the depth "
                                "window to equal the depth stride"));

      // The current version of depthwise max is only implemented on CPU.
      OP_REQUIRES(context, (DeviceType(static_cast<Device*>(context->device())
                                  ->attributes()
                                  .device_type()) == DeviceType(DEVICE_CPU)),
                  errors::Unimplemented("Depthwise max pooling is currently "
                                        "only implemented for CPU devices."));

      pad_depth = 0;
      out_depth = depth / depth_window;
    }
  }

  // Returns the shape of the output for "forward" pooling operations.
  TensorShape forward_output_shape() {
    if (depth_window == 1) {
      // Spatial pooling
      return ShapeFromFormat(data_format, tensor_in_batch, out_height, out_width,
                            depth);
    } else {
      // Depthwise pooling
      return TensorShape(
          {tensor_in_batch, tensor_in_rows, tensor_in_cols, out_depth});
    }
  }

  int depth;

  int tensor_in_cols;
  int tensor_in_rows;
  int tensor_in_batch;

  int window_rows;
  int window_cols;
  int depth_window;

  int row_stride;
  int col_stride;
  int depth_stride;

  int64 out_height;
  int64 out_width;
  int out_depth;

  int64 pad_rows;
  int64 pad_cols;
  int pad_depth;
  Padding padding;

  TensorFormat data_format;
};

class SecurePooling2DOp : public SecureOpKernel {
 public:
  SecurePooling2DOp(OpKernelConstruction* context)
      : SecureOpKernel(context) {
    // data_format only support NHWC
    string data_format;
    auto status = context->GetAttr("data_format", &data_format);
    if (status.ok()) {
      OP_REQUIRES(context, FormatFromString(data_format, &data_format_),
                  errors::InvalidArgument("Invalid data format"));
      OP_REQUIRES(
          context, data_format_ == FORMAT_NHWC,
          errors::InvalidArgument("Default MaxPoolingOp only supports NHWC ",
                                  "on device type ",
                                  DeviceTypeString(context->device_type())));
    } else {
      data_format_ = FORMAT_NHWC;
    }

    OP_REQUIRES_OK(context, context->GetAttr("ksize", &ksize_));
    OP_REQUIRES(context, ksize_.size() == 4,
                errors::InvalidArgument("Sliding window ksize field must "
                                        "specify 4 dimensions"));
    OP_REQUIRES_OK(context, context->GetAttr("strides", &stride_));
    OP_REQUIRES(context, stride_.size() == 4,
                errors::InvalidArgument("Sliding window stride field must "
                                        "specify 4 dimensions"));
    OP_REQUIRES_OK(context, context->GetAttr("padding", &padding_));
    OP_REQUIRES(context, ksize_[0] == 1 && stride_[0] == 1,
                errors::Unimplemented("Pooling is not yet supported on the batch dimension."));
    
    log_debug << "construct SecurePooling2DOp info\n"
              << "data_format:  " << data_format ;
              // << "padding: " << padding_ << std::endl 
              // << "ksize: " << ksize_[0] << "," << ksize_[1] << "," << ksize_[2] < "," << ksize_[3] 
              // << "strides: " << stride_ ;

  }

  void ComputeImpl(OpKernelContext* context) {
    log_info << "PoolOp compuate...";
    const Tensor& tensor_in = context->input(0);
    SecurePoolParameter params{context,  ksize_,      stride_,
                          padding_, FORMAT_NHWC, tensor_in.shape()};
    if (!context->status().ok()) {
      return;
    }

    Tensor* output = nullptr;
    OP_REQUIRES_OK(context, context->allocate_output(0, params.forward_output_shape(), &output));
    log_info << "--->    input shape: " << tensor_in.shape() << ", output shape: " << output->shape();
    if (params.depth_window > 1) {
      throw std::runtime_error("not support depth_windows>1 now!");
    } else {
      PoolCompuateBatchAll(context, output, tensor_in, params);
    }
  }

 private:
  // "VALID" only ever drops the right-most columns (or bottom-most rows).
  // "SAME" tries to pad evenly left and right, but if the amount of 
  // columns to be added is odd, it will add the extra column to the right, 
  // as is the case in this example (the same logic applies vertically: 
  // there may be an extra row of zeros at the bottom).
  // during the procedure all window blocks will be handled together
  void PoolCompuateBatchAll(OpKernelContext* context, Tensor* output,
                        const Tensor& tensor_in, const SecurePoolParameter& params) {
    const int32_t in_rows = params.tensor_in_rows;
    const int32_t in_cols = params.tensor_in_cols;
    const int32_t in_batches = params.tensor_in_batch;
    const int32_t in_channels = params.depth;
    const int32_t pad_rows = params.pad_rows;
    const int32_t pad_cols = params.pad_cols;
    const int32_t window_rows = params.window_rows;
    const int32_t window_cols = params.window_cols;
    const int32_t row_stride = params.row_stride;
    const int32_t col_stride = params.col_stride;
    const int32_t out_height = params.out_height;
    const int32_t out_width = params.out_width;

    // depth of max-pool windows is specified with 1
    auto in_flat = tensor_in.flat<string>();
    auto out_flat = output->flat<string>();
    

    log_info << "PoolCompuateBatch..., in_num: " << tensor_in.NumElements() << ", out_num: " << output->NumElements();
    log_debug << "PoolCompuateBatch..., in_rows: " << in_rows << ", in_cols: " << in_cols;
    log_debug << "PoolCompuateBatch..., in_batches: " << in_batches << ", in_channels: " << in_channels;
    log_debug << "PoolCompuateBatch..., pad_rows: " << pad_rows << ", pad_cols: " << pad_cols;

    
    // vectorization and reshape to data_format: "NCHW"
    // then get the max of row*cols sub-vector
    vector<string> out_vec(in_batches * out_height * out_width * in_channels);//output will remain the same channel, batches
    int32_t padding_cols = 0;
    int32_t padding_rows = 0;
    int32_t padding_left = 0;
    int32_t padding_right = 0;
    int32_t padding_upper = 0;
    int32_t padding_bottom = 0;
    int32_t row_upper = in_rows;
    int32_t col_upper = in_cols;
    
    if (params.padding == VALID) {//abandon elements
      padding_rows = (out_height * row_stride + (window_rows - row_stride)) - in_rows;// padding = stride * out + (window - stride) - input
      padding_cols = (out_width * col_stride + (window_cols - col_stride)) - in_cols;
      padding_right = padding_cols;
      padding_bottom = padding_rows;
      row_upper = out_height * row_stride;
      col_upper = out_width * col_stride;
    } else {//padding element with zeros
      padding_rows = out_height * row_stride + (window_rows - row_stride) - in_rows;// padding = stride * out + (window - stride) - input
      padding_cols = out_width * col_stride + (window_cols - col_stride) - in_cols;
      padding_right = (padding_cols+1)/2;
      padding_left = padding_cols/2;
      padding_bottom = (padding_rows+1)/2;
      padding_upper = padding_rows/2;
    }
    const int32_t expand_rows = in_rows + padding_rows;
    const int32_t expand_cols = in_cols + padding_cols;
    log_debug << "PoolCompuateBatch..., padding_rows: " << padding_rows << ", padding_cols: " << padding_cols 
            << ", padding_left: " << padding_left << ", padding_upper: "  << padding_upper;
    log_debug << "PoolCompuateBatch..., expand_rows: " << expand_rows << ", expand_cols: " << expand_cols;

    int32_t in_expand_index = 0;
    vector<string> in_expand_vec(in_batches * expand_rows * expand_cols *in_channels, "0");
    // reshape and padding, convert from "NHWC" to "NCHW" data_format
    for (size_t b = 0; b < in_batches; b++) {
      for (size_t n = 0; n < in_channels; n++) {
        for (size_t r = 0; r < expand_rows; r++) {
          if (r < padding_upper) {// jump one padding line
            in_expand_index += expand_cols;
          }

          if (r < in_rows && r < expand_rows) {
            for (size_t c = 0; c < expand_cols; c++) {
              if (c < padding_left) {// jump left padding (SAME padding)
                in_expand_index += padding_left;
              }

              if (c < in_cols && c < expand_cols) {// handle real input elements
                size_t in_index = b*in_rows*in_cols*in_channels + r*in_cols*in_channels + c*in_channels + n;
                in_expand_vec[in_expand_index++] = in_flat(in_index);
              } else if (c < expand_cols-padding_left) {// jump right padding (SAME padding)
                in_expand_index++;
              }
            }//cols
          } else if (r < expand_rows-padding_upper) {// jump bottom padding with cols steps (SAME padding)
            in_expand_index += expand_cols;//in_cols;
          }
        }//rows
      }//channels
    }//batches

    log_debug << "reshape to NCHW data_format and vectorization.";

    // calling secure ops, with "NCHW" data_format
    size_t max_batches = in_batches * in_channels * out_width * out_height;
    vector<string> window_blocks(max_batches * window_rows*window_cols);
    vector<string> window_out_blocks(max_batches);
    size_t windows_index = 0;
    vector<size_t> out_indexes(max_batches);
    for (size_t b = 0; b < in_batches; b++) {
      for (size_t n = 0; n < in_channels; n++) {
        for (size_t r = 0, o_h_index=0; r < row_upper; r += row_stride, o_h_index++) {
          for (size_t c = 0, o_w_index=0; c < col_upper; c += col_stride, o_w_index++) {
            // fill a window block
            size_t block_start_index = b*in_channels*expand_rows*expand_cols + n*expand_rows*expand_cols + r *expand_cols + c;
            size_t window_blocks_start_index = (b*in_channels*out_height*out_width + n*out_height*out_width + o_h_index*out_width + o_w_index) * window_rows*window_cols;
            size_t window_index = block_start_index;
            for (size_t win_r = 0; win_r < window_rows; ++win_r) {
              window_index = block_start_index + win_r * expand_cols;
              for (size_t win_c = 0; win_c < window_cols; ++win_c) {
                // cout <<  "push window, window_blocks_start_index: " << window_blocks_start_index << ", block_start_index:" << block_start_index << ", o_w_index: " << o_w_index << ", window_index: " << window_index << endl;
                window_blocks[window_blocks_start_index + win_r*window_cols+win_c] = in_expand_vec[window_index++];// window_block will batched
              }
            }

            // output will placed with "NHWC" data_format
            size_t out_index = b* out_height* out_width * in_channels + (r/row_stride)*out_width*in_channels + (c/col_stride)*in_channels + n;
            // log_debug << "pool procedure out_index: " << out_index;
            if (out_index >=  output->NumElements())
            {
              log_error << "!!!!! out_index: " << out_index << " out of range, NumElements: " << output->NumElements();
              throw std::runtime_error("out of range, maxpool output index");
            }
            
            out_indexes[windows_index++] = out_index;
          }
       }
      }
    }

    // batch pooling
    attrs_["rows"] = std::to_string(max_batches);
    attrs_["cols"] = std::to_string(window_rows*window_cols);
    PoolMatrixImpl(window_blocks, window_out_blocks, context);// TODO: handle the whole window blocks in batch
    for (size_t idx = 0; idx < max_batches; idx++)
    {
      out_flat(out_indexes[idx]) = std::move(window_out_blocks[idx]);
    }

    log_debug << "PoolCompuateBatchAll ok.";
  }//PoolComputeBatch

  virtual void PoolImpl(const vector<string>& window, vector<string>& output, OpKernelContext* context) = 0;
  virtual void PoolMatrixImpl(const vector<string>& window_blocks, vector<string>& window_out_blocks, OpKernelContext* context) = 0;

  std::vector<int32> ksize_;
  std::vector<int32> stride_;
  Padding padding_;
  TensorFormat data_format_;
};//SecurePooling2DOp

class SecureMaxPool2DOp : public SecurePooling2DOp {
 public:
  SecureMaxPool2DOp(OpKernelConstruction* ctx)
      : SecurePooling2DOp(ctx) {
  }

  void PoolImpl(const vector<string>& window, vector<string>& output, OpKernelContext* context) override {
    // window block calling secure protocol max ops
    log_debug << "SecureMaxPool2DOp...";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Max);
    ProtocolManager::Instance()
      ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->Max(window, output, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Max);
    log_debug << "SecureMaxPool2DOp OK.";
  }

  void PoolMatrixImpl(const vector<string>& window_blocks, vector<string>& window_out_blocks, OpKernelContext* context) override {
    // window block calling secure protocol max ops
    log_debug << "SecureMaxPool2DOp, matrix with axis...";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Max);
    ProtocolManager::Instance()
      ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->Max(window_blocks, window_out_blocks, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Max);
    log_debug << "SecureMaxPool2DOp, matrix with axis... OK.";
  }
};//MaxPool2D


class SecureAvgPool2DOp : public SecurePooling2DOp {
 public:
  SecureAvgPool2DOp(OpKernelConstruction* ctx)
       : SecurePooling2DOp(ctx) {
  }

  void PoolImpl(const vector<string>& window, vector<string>& output, OpKernelContext* context) override {
    log_debug << "SecureAvgPool2DOp...";
    // window block calling secure protocol mean ops
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Mean);
    ProtocolManager::Instance()
      ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->Mean(window, output, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Mean);
    log_debug << "SecureAvgPool2DOp OK.";
  }

  void PoolMatrixImpl(const vector<string>& window_blocks, vector<string>& window_out_blocks, OpKernelContext* context) override {
    // window block calling secure protocol max ops
    log_debug << "SecureMeanPool2DOp, matrix with axis...";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Mean);
    ProtocolManager::Instance()
      ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->Mean(window_blocks, window_out_blocks, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Mean);
    log_debug << "SecureMeanPool2DOp, matrix with axis... OK.";
  }
};//AvgPool2D

REGISTER_STR_CPU_KERNEL(SecureAvgPool, SecureAvgPool2DOp);
REGISTER_STR_CPU_KERNEL(SecureMaxPool, SecureMaxPool2DOp);

}
