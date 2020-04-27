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

class MpcBinaryOpFunctor : public BaseFunctor {
  using BaseFunctor::BaseFunctor;

 public:
  virtual ~MpcBinaryOpFunctor() = default;
  virtual std::string opname() {
    return "";
  }
};

class MpcPowOpFunctor : public MpcBinaryOpFunctor {
  using MpcBinaryOpFunctor::MpcBinaryOpFunctor;

 public:
  virtual std::string opname() {
    return "Pow";
  }
  template <typename T1, typename T2>
  void operator()(const vector<T1>& a, const vector<T2>& b, vector<mpc_t>& c, size_t size) {
    cout << "pow not supported!" << endl;
    throw;
  }
};

template <>
void MpcPowOpFunctor::operator()(
  const vector<mpc_t>& a, const vector<double>& b, vector<mpc_t>& c, size_t size) {
  // special for pow const. c = a^n
  // T1 is mpc_t
  // T2 is double (convert to int64_t)
  vector<int64_t> n(b.size());
  for (int i = 0; i < b.size(); i++) {
    n[i] = static_cast<int64_t>(b[i]);
  }
  tfGetMpcOp(Pow)->Run(a, n, c, size);
}

class MpcAddOpFunctor : public MpcBinaryOpFunctor {
  using MpcBinaryOpFunctor::MpcBinaryOpFunctor;

 public:
  template <typename T1, typename T2>
  void operator()(const vector<T1>& a, const vector<T2>& b, vector<mpc_t>& c, size_t size) {
    tfGetMpcOp(Add)->Run(a, b, c, size);
  }
};
class MpcSubOpFunctor : public MpcBinaryOpFunctor {
  using MpcBinaryOpFunctor::MpcBinaryOpFunctor;

 public:
  template <typename T1, typename T2>
  void operator()(const vector<T1>& a, const vector<T2>& b, vector<mpc_t>& c, size_t size) {
    tfGetMpcOp(Sub)->Run(a, b, c, size);
  }
};
class MpcMulOpFunctor : public MpcBinaryOpFunctor {
  using MpcBinaryOpFunctor::MpcBinaryOpFunctor;

 public:
  template <typename T1, typename T2>
  void operator()(const vector<T1>& a, const vector<T2>& b, vector<mpc_t>& c, size_t size) {
    tfGetMpcOp(Mul)->Run(a, b, c, size);
  }
};
class MpcDivOpFunctor : public MpcBinaryOpFunctor {
  using MpcBinaryOpFunctor::MpcBinaryOpFunctor;

 public:
  template <typename T1, typename T2>
  void operator()(const vector<T1>& a, const vector<T2>& b, vector<mpc_t>& c, size_t size) {
    tfGetMpcOp(Div)->Run(a, b, c, size);
  }
};
class MpcTruedivOpFunctor : public MpcBinaryOpFunctor {
  using MpcBinaryOpFunctor::MpcBinaryOpFunctor;

 public:
  template <typename T1, typename T2>
  void operator()(const vector<T1>& a, const vector<T2>& b, vector<mpc_t>& c, size_t size) {
    tfGetMpcOp(Truediv)->Run(a, b, c, size);
  }
};
class MpcGreaterOpFunctor : public MpcBinaryOpFunctor {
  using MpcBinaryOpFunctor::MpcBinaryOpFunctor;

 public:
  template <typename T1, typename T2>
  void operator()(const vector<T1>& a, const vector<T2>& b, vector<mpc_t>& c, size_t size) {
    tfGetMpcOp(Greater)->Run(a, b, c, size);
  }
};
class MpcLessOpFunctor : public MpcBinaryOpFunctor {
  using MpcBinaryOpFunctor::MpcBinaryOpFunctor;

 public:
  template <typename T1, typename T2>
  void operator()(const vector<T1>& a, const vector<T2>& b, vector<mpc_t>& c, size_t size) {
    tfGetMpcOp(Less)->Run(a, b, c, size);
  }
};
class MpcEqualOpFunctor : public MpcBinaryOpFunctor {
  using MpcBinaryOpFunctor::MpcBinaryOpFunctor;

 public:
  template <typename T1, typename T2>
  void operator()(const vector<T1>& a, const vector<T2>& b, vector<mpc_t>& c, size_t size) {
    tfGetMpcOp(Equal)->Run(a, b, c, size);
  }
};
class MpcGreaterEqualOpFunctor : public MpcBinaryOpFunctor {
  using MpcBinaryOpFunctor::MpcBinaryOpFunctor;

 public:
  template <typename T1, typename T2>
  void operator()(const vector<T1>& a, const vector<T2>& b, vector<mpc_t>& c, size_t size) {
    tfGetMpcOp(GreaterEqual)->Run(a, b, c, size);
  }
};
class MpcLessEqualOpFunctor : public MpcBinaryOpFunctor {
  using MpcBinaryOpFunctor::MpcBinaryOpFunctor;

