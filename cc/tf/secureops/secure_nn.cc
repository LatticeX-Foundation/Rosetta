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
#include "cc/tf/secureops/secure_conv2d.h"
#include "cc/modules/common/include/utils/rtt_logger.h"
#include "cc/modules/protocol/public/include/protocol_manager.h"

using rosetta::ProtocolManager;

#include "third_party/eigen3/unsupported/Eigen/CXX11/Tensor"
#include "tensorflow/core/framework/op_kernel.h"
#include "tensorflow/core/framework/register_types.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/framework/tensor_types.h"
#include "tensorflow/core/util/env_var.h"
#include "tensorflow/core/util/tensor_format.h"

#include <cmath>

// Binary OP: Add/Sub/Mul/Div/...
namespace tensorflow {

class SecureSigmoidOp : public SecureUnaryOp {
 private:
  /* data */
 public:
  SecureSigmoidOp(OpKernelConstruction* context) : SecureUnaryOp(context) {}
  ~SecureSigmoidOp() {}

  int UnaryCompute(const vector<string>& input, vector<string>& output, OpKernelContext* context) {
    log_debug << "--> Sigmoid OpKernel compute.";
    output.resize(input.size());
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Sigmoid);
    ProtocolManager::Instance()
      ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->Sigmoid(input, output, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Sigmoid);
    log_debug << "Sigmoid OpKernel compute ok. <--";
    return 0;
  }
};

class SecureSigmoidCrossEntropyOp : public SecureBinaryOp<BinaryOpState> {
 private:
  /* data */
 public:
  SecureSigmoidCrossEntropyOp(OpKernelConstruction* context) : SecureBinaryOp(context) {}
  ~SecureSigmoidCrossEntropyOp() {}

  int BinaryCompute(const vector<string>& in1, const vector<string>& in2, vector<string>& output, OpKernelContext* context) {
    log_debug << "--> SigmoidCrossEntropy OpKernel compute.";
    output.resize(in1.size());
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(SigmoidCrossEntropy);
    ProtocolManager::Instance()
      ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->SigmoidCrossEntropy(in1, in2, output, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(SigmoidCrossEntropy);
    log_debug << "SigmoidCrossEntropy OpKernel compute ok. <--";
    return 0;
  }
};

class SecureReluOp : public SecureUnaryOp {
 private:
  /* data */
 public:
  SecureReluOp(OpKernelConstruction* context) : SecureUnaryOp(context) {}
  ~SecureReluOp() {}

  int UnaryCompute(const vector<string>& input, vector<string>& output, OpKernelContext* context) {
    log_debug << "--> Relu OpKernel compute.";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Relu);
    ProtocolManager::Instance()
      ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->Relu(input, output, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Relu);
    log_debug << "Relu OpKernel compute ok. <--";
    return 0;
  }
};

class SecureReluPrimeOp : public SecureUnaryOp {
 private:
  /* data */
 public:
  SecureReluPrimeOp(OpKernelConstruction* context) : SecureUnaryOp(context) {}
  ~SecureReluPrimeOp() {}

  int UnaryCompute(const vector<string>& input, vector<string>& output, OpKernelContext* context) {
    log_debug << "--> ReluPrime OpKernel compute.";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(ReluPrime);
    ProtocolManager::Instance()
      ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->ReluPrime(input, output, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(ReluPrime);
    log_debug << "ReluPrime OpKernel compute ok. <--";
    return 0;
  }
};

//--------------------------------------------------------------------------
class SecureConv2DOp : public SecureOpKernel {
 public:
  SecureConv2DOp(OpKernelConstruction* context) : SecureOpKernel(context) {
    OP_REQUIRES_OK(context, InitConv2DParameters(context, &params_));

    OP_REQUIRES_OK(context, context->GetAttr("use_cudnn_on_gpu", &use_cudnn_));
    //use_cudnn_ &= CanUseCudnn();
    //cudnn_use_autotune_ = CudnnUseAutotune();
    // now not support gpu
    use_cudnn_ = false;
    cudnn_use_autotune_ = false;
  }
  ~SecureConv2DOp() {}

