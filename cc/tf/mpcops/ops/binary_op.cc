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
#include "binary_op.h"

// Binary OP: Add/Sub/Mul/Div/...
namespace tensorflow {

#define REGISTER_MPC_BINARY_OP(name)                          \
  REGISTER_OP(#name)                                          \
    .Input("x: T")                                            \
    .Input("y: T")                                            \
    .Output("z: T")                                           \
    .Attr("T: {float, double, int32, int64, uint32, uint64}") \
    .Attr("lh_is_const: bool = false")                        \
    .Attr("rh_is_const: bool = false")                        \
    .SetShapeFn(shape_inference::BroadcastBinaryOpShapeFn)

#define REGISTER_MPC_BINARY_CMP_OP(name)                      \
  REGISTER_OP(#name)                                          \
    .Input("x: T")                                            \
    .Input("y: T")                                            \
    .Output("z: uint64")                                      \
    .Attr("T: {float, double, int32, int64, uint32, uint64}") \
    .Attr("lh_is_const: bool = false")                        \
    .Attr("rh_is_const: bool = false")                        \
    .SetShapeFn(shape_inference::BroadcastBinaryOpShapeFn)

REGISTER_MPC_BINARY_OP(MpcPow).Doc(R"doc(
    MpcPow
)doc");

REGISTER_MPC_BINARY_OP(MpcAdd).Doc(R"doc(
    MpcAdd
)doc");

REGISTER_MPC_BINARY_OP(MpcSub).Doc(R"doc(
    MpcSub
)doc");

REGISTER_MPC_BINARY_OP(MpcMul).Doc(R"doc(
    MpcMul
)doc");

REGISTER_MPC_BINARY_OP(MpcDiv).Doc(R"doc(
    MpcDiv
)doc");

REGISTER_MPC_BINARY_OP(MpcTruediv).Doc(R"doc(
    MpcTruediv
)doc");

REGISTER_MPC_BINARY_OP(MpcRealdiv).Doc(R"doc(
    MpcRealdiv
)doc");

REGISTER_MPC_BINARY_OP(MpcGreater).Doc(R"doc(
    MpcGreater
)doc");

REGISTER_MPC_BINARY_OP(MpcLess).Doc(R"doc(
    MpcLess
)doc");

REGISTER_MPC_BINARY_OP(MpcEqual).Doc(R"doc(
    MpcEqual
)doc");

REGISTER_MPC_BINARY_OP(MpcGreaterEqual).Doc(R"doc(
    MpcGreaterEqual
)doc");

REGISTER_MPC_BINARY_OP(MpcLessEqual).Doc(R"doc(
    MpcLessEqual
)doc");

REGISTER_MPCOP_KERNELS_ALL_TYPES_BINARYOP(MpcPow, MpcBinaryOp, MpcPowOpFunctor)

REGISTER_MPCOP_KERNELS_ALL_TYPES_BINARYOP(MpcAdd, MpcBinaryOp, MpcAddOpFunctor)
REGISTER_MPCOP_KERNELS_ALL_TYPES_BINARYOP(MpcSub, MpcBinaryOp, MpcSubOpFunctor)
REGISTER_MPCOP_KERNELS_ALL_TYPES_BINARYOP(MpcMul, MpcBinaryOp, MpcMulOpFunctor)
REGISTER_MPCOP_KERNELS_ALL_TYPES_BINARYOP(MpcDiv, MpcBinaryOp, MpcDivOpFunctor)
REGISTER_MPCOP_KERNELS_ALL_TYPES_BINARYOP(MpcTruediv, MpcBinaryOp, MpcTruedivOpFunctor)
REGISTER_MPCOP_KERNELS_ALL_TYPES_BINARYOP(MpcRealdiv, MpcBinaryOp, MpcDivOpFunctor)

REGISTER_MPCOP_KERNELS_ALL_TYPES_BINARYOP(MpcGreater, MpcBinaryOp, MpcGreaterOpFunctor)
REGISTER_MPCOP_KERNELS_ALL_TYPES_BINARYOP(MpcLess, MpcBinaryOp, MpcLessOpFunctor)
REGISTER_MPCOP_KERNELS_ALL_TYPES_BINARYOP(MpcEqual, MpcBinaryOp, MpcEqualOpFunctor)
REGISTER_MPCOP_KERNELS_ALL_TYPES_BINARYOP(MpcGreaterEqual, MpcBinaryOp, MpcGreaterEqualOpFunctor)
REGISTER_MPCOP_KERNELS_ALL_TYPES_BINARYOP(MpcLessEqual, MpcBinaryOp, MpcLessEqualOpFunctor)
} // namespace tensorflow