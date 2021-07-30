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
#include <iostream>
#include <memory>
#include <assert.h>
#include <vector>
#include <string>

#include "Eigen/Core"
#include "Eigen/Dense"
#include "tensorflow/core/framework/op.h"
#include "tensorflow/core/framework/op_kernel.h"
#include "tensorflow/core/framework/shape_inference.h"
#include "tensorflow/core/util/bcast.h"

using Eigen::Dynamic;
using Eigen::Index;
using Eigen::Matrix;

using std::cout;
using std::endl;
using std::string;
using std::vector;

using namespace tensorflow;

#if 1 //defined(_NDEBUG)
#define PRINT_BEG(name)                                                                   \
  cout << "--- " << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << " =>\t" << name \
       << " rtt start ---" << endl                                                            \
       << std::flush
#define PRINT_END(name)                                                                   \
  cout << "--- " << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << " =>\t" << name \
       << " rtt end ---" << endl                                                              \
       << std::flush
#else
#define PRINT_BEG(name)
#define PRINT_END(name)
#endif

typedef double RttType;
typedef Matrix<RttType, Dynamic, Dynamic> MatrixRtt;

namespace tensorflow {
static std::pair<int, int> GetRowCol(const Tensor& input_tensor) {
  assert(input_tensor.NumElements() != 0 && "empty elements is not allowed");

  auto rows = input_tensor.shape().dim_size(0);
  auto cols = input_tensor.shape().dim_size(1);
  auto out_dims = input_tensor.shape().dims();
  if (out_dims == 0) {
    rows = 1;
    cols = 1;
  } else if (out_dims == 1) {
    rows = 1;
    cols = input_tensor.shape().dim_size(0);
  } else if (out_dims == 2) {
    rows = input_tensor.shape().dim_size(0);
    cols = input_tensor.shape().dim_size(1);
  } else {
    cout << "------  not support out_dims:" << out_dims << endl;
  }

  //cout << "row: " << rows << ", cols:  " << cols << endl;
  return std::make_pair<int, int>(rows, cols);
}

static int MatrixToRtt(const MatrixRtt& u64_matrix, Tensor& out_tensor) {
  std::pair<int, int> row_cols = GetRowCol(out_tensor);
  auto flat_out = out_tensor.flat<string>();

  int i = 0;
  for (auto r = 0; r < row_cols.first; ++r) {
    for (auto c = 0; c < row_cols.second; ++c) {
      flat_out(i++) = std::to_string(u64_matrix(r, c));
    }
  }

  return i == (row_cols.first * row_cols.second) ? 0 : -1;
}

static std::shared_ptr<MatrixRtt> GetDoubleMatrix(OpKernelContext* context, int index) {
  const Tensor& input_tensor = context->input(index);
  std::pair<int, int> row_cols = GetRowCol(input_tensor);

  auto rtt_matrix = std::make_shared<MatrixRtt>(row_cols.first, row_cols.second);
  auto flat_input = input_tensor.flat<string>();

  //cout << "total elems: " << input_tensor.NumElements() << endl;
  int i = 0;
  for (auto r = 0; r < row_cols.first; ++r) {
    for (auto c = 0; c < row_cols.second; ++c) {
      (*rtt_matrix)(r, c) = std::stod(flat_input(i++));
    }
  }

  return rtt_matrix;
}

template <typename Device>
class RttNegOp : public OpKernel {
 public:
  explicit RttNegOp(OpKernelConstruction* context) : OpKernel(context) {}

