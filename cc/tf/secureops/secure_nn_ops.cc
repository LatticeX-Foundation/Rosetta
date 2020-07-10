#include "tensorflow/core/framework/common_shape_fns.h"
#include "tensorflow/core/framework/op.h"
#include "tensorflow/core/framework/shape_inference.h"


REGISTER_OP("SecureSigmoidCrossEntropy")
    .Input("logits: string")
    .Input("labels: string")
    .Output("loss: string")
    .Doc(R"doc(
SecureSigmoidCrossEntropyOp
)doc");

REGISTER_OP("SecureSigmoid")
  .Input("x: string")
  .Output("y: string")
  // .SetShapeFn(shape_inference::UnchangedShape)
  .Doc(R"doc(
SecureSigmoidOp
)doc");

REGISTER_OP("SecureRelu")
  .Input("x: string")
  .Output("y: string")
  // .SetShapeFn(shape_inference::UnchangedShape)
  .Doc(R"doc(
SecureReluOp
)doc");

REGISTER_OP("SecureReluPrime")
  .Input("x: string")
  .Output("y: string")
  // .SetShapeFn(shape_inference::UnchangedShape)
  .Doc(R"doc(
SecureReluPrimeOp
)doc");