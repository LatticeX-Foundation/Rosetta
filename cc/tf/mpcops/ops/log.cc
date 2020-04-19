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
class MpcLogOp : public MpcOpKernel {
 public:
  explicit MpcLogOp(OpKernelConstruction* context) : MpcOpKernel(context) {}
  
  void MpcCompute(OpKernelContext* context) {
    const Tensor& x = context->input(0);

    Tensor* z = nullptr;
    OP_REQUIRES_OK(context, context->allocate_output(0, x.shape(), &z));
    auto out_flat = z->flat<T>();
    out_flat.setZero();
    auto input_ele_nums = x.NumElements();

#if USE_MPC_OP
    auto in_flatx = x.flat<T>();

    vector<double> inputx(input_ele_nums);
    for(auto i = 0; i < input_ele_nums; ++i) {
      inputx[i] = in_flatx(i);  
    }

    vector<mpc_t> shared_x_vec(input_ele_nums);
    vector<mpc_t> shared_z_vec(input_ele_nums);
    //convert_double_to_mytype(inputx, shared_x_vec);
    tf_convert_double_to_mpctype(inputx, shared_x_vec);
    tfGetMpcOp(Log)->Run(shared_x_vec, shared_z_vec, input_ele_nums);

#if PRINT_REVEAL
    {
      vector<double> vc;
      debug_print_reveal(shared_x_vec, vc, "input X");
      debug_print_reveal(shared_z_vec, vc, "output Z");
      out_flat.setZero();
      for (int i = 0; i < input_ele_nums; i++) {
        out_flat(i) = vc[i];
      }
      debug_print_tensor(z, "MpcLog mpc(plain) z");
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

REGISTER_OP("MpcLog")
    .Input("x: T")
    .Output("y: T")
    .Attr("T: {float, double, int32, int64, uint32, uint64}")
    .SetShapeFn(shape_inference::UnchangedShape)
    .Doc(R"doc(
MpcLogOp
)doc");
REGISTER_MPCOP_KERNELS_ALL_TYPES(MpcLog, MpcLogOp)


template <typename Device, typename T>
class MpcHLogOp : public MpcOpKernel {
 public:
  explicit MpcHLogOp(OpKernelConstruction* context) : MpcOpKernel(context) {}
  
  void MpcCompute(OpKernelContext* context) {
    const Tensor& x = context->input(0);

    Tensor* z = nullptr;
    OP_REQUIRES_OK(context, context->allocate_output(0, x.shape(), &z));
    auto out_flat = z->flat<T>();
    out_flat.setZero();
    auto input_ele_nums = x.NumElements();

#if USE_MPC_OP
    auto in_flatx = x.flat<T>();

    vector<double> inputx(input_ele_nums);
    for(auto i = 0; i < input_ele_nums; ++i) {
      inputx[i] = in_flatx(i);  
    }

    vector<mpc_t> shared_x_vec(input_ele_nums);
    vector<mpc_t> shared_z_vec(input_ele_nums);
    //convert_double_to_mytype(inputx, shared_x_vec);
    tf_convert_double_to_mpctype(inputx, shared_x_vec);
    tfGetMpcOp(Log)->RunHd(shared_x_vec, shared_z_vec, input_ele_nums);

#if PRINT_REVEAL
    {
      vector<double> vc;
      debug_print_reveal(shared_x_vec, vc, "input X");
      debug_print_reveal(shared_z_vec, vc, "output Z");
      out_flat.setZero();
      for (int i = 0; i < input_ele_nums; i++) {
        out_flat(i) = vc[i];
      }
      debug_print_tensor(z, "MpcLog mpc(plain) z");
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

REGISTER_OP("MpcHLog")
    .Input("x: T")
    .Output("y: T")
    .Attr("T: {float, double, int32, int64, uint32, uint64}")
    .SetShapeFn(shape_inference::UnchangedShape)
    .Doc(R"doc(
MpcHLogOp
)doc");
REGISTER_MPCOP_KERNELS_ALL_TYPES(MpcHLog, MpcHLogOp)

} // namespace tensorflow