  void Compute(OpKernelContext* context) override {
    PRINT_BEG("Rttneg");
    throw;
  }
};

static std::shared_ptr<MatrixRtt> AdjustInputForBcast(
  OpKernelContext* context, const BCast& bcast, const Tensor* out, int index) {
  const TensorShape output_shape = BCast::ToShape(bcast.output_shape());
  auto inp = context->input(index);
  const auto& in_flat = inp.flat<std::string>();
  auto inp_dims = inp.dims();
  auto out_dims = output_shape.dims();

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

  std::shared_ptr<MatrixRtt> vec(new MatrixRtt(rows, cols));
  // row first
  if (out_dims == 0) {
    (*vec)(0, 0) = std::stod(in_flat(0));
  } else if (out_dims == 1) {
    if (inp_dims == 0) {
      for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
          (*vec)(i, j) = std::stod(in_flat(0)); // ()
        }
      }
    } else if (inp_dims == 1) {
      int inp_cols = inp.dim_size(0);
      for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
          if (inp_cols == 1)
            (*vec)(i, j) = std::stod(in_flat(0)); // (1,)
          else
            (*vec)(i, j) = std::stod(in_flat(j)); // (1,)
        }
      }
    }

  } else if (out_dims == 2) {
    if (inp_dims == 0) {
      for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
          (*vec)(i, j) = std::stod(in_flat(0)); // ()
        }
      }

    } else if (inp_dims == 1) {
      int inp_cols = inp.dim_size(0);
      for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
          if (inp_cols == 1)
            (*vec)(i, j) = std::stod(in_flat(0)); // (1,)
          else
            (*vec)(i, j) = std::stod(in_flat(j)); // (1,)
        }
      }

    } else if (inp_dims == 2) {
      int inp_rows = inp.dim_size(0);
      int inp_cols = inp.dim_size(1);
      if (inp_cols == 1) {
        for (int i = 0; i < rows; i++) {
          for (int j = 0; j < cols; j++) {
            if (rows == inp_rows)
              (*vec)(i, j) = std::stod(in_flat(i)); // (n, 1)
            else
              (*vec)(i, j) = std::stod(in_flat(0)); // (1, 1)
          }
        }
      } else {
        for (int i = 0; i < rows; i++) {
          for (int j = 0; j < cols; j++) {
            if (rows == inp_rows)
              (*vec)(i, j) = std::stod(in_flat(i * cols + j)); // (n, m)
            else {
              (*vec)(i, j) = std::stod(in_flat(j)); // (1, m)
            }
          }
        }
      }
    }
  }

  return vec;
} //AdjustInputForBcast

#if defined(_NDEBUG)
#define PRINT_BINARY_SHPAE(CTX, B)                                                           \
  {                                                                                          \
    auto in0 = CTX->input(0);                                                                \
    auto in1 = CTX->input(1);                                                                \
    const TensorShape output_shape = BCast::ToShape(B.output_shape());                       \
    {                                                                                        \
      cout << "        output.shape:" << output_shape << endl;                               \
      cout << "          in0.dims():" << in0.dims() << endl;                                 \
      cout << "          in1.dims():" << in1.dims() << endl;                                 \
      cout << " output_shape.dims():" << output_shape.dims() << endl;                        \
      cout << "               ndims:" << static_cast<int>(bcast.x_reshape().size()) << endl; \
      cout << "    in0_num_elements:" << in0.NumElements() << endl;                          \
      cout << "    in1_num_elements:" << in1.NumElements() << endl;                          \
      cout << "    out_num_elements:" << output_shape.num_elements() << endl;                \
    }                                                                                        \
  }
#else
#define PRINT_BINARY_SHPAE(CTX, B)   
#endif//


template <typename Device>
class RttSquareOp : public OpKernel {
 public:
  explicit RttSquareOp(OpKernelConstruction* context) : OpKernel(context) {}

  void Compute(OpKernelContext* context) override {
    PRINT_BEG("RttMul");
    throw;
  }
};

template <typename Device>
class RttExpOp : public OpKernel {
 public:
  explicit RttExpOp(OpKernelConstruction* context) : OpKernel(context) {}
  
  void Compute(OpKernelContext* context) {
    PRINT_BEG("RttExpOp");
    throw;
  }
};

template <typename Device>
class RttRsqrtOp : public OpKernel {
 public:
  explicit RttRsqrtOp(OpKernelConstruction* context) : OpKernel(context) {}
  
  void Compute(OpKernelContext* context) {
    PRINT_BEG("RttRsqrtOp");
    throw;
  }
};

template <typename Device>
class RttSqrtOp : public OpKernel {
 public:
  explicit RttSqrtOp(OpKernelConstruction* context) : OpKernel(context) {}
  
  void Compute(OpKernelContext* context) {
    PRINT_BEG("RttSqrtOp");
    throw;
  }
};