  void ComputeImpl(OpKernelContext* context) {
    log_info << "--> SecureConv2DOp OpKernel compute.";
    // Input tensor is of the following dimensions:
    // [ batch, in_rows, in_cols, in_depth ]
    const Tensor& input = context->input(0);
    const TensorShape& input_shape = input.shape();

    log_debug << "input_shape.num_elements:" << input_shape.num_elements();
    for (int i = 0; i < input_shape.dims(); i++) {
      log_debug << "input dim " << i << ":" << input_shape.dim_size(i) ;
    }

    const auto& input_flat = input.flat<string>();
    for (int i = 0; i < input.NumElements(); i++) {
      log_debug << "CCCC input i:" << i << "--->" << input_flat(i) ;
    }

    // Input filter is of the following dimensions:
    // [ filter_rows, filter_cols, in_depth, out_depth]
    const Tensor& filter = context->input(1);
    const TensorShape& filter_shape = filter.shape();

    log_debug << "filter_shape.num_elements:" << filter_shape.num_elements();
    for (int i = 0; i < filter_shape.dims(); i++) {
      log_debug << "filter dim " << i << ":" << filter_shape.dim_size(i) ;
    }

    const auto& filter_flat = filter.flat<string>();
    for (int i = 0; i < filter.NumElements(); i++) {
      log_debug << "CCCC filter i:" << i << "--->" << filter_flat(i) ;
    }

    // dimensions, data_format, etc.
    Conv2DDimensions dimensions;
    OP_REQUIRES_OK(context, ComputeConv2DDimension(params_, input, filter, &dimensions));

    // Shape
    TensorShape out_shape = ShapeFromFormat(
      params_.data_format, dimensions.batch, dimensions.out_rows, dimensions.out_cols,
      dimensions.out_depth);

    // Output tensor is of the following dimensions:
    // [ in_batch, out_rows, out_cols, out_depth ]
    Tensor* output = nullptr;
    OP_REQUIRES_OK(context, context->allocate_output(0, out_shape, &output));
    auto out_flat = output->flat<string>();

    VLOG(2) << "Conv2D: in_depth = " << dimensions.in_depth
            << ", patch_depth = " << dimensions.patch_depth
            << ", input_cols = " << dimensions.input_cols
            << ", filter_cols = " << dimensions.filter_cols
            << ", input_rows = " << dimensions.input_rows
            << ", filter_rows = " << dimensions.filter_rows
            << ", stride_rows = " << dimensions.stride_rows
            << ", stride_cols = " << dimensions.stride_cols
            << ", dilation_rows = " << dimensions.dilation_rows
            << ", dilation_cols = " << dimensions.dilation_cols
            << ", out_rows = " << dimensions.out_rows << ", out_cols = " << dimensions.out_cols
            << ", out_depth = " << dimensions.out_depth;
    VLOG(2) << "Conv2D: padding = " << params_.padding
            << ", pad_rows_before = " << dimensions.pad_rows_before
            << ", pad_rows_after = " << dimensions.pad_rows_after
            << ", pad_cols_before = " << dimensions.pad_cols_before
            << ", pad_cols_after = " << dimensions.pad_cols_after;

    // If there is nothing to compute, return.
    if (out_shape.num_elements() == 0) {
      return;
    }

    log_debug << "out_shape.dims:" << out_shape.dims();
    log_debug << "out_shape.num_elements:" << out_shape.num_elements();
    for (int i = 0; i < out_shape.dims(); i++) {
      log_debug << "dim " << i << ":" << out_shape.dim_size(i) ;
    }
    auto vs = out_shape.dim_sizes();
    assert(vs.size() == out_shape.dims());
    assert(vs.size() == 4);
    for (int i = 0; i < vs.size(); i++) {
      log_debug << this << ",i:" << i << "->" << vs[i] ;
    }

    //! @todo real compute

    // shape
    auto ds = input_shape.dim_sizes();
    auto ks = filter_shape.dim_sizes();

    // strides
    int sr = dimensions.stride_rows;
    int sc = dimensions.stride_cols;

    // padding
    int64_t pad_rows_before = dimensions.pad_rows_before;
    int64_t pad_rows_after = dimensions.pad_rows_after;
    int64_t pad_cols_before = dimensions.pad_cols_before;
    int64_t pad_cols_after = dimensions.pad_cols_after;

    // image size (padding, for Padding::SAME)
    int64_t img_h = ds[1] + pad_rows_before + pad_rows_after;
    int64_t img_w = ds[2] + pad_cols_before + pad_cols_after;

    // input
    int64_t in_batch = dimensions.batch;
    int64_t in_rows = dimensions.input_rows;
    int64_t in_cols = dimensions.input_cols;
    int64_t in_depth = dimensions.in_depth;

    // filter(kernel)
    int64_t filter_rows = dimensions.filter_rows;
    int64_t filter_cols = dimensions.filter_cols;
    int64_t stride_cols = dimensions.stride_cols;
    int64_t stride_rows = dimensions.stride_rows;

    // output
    int64_t out_batch = dimensions.batch;
    int64_t out_rows = dimensions.out_rows;
    int64_t out_cols = dimensions.out_cols;
    int64_t out_depth = dimensions.out_depth;

    // how many zero-padding
    int64_t zero_padding_nums =
      in_batch * img_h * img_w * in_depth - in_batch * in_rows * in_cols * in_depth;

    /**
     * @note The details, notes and demo version, see commit id or before e8d55dd3bd80dcaefeed59f7e0766f99735595c2
     */

    // for pading
    string szero_padding("");
    if (zero_padding_nums > 0) {
      vector<double> dzero(1, 0);
      vector<string> szero(1);
      SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(PublicInput);
      auto protocol = ProtocolManager::Instance()
        ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()));
      protocol->GetOps(msg_id())->PublicInput(protocol->GetNetHandler()->GetNodeId(0), dzero, szero);
      SECURE_OP_CALL_PROTOCOL_OP_STATS_END(PublicInput);

      szero_padding = szero[0];
      log_debug << "     zero_padding_nums:" << zero_padding_nums ;
    }