 public:
  template <typename T1, typename T2>
  void operator()(const vector<T1>& a, const vector<T2>& b, vector<mpc_t>& c, size_t size) {
    tfGetMpcOp(LessEqual)->Run(a, b, c, size);
  }
};

namespace tensorflow {

template <
  typename Device, typename T, typename Functor, typename OutT = T, bool StandaloneOutT = false>
class MpcBinaryOp : public MpcOpKernel {
 protected:
  // ref cwise_ops_common.h
  struct BinaryOpState {
    // Sets up bcast with the shape of in0 and in1, ensures that the bcast
    // is valid, and if so, set out, either by allocating a new buffer using
    // ctx->output(...) or by creating an alias for an owned input buffer for
    // in-place computation.
    // Caller must check ctx->status() upon return for non-ok status.
    // If ctx->status().ok() is true, then out is guaranteed to be allocated.
    BinaryOpState(OpKernelContext* ctx)
        : in0(ctx->input(0)),
          in1(ctx->input(1)),
          bcast(BCast::FromShape(in0.shape()), BCast::FromShape(in1.shape())) {
      if (!bcast.IsValid()) {
        cout << "Incompatible shapes: " << in0.shape().DebugString() << " vs. "
             << in1.shape().DebugString() << endl;
        ctx->SetStatus(errors::InvalidArgument(
          "Incompatible shapes: ", in0.shape().DebugString(), " vs. ", in1.shape().DebugString()));
        return;
      }
      const TensorShape output_shape = BCast::ToShape(bcast.output_shape());
      out_num_elements = output_shape.num_elements();
      in0_num_elements = in0.NumElements();
      in1_num_elements = in1.NumElements();
      OP_REQUIRES_OK(ctx, ctx->forward_input_or_allocate_output({0, 1}, 0, output_shape, &out));

      ndims = static_cast<int>(bcast.x_reshape().size());
      in0_dims = in0.dims();
      in1_dims = in1.dims();
      out_dims = output_shape.dims();

      if (verbose_ > 1) {
        cout << "        output.shape:" << output_shape << endl;
        cout << "          in0.dims():" << in0.dims() << endl;
        cout << "          in1.dims():" << in1.dims() << endl;
        cout << " output_shape.dims():" << output_shape.dims() << endl;
        cout << "               ndims:" << ndims << endl;
        cout << "    in0_num_elements:" << in0_num_elements << endl;
        cout << "    in1_num_elements:" << in1_num_elements << endl;
        cout << "    out_num_elements:" << out_num_elements << endl;
      }
    }
    int verbose_ = 1;

    const Tensor& in0;
    const Tensor& in1;

    BCast bcast;
    Tensor* out = nullptr;

    int64 in0_num_elements;
    int64 in1_num_elements;
    int64 out_num_elements;

    int in0_dims;
    int in1_dims;
    int out_dims;

    int ndims;
  };

  bool lh_is_const_ = false;
  bool rh_is_const_ = false;

 public:
  explicit MpcBinaryOp(OpKernelConstruction* context) : MpcOpKernel(context) {
    OP_REQUIRES_OK(context, context->GetAttr("lh_is_const", &lh_is_const_));
    OP_REQUIRES_OK(context, context->GetAttr("rh_is_const", &rh_is_const_));
#if PRINT_REVEAL
    cout << "input0 is " << string(lh_is_const_ ? "const" : "not const") << ", input1 is "
         << string(rh_is_const_ ? "const" : "not const") << endl;
#endif
    if (lh_is_const_ && rh_is_const_) {
      throw mpc_not_supported_exp("MpcBinaryOp inputs are const");
    }
  }

