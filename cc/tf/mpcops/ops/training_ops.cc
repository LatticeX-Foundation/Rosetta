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
// this is a derived file based on 
// tensorflow/tensorflow/core/kernels/training_ops.cc etc.
// Thanks to the Tensorflow team.

#include "mpcop_kernel.h"

#include <algorithm>

#include "tensorflow/core/framework/bounds_check.h"
#include "tensorflow/core/framework/op_kernel.h"
#include "tensorflow/core/framework/register_types.h"

#include "tensorflow/core/framework/op.h"
#include "tensorflow/core/framework/shape_inference.h"

namespace tensorflow {
template <typename Device, typename T>
class MpcApplyGradientDescentOp : public MpcOpKernel {
 public:
    explicit MpcApplyGradientDescentOp(OpKernelConstruction* ctx) : MpcOpKernel(ctx) {
    OP_REQUIRES_OK(ctx, ctx->GetAttr("use_locking", &use_exclusive_lock_));
  }

  void MpcCompute(OpKernelContext* ctx) override {
    // Step 1: check the lock level and validity of inputs the same as the native one.
    //  Note: For now, we do NOT support the exclusice_lock feature.
    OP_REQUIRES(ctx, use_exclusive_lock_ == false, 
                errors::FailedPrecondition(
                  "In this MPC version, we do NOT support the 'use_locking' attr be true"));
    // TMP: only non-resource variable
    OP_REQUIRES(ctx, ctx->input_dtype(0) != DT_RESOURCE,
                   errors::FailedPrecondition(
                  "In this MPC version, we do NOT support the 'ResourceVarible'-type variable.")); 
    Tensor var;
    //Tensor* var_p;
    var = ctx->mutable_input(0, use_exclusive_lock_);
    
    OP_REQUIRES(
        ctx, var.IsInitialized(),
        errors::FailedPrecondition(
            "Attempting to use uninitialized variables: ", requested_input(0)));
    const Tensor& alpha = ctx->input(1);
    OP_REQUIRES(ctx, IsLegacyScalar(alpha.shape()),
                errors::InvalidArgument("alpha is not a scalar: ",
                                        alpha.shape().DebugString()));
    const Tensor& delta = ctx->input(2);
    OP_REQUIRES(
        ctx, var.shape().IsSameSize(delta.shape()),
        errors::InvalidArgument("var and delta do not have the same shape",
                                var.shape().DebugString(), " ",
                                delta.shape().DebugString()));
    
    // Step 2: 
    //cout << "======DEBUG: AGD MPC part =======" <<endl;
    // Attention!：In this version, we still need to convert the nums to MpcType
    auto new_var = var.flat<T>();
    auto alpha_flat = alpha.scalar<T>();
    auto delta_flat = delta.flat<T>();

    auto ele_nums = delta.NumElements();
  #if USE_MPC_OP
      vector<double> input_alpha(ele_nums);
      vector<double> input_delta(ele_nums);
      vector<double> input_var(ele_nums);
      for(auto i = 0; i < ele_nums; ++i) {
        // original alpha is scalar!
        input_alpha[i] = alpha_flat();
        input_delta[i] = delta_flat(i);
        input_var[i] = new_var(i); 
      }
      //vector<mpc_t> mpc_alpha_vec(ele_nums);
      vector<mpc_t> mpc_delta_vec(ele_nums);
      vector<mpc_t> mpc_var_vec(ele_nums);
      vector<mpc_t> mpc_res_vec(ele_nums);
      //tf_convert_double_to_mpctype(input_alpha, mpc_alpha_vec);
      tf_convert_double_to_mpctype(input_delta, mpc_delta_vec);      
      tf_convert_double_to_mpctype(input_var, mpc_var_vec);

      //cout << "alpha: " << alpha_flat() <<endl;
      //new_var -= delta_flat * alpha_flat();
      //reuse the MUL when left-side-hand is local const:
      tfGetMpcOp(Mul)->Run(input_alpha, mpc_delta_vec, mpc_res_vec, ele_nums);
      tfGetMpcOp(Sub)->Run(mpc_var_vec, mpc_res_vec, mpc_res_vec, ele_nums);

      vector<double> output_var(ele_nums);
      tf_convert_mpctype_to_double(mpc_res_vec, output_var);
      new_var.setZero();
      for(int i = 0; i < ele_nums; ++i){
        new_var(i) = output_var[i];
      }
  #endif
    //cout << "======DEBUG: AGD MPC part END=======" <<endl; 
    if (ctx->input_dtype(0) != DT_RESOURCE) {
      ctx->forward_ref_input_to_ref_output(0, 0);
    }
  
  }