    // SimpleTimer timerx;
    //
    // pre-process input
    //cout << "input:" << endl;
    vector<vector<vector<string>>> img_channels; // N*((H*W)*C)
    for (int b = 0; b < ds[0]; b++) { // batch
      vector<vector<string>> img_channel; // ((H*W)*C)
      for (int c = 0; c < ds[3]; c++) { // input channels
        int64_t img_i = 0;
        vector<string> img(img_h * img_w); // (H*W)*1

        // pad_rows_before
        for (int64_t pr = 0; pr < pad_rows_before; pr++) {
          for (int64_t w = 0; w < ds[2] + pad_cols_before + pad_cols_after; w++) {
            img[img_i * img_w + w] = szero_padding;
          }
          img_i++;
        }

        // real data
        for (int64_t h = 0; h < ds[1]; h++) { // height
          // pad_cols_before
          for (int64_t pc = 0; pc < pad_cols_before; pc++) {
            img[img_i * img_w + pc] = szero_padding;
          }
          for (int64_t w = 0; w < ds[2]; w++) { // width
            int64_t index = b * ds[1] * ds[2] * ds[3] + h * ds[2] * ds[3] + w * ds[3] + c;
            img[img_i * img_w + pad_cols_before + w] = input_flat(index);
          }
          // pad_cols_after
          for (int64_t pc = 0; pc < pad_cols_after; pc++) {
            img[img_i * img_w + ds[2] + pad_cols_before] = szero_padding;
          }
          img_i++;
        }
        // pad_rows_after
        for (int64_t pr = 0; pr < pad_rows_after; pr++) {
          for (int64_t w = 0; w < ds[2] + pad_cols_before + pad_cols_after; w++) {
            img[img_i * img_w + w] = szero_padding;
          }
          img_i++;
        }
        img_channel.push_back(img);
      }
      img_channels.push_back(img_channel);
    }
    // log_error << __FUNCTION__ << " 1timerx:" << timerx.elapse() ;
    // timerx.start();

    // martix version

    // 0/3/1/2  --> N/C/H/W

    // about inputs [in_batch * out_rows * out_cols, filter_rows * filter_cols * in_depth]
    // N,C,H,W
    vector<vector<string>> res_inputs(out_batch * out_rows * out_cols);
    for (int64_t b = 0; b < in_batch; b++) {
      const auto& img_channel = img_channels[b];

      //for (int64_t oc = 0; oc < out_depth; oc++) {
      int64_t h_beg = 0, h_end = 0, w_beg = 0, w_end = 0;
      // ===1 out_rows * out_cols * out_depth(1)
      for (int64_t oh = 0; oh < out_rows; oh++) {
        for (int64_t ow = 0; ow < out_cols; ow++) {
          vector<string> res_input(filter_rows * filter_cols * in_depth);

          for (int64_t c = 0; c < in_depth; c++) {
            const auto& img = img_channel[c];

            // one filter size: filter[filter_rows, filter_cols] dotproduct img[filter_rows, filter_cols]
            h_beg = oh * stride_cols;
            h_end = filter_rows + oh * stride_cols;
            w_beg = ow * stride_rows;
            w_end = filter_cols + ow * stride_rows;
            for (int64_t h = h_beg; h < h_end /*img_h*/; h++) { // from kr to kr+filter_rows
              for (int64_t w = w_beg; w < w_end /*img_w*/; w++) { // from kc to kc+filter_cols
                // get the index of img
                int64_t img_index = h * img_w + w;

                // get the index of res_input
                int64_t resk_index = (h - h_beg) * filter_cols + (w - w_beg);
                int64_t res_index = c * filter_rows * filter_cols + resk_index;

                res_input[res_index] = img[img_index];
              }
            }
          }
          int64_t out_index1 =
            b * out_rows * out_cols * out_depth + oh * out_cols * out_depth + ow * out_depth;

          int64_t out_index = out_index1 / out_depth + out_index1 % out_depth;
          res_inputs[out_index] = res_input;
        }
        // ===1
      }
      //}
    }

    // log_error << __FUNCTION__ << " 2timerx:" << timerx.elapse() ;
    // timerx.start();

#if 0
    // about filters [filter_rows * filter_cols * in_depth, out_depth]
    vector<vector<string>> res_filtrs(out_depth);
    for (int64_t oc = 0; oc < ks[3]; oc++) { // output channels
      vector<string> filter(ks[2] * ks[0] * ks[1]);
      int64_t i = 0;
      for (int64_t ic = 0; ic < ks[2]; ic++) { // input channels
        for (int64_t h = 0; h < ks[0]; h++) { // height
          for (int64_t w = 0; w < ks[1]; w++) { // width
            int64_t index = h * ks[1] * ks[2] * ks[3] + w * ks[2] * ks[3] + ic * ks[3] + oc;
            filter[i++] = filter_flat(index);
          }
        }
      }
      res_filtrs[oc] = filter;
    }

    vector<string> tmpa, tmpb;
    tmpb.resize(out_depth * filter_rows * filter_cols * in_depth, res_filtrs[0][0]);
    for (int64_t i = 0; i < res_inputs.size(); i++) {
      tmpa.insert(tmpa.end(), res_inputs[i].begin(), res_inputs[i].end());
    }
    for (int64_t i = 0; i < out_depth; i++) {
      for (int64_t j = 0; j < filter_rows * filter_cols * in_depth; j++) {
        int64_t index = j * out_depth + i;
        tmpb[index] = res_filtrs[i][j];
      }
    }
#else // optimized
    // about filters [filter_rows * filter_cols * in_depth, out_depth]
    vector<string> tmpa;
    for (int64_t i = 0; i < res_inputs.size(); i++) {
      tmpa.insert(tmpa.end(), res_inputs[i].begin(), res_inputs[i].end());
    }

