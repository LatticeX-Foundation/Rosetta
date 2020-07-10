#include "tensorflow/core/framework/common_shape_fns.h"
#include "tensorflow/core/framework/op.h"
#include "tensorflow/core/framework/shape_inference.h"


#define REGISTER_SECURE_BINARY_OP(name)                       \
  REGISTER_OP(#name)                                          \
    .Input("x: string")                                       \
    .Input("y: string")                                       \
    .Output("z: string")                                      \
    .Attr("lh_is_const: bool = false")                        \
    .Attr("rh_is_const: bool = false") // .SetShapeFn(shape_inference::BroadcastBinaryOpShapeFn)

#define REGISTER_SECURE_BINARY_CONST_OP(name)                       \
  REGISTER_OP(#name)                                          \
    .Input("x: string")                                       \
    .Input("y: string")                                       \
    .Output("z: string")                                      \
    .Attr("lh_is_const: bool = false")                        \
    .Attr("rh_is_const: bool = true") // .SetShapeFn(shape_inference::BroadcastBinaryOpShapeFn)

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

REGISTER_OP("SecureNegative")
  .Input("x: string")
  .Output("res: string")
  .SetIsStateful();

REGISTER_OP("SecureAbs")
  .Input("x: string")
  .Output("res: string")
  .SetIsStateful();

REGISTER_OP("SecureAbsPrime")
  .Input("x: string")
  .Output("res: string")
  .SetIsStateful();

REGISTER_OP("SecureLog")
  .Input("x: string")
  .Output("res: string")
  .SetIsStateful();

REGISTER_OP("SecureHLog")
  .Input("x: string")
  .Output("res: string")
  .SetIsStateful();

REGISTER_OP("SecureLog1p")
  .Input("x: string")
  .Output("res: string")
  .SetIsStateful();

REGISTER_OP("SecureReveal")
  .Input("x: string")
  .Output("res: string")
  .Attr("receive_party: int = 1")
  .SetIsStateful();

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
  .SetIsStateful();

REGISTER_OP("SecureSquare").Input("x: string").Output("res: string").SetIsStateful();


REGISTER_OP("SecureReduceMean")
  .Input("input: string")
  .Input("reduction_indices: Tidx")
  .Output("output: string")
  .Attr("keep_dims: bool = false")
  .Attr("Tidx: {int32, int64} = DT_INT32");

REGISTER_OP("SecureReduceSum")
  .Input("input: string")
  .Input("reduction_indices: Tidx")
  .Output("output: string")
  .Attr("keep_dims: bool = false")
  .Attr("Tidx: {int32, int64} = DT_INT32");

REGISTER_OP("SecureReduceMin")
  .Input("input: string")
  .Input("reduction_indices: Tidx")
  .Output("output: string")
  .Attr("keep_dims: bool = false")
  .Attr("Tidx: {int32, int64} = DT_INT32");

REGISTER_OP("SecureReduceMax")
  .Input("input: string")
  .Input("reduction_indices: Tidx")
  .Output("output: string")
  .Attr("keep_dims: bool = false")
  .Attr("Tidx: {int32, int64} = DT_INT32");
