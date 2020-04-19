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
class MpcMatmulOp : public MpcOpKernel {
  bool transpose_a_;
  bool transpose_b_;

 public:
  explicit MpcMatmulOp(OpKernelConstruction* context) : MpcOpKernel(context) {
    OP_REQUIRES_OK(context, context->GetAttr("transpose_a", &transpose_a_));
    OP_REQUIRES_OK(context, context->GetAttr("transpose_b", &transpose_b_));
    //cout << "transpose_a:" << transpose_a_ << endl;
    //cout << "transpose_b:" << transpose_b_ << endl;
  }

  void MpcCompute(OpKernelContext* context) {
    const Tensor& x = context->input(0);
    const Tensor& y = context->input(1);

    // Check that the dimensions of the two matrices are valid.
    OP_REQUIRES(
      context, TensorShapeUtils::IsMatrix(x.shape()),
      errors::InvalidArgument(
        "In[0] is not a matrix. Instead it has shape ", x.shape().DebugString()));
    OP_REQUIRES(
      context, TensorShapeUtils::IsMatrix(y.shape()),
      errors::InvalidArgument(
        "In[1] is not a matrix. Instead it has shape ", y.shape().DebugString()));

    Eigen::array<Eigen::IndexPair<Eigen::DenseIndex>, 1> dim_pair;
    dim_pair[0].first = transpose_a_ ? 0 : 1;
    dim_pair[0].second = transpose_b_ ? 1 : 0;

    OP_REQUIRES(
      context, x.dim_size(dim_pair[0].first) == y.dim_size(dim_pair[0].second),
      errors::InvalidArgument(
        "Matrix size-incompatible: In[0]: ", x.shape().DebugString(),
        ", In[1]: ", y.shape().DebugString()));
    int a_dim_remaining = 1 - dim_pair[0].first;
    int b_dim_remaining = 1 - dim_pair[0].second;

    // (m,K) * (K,n) = (m,n)
    int m = x.dim_size(a_dim_remaining);
    int n = y.dim_size(b_dim_remaining);
    int K = x.dim_size(dim_pair[0].first);
  #if PRINT_REVEAL
    cout << "(m,K,n):" << m << "," << K << "," << n << endl;
  #endif
    TensorShape zshape({m, n});
    Tensor* z = nullptr;
    OP_REQUIRES_OK(context, context->allocate_output(0, zshape, &z));

    if (z->NumElements() == 0) {
      // If x has shape [0, x] or y has shape [x, 0], the output shape
      // is a 0-element matrix, so there is nothing to do.
      return;
    }

    auto out_flat = z->flat<T>();
    out_flat.setZero();
  #if PRINT_REVEAL
    debug_print_tensor(z, "MpcMatmulOp");
  #endif
    if (x.NumElements() == 0 && y.NumElements() == 0) {
      // If x has shape [x, 0] and y has shape [0, y], the
      // output shape is [x, y] where x and y are non-zero, so we fill
      // the output with zeros.
      return;
    }

    if (!(std::is_same<T, float>::value || std::is_same<T, double>::value)) {
      // for mpc, only support float and double at present
      cerr << "not supported for T == " << typeid(T).name() << endl;
      return;
    }

    auto in_flatx = x.flat<T>();
    auto in_flaty = y.flat<T>();

#if USE_MPC_OP
    vector<double> inputx(m * K);
    vector<double> inputy(K * n);
    for (int k = 0; k <= K - 1; k++) {
      for (int i = 0; i <= m - 1; i++) {
        int idx = i * K + k;
        inputx[idx] = in_flatx(idx);
      }
      for (int j = 0; j <= n - 1; j++) {
        int idx = k * n + j;
        inputy[idx] = in_flaty(idx);
      }
    }

    vector<mpc_t> a(m * K, 0), b(K * n, 0), c(m * n, 0);
    //convert_double_to_mytype(inputx, a);
    tf_convert_double_to_mpctype(inputx, a);
    //convert_double_to_mytype(inputy, b);
    tf_convert_double_to_mpctype(inputy, b);
    tfGetMpcOp(MatMul)->Run(a, b, c, m, K, n, transpose_a_ ? 1 : 0, transpose_b_ ? 1 : 0);

#if PRINT_REVEAL
    {
      vector<double> vc;
      debug_print_reveal(a, vc, "input a");
      debug_print_reveal(b, vc, "input b");
      debug_print_reveal(c, vc, "output c");

      out_flat.setZero();
      for (int i = 0; i <= m - 1; i++) {
        for (int j = 0; j <= n - 1; j++) {
          int idx = i * n + j;
          out_flat(idx) = vc[idx];
        }
      }

      debug_print_tensor(z, "MpcMatmulOp mpc(plain) z");
    }
#endif

    vector<double> outputz(m * n);
    //convert_mytype_to_double(c, outputz);
    tf_convert_mpctype_to_double(c, outputz);

    out_flat.setZero();
    for (int i = 0; i <= m - 1; i++) {
      for (int j = 0; j <= n - 1; j++) {
        int idx = i * n + j;
        out_flat(idx) = outputz[idx];
      }
    }
  #if PRINT_REVEAL
    debug_print_tensor(z, "MpcMatmulOp mpc z");
  #endif
#endif
  }
}; // namespace tensorflow

REGISTER_OP("MpcMatMul")
  .Input("a: T")
  .Input("b: T")
  .Output("product: T")
  .Attr("transpose_a: bool = false")
  .Attr("transpose_b: bool = false")
  .Attr("T: {float, double, int32, int64, uint32, uint64}")
  //.SetShapeFn(shape_inference::MatMulShape)
  .Doc(R"doc(
MpcMatmulOp
)doc");

REGISTER_MPCOP_KERNELS_ALL_TYPES(MpcMatMul, MpcMatmulOp)

} // namespace tensorflow
