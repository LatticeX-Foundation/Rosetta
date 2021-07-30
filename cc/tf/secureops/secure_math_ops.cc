#include "tensorflow/core/framework/common_shape_fns.h"
#include "tensorflow/core/framework/op.h"
#include "tensorflow/core/framework/shape_inference.h"

#if ROSETTA_ENABLES_SHAPE_INFERENCE
#define SECURE_OP_SET_SHAPE_FN(ShapeFn) \
    .SetShapeFn(ShapeFn)
#else
#define SECURE_OP_SET_SHAPE_FN(ShapeFn) 
#endif

#define REGISTER_SECURE_BINARY_OP(name)                     \
  REGISTER_OP(#name)                                        \
    .Input("x: string")                                     \
    .Input("y: string")                                     \
    .Output("z: string")                                    \
    .Attr("lh_is_const: bool = false")                      \
    .Attr("rh_is_const: bool = false")                      \
    SECURE_OP_SET_SHAPE_FN(::tensorflow::shape_inference::BroadcastBinaryOpShapeFn)

#define REGISTER_SECURE_BINARY_CONST_OP(name)               \
  REGISTER_OP(#name)                                        \
    .Input("x: string")                                     \
    .Input("y: string")                                     \
    .Output("z: string")                                    \
    .Attr("lh_is_const: bool = false")                      \
    .Attr("rh_is_const: bool = true")                       \
    SECURE_OP_SET_SHAPE_FN(::tensorflow::shape_inference::BroadcastBinaryOpShapeFn)

#define UNARY()                                             \
  Input("x: string")                                        \
    .Output("y: string")                                    \
    SECURE_OP_SET_SHAPE_FN(::tensorflow::shape_inference::UnchangedShape)


REGISTER_SECURE_BINARY_CONST_OP(SecurePow).Doc(R"doc(
    SecurePow
)doc");

REGISTER_SECURE_BINARY_OP(SecureAdd).Doc(R"doc(
    SecureAdd
)doc");

REGISTER_SECURE_BINARY_OP(SecureSub).Doc(R"doc(
    SecureSub
)doc");

REGISTER_SECURE_BINARY_OP(SecureMul).Doc(R"doc(
    SecureMul
)doc");

REGISTER_SECURE_BINARY_OP(SecureDiv).Doc(R"doc(
    SecureDiv
)doc");

REGISTER_SECURE_BINARY_OP(SecureReciprocaldiv).Doc(R"doc(
    SecureReciprocaldiv
)doc");

REGISTER_SECURE_BINARY_OP(SecureTruediv).Doc(R"doc(
    SecureTruediv
)doc");

REGISTER_SECURE_BINARY_OP(SecureRealdiv).Doc(R"doc(
    SecureRealdiv
)doc");

REGISTER_SECURE_BINARY_OP(SecureFloordiv).Doc(R"doc(
    SecureFloordiv
)doc");

REGISTER_SECURE_BINARY_OP(SecureGreater).Doc(R"doc(
    SecureGreater
)doc");

REGISTER_SECURE_BINARY_OP(SecureLess).Doc(R"doc(
    SecureLess
)doc");

REGISTER_SECURE_BINARY_OP(SecureEqual).Doc(R"doc(
    SecureEqual
)doc");

REGISTER_SECURE_BINARY_OP(SecureNotEqual).Doc(R"doc(
    SecureNotEqual
)doc");

REGISTER_SECURE_BINARY_OP(SecureGreaterEqual).Doc(R"doc(
    SecureGreaterEqual
)doc");

REGISTER_SECURE_BINARY_OP(SecureLessEqual).Doc(R"doc(
    SecureLessEqual
)doc");

REGISTER_SECURE_BINARY_OP(SecureLogicalAnd).Doc(R"doc(
    SecureLogicalAnd
)doc");

REGISTER_SECURE_BINARY_OP(SecureLogicalOr).Doc(R"doc(
    SecureLogicalOr
)doc");

REGISTER_SECURE_BINARY_OP(SecureLogicalXor).Doc(R"doc(
    SecureLogicalXor
)doc");


REGISTER_OP("SecureLogicalNot").UNARY().Doc(R"doc(
  SecureLogicalNot
)doc");

REGISTER_OP("SecureNegative").UNARY().Doc(R"doc(
  SecureNegative
)doc");

REGISTER_OP("SecureAbs").UNARY().Doc(R"doc(
  SecureAbs
)doc");

REGISTER_OP("SecureAbsPrime").UNARY().Doc(R"doc(
  SecureAbsPrime
)doc");

REGISTER_OP("SecureLog").UNARY().Doc(R"doc(
  SecureLog
)doc");

REGISTER_OP("SecureHLog").UNARY().Doc(R"doc(
  SecureHLog
)doc");

REGISTER_OP("SecureLog1p").UNARY().Doc(R"doc(
  SecureLog1p
)doc");

REGISTER_OP("SecureSquare").UNARY().Doc(R"doc(
  SecureSquare
)doc");

REGISTER_OP("SecureExp").UNARY().Doc(R"doc(
  SecureExp
)doc");

REGISTER_OP("SecureSqrt").UNARY().Doc(R"doc(
  SecureSqrt
)doc");

REGISTER_OP("SecureRsqrt").UNARY().Doc(R"doc(
  SecureRsqrt
)doc");

REGISTER_OP("SecureReveal")
  .Input("x: string")
  .Output("res: string")
  .Attr("receive_parties: string")
  .SetIsStateful();;

REGISTER_OP("SecureAddN")
  .Input("inputs: N * T")
  .Output("sum: T")
  .Attr("N: int >= 1")
  .Attr("T: {string}")
  .SetIsCommutative()
  .SetIsAggregate();

REGISTER_OP("SecureMatmul")
  .Input("x: string")
  .Input("y: string")
  .Output("res: string")
  .Attr("transpose_a: bool = false")
  .Attr("transpose_b: bool = false")
#if ROSETTA_ENABLES_SHAPE_INFERENCE
  .SetShapeFn(::tensorflow::shape_inference::MatMulShape)
#endif
;

REGISTER_OP("SecureReduceMean")
  .Input("input: string")
  .Input("reduction_indices: Tidx")
  .Output("output: string")
  .Attr("keep_dims: bool = false")
  .Attr("Tidx: {int32, int64} = DT_INT32")
#if ROSETTA_ENABLES_SHAPE_INFERENCE
  .SetShapeFn(::tensorflow::shape_inference::ReductionShape)
#endif
;

REGISTER_OP("SecureReduceSum")
  .Input("input: string")
  .Input("reduction_indices: Tidx")
  .Output("output: string")
  .Attr("keep_dims: bool = false")
  .Attr("Tidx: {int32, int64} = DT_INT32")
#if ROSETTA_ENABLES_SHAPE_INFERENCE
  .SetShapeFn(::tensorflow::shape_inference::ReductionShape)
#endif
;

REGISTER_OP("SecureReduceMin")
  .Input("input: string")
  .Input("reduction_indices: Tidx")
  .Output("output: string")
  .Attr("keep_dims: bool = false")
  .Attr("Tidx: {int32, int64} = DT_INT32")
#if ROSETTA_ENABLES_SHAPE_INFERENCE
  .SetShapeFn(::tensorflow::shape_inference::ReductionShape)
#endif
;

REGISTER_OP("SecureReduceMax")
  .Input("input: string")
  .Input("reduction_indices: Tidx")
  .Output("output: string")
  .Attr("keep_dims: bool = false")
  .Attr("Tidx: {int32, int64} = DT_INT32")
#if ROSETTA_ENABLES_SHAPE_INFERENCE
  .SetShapeFn(::tensorflow::shape_inference::ReductionShape)
#endif
;

REGISTER_OP("SecureArgMax")
  .Input("input: string")
  .Input("dimension: Tidx")
  .Output("output: output_type")
  .Attr("Tidx: {int32, int64} = DT_INT32")
  .Attr("output_type: {string,} = DT_STRING");
  
