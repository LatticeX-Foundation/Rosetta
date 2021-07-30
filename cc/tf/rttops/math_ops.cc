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
#include "tensorflow/core/framework/numeric_op.h"
#include "tensorflow/core/framework/op.h"
#include "tensorflow/core/framework/shape_inference.h"


namespace {
  using ::tensorflow::shape_inference::DimensionHandle;
  using ::tensorflow::shape_inference::InferenceContext;
  using ::tensorflow::shape_inference::ShapeHandle;
  using ::tensorflow::Status;
  using ::tensorflow::Tensor;
  using ::tensorflow::int32;
  using ::tensorflow::int64;

  Status ArgOpShape(InferenceContext* c) {
    ShapeHandle dimension_shape;
    TF_RETURN_IF_ERROR(c->WithRank(c->input(1), 0, &dimension_shape));

    ShapeHandle input_shape = c->input(0);
    if (!c->RankKnown(input_shape)) {
      return UnknownShape(c);
    }

    const int32 input_rank = c->Rank(input_shape);
      if (input_rank <= 1) {
        // Reducing a scalar/vector must return a scalar.
        return ScalarShape(c);
      }

      const Tensor* dim_t = c->input_tensor(1);
      if (dim_t == nullptr) {
        // We don't know the value of the dimension, but we
        // know the rank of the input, so return the correct
        // rank with unknown dimensions.
        std::vector<DimensionHandle> dims(input_rank - 1);
        for (int i = 0; i < dims.size(); ++i) {
          dims[i] = c->UnknownDim();
        }

        c->set_output(0, c->MakeShape(dims));
        return Status::OK();
      }

      int64 dimension_val;
      if (dim_t->dtype() == ::tensorflow::DT_INT32) {
        dimension_val = dim_t->scalar<int32>()();
      } else {
        dimension_val = dim_t->scalar<int64>()();
      }

      int64 axis = dimension_val < 0 ? dimension_val + input_rank : dimension_val;
      if (axis < 0 || axis >= input_rank) {
        return ::tensorflow::errors::InvalidArgument(
            "Dimension (", dimension_val, ") must be in the range [", -input_rank,
            ", ", input_rank, "), where ", input_rank,
            " is the number of dimensions in the input.");
      }

      // Return the input shape without the dimension being reduced.
      std::vector<DimensionHandle> dims;
      for (int i = 0; i < input_rank; ++i) {
        if (axis != i) {
          dims.emplace_back(c->Dim(input_shape, i));
        }
      }
      c->set_output(0, c->MakeShape(dims));
      return Status::OK();
    }
} // namespace

#if ROSETTA_ENABLES_SHAPE_INFERENCE
#define RTT_OP_SET_SHAPE_FN(ShapeFn) \
    .SetShapeFn(ShapeFn)
#else
#define RTT_OP_SET_SHAPE_FN(ShapeFn) 
#endif