    vector<string> tmpb;
    tmpb.resize(out_depth * filter_rows * filter_cols * in_depth); //, filter_flat(0));

    int64_t JJ = ks[2] * ks[0] * ks[1];
    vector<vector<string>> res_filtrs(out_depth, vector<string>(JJ, filter_flat(0)));
    vector<int64_t> indexs;
    for (int64_t ic = 0; ic < ks[2]; ic++) { // input channels
      for (int64_t h = 0; h < ks[0]; h++) { // height
        for (int64_t w = 0; w < ks[1]; w++) { // width
          int64_t index = h * ks[1] * ks[2] * ks[3] + w * ks[2] * ks[3] + ic * ks[3]; // + oc;
          indexs.push_back(index);
        }
      }
    }
    int TN = std::thread::hardware_concurrency();
    TN = (TN <= 0) ? 4 : TN;
    int64_t totals = ks[3];
    int64_t block = (totals + TN - 1) / TN;

    vector<std::thread> tts(TN);
    auto ff = [&](int tid, int64_t start, int64_t end) {
      for (int64_t i = start; i < end; i++) { // output channels
        for (int64_t j = 0; j < indexs.size(); j++) {
          res_filtrs[i][j] = filter_flat(indexs[j] + i);
          //cout << "---> i:" << i << ",j:" << j << ",k:" << indexs[j] + i << endl;
        }
        for (int64_t j = 0; j < filter_rows * filter_cols * in_depth; j++) {
          int64_t index = j * out_depth + i;
          tmpb[index] = std::move(res_filtrs[i][j]);
          //cout << "<--- i:" << i << ",j:" << j << ",k:" << index << endl;
        }
      }
    };

    for (int i = 0; i < TN; i++) {
      int64_t start = block * i;
      int64_t end = block * (i + 1);
      if (end > totals)
        end = totals;
      tts[i] = std::thread(ff, i, start, end);
    }
    for (int i = 0; i < TN; i++) {
      tts[i].join();
    }
#endif

    // log_error << __FUNCTION__ << " 3timerx:" << timerx.elapse() << ", out_depth:" << out_depth
    //           << ",ks[2]:" << ks[2] << ",ks[0]:" << ks[0] << ",ks[1]:" << ks[2] ;

    attrs_["m"] = to_string(in_batch * out_rows * out_cols);
    attrs_["k"] = to_string(filter_rows * filter_cols * in_depth);
    attrs_["n"] = to_string(out_depth);
    attrs_["rh_is_const"] = is_public_or_constant_input_by_restore_mode(context) ? "1" : "0";

    // log_error << __FUNCTION__ << " 4timerx:" << timerx.elapse() << " m:" << attrs_["m"]
    //           << " k:" << attrs_["k"] << " n:" << attrs_["n"] << " padding:" << zero_padding_nums
    //           ;
    log_debug << __FUNCTION__ << " m:" << attrs_["m"] ;
    log_debug << __FUNCTION__ << " k:" << attrs_["k"] ;
    log_debug << __FUNCTION__ << " n:" << attrs_["n"] ;
    log_debug << __FUNCTION__ << " rh_is_const:" << attrs_["rh_is_const"] ;

    // [ out_batch, out_rows, out_cols, out_depth ]
    vector<string> tmpc; //(out_batch * out_rows * out_cols * out_depth); // N*((H*W)*C)
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Matmul);
    ProtocolManager::Instance()
      ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->Matmul(tmpa, tmpb, tmpc, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Matmul);

    // assign
    for (int64_t oc = 0; oc < out_depth; oc++) {
      for (int64_t n = 0; n < in_batch * out_rows * out_cols; n++) {
        int64_t index = n * out_depth + oc;
        out_flat(index) = std::move(tmpc[index]);
      }
    }

    log_debug << "SecureConv2DOp OpKernel compute ok. <--";
  }

 private:
  Conv2DParameters params_;
  bool use_cudnn_;
  bool cudnn_use_autotune_;

  //LaunchConv2DOp<Device, T> launcher_;
};

template <int NDIMS>
void ASSIGN_TENSOR2(
  const Tensor& input,
  const Tensor& bias,
  Tensor& in0_tensor,
  Tensor& in1_tensor,
  const CPUDevice& eigen_device) {
  auto _input = input.tensor<string, NDIMS>();
  auto _bias = bias.vec<string>();
  const int64_t bias_size = _bias.dimension(0);
  const int64_t rest_size = _input.size() / bias_size;
  Eigen::DSizes<int64_t, 1> one_d(_input.size());
  Eigen::DSizes<int64_t, 1> bcast(rest_size);

  in0_tensor.tensor<string, NDIMS>().reshape(one_d).device(eigen_device) = _input.reshape(one_d);
  in1_tensor.tensor<string, NDIMS>().reshape(one_d).device(eigen_device) =
    _bias.broadcast(bcast).reshape(one_d);
}

