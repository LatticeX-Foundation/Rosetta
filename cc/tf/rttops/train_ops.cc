//#include "tensorflow/core/framework/common_shape_fns.h"
#include "tensorflow/core/framework/op.h"
//#include "tensorflow/core/framework/shape_inference.h"

REGISTER_OP("RttApplyGradientDescent")
  .Input("var: Ref(string)")
  .Input("alpha: T")
  .Input("delta: string")
  .Output("out: Ref(string)")
  .Attr("T: numbertype")
  .Attr("use_locking: bool = false");


REGISTER_OP("ApplyAdam")
    .Input("var: Ref(string)")
    .Input("m: Ref(string)")
    .Input("v: Ref(string)")
    .Input("beta1_power: string")
    .Input("beta2_power: string")
    .Input("lr: T")
    .Input("beta1: T")
    .Input("beta2: T")
    .Input("epsilon: T")
    .Input("grad: string")
    .Output("out: Ref(string)")
    .Attr("T: numbertype")
    .Attr("use_locking: bool = false")
    .Attr("use_nesterov: bool = false");