template <typename Device>
class RttAbsOp : public OpKernel {
 public:
  explicit RttAbsOp(OpKernelConstruction* context) : OpKernel(context) {}

  void Compute(OpKernelContext* context) override {
    PRINT_BEG("Abs");
    throw;
  }
};

#include <stdexcept>
#define THROW_FN throw std::runtime_error(string("please implements BinaryCompute in subclass"))

template <typename Device>
class RttBinaryOp : public OpKernel {
 protected:
  string name_;
  bool lh_is_const_ = false;
  bool rh_is_const_ = false;

 public:
  explicit RttBinaryOp(OpKernelConstruction* context, const string& name) : OpKernel(context), name_(name) {
    context->GetAttr("lh_is_const", &lh_is_const_);
    context->GetAttr("rh_is_const", &rh_is_const_);
  }

  virtual std::shared_ptr<MatrixRtt> BinaryCompute(std::shared_ptr<MatrixRtt> matrix0, 
                                                  std::shared_ptr<MatrixRtt> matrix1,
                                                  OpKernelContext* context) {
    THROW_FN;
  }

  void Compute(OpKernelContext* context) override {
    PRINT_BEG(name_);
    if (!(lh_is_const_ && lh_is_const_))
    {
      std::cout << "lh_is_const: " << lh_is_const_ << ", rh_is_const: " << rh_is_const_ << endl;
      //throw std::runtime_error(string("please use const for both inputs"));
    }

    // set up bcast
    BCast bcast(
      BCast::FromShape(context->input(0).shape()), BCast::FromShape(context->input(1).shape()));

    if (!bcast.IsValid()) {
      cout << "Incompatible shapes: " << context->input(0).shape().DebugString() << " vs. "
           << context->input(1).shape().DebugString() << endl;
      context->SetStatus(errors::InvalidArgument(
        "Incompatible shapes: ", context->input(0).shape().DebugString(), " vs. ",
        context->input(1).shape().DebugString()));
      return;
    }

    PRINT_BINARY_SHPAE(context, bcast);

    // set output
    Tensor* output;
    const TensorShape output_shape = BCast::ToShape(bcast.output_shape());
    OP_REQUIRES_OK(
      context, context->forward_input_or_allocate_output({0, 1}, 0, output_shape, &output));

    // adjust input or just get input matrix
    std::shared_ptr<MatrixRtt> matrix0 = GetDoubleMatrix(context, 0);
    std::shared_ptr<MatrixRtt> matrix1 = GetDoubleMatrix(context, 1);
    if (!(matrix0->cols() == matrix1->cols() && matrix0->rows() == matrix1->rows())) {
      // adjust input
      matrix0 = AdjustInputForBcast(context, bcast, output, 0);
      matrix1 = AdjustInputForBcast(context, bcast, output, 1);
    }

    // Binary op compute
    auto result = BinaryCompute(matrix0, matrix1, context);
    if (result.get() == nullptr)
    {
      std::cerr << "Exception Binary Compute get null !" << endl;
      return;
    }

    // to rrt output
    MatrixToRtt(*result, *output);

    PRINT_END(name_);
  }
};

template <typename Device>
class RttAddOp : public RttBinaryOp<Device> {
 public:
  explicit RttAddOp(OpKernelConstruction* context) : RttBinaryOp<Device>(context, "RttAddOp") {
  }

  std::shared_ptr<MatrixRtt> BinaryCompute(std::shared_ptr<MatrixRtt> matrix0, 
                                          std::shared_ptr<MatrixRtt> matrix1,
                                          OpKernelContext* context) override {
    // add
    *matrix0 = *matrix0 + *matrix1;
    return matrix0;
  }
};

template <typename Device>
class RttSubOp : public RttBinaryOp<Device> {
 public:
  explicit RttSubOp(OpKernelConstruction* context) : RttBinaryOp<Device>(context, "RttSubOp") {}

  std::shared_ptr<MatrixRtt> BinaryCompute(std::shared_ptr<MatrixRtt> matrix0, 
                                          std::shared_ptr<MatrixRtt> matrix1,
                                          OpKernelContext* context) override {
    // mpc sub
    *matrix0 = *matrix0 - *matrix1;
    return matrix0;
  }
};