  void MpcCompute(OpKernelContext* context) {
    string opname(TYPENAME(typeid(Functor).name()));
    BinaryOpState state(context);
    if (!context->status().ok())
      return;
    if (state.out_num_elements == 0) {
      // no any inputs or outputs
      return;
    }
    Tensor* out = state.out;
    BCast* bcast = &state.bcast;
    auto& in0 = state.in0;
    auto& in1 = state.in1;
    const Tensor& x0 = context->input(0);
    const Tensor& x1 = context->input(1);
    const auto& in0_flat = x0.flat<T>();
    const auto& in1_flat = x1.flat<T>();

    const int ndims = state.ndims;
    int in0_dims = state.in0_dims;
    int in1_dims = state.in1_dims;
    int out_dims = state.out_dims;
    if ((in0_dims > 2) || (in1_dims > 2)) {
      // not support
      throw mpc_not_supported_exp(opname + " ndims:" + std::to_string(ndims));
    }

    // assuming element-wise
    size_t size = state.out_num_elements;
    vector<double> input0;
    vector<double> input1;

    int rows = 0, cols = 0;
    int shape_case = 0;
    if (out_dims == 0) {
      rows = 1;
      cols = 1;
    } else if (out_dims == 1) {
      rows = 1;
      cols = out->dim_size(0);
    } else if (out_dims == 2) {
      rows = out->dim_size(0);
      cols = out->dim_size(1);
    }

    ///////////////////////////////////////
    vector<vector<double>> vec0(rows, vector<double>(cols));
    vector<vector<double>> vec1(rows, vector<double>(cols));
    auto f = [&](vector<vector<double>>& vec, int index) {
      const Tensor& inp = context->input(index);
      const auto& in_flat = inp.flat<T>();
      int inp_dims = inp.dims();

      // row first
      if (out_dims == 0) {
        vec[0][0] = in_flat(0);
      } else if (out_dims == 1) {
        if (inp_dims == 0) {
          for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
              vec[i][j] = in_flat(0); // ()
            }
          }
        } else if (inp_dims == 1) {
          int inp_cols = inp.dim_size(0);
          for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
              if (inp_cols == 1)
                vec[i][j] = in_flat(0); // (1,)
              else
                vec[i][j] = in_flat(j); // (1,)
            }
          }
        }

      } else if (out_dims == 2) {
        if (inp_dims == 0) {
          for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
              vec[i][j] = in_flat(0); // ()
            }
          }
        } else if (inp_dims == 1) {
          int inp_cols = inp.dim_size(0);
          for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
              if (inp_cols == 1)
                vec[i][j] = in_flat(0); // (1,)
              else
                vec[i][j] = in_flat(j); // (1,)
            }
          }
        } else if (inp_dims == 2) {
          int inp_rows = inp.dim_size(0);
          int inp_cols = inp.dim_size(1);
          if (inp_cols == 1) {
            for (int i = 0; i < rows; i++) {
              for (int j = 0; j < cols; j++) {
                if (rows == inp_rows)
                  vec[i][j] = in_flat(i); // (n, 1)
                else
                  vec[i][j] = in_flat(0); // (1, 1)
              }
            }
          } else {
            for (int i = 0; i < rows; i++) {
              for (int j = 0; j < cols; j++) {
                if (rows == inp_rows)
                  vec[i][j] = in_flat(i * cols + j); // (n, m)
                else {
                  vec[i][j] = in_flat(j); // (1, m)
                }
              }
            }
          }
        }
      }
    };
    f(vec0, 0);
    f(vec1, 1);
    ///////////////////////////////////////

    // output raw data
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
        int idx = i * cols + j;
        input0.push_back(vec0[i][j]);
        if (idx < 2) {
#if PRINT_REVEAL
          cout << "input0 (i,j,I)" << i << " " << j << " " << idx << " : " << input0[idx] << endl;
#endif
        }
      }
    }
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
        int idx = i * cols + j;
        input1.push_back(vec1[i][j]);
        if (idx < 2) {
#if PRINT_REVEAL
          cout << "input1 (i,j,I)" << i << " " << j << " " << idx << " : " << input1[idx] << endl;
#endif
        }
      }
    }

    vector<mpc_t> a, b, c(size);
    tf_convert_double_to_mpctype(input0, a);
    tf_convert_double_to_mpctype(input1, b);
    //convert_double_to_mpctype(input0, a);
    //convert_double_to_mpctype(input1, b);

    Functor functor(baseop());
    if (functor.opname() == "Pow") {
      lh_is_const_ = false;
      rh_is_const_ = true;
    }
    if (lh_is_const_) {
      functor(input0, b, c, size);
    } else if (rh_is_const_) {
      functor(a, input1, c, size);
    } else {
      functor(a, b, c, size);
    }

#if PRINT_REVEAL
    {
      vector<double> vc;
      if (lh_is_const_)
        debug_print_reveal(input0, "input a");
      else
        debug_print_reveal(a, vc, "input a");
      if (rh_is_const_)
        debug_print_reveal(input1, "input b");
      else
        debug_print_reveal(b, vc, "input b");
      debug_print_reveal(c, vc, "output c");
    }
#endif

    auto out_flat = out->flat<T>();
    out_flat.setZero();

    if (!StandaloneOutT) {
      vector<double> output(size);
      //convert_mpctype_to_double(c, output);
      tf_convert_mpctype_to_double(c, output);

      for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
          int idx = i * cols + j;
          out_flat(idx) = output[idx];
        }
      }
    } else { // directly use the mytype, for compare type ops
      for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
          int idx = i * cols + j;
          out_flat(idx) = c[idx];
        }
      }
    }
#if PRINT_REVEAL
    debug_print_tensor(out, string(opname + " mpc out").c_str());
#endif
  }
}; // namespace tensorflow
} // namespace tensorflow