class SecureBiasAddOp : public SecureBinaryOp<BinaryOpState> {
 public:
  explicit SecureBiasAddOp(OpKernelConstruction* context) : SecureBinaryOp(context) {
    string data_format;
    if (context->GetAttr("data_format", &data_format).ok()) {
      OP_REQUIRES(
        context, FormatFromString(data_format, &data_format_),
        errors::InvalidArgument("Invalid data format"));
    }
    // At present we only support "NHWC"
    OP_REQUIRES(
      context, data_format_ == FORMAT_NHWC,
      errors::InvalidArgument("Bad data_format ", data_format_, ", only support NHWC now."));
  }
  ~SecureBiasAddOp() {}

  int BinaryCompute(const vector<string>& in1, const vector<string>& in2, vector<string>& output, OpKernelContext* context) {
    log_debug << "--> SecureBiasAddOp OpKernel compute.";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(BiasAdd);
    int ret = ProtocolManager::Instance()
                ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
                ->GetOps(msg_id())
                ->Add(in1, in2, output, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(BiasAdd);
    log_debug << "SecureBiasAddOp OpKernel compute ok. <--";
    return ret;
  }

  void ComputeImpl(OpKernelContext* context) override {
    const Tensor& input = context->input(0);
    const Tensor& bias = context->input(1);

    OP_REQUIRES(
      context, TensorShapeUtils::IsMatrixOrHigher(input.shape()),
      errors::InvalidArgument("Input tensor must be at least 2D: ", input.shape().DebugString()));
    OP_REQUIRES(
      context, TensorShapeUtils::IsVector(bias.shape()),
      errors::InvalidArgument("Biases must be 1D: ", bias.shape().DebugString()));

    // Added by intel_tf to support NCHW on CPU regardless of MKL used or not.
    size_t channel_dim;
    if (data_format_ == FORMAT_NCHW) {
      channel_dim = 1; // NCHW always have channel dim in 1 (with 3, 4, 5
        // dimensions data).
    } else {
      channel_dim = input.shape().dims() - 1; // End of code by intel_tf.
    }

    OP_REQUIRES(
      context, bias.shape().dim_size(0) == input.shape().dim_size(channel_dim),
      errors::InvalidArgument(
        "Must provide as many biases as the last dimension "
        "of the input tensor: ",
        bias.shape().DebugString(), " vs. ", input.shape().DebugString()));

    Tensor* out = nullptr;
    OP_REQUIRES_OK(context, context->forward_input_or_allocate_output({0}, 0, input.shape(), &out));

    if (input.NumElements() == 0)
      return;

#if 0
    // Added by intel_tf to support NCHW on CPU regardless of MKL used or not.
    if (data_format_ == FORMAT_NCHW) {
      //! @ref tensorflow/core/kernels/bias_op.cc
      return;
    }  // End of code by intel_tf.
#endif

    auto eigen_device = context->eigen_device<CPUDevice>();
    auto out_shape = out->shape();
    Tensor in0_tensor;
    Tensor in1_tensor;
    OP_REQUIRES_OK(context, context->allocate_temp(DT_STRING, out_shape, &in0_tensor));
    OP_REQUIRES_OK(context, context->allocate_temp(DT_STRING, out_shape, &in1_tensor));

    switch (input.shape().dims()) {
      case 2:
        ASSIGN_TENSOR2<2>(input, bias, in0_tensor, in1_tensor, eigen_device);
        break;
      case 3:
        ASSIGN_TENSOR2<3>(input, bias, in0_tensor, in1_tensor, eigen_device);
        break;
      case 4:
        ASSIGN_TENSOR2<4>(input, bias, in0_tensor, in1_tensor, eigen_device);
        break;
      case 5:
        ASSIGN_TENSOR2<5>(input, bias, in0_tensor, in1_tensor, eigen_device);
        break;
      default:
        OP_REQUIRES(
          context, false,
          errors::InvalidArgument("Only ranks up to 5 supported: ", input.shape().DebugString()));
    }

    int64_t size = input.NumElements();
    vector<string> input0(size);
    vector<string> input1(size);
    vector<string> output(size);

    //! @note the same as binary op beg
    const auto& in0_flat = in0_tensor.flat<string>();
    const auto& in1_flat = in1_tensor.flat<string>();
    for (int64_t i = 0; i < size; i++) {
      input0[i] = in0_flat(i);
      input1[i] = in1_flat(i);
    }

    // fill attributes
    attrs_["lh_is_const"] = lh_is_const_ ? "1" : "0";
    attrs_["rh_is_const"] = rh_is_const_ ? "1" : "0";
    attrs_["rh_is_const"] = is_public_or_constant_input_by_restore_mode(context) ? "1" : "0";

    // compute with protocol
    BinaryCompute(input0, input1, output, context);

    // set output
    auto out_flat = out->flat<string>();
    for (int64_t i = 0; i < size; i++) {
      out_flat(i) = std::move(output[i]);
    }
    //! @note the same as binary op end
  }

 private:
  TensorFormat data_format_;
};

class SecureL2LossOp : public SecureUnaryOp {
 private:
  /* data */
 public:
  SecureL2LossOp(OpKernelConstruction* context) : SecureUnaryOp(context) {}
  ~SecureL2LossOp() {}

