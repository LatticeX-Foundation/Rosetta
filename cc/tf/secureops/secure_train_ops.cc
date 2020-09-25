#include "tensorflow/core/framework/common_shape_fns.h"
#include "tensorflow/core/framework/op.h"
#include "tensorflow/core/framework/shape_inference.h"

REGISTER_OP("SecureApplyGradientDescent")
  .Input("var: Ref(string)")
  .Input("alpha: T")
  .Input("delta: string")
  .Output("out: Ref(string)")
  .Attr("T: numbertype")
  .Attr("use_locking: bool = false");

REGISTER_OP("SecureAssign")
  .Input("var: Ref(string)")
  .Input("value: string")
  .Output("out: Ref(string)")
  .Attr("validate_shape: bool = false")
  .Attr("use_locking: bool = true");
