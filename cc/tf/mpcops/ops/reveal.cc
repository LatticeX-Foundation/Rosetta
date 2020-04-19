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
class MpcRevealOp : public MpcOpKernel {
  int32_t reveal_party_ = -1;

 public:
  explicit MpcRevealOp(OpKernelConstruction* context) : MpcOpKernel(context) {
    OP_REQUIRES_OK(context, context->GetAttr("reveal_party", &reveal_party_));
    //cout << "reveal_party:" << reveal_party_ << endl;
    if ((reveal_party_ < -1) || (reveal_party_ > 1)) {
      reveal_party_ = -1;
    }
  }
  void MpcCompute(OpKernelContext* context) {
    const Tensor& x = context->input(0);
    auto in_flatx = x.flat<T>();

    Tensor* z = nullptr;
    OP_REQUIRES_OK(context, context->allocate_output(0, x.shape(), &z));
    auto out_flat = z->flat<T>();
    out_flat.setZero();
  #if PRINT_REVEAL
    debug_print_tensor(z, "MpcRevealOp z");
  #endif
    if (x.NumElements() == 0)
      return;

    vector<double> inputx;
    for (int i = 0; i < x.NumElements(); i++) {
      inputx.push_back(in_flatx(i));
    }

    size_t size = inputx.size();
    vector<mpc_t> a, b(size);
    //convert_double_to_mytype(inputx, a);
    tf_convert_double_to_mpctype(inputx, a);


    if ((reveal_party_ == -1) || (reveal_party_ == PARTY_A)) {
      tfGetMpcOp(Reconstruct2PC)->Run(a, size, b, PARTY_A);

      vector<double> outputz(size);
      convert_mytype_to_double(b, outputz);

      if (PARTY_A == partyNum) {
        for (int i = 0; i < size; i++) {
          out_flat(i) = outputz[i];
        }
      }
    }

    if ((reveal_party_ == -1) || (reveal_party_ == PARTY_B)) {
      tfGetMpcOp(Reconstruct2PC)->Run(a, size, b, PARTY_B);

      vector<double> outputz(size);
      convert_mytype_to_double(b, outputz);

      if (PARTY_B == partyNum) {
        for (int i = 0; i < size; i++) {
          out_flat(i) = outputz[i];
        }
      }
    }
  #if PRINT_REVEAL
    debug_print_tensor(z, "MpcRevealOp mpc z");
  #endif
  }
}; // namespace tensorflow

REGISTER_OP("MpcReveal")
  .Input("x: T")
  .Output("y: T")
  .Attr("T: {float, double, int32, int64, uint32, uint64}")
  .Attr("reveal_party: int = -1")
  .SetShapeFn(shape_inference::UnchangedShape)
  .Doc(R"doc(
MpcRevealOp
)doc");
REGISTER_MPCOP_KERNELS_ALL_TYPES(MpcReveal, MpcRevealOp)

} // namespace tensorflow