 private:
  bool use_exclusive_lock_;
};

// Note: based on tensorflow/core/ops/training_ops.cc

using shape_inference::DimensionHandle;
using shape_inference::InferenceContext;
using shape_inference::ShapeHandle;

static ShapeHandle ShapeOrHandleShape(InferenceContext* c, int input) {
  auto* handle_data = c->input_handle_shapes_and_types(input);
  if (handle_data != nullptr && !handle_data->empty() &&
      (*handle_data)[0].dtype != DT_INVALID) {
    return (*handle_data)[0].shape;
  }
  return c->input(input);
}

// Handle the gradient and, if <sparse>, indices inputs.
// <s> is an input+output parameter, containing the current known input shape to
// the gradient.
static Status HandleGradAndIndicesInputs(InferenceContext* c, bool sparse,
                                         int grad_idx, ShapeHandle* s) {
  ShapeHandle grad = ShapeOrHandleShape(c, grad_idx);
  if (!sparse) {
    TF_RETURN_IF_ERROR(c->Merge(*s, grad, s));
    return Status::OK();
  }
  // Indices is a vector where indices.dim[0].rank == grad[0].rank.
  ShapeHandle indices;
  TF_RETURN_IF_ERROR(c->WithRank(c->input(grad_idx + 1), 1, &indices));
  DimensionHandle unused;
  TF_RETURN_IF_ERROR(c->Merge(c->Dim(indices, 0), c->Dim(grad, 0), &unused));

  // Trailing part of grad matches trailing part of *s.
  ShapeHandle grad_unknown_first;
  TF_RETURN_IF_ERROR(
      c->ReplaceDim(grad, 0, c->UnknownDim(), &grad_unknown_first));
  TF_RETURN_IF_ERROR(c->Merge(*s, grad_unknown_first, s));

  return Status::OK();
}

static Status ApplyGradientDescentShapeFn(InferenceContext* c) {
  ShapeHandle unused;
  ShapeHandle s = ShapeOrHandleShape(c, 0);                  // var
  TF_RETURN_IF_ERROR(c->WithRank(c->input(1), 0, &unused));  // alpha
  TF_RETURN_IF_ERROR(c->Merge(s, c->input(2), &s));          // delta
  if (c->num_outputs() > 0) {
    c->set_output(0, s);
  }
  return Status::OK();
}

/**
  * MPC version of ApplyGradientDescent OP.
  * 
  * The paramters 'var' and 'delta', which are of the same shape,
  * are 'secret-shared' among 2 or 3 parties.
  * 
  * The hyperprameter 'alpha' is the 'plaintext' value, which all
  * parties can know.
  * 
  * the result, 'out', is also 'secret-shared' among 2 or 3 parties.
*/
REGISTER_OP("MpcApplyGradientDescent")
    .Input("var: Ref(T)")
    .Input("alpha: T")
    .Input("delta: T")
    .Output("out: Ref(T)")
    .Attr("T: numbertype")
    .Attr("use_locking: bool = false")
    // .SetShapeFn(ApplyGradientDescentShapeFn)
    .Doc(R"doc(
MpcApplyGradientDescentOp
)doc");

REGISTER_MPCOP_KERNELS_ALL_TYPES(MpcApplyGradientDescent, MpcApplyGradientDescentOp)

VAR_REGISTER_MPCOP_KERNELS_ALL_TYPES(ResourceMpcApplyGradientDescent, MpcApplyGradientDescentOp)

} // namespace tensorflow