template <typename Device>
class RttMulOp : public RttBinaryOp<Device> {
 public:
  explicit RttMulOp(OpKernelConstruction* context) : RttBinaryOp<Device>(context, "RttMulOp") {}

  std::shared_ptr<MatrixRtt> BinaryCompute(std::shared_ptr<MatrixRtt> matrix0, 
                                          std::shared_ptr<MatrixRtt> matrix1,
                                          OpKernelContext* context) override {
    // mpc dot product
    for (int i = 0; i < matrix0->rows(); ++i) {
      for (int j = 0; j < matrix0->cols(); ++j) {
        (*matrix0)(i, j) = (*matrix0)(i, j) * (*matrix1)(i, j);
      }
    }

    return matrix0;
  }
};

template <typename Device>
class RttDivOp : public RttBinaryOp<Device> {
 public:
  explicit RttDivOp(OpKernelConstruction* context) : RttBinaryOp<Device>(context, "RttDivOp") {}

  std::shared_ptr<MatrixRtt> BinaryCompute(std::shared_ptr<MatrixRtt> matrix0, 
                                          std::shared_ptr<MatrixRtt> matrix1,
                                          OpKernelContext* context) override {
    // div
    for (int i = 0; i < matrix0->rows(); ++i) {
      for (int j = 0; j < matrix0->cols(); ++j) {
        (*matrix0)(i, j) = ((*matrix0)(i, j)) / ((*matrix1)(i, j));
      }
    }

    return matrix0;
  }
};

template <typename Device>
class RttReciprocaldivOp : public RttBinaryOp<Device> {
 public:
  explicit RttReciprocaldivOp(OpKernelConstruction* context) : RttBinaryOp<Device>(context, "RttReciprocaldivOp") {}

  std::shared_ptr<MatrixRtt> BinaryCompute(std::shared_ptr<MatrixRtt> matrix0, std::shared_ptr<MatrixRtt> matrix1) override {
    // Reciprocaldiv
    for (int i = 0; i < matrix0->rows(); ++i) {
      for (int j = 0; j < matrix0->cols(); ++j) {
        (*matrix0)(i, j) = ((*matrix0)(i, j)) / ((*matrix1)(i, j));
      }
    }

    return matrix0;
  }
};

template <typename Device>
class RttFloordivOp : public RttBinaryOp<Device> {
 public:
  explicit RttFloordivOp(OpKernelConstruction* context) : RttBinaryOp<Device>(context, "RttFloolDivOp") {}

  std::shared_ptr<MatrixRtt> BinaryCompute(std::shared_ptr<MatrixRtt> matrix0, 
                                          std::shared_ptr<MatrixRtt> matrix1,
                                          OpKernelContext* context) override {
    // floor div
    for (int i = 0; i < matrix0->rows(); ++i) {
      for (int j = 0; j < matrix0->cols(); ++j) {
        (*matrix0)(i, j) = std::floor( ((*matrix0)(i, j)) / ((*matrix1)(i, j)) );
      }
    }

    return matrix0;
  }
};

template <typename Device>
class RttMatMulOp : public OpKernel {
 public:
  explicit RttMatMulOp(OpKernelConstruction* context) : OpKernel(context) {
    OP_REQUIRES_OK(context, context->GetAttr("transpose_a", &transpose_a_));
    OP_REQUIRES_OK(context, context->GetAttr("transpose_b", &transpose_b_));
  }

