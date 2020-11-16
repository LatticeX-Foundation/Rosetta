#pragma once

#include <iostream>
#include <memory>
#include <assert.h>
#include <vector>
#include <string>

// #include "Eigen/Core"
// #include "Eigen/Dense"
#include "tensorflow/core/framework/op.h"
#include "tensorflow/core/framework/op_kernel.h"
#include "tensorflow/core/framework/shape_inference.h"
#include "tensorflow/core/util/bcast.h"

// using Eigen::Dynamic;
// using Eigen::Index;
// using Eigen::Matrix;

using std::cout;
using std::endl;
using std::string;
using std::vector;

using namespace tensorflow;

// typedef Matrix<string, Dynamic, Dynamic> MatrixRtt;

namespace rosett {
namespace utils {

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
    log_error << "------  not support out_dims:" << out_dims << endl;
  }

  //log_debug << "row: " << rows << ", cols:  " << cols << endl;
  return std::make_pair<int, int>(rows, cols);
}

static int vec_to_tensor(const vector<string>& vec, int rows, int cols) {
  Tensor out_tensor;
  std::pair<int, int> row_cols = GetRowCol(out_tensor);
  auto flat_out = out_tensor.flat<string>();

  int i = 0;
  for (auto r = 0; r < rows; ++r) {
    for (auto c = 0; c < cols; ++c) {
      flat_out(i++) = std::to_string(vec[r*cols + c]);
    }
  }

  return i == (rows * cols) ? 0 : -1;
}

static std::shared_ptr<MatrixRtt> GetDoubleMatrix(OpKernelContext* context, int index) {
  const Tensor& input_tensor = context->input(index);
  std::pair<int, int> row_cols = GetRowCol(input_tensor);

  auto rtt_matrix = std::make_shared<MatrixRtt>(row_cols.first, row_cols.second);
  auto flat_input = input_tensor.flat<string>();

  //log_debug << "total elems: " << input_tensor.NumElements() << endl;
  int i = 0;
  for (auto r = 0; r < row_cols.first; ++r) {
    for (auto c = 0; c < row_cols.second; ++c) {
      (*rtt_matrix)(r, c) = std::stod(flat_input(i++));
    }
  }

  return rtt_matrix;
}

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

} // namespace utils
} // namespace rosetta

