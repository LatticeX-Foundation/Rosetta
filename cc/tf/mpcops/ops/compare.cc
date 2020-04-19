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
class MpcCompareOp : public MpcOpKernel {
 public:
  explicit MpcCompareOp(OpKernelConstruction* context) : MpcOpKernel(context) {}
  void MpcCompute(OpKernelContext* context) {
    const Tensor& x = context->input(0);
    const Tensor& y = context->input(1);

    Tensor* z = nullptr;
    OP_REQUIRES_OK(context, context->allocate_output(0, x.shape(), &z));
    auto out_flat = z->flat<T>();
    out_flat.setZero();
    debug_print_tensor(z, "MpcCompareOp");

    // TODO
  }
};

REGISTER_OP("MpcCompare")
    .Input("x: T")
    .Input("y: T")
    .Output("z: T")
    .Attr("T: {float, double, int32, int64, uint32, uint64}")
    .Doc(R"doc(
MpcCompareOp
)doc");
REGISTER_MPCOP_KERNELS_ALL_TYPES(MpcCompare, MpcCompareOp)

} // namespace tensorflow