  void Compute(OpKernelContext* context) override {
    PRINT_BEG("RttMatMul");

    // Check if the dimensions of the two matrices are valid
    const Tensor& x = context->input(0);
    const Tensor& y = context->input(1);

    cout << "input1: (" << x.dim_size(0) << "," << x.dim_size(1) << ")"
         << ", dims: " << x.dims() << endl;
    cout << "input2: (" << y.dim_size(0) << "," << y.dim_size(1) << ")"
         << ", dims: " << y.dims() << endl;
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

    // get u64 matrix
    std::shared_ptr<MatrixRtt> matrix0 = GetDoubleMatrix(context, 0);
    std::shared_ptr<MatrixRtt> matrix1 = GetDoubleMatrix(context, 1);

    // mpc matmul
    if (transpose_a_)
      matrix0.reset(new MatrixRtt(matrix0->transpose()));

    if (transpose_b_)
      matrix1.reset(new MatrixRtt(matrix1->transpose()));

    auto res = (*matrix0) * (*matrix1);

    Tensor* output;
    TensorShape shape({matrix0->rows(), matrix1->cols()});
    OP_REQUIRES_OK(context, context->allocate_output(0, shape, &output));

// #ifndef _NDEBUG
//     cout << "==== matmul (" << res.rows() << "," << res.cols() << "), output: " << endl;
//     for (auto r = 0; r < res.rows(); r++) {
//       for (auto c = 0; c < res.cols(); c++) {
//         cout << res(r, c) << "\t";
//       }
//       cout << endl;
//     }
//     cout << "========================\n" << endl;
// #endif
    // to rrt output
    MatrixToRtt(res, *output);

    PRINT_END("RttMatMul");
  }

 private:
  bool transpose_a_;
  bool transpose_b_;
};

template <typename Device>
class RttLessOp : public RttBinaryOp<Device> {
 public:
  explicit RttLessOp(OpKernelConstruction* context)  : RttBinaryOp<Device>(context, "RttLessOp") { }

  std::shared_ptr<MatrixRtt> BinaryCompute(std::shared_ptr<MatrixRtt> matrix0, 
                                          std::shared_ptr<MatrixRtt> matrix1,
                                          OpKernelContext* context) override {
    for (int i = 0; i < matrix0->rows(); ++i) {
      for (int j = 0; j < matrix0->cols(); ++j) {
        (*matrix0)(i, j) = ((*matrix0)(i, j)) < ((*matrix1)(i, j)) ? 1 : 0;
      }
    }

    return matrix0;
  }
};

template <typename Device>
class RttLessEqualOp : public RttBinaryOp<Device> {
 public:
  explicit RttLessEqualOp(OpKernelConstruction* context)  : RttBinaryOp<Device>(context, "RttLessEqualOp") {}

  std::shared_ptr<MatrixRtt> BinaryCompute(std::shared_ptr<MatrixRtt> matrix0, 
                                          std::shared_ptr<MatrixRtt> matrix1,
                                          OpKernelContext* context) override {
    for (int i = 0; i < matrix0->rows(); ++i) {
      for (int j = 0; j < matrix0->cols(); ++j) {
        (*matrix0)(i, j) = ((*matrix0)(i, j)) <= ((*matrix1)(i, j)) ? 1 : 0;
      }
    }

    return matrix0;
  }
};

template <typename Device>
class RttEqualOp : public RttBinaryOp<Device> {
 public:
  explicit RttEqualOp(OpKernelConstruction* context)  : RttBinaryOp<Device>(context, "RttEqualOp") { }

  std::shared_ptr<MatrixRtt> BinaryCompute(std::shared_ptr<MatrixRtt> matrix0, 
                                          std::shared_ptr<MatrixRtt> matrix1,
                                          OpKernelContext* context) override {
    for (int i = 0; i < matrix0->rows(); ++i) {
      for (int j = 0; j < matrix0->cols(); ++j) {
        (*matrix0)(i, j) = ((*matrix0)(i, j)) == ((*matrix1)(i, j)) ? 1 : 0;
      }
    }

    return matrix0;
  }
};

template <typename Device>
class RttNotEqualOp : public RttBinaryOp<Device> {
 public:
  explicit RttNotEqualOp(OpKernelConstruction* context)  : RttBinaryOp<Device>(context, "RttNotEqualOp") {}

  std::shared_ptr<MatrixRtt> BinaryCompute(std::shared_ptr<MatrixRtt> matrix0, 
                                          std::shared_ptr<MatrixRtt> matrix1,
                                          OpKernelContext* context) override {
    for (int i = 0; i < matrix0->rows(); ++i) {
      for (int j = 0; j < matrix0->cols(); ++j) {
        (*matrix0)(i, j) = ((*matrix0)(i, j)) != ((*matrix1)(i, j)) ? 1 : 0;
      }
    }

    return matrix0;
  }
};

