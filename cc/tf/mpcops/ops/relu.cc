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
class MpcReluOp : public MpcOpKernel {
 public:
  explicit MpcReluOp(OpKernelConstruction* context) : MpcOpKernel(context) {}
  void MpcCompute(OpKernelContext* context) {
    const Tensor& x = context->input(0);
    auto in_flatx = x.flat<T>();

    Tensor* z = nullptr;
    OP_REQUIRES_OK(context, context->allocate_output(0, x.shape(), &z));
    auto out_flat = z->flat<T>();
    out_flat.setZero();
    auto input_ele_nums = x.NumElements();
#if USE_MPC_OP
    vector<double> inputx;
    for (int i = 0; i < input_ele_nums; ++i) {
      inputx.push_back(in_flatx(i));
    }

    vector<mpc_t> a(input_ele_nums);
    vector<mpc_t> b(input_ele_nums);
    //convert_double_to_mytype(inputx, a);
    tf_convert_double_to_mpctype(inputx, a);
    tfGetMpcOp(Relu)->Run(a, b, input_ele_nums);

#if PRINT_REVEAL
    {
      vector<double> vb;
      debug_print_reveal(a, vb, "input a");
      debug_print_reveal(b, vb, "output b");

      out_flat.setZero();
      for (int i = 0; i < input_ele_nums; i++) {
        out_flat(i) = vb[i];
      }
      debug_print_tensor(z, "MpcReluOp mpc(plain) z");
    }
#endif

    out_flat.setZero();
    vector<double> outputz(input_ele_nums);
    //convert_mytype_to_double(b, outputz);
    tf_convert_mpctype_to_double(b, outputz);
    for (int i = 0; i < input_ele_nums; i++) {
      out_flat(i) = outputz[i];
    }

#endif
  }
}; 

template <typename Device, typename T>
class MpcReluPrimeOp : public MpcOpKernel {
 public:
  explicit MpcReluPrimeOp(OpKernelConstruction* context) : MpcOpKernel(context) {}
  void MpcCompute(OpKernelContext* context) {
    const Tensor& x = context->input(0);
    auto in_flatx = x.flat<T>();

    Tensor* z = nullptr;
    OP_REQUIRES_OK(context, context->allocate_output(0, x.shape(), &z));
    auto out_flat = z->flat<T>();
    out_flat.setZero();
    auto input_ele_nums = x.NumElements();
#if USE_MPC_OP
    vector<double> inputx;
    for (int i = 0; i < input_ele_nums; ++i) {
      inputx.push_back(in_flatx(i));
    }

    size_t size = inputx.size();
    vector<mpc_t> a(input_ele_nums);
    vector<mpc_t> b(input_ele_nums);
    //convert_double_to_mytype(inputx, a);
    tf_convert_double_to_mpctype(inputx, a);
    tfGetMpcOp(ReluPrime)->Run(a, b, input_ele_nums);

#if PRINT_REVEAL
    {
      vector<double> vb;
      debug_print_reveal(a, vb, "input a");
      debug_print_reveal(b, vb, "output b");

      out_flat.setZero();
      for (int i = 0; i < input_ele_nums; i++) {
        out_flat(i) = vb[i];
      }
      debug_print_tensor(z, "MpcReluPrimeOp mpc(plain) z");
    }
#endif
    out_flat.setZero();
    vector<double> outputz(input_ele_nums);
    //convert_mytype_to_double(b, outputz);
    tf_convert_mpctype_to_double(b, outputz);
    for (int i = 0; i < input_ele_nums; i++) {
      out_flat(i) = outputz[i];
    }
#endif
  }
}; 

REGISTER_OP("MpcRelu")
    .Input("x: T")
    .Output("y: T")
    .Attr("T: {float, double, int32, int64, uint32, uint64}")
    .SetShapeFn(shape_inference::UnchangedShape)
    .Doc(R"doc(
MpcReluOp
)doc");

REGISTER_OP("MpcReluPrime")
    .Input("x: T")
    .Output("y: T")
    .Attr("T: {float, double, int32, int64, uint32, uint64}")
    .SetShapeFn(shape_inference::UnchangedShape)
    .Doc(R"doc(
MpcReluPrimeOp, the derivative of Relu
)doc");

REGISTER_MPCOP_KERNELS_ALL_TYPES(MpcRelu, MpcReluOp)
REGISTER_MPCOP_KERNELS_ALL_TYPES(MpcReluPrime, MpcReluPrimeOp)

} // namespace tensorflow