  int UnaryCompute(const vector<string>& input, vector<string>& output, OpKernelContext* context) {
    log_debug << "--> SecureL2LossOp OpKernel compute.";
    log_debug << "SecureL2LossOp OpKernel compute ok. <--";
    return 0;
  }
};

class SecureFusedBatchNormOp : public SecureOpKernel {
 private:
  float epsilon_;
  TensorFormat tensor_format_;
  bool is_training_;

 public:
  SecureFusedBatchNormOp(OpKernelConstruction* context) : SecureOpKernel(context) {
    float epsilon;
    OP_REQUIRES_OK(context, context->GetAttr("epsilon", &epsilon));
    epsilon_ = float(epsilon);
    string tensor_format;
    OP_REQUIRES_OK(context, context->GetAttr("data_format", &tensor_format));
    OP_REQUIRES(
      context, FormatFromString(tensor_format, &tensor_format_),
      errors::InvalidArgument("Invalid data format"));
    OP_REQUIRES_OK(context, context->GetAttr("is_training", &is_training_));
  }

  ~SecureFusedBatchNormOp() {}

  void ComputeImpl(OpKernelContext* context) {
    log_debug << "--> SecureFusedBatchNormOp OpKernel compute.";

    const Tensor& x = context->input(0);
    const Tensor& scale = context->input(1);
    const Tensor& offset = context->input(2);
    const Tensor& estimated_mean = context->input(3);
    const Tensor& estimated_variance = context->input(4);

    OP_REQUIRES(
      context, x.dims() == 4,
      errors::InvalidArgument("input must be 4-dimensional", x.shape().DebugString()));
    OP_REQUIRES(
      context, scale.dims() == 1,
      errors::InvalidArgument("scale must be 1-dimensional", scale.shape().DebugString()));
    OP_REQUIRES(
      context, offset.dims() == 1,
      errors::InvalidArgument("offset must be 1-dimensional", offset.shape().DebugString()));
    OP_REQUIRES(
      context, estimated_mean.dims() == 1,
      errors::InvalidArgument(
        "estimated_mean must be 1-dimensional", estimated_mean.shape().DebugString()));
    OP_REQUIRES(
      context, estimated_variance.dims() == 1,
      errors::InvalidArgument(
        "estimated_variance must be 1-dimensional", estimated_variance.shape().DebugString()));
    // training is not supported for NOW.
    if (is_training_) {
      OP_REQUIRES(
        context, estimated_mean.dim_size(0) == 0,
        errors::InvalidArgument(
          "estimated_mean must be empty for training", estimated_mean.shape().DebugString()));
      OP_REQUIRES(
        context, estimated_variance.dim_size(0) == 0,
        errors::InvalidArgument(
          "estimated_variance must be empty for training",
          estimated_variance.shape().DebugString()));
    }

    Tensor* y = nullptr;
    OP_REQUIRES_OK(context, context->forward_input_or_allocate_output({0}, 0, x.shape(), &y));
    Tensor* batch_mean = nullptr;
    OP_REQUIRES_OK(context, context->allocate_output(1, scale.shape(), &batch_mean));
    Tensor* batch_var = nullptr;
    OP_REQUIRES_OK(context, context->allocate_output(2, scale.shape(), &batch_var));
    Tensor* saved_mean = nullptr;
    OP_REQUIRES_OK(context, context->allocate_output(3, scale.shape(), &saved_mean));
    Tensor* saved_maybe_inv_var = nullptr;
    OP_REQUIRES_OK(context, context->allocate_output(4, scale.shape(), &saved_maybe_inv_var));

    log_info << " begin real execution!" ;
    ////// the following is adapted from `FusedBatchNorm<CPUDevice, T, U>` in native TF.
    OP_REQUIRES(
      context, tensor_format_ == FORMAT_NHWC,
      errors::Internal("The CPU implementation of FusedBatchNorm "
                       "only supports NHWC tensor format for now."));

    const int depth = x.dim_size(3);
    const int size = x.NumElements();
    const int rest_size = size / depth;
    // Eigen::DSizes<Eigen::Index, 2> rest_by_depth(rest_size, depth);

    // Todo: these are for training:
    const int rest_size_minus_one = (rest_size > 1) ? (rest_size - 1) : 1;
    double rest_size_inv = static_cast<double>(1.0f / static_cast<double>(rest_size));
    // This adjustment is for Bessel's correction
    double rest_size_adjust =
      static_cast<double>(rest_size) / static_cast<double>(rest_size_minus_one);

    Tensor mean;
    OP_REQUIRES_OK(context, context->allocate_temp(DT_STRING, TensorShape({1, depth}), &mean));

    Tensor variance;
    OP_REQUIRES_OK(context, context->allocate_temp(DT_STRING, TensorShape({1, depth}), &variance));

    if (is_training_) {
      log_error << "Not supported yet!";
      // mean.device(d) = (x_rest_by_depth.sum(reduce_dims) * rest_size_inv);
      // batch_mean.device(d) = mean;
      // saved_mean.device(d) = mean;

      // variance.device(d) = x_centered.square().sum(reduce_dims) * rest_size_inv;
      // batch_var.device(d) = variance * rest_size_adjust;
      // saved_var.device(d) = variance;
    } else {
      mean = estimated_mean;
      variance = estimated_variance;
    }

    // for now, this is not the most efficient one, since we not use vectorization.
    // vector<vector<string>> all_input(depth, vector<string>(rest_size));

    attrs_["rh_is_const"] = is_public_or_constant_input_by_restore_mode(context) ? "1" : "0";
    log_info << __FUNCTION__ << " rh_is_const:" << attrs_["rh_is_const"] ;
    bool rh_is_const = (attrs_["rh_is_const"] == "1");

    vector<string> inner_flat_x(size);

    const auto& x_flat = x.flat<string>();
    const auto& mean_flat = mean.flat<string>();
    const auto& var_flat = variance.flat<string>();
    const auto& scale_flat = scale.flat<string>();
    const auto& offset_flat = offset.flat<string>();

    auto ops = ProtocolManager::Instance()
                ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
                ->GetOps(msg_id());

    // for debuging:
    #if 0
    vector<string> vec_mean(depth);
    vector<string> vec_var(depth);
    vector<string> vec_scale(depth);
    vector<string> vec_offset(depth);
    for (int i = 0; i < depth; ++i) {
      vec_mean[i] = mean_flat(i);
      vec_var[i] = var_flat(i);
      vec_scale[i] = scale_flat(i);
      vec_offset[i] = offset_flat(i);
    }

    if(rh_is_const) {

      vector<double> const_vec_mean(depth);
      vector<double> const_vec_var(depth);
      vector<double> const_vec_scale(depth);
      vector<double> const_vec_offset(depth);
      for(int i =0; i < depth; ++i) {
        #if !use_literal_value_binary_version
          memcpy((char*)&const_vec_var[i], (char*)var_flat(i).c_str(), sizeof(double));
          memcpy((char*)&const_vec_scale[i], (char*)scale_flat(i).c_str(), sizeof(double));
          memcpy((char*)&const_vec_mean[i], (char*)mean_flat(i).c_str(), sizeof(double));
          memcpy((char*)&const_vec_offset[i], (char*)offset_flat(i).c_str(), sizeof(double));          
        #else
          const_vec_var[i] = to_double(var_flat(i).c_str());
          const_vec_scale[i]= to_double(scale_flat(i).c_str());
          const_vec_mean[i] = to_double(mean_flat(i).c_str());
          const_vec_offset[i] = to_double(offset_flat(i).c_str());
        #endif
      }
      print_vec(const_vec_mean, 20, "debug BN const Mean:");
      print_vec(const_vec_var, 20,  "debug BN const var:");
      print_vec(const_vec_scale, 20, "debug BN const scale:");
      print_vec(const_vec_offset, 20, "debug BN const offset:");
      cout << "debug BN const epsilon_: " << epsilon_ << endl;
    } else {
      vector<double> revealed_para(depth);
      ops->Reveal(vec_mean, revealed_para);
      print_vec(revealed_para, 20, "debug BN private Mean:");

      ops->Reveal(vec_var, revealed_para);
      print_vec(revealed_para, 20,  "debug BN private var:");

      ops->Reveal(vec_scale, revealed_para);
      print_vec(revealed_para, 20, "debug BN private scale:");

      ops->Reveal(vec_offset, revealed_para);
      print_vec(revealed_para, 20, "debug BN private offset:");

      cout << "debug BN private epsilon_: " << epsilon_ << endl;
    }
    #endif

    vector<string> scaling_factor(depth);
    if (rh_is_const) {
#if !use_literal_value_binary_version
      std::string stmp(sizeof(double) + 1, '$');
#endif
      // stabilized variance
      double tmp_var, tmp_scale, tmp_val;
      for (int i = 0; i < depth; ++i) {
#if !use_literal_value_binary_version
        memcpy((char*)&tmp_var, (char*)var_flat(i).c_str(), sizeof(double));
        memcpy((char*)&tmp_scale, (char*)scale_flat(i).c_str(), sizeof(double));
        
#else
        tmp_var = to_double(var_flat(i).c_str());
        tmp_scale = to_double(scale_flat(i).c_str());
#endif
        tmp_val = tmp_var + epsilon_;
        tmp_val = std::sqrt(tmp_val);
        tmp_val = tmp_scale / tmp_val;
#if !use_literal_value_binary_version
        memcpy((char*)stmp.data(), (char*)&tmp_val, sizeof(double));
        scaling_factor[i] = stmp;
#else
        scaling_factor[i] = std::to_string(tmp_val);
#endif
      }
    } else {
      // stabilized variance
      vector<string> flat_var(depth);
      vector<string> flat_scale(depth);
      vector<string> flat_epsilon(depth, to_string(epsilon_));
      for (int i = 0; i < depth; ++i) {
        flat_var[i] = var_flat(i);
        flat_scale[i] = scale_flat(i);
      }

      vector<string> new_var(depth);
      SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Add);
      ops->Add(flat_var, flat_epsilon, new_var, &attrs_);
      SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Add);

