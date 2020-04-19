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
class MpcAbsOp : public MpcOpKernel {
 public:
  explicit MpcAbsOp(OpKernelConstruction* context) : MpcOpKernel(context) {}
  void MpcCompute(OpKernelContext* context) {
    const Tensor& x = context->input(0);
    auto in_flatx = x.flat<T>();

    Tensor* z = nullptr;
    OP_REQUIRES_OK(context, context->allocate_output(0, x.shape(), &z));
    auto out_flat = z->flat<T>();
    cout << typeid(out_flat).name() << endl;

    out_flat.setZero();
    debug_print_tensor(z, "MpcAbsOp z");

#if USE_MPC_OP
    vector<double> inputx;
    for (int i = 0; i < x.dim_size(0); i++) {
      inputx.push_back(in_flatx(i));
    }

    size_t size = inputx.size();
    vector<mpc_t> a, b(size);
    //convert_double_to_mytype(inputx, a);
    tf_convert_double_to_mpctype(inputx, a);
    tfGetMpcOp(SigmoidCrossEntropy)->ABS(a, b, size);

#if PRINT_REVEAL
    {
      vector<double> vb;
      debug_print_reveal(a, vb, "input a");
      debug_print_reveal(b, vb, "output b");

      out_flat.setZero();
      for (int i = 0; i < x.dim_size(0); i++) {
        out_flat(i) = vb[i];
      }
      debug_print_tensor(z, "MpcAbsp mpc(plain) z");
    }
#endif

    out_flat.setZero();
    vector<double> outputz(size);
    //convert_mytype_to_double(b, outputz);
    tf_convert_mpctype_to_double(b, outputz);

    for (int i = 0; i < x.dim_size(0); i++) {
      out_flat(i) = outputz[i];
    }

    debug_print_tensor(z, "MpcAbsOp mpc z");
#endif
  }
}; 

template <typename Device, typename T>
class MpcAbsPrimeOp : public MpcOpKernel {
 public:
  explicit MpcAbsPrimeOp(OpKernelConstruction* context) : MpcOpKernel(context) {}
  void MpcCompute(OpKernelContext* context) {
    const Tensor& x = context->input(0);
    auto in_flatx = x.flat<T>();

    Tensor* z = nullptr;
    OP_REQUIRES_OK(context, context->allocate_output(0, x.shape(), &z));
    auto out_flat = z->flat<T>();
    cout << typeid(out_flat).name() << endl;

    out_flat.setZero();
    debug_print_tensor(z, "MpcAbsPrimeOp z");

#if USE_MPC_OP
    vector<double> inputx;
    for (int i = 0; i < x.dim_size(0); i++) {
      inputx.push_back(in_flatx(i));
    }

    size_t size = inputx.size();
    vector<mpc_t> a, b(size);
    //convert_double_to_mytype(inputx, a);
    tf_convert_double_to_mpctype(inputx, a);
    tfGetMpcOp(SigmoidCrossEntropy)->ABSPrime(a, b, size);

#if PRINT_REVEAL
    {
      vector<double> vb;
      debug_print_reveal(a, vb, "input a");
      debug_print_reveal(b, vb, "output b");

      out_flat.setZero();
      for (int i = 0; i < x.dim_size(0); i++) {
        out_flat(i) = vb[i];
      }
      debug_print_tensor(z, "MpcAbsPrimeOp mpc(plain) z");
    }
#endif

    out_flat.setZero();
    vector<double> outputz(size);
    //convert_mytype_to_double(b, outputz);
    tf_convert_mpctype_to_double(b, outputz);

    for (int i = 0; i < x.dim_size(0); i++) {
      out_flat(i) = outputz[i];
    }

    debug_print_tensor(z, "MpcAbsPrimeOp mpc z");
#endif
  }
}; 

REGISTER_OP("MpcAbs")
    .Input("x: T")
    .Output("y: T")
    .Attr("T: {float, double, int32, int64, uint32, uint64}")
    .SetShapeFn(shape_inference::UnchangedShape)
    .Doc(R"doc(
MpcAbsOp, |X|
)doc");

REGISTER_OP("MpcAbsPrime")
    .Input("x: T")
    .Output("y: T")
    .Attr("T: {float, double, int32, int64, uint32, uint64}")
    .SetShapeFn(shape_inference::UnchangedShape)
    .Doc(R"doc(
MpcAbsPrime, 
)doc");

REGISTER_MPCOP_KERNELS_ALL_TYPES(MpcAbs, MpcAbsOp)
REGISTER_MPCOP_KERNELS_ALL_TYPES(MpcAbsPrime, MpcAbsPrimeOp)

} // namespace tensorflow