#define REGISTER_RTT_BINARY_OP(name)                          \
  REGISTER_OP(#name)                                          \
    .Input("x: string")                                       \
    .Input("y: string")                                       \
    .Output("z: string")                                      \
    .Attr("lh_is_const: bool = false")                        \
    .Attr("rh_is_const: bool = false")                        \
    RTT_OP_SET_SHAPE_FN(::tensorflow::shape_inference::BroadcastBinaryOpShapeFn)

#define REGISTER_RTT_BINARY_CONST_OP(name)                    \
  REGISTER_OP(#name)                                          \
    .Input("x: string")                                       \
    .Input("y: string")                                       \
    .Output("z: string")                                      \
    .Attr("lh_is_const: bool = false")                        \
    .Attr("rh_is_const: bool = true")                         \
    RTT_OP_SET_SHAPE_FN(::tensorflow::shape_inference::BroadcastBinaryOpShapeFn)

#define UNARY()                                               \
  Input("x: string")                                          \
    .Output("y: string")                                      \
    RTT_OP_SET_SHAPE_FN(::tensorflow::shape_inference::UnchangedShape)


REGISTER_RTT_BINARY_CONST_OP(RttPow).Doc(R"doc(
    RttPow
)doc");

REGISTER_RTT_BINARY_OP(RttAdd).Doc(R"doc(
    RttAdd
)doc");

REGISTER_RTT_BINARY_OP(RttSub).Doc(R"doc(
    RttSub
)doc");

REGISTER_RTT_BINARY_OP(RttMul).Doc(R"doc(
    RttMul
)doc");

REGISTER_RTT_BINARY_OP(RttDiv).Doc(R"doc(
    RttDiv
)doc");

REGISTER_RTT_BINARY_OP(RttReciprocaldiv).Doc(R"doc(
    RttReciprocaldiv
)doc");

REGISTER_RTT_BINARY_OP(RttTruediv).Doc(R"doc(
    RttTruediv
)doc");

REGISTER_RTT_BINARY_OP(RttRealdiv).Doc(R"doc(
    RttRealdiv
)doc");

REGISTER_RTT_BINARY_OP(RttFloordiv).Doc(R"doc(
    RttFloordiv
)doc");

REGISTER_RTT_BINARY_OP(RttGreater).Doc(R"doc(
    RttGreater
)doc");

REGISTER_RTT_BINARY_OP(RttLess).Doc(R"doc(
    RttLess
)doc");

REGISTER_RTT_BINARY_OP(RttEqual).Doc(R"doc(
    RttEqual
)doc");

REGISTER_RTT_BINARY_OP(RttNotEqual).Doc(R"doc(
    RttNotEqual
)doc");

REGISTER_RTT_BINARY_OP(RttGreaterEqual).Doc(R"doc(
    RttGreaterEqual
)doc");

REGISTER_RTT_BINARY_OP(RttLessEqual).Doc(R"doc(
    RttLessEqual
)doc");

REGISTER_RTT_BINARY_OP(RttLogicalAnd).Doc(R"doc(
    RttLogicalAnd
)doc");
REGISTER_RTT_BINARY_OP(RttLogicalOr).Doc(R"doc(
    RttLogicalOr
)doc");
REGISTER_RTT_BINARY_OP(RttLogicalXor).Doc(R"doc(
    RttLogicalXor
)doc");

REGISTER_OP("RttLogicalNot").UNARY().Doc(R"doc(
  RttLogicalNot
)doc");

REGISTER_OP("RttNegative").UNARY().Doc(R"doc(
  RttNegative
)doc");

REGISTER_OP("RttAbs").UNARY().Doc(R"doc(
  RttAbs
)doc");

REGISTER_OP("RttAbsPrime").UNARY().Doc(R"doc(
  RttAbsPrime
)doc");

REGISTER_OP("RttLog").UNARY().Doc(R"doc(
  RttLog
)doc");

REGISTER_OP("RttLog1p").UNARY().Doc(R"doc(
  RttLog1p
)doc");

REGISTER_OP("RttExp").UNARY().Doc(R"doc(
  RttExp
)doc");

REGISTER_OP("RttRsqrt").UNARY().Doc(R"doc(
  RttRsqrt
)doc");

REGISTER_OP("RttSqrt").UNARY().Doc(R"doc(
  RttSqrt
)doc");

REGISTER_OP("RttSquare").UNARY().Doc(R"doc(
  RttSquare
)doc");

REGISTER_OP("RttReveal")
  .Input("x: string")
  .Output("res: string")
  .Attr("receive_parties: string")
  .SetIsStateful();

REGISTER_OP("RttAddN")
    .Input("inputs: N * T")
    .Output("sum: T")
    .Attr("N: int >= 1")
    .Attr("T: {string}")
    .SetIsCommutative()
    .SetIsAggregate();

REGISTER_OP("RttMatmul")
  .Input("x: string")
  .Input("y: string")
  .Output("res: string")
  .Attr("transpose_a: bool = false")
  .Attr("transpose_b: bool = false")
#if ROSETTA_ENABLES_SHAPE_INFERENCE
  .SetShapeFn(::tensorflow::shape_inference::MatMulShape)
#endif
;

REGISTER_OP("RttReduceMean")
  .Input("input: string")
  .Input("reduction_indices: Tidx")
  .Output("output: string")
  .Attr("keep_dims: bool = false")
  .Attr("Tidx: {int32, int64} = DT_INT32")
#if ROSETTA_ENABLES_SHAPE_INFERENCE
  .SetShapeFn(::tensorflow::shape_inference::ReductionShape)
#endif
;

REGISTER_OP("RttReduceSum")
  .Input("input: string")
  .Input("reduction_indices: Tidx")
  .Output("output: string")
  .Attr("keep_dims: bool = false")
  .Attr("Tidx: {int32, int64} = DT_INT32")
#if ROSETTA_ENABLES_SHAPE_INFERENCE
  .SetShapeFn(::tensorflow::shape_inference::ReductionShape)
#endif
;

REGISTER_OP("RttReduceMin")
  .Input("input: string")
  .Input("reduction_indices: Tidx")
  .Output("output: string")
  .Attr("keep_dims: bool = false")
  .Attr("Tidx: {int32, int64} = DT_INT32")
#if ROSETTA_ENABLES_SHAPE_INFERENCE
  .SetShapeFn(::tensorflow::shape_inference::ReductionShape)
#endif
;

REGISTER_OP("RttReduceMax")
  .Input("input: string")
  .Input("reduction_indices: Tidx")
  .Output("output: string")
  .Attr("keep_dims: bool = false")
  .Attr("Tidx: {int32, int64} = DT_INT32")
#if ROSETTA_ENABLES_SHAPE_INFERENCE
  .SetShapeFn(::tensorflow::shape_inference::ReductionShape)
#endif
;

REGISTER_OP("RttArgMax")
    .Input("input: string")
    .Input("dimension: Tidx")
    .Output("output: output_type")
    .Attr("Tidx: {int32, int64} = DT_INT32")
    .Attr("output_type: {string,} = DT_STRING")
#if ROSETTA_ENABLES_SHAPE_INFERENCE
    .SetShapeFn(ArgOpShape)
#endif
;

