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
//#include "tensorflow/core/framework/common_shape_fns.h"
#include "tensorflow/core/framework/op.h"
//#include "tensorflow/core/framework/shape_inference.h"





#define REGISTER_RTT_BINARY_OP(name)                       \
  REGISTER_OP(#name)                                          \
    .Input("x: string")                                       \
    .Input("y: string")                                       \
    .Output("z: string")                                      \
    .Attr("lh_is_const: bool = false")                        \
    .Attr("rh_is_const: bool = false") // .SetShapeFn(shape_inference::BroadcastBinaryOpShapeFn)

#define REGISTER_RTT_BINARY_CONST_OP(name)                       \
  REGISTER_OP(#name)                                          \
    .Input("x: string")                                       \
    .Input("y: string")                                       \
    .Output("z: string")                                      \
    .Attr("lh_is_const: bool = false")                        \
    .Attr("rh_is_const: bool = true") // .SetShapeFn(shape_inference::BroadcastBinaryOpShapeFn)

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

REGISTER_OP("RttNegative")
  .Input("x: string")
  .Output("res: string");

REGISTER_OP("RttAbs")
  .Input("x: string")
  .Output("res: string");

REGISTER_OP("RttAbsPrime")
  .Input("x: string")
  .Output("res: string");

REGISTER_OP("RttLog")
  .Input("x: string")
  .Output("res: string");

REGISTER_OP("RttLog1p")
  .Input("x: string")
  .Output("res: string");

REGISTER_OP("RttReveal")
  .Input("x: string")
  .Output("res: string")
  .Attr("receive_party: int = 0");

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
  .Attr("transpose_b: bool = false");

REGISTER_OP("RttSquare").Input("x: string").Output("res: string");


REGISTER_OP("RttReduceMean")
  .Input("input: string")
  .Input("reduction_indices: Tidx")
  .Output("output: string")
  .Attr("keep_dims: bool = false")
  .Attr("Tidx: {int32, int64} = DT_INT32");

REGISTER_OP("RttReduceSum")
  .Input("input: string")
  .Input("reduction_indices: Tidx")
  .Output("output: string")
  .Attr("keep_dims: bool = false")
  .Attr("Tidx: {int32, int64} = DT_INT32");

REGISTER_OP("RttReduceMin")
  .Input("input: string")
  .Input("reduction_indices: Tidx")
  .Output("output: string")
  .Attr("keep_dims: bool = false")
  .Attr("Tidx: {int32, int64} = DT_INT32");

REGISTER_OP("RttReduceMax")
  .Input("input: string")
  .Input("reduction_indices: Tidx")
  .Output("output: string")
  .Attr("keep_dims: bool = false")
  .Attr("Tidx: {int32, int64} = DT_INT32");

