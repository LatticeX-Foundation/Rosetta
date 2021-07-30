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

#include "tensorflow/core/framework/common_shape_fns.h"
#include "tensorflow/core/framework/op.h"
#include "tensorflow/core/framework/shape_inference.h"


std::string GetPaddingAttrString() { return "padding: {'SAME', 'VALID'}"; }

std::string GetPaddingAttrStringWithExplicit() {
  return "padding: {'SAME', 'VALID', 'EXPLICIT'}";
}

std::string GetExplicitPaddingsAttrString() {
  return "explicit_paddings: list(int) = []";
}

std::string GetConvnetDataFormatAttrString() {
  return "data_format: { 'NHWC', 'NCHW' } = 'NHWC' ";
}


REGISTER_OP("RttSigmoidCrossEntropy")
    .Input("logits: string")
    .Input("labels: string")
    .Output("loss: string")
    .Doc(R"doc(
RttSigmoidCrossEntropyOp
)doc");

REGISTER_OP("RttSigmoid")
  .Input("x: string")
  .Output("y: string")
#if ROSETTA_ENABLES_SHAPE_INFERENCE
  .SetShapeFn(::tensorflow::shape_inference::UnchangedShape)
#endif
  .Doc(R"doc(
RttSigmoidOp
)doc");

REGISTER_OP("RttRelu")
  .Input("x: string")
  .Output("y: string")
#if ROSETTA_ENABLES_SHAPE_INFERENCE
  .SetShapeFn(::tensorflow::shape_inference::UnchangedShape)
#endif
  .Doc(R"doc(
RttReluOp
)doc");

REGISTER_OP("RttConv2D")
    .Input("input: string")
    .Input("filter: string")
    .Output("output: string")
    .Attr("strides: list(int)")
    .Attr("use_cudnn_on_gpu: bool = true")
    .Attr(GetPaddingAttrStringWithExplicit())
    .Attr(GetExplicitPaddingsAttrString())
    .Attr(GetConvnetDataFormatAttrString())
    .Attr("dilations: list(int) = [1, 1, 1, 1]")
#if ROSETTA_ENABLES_SHAPE_INFERENCE
    .SetShapeFn(::tensorflow::shape_inference::Conv2DShapeWithExplicitPadding)
#endif
;


REGISTER_OP("RttBiasAdd")
    .Input("value: string")
    .Input("bias: string")
    .Output("output: string")
    .Attr(GetConvnetDataFormatAttrString())
#if ROSETTA_ENABLES_SHAPE_INFERENCE
    .SetShapeFn(::tensorflow::shape_inference::BiasAddShape)
#endif
;


REGISTER_OP("RttL2Loss")
    .Input("t: string")
    .Output("output: string")
#if ROSETTA_ENABLES_SHAPE_INFERENCE
    .SetShapeFn(::tensorflow::shape_inference::ScalarShape)
#endif
;


REGISTER_OP("RttFusedBatchNorm")
    .Input("x: string")
    .Input("scale: string")
    .Input("offset: string")
    .Input("mean: string")
    .Input("variance: string")
    .Output("y: string")
    .Output("batch_mean: string")
    .Output("batch_variance: string")
    .Output("reserve_space_1: string")
    .Output("reserve_space_2: string")
    .Attr("epsilon: float = 0.0001")
    .Attr(GetConvnetDataFormatAttrString())
    .Attr("is_training: bool = true")
#if ROSETTA_ENABLES_SHAPE_INFERENCE
    .SetShapeFn(::tensorflow::shape_inference::FusedBatchNormShape)
#endif
;


REGISTER_OP("RttAvgPool")
    .Input("value: string")
    .Output("output: string")
    .Attr("ksize: list(int) >= 4")
    .Attr("strides: list(int) >= 4")
    .Attr(GetPaddingAttrString())
    .Attr(GetConvnetDataFormatAttrString())
#if ROSETTA_ENABLES_SHAPE_INFERENCE
    .SetShapeFn(::tensorflow::shape_inference::AvgPoolShape)
#endif
;


REGISTER_OP("RttMaxPool")
    .Input("input: string")
    .Output("output: string")
    .Attr("ksize: list(int) >= 4")
    .Attr("strides: list(int) >= 4")
    .Attr(GetPaddingAttrString())
    .Attr("data_format: {'NHWC', 'NCHW', 'NCHW_VECT_C'} = 'NHWC'")
#if ROSETTA_ENABLES_SHAPE_INFERENCE
    .SetShapeFn(::tensorflow::shape_inference::MaxPoolShape)
#endif
;


REGISTER_OP("RttSoftmax")
    .Input("logits: string")
    .Output("softmax: string")
#if ROSETTA_ENABLES_SHAPE_INFERENCE
    .SetShapeFn([](::tensorflow::shape_inference::InferenceContext* c) {
      return ::tensorflow::shape_inference::UnchangedShapeWithRankAtLeast(c, 1);
    })
#endif
;
