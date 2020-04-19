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
#pragma once

#include "mpcop_kernel.h"

class MpcReduceOpFunctor : public BaseFunctor {
  using BaseFunctor::BaseFunctor;

 public:
  virtual ~MpcReduceOpFunctor() = default;
  virtual void operator()(
    const vector<mpc_t>& a, vector<mpc_t>& b, vector<mpc_t>& indexs, size_t rows, size_t cols) = 0;
};

class MpcMeanOpFunctor : public MpcReduceOpFunctor {
  using MpcReduceOpFunctor::MpcReduceOpFunctor;

 public:
  void operator()(
    const vector<mpc_t>& a, vector<mpc_t>& b, vector<mpc_t>& indexs, size_t rows, size_t cols) {
    indexs.resize(a.size());
    tfGetMpcOp(Mean)->Run(a, b, rows, cols);
  }
};
class MpcMaxOpFunctor : public MpcReduceOpFunctor {
  using MpcReduceOpFunctor::MpcReduceOpFunctor;

 public:
  void operator()(
    const vector<mpc_t>& a, vector<mpc_t>& b, vector<mpc_t>& indexs, size_t rows, size_t cols) {
    tfGetMpcOp(Max)->Run(a, b, indexs, rows, cols);
  }
};
class MpcMinOpFunctor : public MpcReduceOpFunctor {
  using MpcReduceOpFunctor::MpcReduceOpFunctor;

 public:
  void operator()(
    const vector<mpc_t>& a, vector<mpc_t>& b, vector<mpc_t>& indexs, size_t rows, size_t cols) {
    // to do
    tfGetMpcOp(Max)->Run(a, b, indexs, rows, cols);
  }
};
class MpcSumOpFunctor : public MpcReduceOpFunctor {
  using MpcReduceOpFunctor::MpcReduceOpFunctor;

 public:
  void operator()(
    const vector<mpc_t>& a, vector<mpc_t>& b, vector<mpc_t>& indexs, size_t rows, size_t cols) {
    // to do
    tfGetMpcOp(Max)->Run(a, b, indexs, rows, cols);
  }
};

namespace tensorflow {

template <typename Device, typename T, typename Functor>
class MpcReduceOp : public MpcOpKernel {
  bool keep_dims_;

 public:
  explicit MpcReduceOp(OpKernelConstruction* context) : MpcOpKernel(context) {
    OP_REQUIRES_OK(context, context->GetAttr("keep_dims", &keep_dims_));
    cout << "keep_dims:" << keep_dims_ << endl;
  }
  // axis is None (-1)
  // now, only supports at most 2 dims
  void MpcCompute(OpKernelContext* context) {
    string opname(TYPENAME(typeid(Functor).name()));
    int32_t axis = -1;
    const Tensor& x = context->input(0);
    auto in_flatx = x.flat<T>();
    int dims = x.dims();

    const Tensor& y = context->input(1);
    if (dims == y.NumElements()) {
      axis = -1;
    } else {
      auto rindices = y.flat<int32_t>(); // === reduction_indices === axis
      axis = rindices(0);
    }

    Tensor* z = nullptr;

    int rows, cols;
    if (dims == 0) {
      rows = 1;
      cols = 1;
    } else if (dims == 1) {
      rows = 1;
      cols = x.dim_size(0);
    } else if (dims == 2) {
      rows = x.dim_size(0);
      cols = x.dim_size(1);
    } else {
      OP_REQUIRES_OK(context, context->allocate_output(0, x.shape(), &z));
      cout << "only supported 0 <= dim <= 2, not supported dims:" << dims << endl;
      return;
    }

    int shape_size = rows;
    if (axis == 0) {
      shape_size = cols;
    } else if (axis == 1) {
      shape_size = rows;
    } else {
      shape_size = 1;
    }
    if (dims <= 1) {
      shape_size = 1;
    }

    if ((dims <= 1) || (axis == -1)) {
      TensorShape zshape;
      OP_REQUIRES_OK(context, context->allocate_output(0, zshape, &z));
    } else {
      TensorShape zshape({shape_size});
      OP_REQUIRES_OK(context, context->allocate_output(0, zshape, &z));
    }

    auto out_flat = z->flat<T>();
    out_flat.setZero();

    // assuming element-wise
    vector<double> inputx;
    if (axis == 0) { // col first
      for (int j = 0; j < cols; j++) {
        for (int i = 0; i < rows; i++) {
          int idx = i * cols + j;
#if PRINT_REVEAL
          if (idx < 2)
            cout << "input (i,j,I)" << i << " " << j << " " << idx << " : " << in_flatx(idx)
                 << endl;
#endif
          inputx.push_back(in_flatx(idx));
        }
      }
    } else { // axis == 1 or axis is None (-1)
      //} else if (axis == 1) { // row first
      for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
          int idx = i * cols + j;
#if PRINT_REVEAL
          if (idx < 2)
            cout << "input (i,j,I)" << i << " " << j << " " << idx << " : " << in_flatx(idx)
                 << endl;
#endif
          inputx.push_back(in_flatx(idx));
        }
      }
    }

    size_t size = inputx.size();
    vector<mpc_t> a, b(shape_size), c(size);
    //convert_double_to_mytype(inputx, a);
    tf_convert_double_to_mpctype(inputx, a);

    // return each row's max/mean/sum/... value
    Functor functor(baseop());
    if ((dims == 1) && (axis == 0)) {
      functor(a, b, c, 1, size);
    } else {
      if (axis == 0) {
        functor(a, b, c, cols, rows);
      } else if (axis == 1) {
        functor(a, b, c, rows, cols);
      } else {
        functor(a, b, c, 1, size);
      }
    }

#if PRINT_REVEAL
    {
      vector<double> vc;
      debug_print_reveal(a, vc, "input a");
      debug_print_reveal(b, vc, "output b");
      debug_print_reveal(c, vc, "output b");
    }
#endif

    vector<double> outputz(size);
    //convert_mytype_to_double(b, outputz);
    tf_convert_mpctype_to_double(b, outputz);

    out_flat.setZero();
    for (int i = 0; i < shape_size; i++) {
      out_flat(i) = outputz[i];
    }

    // debug_print_tensor(z, string(opname + " mpc z").c_str());
  }
};
} // namespace tensorflow