template <typename Device>
class RttGreaterOp : public RttBinaryOp<Device> {
 public:
  explicit RttGreaterOp(OpKernelConstruction* context)  : RttBinaryOp<Device>(context, "RttGreaterOp") {}

  std::shared_ptr<MatrixRtt> BinaryCompute(std::shared_ptr<MatrixRtt> matrix0, 
                                          std::shared_ptr<MatrixRtt> matrix1,
                                          OpKernelContext* context) override {
    for (int i = 0; i < matrix0->rows(); ++i) {
      for (int j = 0; j < matrix0->cols(); ++j) {
        (*matrix0)(i, j) = ((*matrix0)(i, j)) > ((*matrix1)(i, j)) ? 1 : 0;
      }
    }

    return matrix0;
  }
};

template <typename Device>
class RttGreaterEqualOp : public RttBinaryOp<Device> {
 public:
  explicit RttGreaterEqualOp(OpKernelConstruction* context)  : RttBinaryOp<Device>(context, "RttGreaterEqualOp") {}

  std::shared_ptr<MatrixRtt> BinaryCompute(std::shared_ptr<MatrixRtt> matrix0, 
                                          std::shared_ptr<MatrixRtt> matrix1,
                                          OpKernelContext* context) override {
    for (int i = 0; i < matrix0->rows(); ++i) {
      for (int j = 0; j < matrix0->cols(); ++j) {
        (*matrix0)(i, j) = ((*matrix0)(i, j)) >= ((*matrix1)(i, j)) ? 1 : 0;
      }
    }

    return matrix0;
  }
};

// logical ops
template <typename Device>
class RttLogicalAndOp : public RttBinaryOp<Device> {
 public:
  explicit RttLogicalAndOp(OpKernelConstruction* context)  : RttBinaryOp<Device>(context, "RttLogicalAndOp") {}

  std::shared_ptr<MatrixRtt> BinaryCompute(std::shared_ptr<MatrixRtt> matrix0, 
                                          std::shared_ptr<MatrixRtt> matrix1,
                                          OpKernelContext* context) override {
    throw;
  }
};
template <typename Device>
class RttLogicalOrOp : public RttBinaryOp<Device> {
 public:
  explicit RttLogicalOrOp(OpKernelConstruction* context)  : RttBinaryOp<Device>(context, "RttLogicalOrOp") {}

  std::shared_ptr<MatrixRtt> BinaryCompute(std::shared_ptr<MatrixRtt> matrix0, 
                                          std::shared_ptr<MatrixRtt> matrix1,
                                          OpKernelContext* context) override {
    throw;
  }
};
template <typename Device>
class RttLogicalXorOp : public RttBinaryOp<Device> {
 public:
  explicit RttLogicalXorOp(OpKernelConstruction* context)  : RttBinaryOp<Device>(context, "RttLogicalXorOp") {}

  std::shared_ptr<MatrixRtt> BinaryCompute(std::shared_ptr<MatrixRtt> matrix0, 
                                          std::shared_ptr<MatrixRtt> matrix1,
                                          OpKernelContext* context) override {
    throw;
  }
};
template <typename Device>
class RttLogicalNotOp : public OpKernel {
 public:
  explicit RttLogicalNotOp(OpKernelConstruction* context) : OpKernel(context) {}
  
  void Compute(OpKernelContext* context) {
    PRINT_BEG("RttLogicalNotOp");
    throw;
  }
};

struct RttMeanFunctor {
  RttMeanFunctor() {}
  ~RttMeanFunctor() {}

  void operator()(const vector<double>& a, vector<string>& out, size_t rows, size_t cols) {
    // TODO
    PRINT_BEG("RttMeanFunctor");
    for (int i = 0; i < rows; i++) {
      double v = 0;
      for (int j = 0; j < cols; j++) {
        int idx = i * cols + j;
        v += a[idx];
      }

      out[i] = std::to_string(v / cols);
    }
  }
};

struct RttMaxFunctor {
  RttMaxFunctor() {}
  ~RttMaxFunctor() {}