      vector<string> norm_factor(depth);
      SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Rsqrt);
      ops->Rsqrt(new_var, norm_factor, &attrs_);
      SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Rsqrt);

      SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Mul);
      ops->Mul(norm_factor, flat_scale, scaling_factor, &attrs_);
      SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Mul);
    }

    // Todo:size of `depth` is enough two!
    vector<string> inner_flat_mean(size);
    vector<string> inner_flat_offset(size);
    log_info << " depth:" << depth << " rest :" << rest_size ;
    // const auto& inner_x_flat = x_inner.flat<string>();
    for (auto i = 0; i < size; ++i) {
      int d = i % depth;
      // int idx = floor(i / depth);
      // // log_info << d << ", " << idx ;
      // all_input[d][idx] = x_flat(i);
      // // log_info << i << "-th input: " << all_input[d][idx] ;
      inner_flat_x[i] = x_flat(i);
      inner_flat_mean[i] = mean_flat(d);

      inner_flat_offset[i] = offset_flat(d);
      log_debug << i << "-th FLAT :" << inner_flat_x[i] << ", " << inner_flat_mean[i];
    }

    vector<string> x_centered(size);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Sub);
    ops->Sub(inner_flat_x, inner_flat_mean, x_centered, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Sub);

    vector<string> x_scaled(size);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Mul);
    attrs_["need_broadcast"] = "1";
    ops->Mul(x_centered, scaling_factor, x_scaled, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Mul);

    vector<string> x_shifted(size);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Add);
    ops->Add(x_scaled, inner_flat_offset, x_shifted, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Add);

    auto out_flat = y->flat<string>();
    for (int i = 0; i < size; ++i) {
      out_flat(i) = std::move(x_shifted[i]);
    }

    log_debug << "SecureFusedBatchNormOp OpKernel compute ok. <--";
  }
};

