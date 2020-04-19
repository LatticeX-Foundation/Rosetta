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
#include "mpcop_kernel.h"

namespace tensorflow {

template <typename Device, typename T>
class MpcSigmoidCrossEntropyOp : public MpcOpKernel {
 public:
  explicit MpcSigmoidCrossEntropyOp(OpKernelConstruction* context) : MpcOpKernel(context) {}
  
  void MpcCompute(OpKernelContext* context) {
    const Tensor& x = context->input(0);
    const Tensor& y = context->input(1);
    OP_REQUIRES(
        context, x.shape().IsSameSize(y.shape()),
        errors::InvalidArgument("var and delta do not have the same shape",
                                x.shape().DebugString(), " ",
                                y.shape().DebugString()));
    Tensor* z = nullptr;
    OP_REQUIRES_OK(context, context->allocate_output(0, x.shape(), &z));
    auto out_flat = z->flat<T>();
    out_flat.setZero();

#if USE_MPC_OP
    auto in_flatx = x.flat<T>();
    auto input_ele_nums = x.NumElements();
    auto in_flaty = y.flat<T>();
    
    vector<double> inputx(input_ele_nums);
    vector<double> inputy(input_ele_nums); 
    for(auto i = 0; i < input_ele_nums; ++i) {
      inputx[i] = in_flatx(i);
      inputy[i] = in_flaty(i);  
    }

    vector<mpc_t> shared_logits_vec(input_ele_nums);
    vector<mpc_t> shared_labels_vec(input_ele_nums);

    vector<mpc_t> shared_z_vec(input_ele_nums);
    //convert_double_to_mytype(inputx, shared_logits_vec);
    tf_convert_double_to_mpctype(inputx, shared_logits_vec);
    //convert_double_to_mytype(inputy, shared_labels_vec);
    tf_convert_double_to_mpctype(inputy, shared_labels_vec);

    tfGetMpcOp(SigmoidCrossEntropy)->Run(shared_logits_vec, shared_labels_vec, shared_z_vec, input_ele_nums);

#if PRINT_REVEAL
    {
      vector<double> vc;
      debug_print_reveal(shared_logits_vec, vc, "input logits");
      debug_print_reveal(shared_labels_vec, vc, "input labels");
      debug_print_reveal(shared_z_vec, vc, "output Z");
      out_flat.setZero();
      for (int i = 0; i < input_ele_nums; i++) {
        out_flat(i) = vc[i];
      }
      debug_print_tensor(z, "MpcSigmoidCrossEntropyOp mpc(plain) z");
    }
#endif
    vector<double> outputz(input_ele_nums);
    //convert_mytype_to_double(shared_z_vec, outputz);
    tf_convert_mpctype_to_double(shared_z_vec, outputz);
    out_flat.setZero();
    for(int i = 0; i < input_ele_nums; ++i) {
      out_flat(i) = outputz[i];
    }
#endif
  }
};

REGISTER_OP("MpcSigmoidCrossEntropy")
    .Input("logits: T")
    .Input("labels: T")
    .Output("loss: T")
    .Attr("T: {float, double, int32, int64, uint32, uint64}")
    .Doc(R"doc(
MpcSigmoidCrossEntropyOp
)doc");

REGISTER_MPCOP_KERNELS_ALL_TYPES(MpcSigmoidCrossEntropy, MpcSigmoidCrossEntropyOp)

} // namespace tensorflow