  void operator()(const vector<double>& a, vector<string>& out, size_t rows, size_t cols) {
    // TODO
    PRINT_BEG("RttMaxFunctor");
    for (int i = 0; i < rows; i++) {
      double v = a[i*cols];
      for (int j = 0; j < cols; j++) {
        int idx = i * cols + j;
        if(v < a[idx])
          v = a[idx];
      }

      out[i] = std::to_string(v);
    }
  }
};

struct RttMinFunctor {
  RttMinFunctor() {}
  ~RttMinFunctor() {}

  void operator()(const vector<double>& a, vector<string>& out, size_t rows, size_t cols) {
    // TODO
    PRINT_BEG("RttMinFunctor");
    for (int i = 0; i < rows; i++) {
      double v = a[i*cols];
      for (int j = 0; j < cols; j++) {
        int idx = i * cols + j;
        if(v > a[idx])
          v = a[idx];
      }

      out[i] = std::to_string(v);
    }
  }
};

struct RttSumFunctor {
  RttSumFunctor() {}
  ~RttSumFunctor() {}

  void operator()(const vector<double>& a, vector<string>& out, size_t rows, size_t cols) {
    // TODO
    PRINT_BEG("RttSumFunctor");
    for (int i = 0; i < rows; i++) {
      double v = 0;
      for (int j = 0; j < cols; j++) {
        int idx = i * cols + j;
        v += a[idx];
      }

      out[i] = std::to_string(v);
    }
  }
};

template <typename Device, typename Functor>
class RttReduceOp : public OpKernel {
 private:
  bool keep_dims_ = true;

 public:
  explicit RttReduceOp(OpKernelConstruction* context) : OpKernel(context) {
    OP_REQUIRES_OK(context, context->GetAttr("keep_dims", &keep_dims_));
  }

  vector<double> convert_to_uint64(const vector<string>& a) {
    vector<double> out(a.size(), 0);
    for (auto i = 0; i < a.size(); ++i) {
      out[i] = std::stod(a[i]);
    }

    return out;
  }

  void Compute(OpKernelContext* context) {
    PRINT_BEG("RttReduceOp");
    throw;
  } //compute
};

template <typename Device>
class RttLogOp : public OpKernel {
 public:
  explicit RttLogOp(OpKernelConstruction* context) : OpKernel(context) {}
  
  void Compute(OpKernelContext* context) {
    PRINT_BEG("RttLogOp");
    throw;
  }
};

template <typename Device>
class RttLog1pOp : public OpKernel {
 public:
  explicit RttLog1pOp(OpKernelConstruction* context) : OpKernel(context) {}
  
  void Compute(OpKernelContext* context) {
    PRINT_BEG("RttLog1pOp");
    throw;
  }
};

template <typename Device>
class RttPowOp : public OpKernel {
 public:
  explicit RttPowOp(OpKernelConstruction* context) : OpKernel(context) {}
  
  void Compute(OpKernelContext* context) {
    PRINT_BEG("RttPowOp");
    throw;
  }
};

template <typename Device>
class RttArgMaxOp : public OpKernel {
 public:
  explicit RttArgMaxOp(OpKernelConstruction* context) : OpKernel(context) {}
  
  void Compute(OpKernelContext* context) {
    PRINT_BEG("RttArgMaxOp");
    throw;
  }
};


////////////////   register kernels (with string type only)   ////////////////
using CPUDevice=Eigen::ThreadPoolDevice;

// reduce opkernel
REGISTER_KERNEL_BUILDER(Name("RttReduceMean").Device(DEVICE_CPU), RttReduceOp<CPUDevice, RttMeanFunctor>);
REGISTER_KERNEL_BUILDER(Name("RttReduceSum").Device(DEVICE_CPU), RttReduceOp<CPUDevice, RttSumFunctor>);
REGISTER_KERNEL_BUILDER(Name("RttReduceMin").Device(DEVICE_CPU), RttReduceOp<CPUDevice, RttMaxFunctor>);
REGISTER_KERNEL_BUILDER(Name("RttReduceMax").Device(DEVICE_CPU), RttReduceOp<CPUDevice, RttMaxFunctor>);
REGISTER_KERNEL_BUILDER(Name("RttArgMax").Device(DEVICE_CPU), RttArgMaxOp<CPUDevice>);