class SecureSoftmaxOp : public SecureOpKernel {
 private:
  /* data */
 public:
  SecureSoftmaxOp(OpKernelConstruction* context) : SecureOpKernel(context) {}
  ~SecureSoftmaxOp() {}

  void ComputeImpl(OpKernelContext* context) {
    log_debug << "--> SecureSoftmaxOp OpKernel compute.";

    const Tensor& logits_in = context->input(0);
    OP_REQUIRES(
      context, TensorShapeUtils::IsVectorOrHigher(logits_in.shape()),
      errors::InvalidArgument(
        "logits must have >= 1 dimension, got ", logits_in.shape().DebugString()));
    Tensor* softmax_out = nullptr;
    OP_REQUIRES_OK(
      context, context->forward_input_or_allocate_output({0}, 0, logits_in.shape(), &softmax_out));

    log_debug << "logits_in.NumElements():" << logits_in.NumElements() ;
    const auto& logits_in_flat = logits_in.flat<string>();

    const TensorShape& logits_in_shape = logits_in.shape();
    log_debug << "logits_in_shape.num_elements:" << logits_in_shape.num_elements();
    for (int64_t i = 0; i < logits_in_shape.dims(); i++) {
      log_debug << "logits_in dim " << i << ":" << logits_in_shape.dim_size(i) ;
    }

    if (logits_in.NumElements() == 0) {
      return;
    }

    // batch_size x num_classes matrix
    int64_t dims = logits_in_shape.dims();
    if (dims <= 0 || dims > 2) {
      log_error << "not supported dims:" << dims ;
      OP_REQUIRES(
        context, false,
        errors::InvalidArgument("not supported dims, got ", logits_in.shape().DebugString()));
    }

    int64_t rows = 0, cols = 0;
    if (dims == 1) {
      rows = 1;
      cols = logits_in_shape.dim_size(0);
    } else if (dims == 2) {
      rows = logits_in_shape.dim_size(0);
      cols = logits_in_shape.dim_size(1);
    }

    log_debug << "rows:" << rows << ", cols:" << cols ;
    vector<string> a(rows * cols), b(rows * cols);
    for (int64_t i = 0; i < rows; i++) {
      for (int64_t j = 0; j < cols; j++) {
        a[i * cols + j] = logits_in_flat(i * cols + j); // (n, m)
      }
    }

    attr_type attrs_;
    attrs_["rows"] = to_string(rows); //batch_size
    attrs_["cols"] = to_string(cols); //num_classes

    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Softmax);
    ProtocolManager::Instance()
                      ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
                      ->GetOps(msg_id())
                      ->Softmax(a, b, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Softmax);

    auto out_flat = softmax_out->flat<string>();
    for (int64_t i = 0; i < rows; i++) {
      for (int64_t j = 0; j < cols; j++) {
        out_flat(i * cols + j) = std::move(b[i * cols + j]);
      }
    }

    log_debug << "SecureSoftmaxOp OpKernel compute ok. <--";
  }
};

//-----------------------------------------------------------------------------
REGISTER_STR_CPU_KERNEL(SecureRelu, SecureReluOp);
REGISTER_STR_CPU_KERNEL(SecureReluPrime, SecureReluPrimeOp);
REGISTER_STR_CPU_KERNEL(SecureSigmoid, SecureSigmoidOp);
REGISTER_STR_CPU_KERNEL(SecureSigmoidCrossEntropy, SecureSigmoidCrossEntropyOp);
REGISTER_STR_CPU_KERNEL(SecureConv2D, SecureConv2DOp);
REGISTER_STR_CPU_KERNEL(SecureBiasAdd, SecureBiasAddOp);
REGISTER_STR_CPU_KERNEL(SecureL2Loss, SecureL2LossOp);
REGISTER_STR_CPU_KERNEL(SecureFusedBatchNorm, SecureFusedBatchNormOp);
REGISTER_STR_CPU_KERNEL(SecureSoftmax, SecureSoftmaxOp);
} // namespace tensorflow
