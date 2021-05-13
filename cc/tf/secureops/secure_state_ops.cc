#include "tensorflow/core/framework/common_shape_fns.h"
#include "tensorflow/core/framework/op.h"
#include "tensorflow/core/framework/shape_inference.h"


REGISTER_OP("SecureAssign")
  .Input("var: Ref(string)")
  .Input("value: string")
  .Output("out: Ref(string)")
  .Attr("validate_shape: bool = false")
  .Attr("use_locking: bool = true")
  .Doc(R"doc(
SecureAssignOp
)doc");

REGISTER_OP("SecureAssignSub")
    .Input("ref: Ref(string)")
    .Input("value: string")
    .Output("output_ref: Ref(string)")
    .Attr("use_locking: bool = false")
#if ROSETTA_ENABLES_SHAPE_INFERENCE
    .SetShapeFn(::tensorflow::shape_inference::MergeBothInputsShapeFn)
#endif
    .Doc(R"doc(
SecureAssignSubOp
)doc");