// register kernel builder of basic math ops
REGISTER_KERNEL_BUILDER(Name("RttNegative").Device(DEVICE_CPU), RttNegOp<CPUDevice>);
REGISTER_KERNEL_BUILDER(Name("RttAdd").Device(DEVICE_CPU), RttAddOp<CPUDevice>);
REGISTER_KERNEL_BUILDER(Name("RttSub").Device(DEVICE_CPU), RttSubOp<CPUDevice>);
REGISTER_KERNEL_BUILDER(Name("RttMul").Device(DEVICE_CPU), RttMulOp<CPUDevice>);
REGISTER_KERNEL_BUILDER(Name("RttExp").Device(DEVICE_CPU), RttExpOp<CPUDevice>);
REGISTER_KERNEL_BUILDER(Name("RttRsqrt").Device(DEVICE_CPU), RttRsqrtOp<CPUDevice>);
REGISTER_KERNEL_BUILDER(Name("RttSqrt").Device(DEVICE_CPU), RttSqrtOp<CPUDevice>);
REGISTER_KERNEL_BUILDER(Name("RttSquare").Device(DEVICE_CPU), RttSquareOp<CPUDevice>);
REGISTER_KERNEL_BUILDER(Name("RttDiv").Device(DEVICE_CPU), RttDivOp<CPUDevice>);
REGISTER_KERNEL_BUILDER(Name("RttReciprocaldiv").Device(DEVICE_CPU), RttDivOp<CPUDevice>);
REGISTER_KERNEL_BUILDER(Name("RttRealdiv").Device(DEVICE_CPU), RttDivOp<CPUDevice>);
REGISTER_KERNEL_BUILDER(Name("RttTruediv").Device(DEVICE_CPU), RttDivOp<CPUDevice>);
REGISTER_KERNEL_BUILDER(Name("RttFloordiv").Device(DEVICE_CPU), RttFloordivOp<CPUDevice>);
REGISTER_KERNEL_BUILDER(Name("RttMatmul").Device(DEVICE_CPU), RttMatMulOp<CPUDevice>);
REGISTER_KERNEL_BUILDER(Name("RttLess").Device(DEVICE_CPU), RttLessOp<CPUDevice>);
REGISTER_KERNEL_BUILDER(Name("RttLessEqual").Device(DEVICE_CPU), RttLessEqualOp<CPUDevice>);
REGISTER_KERNEL_BUILDER(Name("RttEqual").Device(DEVICE_CPU), RttEqualOp<CPUDevice>);
REGISTER_KERNEL_BUILDER(Name("RttNotEqual").Device(DEVICE_CPU), RttNotEqualOp<CPUDevice>);
REGISTER_KERNEL_BUILDER(Name("RttGreater").Device(DEVICE_CPU), RttGreaterOp<CPUDevice>);
REGISTER_KERNEL_BUILDER(Name("RttGreaterEqual").Device(DEVICE_CPU), RttGreaterEqualOp<CPUDevice>);
REGISTER_KERNEL_BUILDER(Name("RttAbs").Device(DEVICE_CPU), RttAbsOp<CPUDevice>);
REGISTER_KERNEL_BUILDER(Name("RttLog").Device(DEVICE_CPU), RttLogOp<CPUDevice>);
REGISTER_KERNEL_BUILDER(Name("RttLog1p").Device(DEVICE_CPU), RttLog1pOp<CPUDevice>);
REGISTER_KERNEL_BUILDER(Name("RttPow").Device(DEVICE_CPU), RttPowOp<CPUDevice>);

// logical ops
REGISTER_KERNEL_BUILDER(Name("RttLogicalAnd").Device(DEVICE_CPU), RttLogicalAndOp<CPUDevice>);
REGISTER_KERNEL_BUILDER(Name("RttLogicalOr").Device(DEVICE_CPU), RttLogicalOrOp<CPUDevice>);
REGISTER_KERNEL_BUILDER(Name("RttLogicalXor").Device(DEVICE_CPU), RttLogicalXorOp<CPUDevice>);
REGISTER_KERNEL_BUILDER(Name("RttLogicalNot").Device(DEVICE_CPU), RttLogicalNotOp<CPUDevice>);

} //namespace tensorflow