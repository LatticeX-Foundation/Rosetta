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
#include "cc/tf/secureops/secure_base_kernel.h"
#include "cc/tf/secureops/secure_reduction_ops_common.h"

namespace tensorflow {

//! @note uncomment ///DETAIL/// for debug
void SecureReduceOp::ComputeImpl(OpKernelContext* context) {
  const Tensor& data = context->input(0);
  const Tensor& axes = context->input(1);
  VLOG(1) << "data shape: " << data.shape().DebugString();
  VLOG(1) << "axes      : " << axes.SummarizeValue(10);

  ReductionHelper_RTT helper;
  OP_REQUIRES_OK(context, helper.Simplify(data, axes, keep_dims_));
  CHECK_GE(helper.ndims(), 0);

  if (helper.ndims() == 0 || (helper.ndims() == 1 && !helper.reduce_first_axis())) {
    Tensor out;
    if (!out.CopyFrom(data, helper.out_shape())) {
      context->SetStatus(errors::Internal("Error during reduction copy."));
    }
    context->set_output(0, out);
    return;
  }

  // We must allocate temp tensors using the same alloc attr as
  // output(0) because it is returned as output(0) in the end.
  const AllocatorAttributes alloc_attr = context->output_alloc_attr(0);

  // A temporary tensor whose size matches the size of the reduced
  // output.
  Tensor tmp_out;
  OP_REQUIRES_OK(
    context,
    context->allocate_temp(
      context->expected_output_dtype(0), helper.out_reshape(), &tmp_out, alloc_attr));

  log_debug << "data shape: " << data.shape().DebugString();
  log_debug << "axes      : " << axes.SummarizeValue(10);
  log_debug << "keep_dims:" << keep_dims_;
  log_debug << "tmp_out:" << tmp_out.DebugString(9);
  log_debug << "data.NumElements():" << data.NumElements();
  log_debug << "axes.NumElements():" << axes.NumElements();
  log_debug << "helper.ndims():" << helper.ndims();

  ////////////////////////
  auto input_shape = data.shape();
  auto ds = input_shape.dim_sizes();
  // for (int i = 0; i < ds.size(); i++) {
  //   log_info << "ds[" << i << "]:" << ds[i];
  // }
  int ds_size = ds.size();
  if (ds_size <= 0) {
    log_error << "ds_size <= 0:" << ds_size;
    throw std::runtime_error("ds_size <= 0:" + to_string(ds_size));
  }

  const auto& axes_flat = axes.flat<int>();
  vector<int> axs;
  for (int64_t i = 0; i < axes.NumElements(); i++) {
    axs.push_back(axes_flat(i));
  }

  const auto& input_flat = data.flat<string>();
  auto tmpout_flat = tmp_out.flat<string>();
  //////////////////////////////////////////////////////////

  log_debug << "input_flat:>>>>size:" << input_flat.size();
  log_debug << "input_flat:>>>>NumElements:" << data.NumElements();

  int64_t size = data.NumElements();
  ///DETAIL/// vector<string> input_text(size);
  ///DETAIL/// for (int64_t i = 0; i < size; i++) {
  ///DETAIL///   input_text[i] = input_flat(i);
  ///DETAIL/// }
  ///DETAIL/// vector<double> input_plain(size);
  ///DETAIL/// ProtocolManager::Instance()
  ///DETAIL///   ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
  ///DETAIL///   ->GetOps(msg_id())
  ///DETAIL///   ->Reveal(input_text, input_plain);

  int64_t vec0_size = 1;
  int64_t vec0_2nd_size = size;
  int64_t dn = size;
  int64_t d0, d1, d2;
  int axis = axs[0];
  ///DETAIL/// vector<vector<double>> vec0;
  vector<vector<string>> svec0;
  //////////////////////////////////////////////////////////
  int64_t rows = 1, cols = 1;
  // ds_size 3                -- input data shape size
  // ds 0,1,2 [index]         -- input data shape
  // axs 0,2  0,1   1         -- axis
  // _axs 1    2   0,2        -- ds - axis
  vector<int> _axs;
  for (int i = 0; i < axs.size(); i++) {
    cols *= ds[axs[i]];
  }
  for (int i = 0; i < ds_size; i++) {
    _axs.push_back(i);
    for (auto sz : axs) {
      if (i == sz) {
        _axs.pop_back();
        break;
      }
    }
  }
  rows = size / cols;
  ///DETAIL/// cout << "rows:" << rows << endl;
  ///DETAIL/// cout << "cols:" << cols << endl;

  ///DETAIL/// vec0.resize(rows);
  svec0.resize(rows);
  //////////////////////////////////////////////////////////
  if (ds_size == 1 || ds_size == axs.size()) {
    for (int64_t i = 0; i < input_flat.size(); i++) {
      int64_t index = i;
      int64_t vec_index = 0;
      ///DETAIL/// vec0[vec_index].push_back(input_plain[index]);
      svec0[vec_index].push_back(input_flat(index));
    }
  } else if (ds_size == 2) {
    for (int64_t i = 0; i < ds[0]; i++) {
      for (int64_t j = 0; j < ds[1]; j++) {
        int64_t index = i * ds[1] + j;
        ///DETAIL/// cout << input_plain[index] << " ";
        int64_t vec_index = 0;
        if (_axs.size() == 1) {
          if (_axs[0] == 0) {
            vec_index = i;
          } else if (_axs[0] == 1) {
            vec_index = j;
          }
        }
        ///DETAIL/// vec0[vec_index].push_back(input_plain[index]);
        svec0[vec_index].push_back(input_flat(index));
      }
      ///DETAIL/// cout << endl;
    }
    ///DETAIL/// cout << endl;
  } else if (ds_size == 3) {
    for (int64_t i = 0; i < ds[0]; i++) {
      for (int64_t j = 0; j < ds[1]; j++) {
        for (int64_t k = 0; k < ds[2]; k++) {
          int64_t index = i * ds[1] * ds[2] + j * ds[2] + k;
          ///DETAIL/// cout << input_plain[index] << " ";
          int64_t vec_index = 0;
          if (_axs.size() == 1) {
            if (_axs[0] == 0) {
              vec_index = i;
            } else if (_axs[0] == 1) {
              vec_index = j;
            } else if (_axs[0] == 2) {
              vec_index = k;
            }
          } else if (_axs.size() == 2) {
            if (_axs[0] == 0 && _axs[1] == 1) {
              vec_index = i * ds[1] + j;
            } else if (_axs[0] == 0 && _axs[1] == 2) {
              vec_index = i * ds[2] + k;
            } else if (_axs[0] == 1 && _axs[1] == 2) {
              vec_index = j * ds[2] + k;
            }
          }
          ///DETAIL/// vec0[vec_index].push_back(input_plain[index]);
          svec0[vec_index].push_back(input_flat(index));
        }
        ///DETAIL/// cout << endl;
      }
      ///DETAIL/// cout << endl;
    }
    ///DETAIL/// cout << endl;
  } else if (ds_size == 4) {
    for (int64_t i = 0; i < ds[0]; i++) {
      for (int64_t j = 0; j < ds[1]; j++) {
        for (int64_t k = 0; k < ds[2]; k++) {
          for (int64_t l = 0; l < ds[3]; l++) {
            int64_t index = i * ds[1] * ds[2] * ds[3] + j * ds[2] * ds[3] + k * ds[3] + l;
            ///DETAIL/// cout << input_plain[index] << " ";
            int64_t vec_index = 0;
            if (_axs.size() == 1) {
              // 0 1 2 3
              if (_axs[0] == 0) {
                vec_index = i;
              } else if (_axs[0] == 1) {
                vec_index = j;
              } else if (_axs[0] == 2) {
                vec_index = k;
              } else if (_axs[0] == 3) {
                vec_index = l;
              }
            } else if (_axs.size() == 2) {
              // 01/02/03 12/13 23
              if (_axs[0] == 0 && _axs[1] == 1) {
                vec_index = i * ds[1] + j;
              } else if (_axs[0] == 0 && _axs[1] == 2) {
                vec_index = i * ds[2] + k;
              } else if (_axs[0] == 0 && _axs[1] == 3) {
                vec_index = i * ds[3] + l;
              } else if (_axs[0] == 1 && _axs[1] == 2) {
                vec_index = j * ds[2] + k;
              } else if (_axs[0] == 1 && _axs[1] == 3) {
                vec_index = j * ds[3] + l;
              } else if (_axs[0] == 2 && _axs[1] == 3) {
                vec_index = k * ds[3] + l;
              }
            } else if (_axs.size() == 3) {
              // 012/013/023 123
              if (_axs[0] == 0 && _axs[1] == 1 && _axs[2] == 2) {
                vec_index = i * ds[1] * ds[2] + j * ds[2] + k;
              } else if (_axs[0] == 0 && _axs[1] == 1 && _axs[2] == 3) {
                vec_index = i * ds[1] * ds[3] + j * ds[3] + l;
              } else if (_axs[0] == 0 && _axs[1] == 2 && _axs[2] == 3) {
                vec_index = i * ds[2] * ds[3] + k * ds[3] + l;
              } else if (_axs[0] == 1 && _axs[1] == 2 && _axs[2] == 3) {
                vec_index = j * ds[2] * ds[3] + k * ds[3] + l;
              }
            }
            ///DETAIL/// vec0[vec_index].push_back(input_plain[index]);
            svec0[vec_index].push_back(input_flat(index));
          }
          ///DETAIL/// cout << endl;
        }
        ///DETAIL/// cout << endl;
      }
      ///DETAIL/// cout << endl;
    }
    ///DETAIL/// cout << endl;
  } else {
    log_error << "ds_size >= 5" << ds_size;
    throw std::runtime_error("ds_size >= 5:" + to_string(ds_size));
  }

  ///DETAIL/// cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
  ///DETAIL/// cout << "vec0.size():" << vec0.size() << ",vec0[0].size():" << vec0[0].size() << endl;
  ///DETAIL/// cout << "svec0.size():" << svec0.size() << ",svec0[0].size():" << svec0[0].size() << endl;
  ///DETAIL/// cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;

  vector<string> input_str;
  for (int64_t i = 0; i < rows; i++) {
    ///DETAIL/// double s = 0.0;
    ///DETAIL/// for (int64_t j = 0; j < cols; j++) {
    ///DETAIL///   s += vec0[i][j];
    ///DETAIL/// }
    ///DETAIL/// cout << s / cols << " ";
    input_str.insert(input_str.end(), svec0[i].begin(), svec0[i].end());
  }
  ///DETAIL/// cout << endl;

  // row first
  // operation on each row, the shape from [rows, cols] to [rows]
  attrs_["rows"] = std::to_string(rows);
  attrs_["cols"] = std::to_string(cols);

  // compute with protocol
  vector<string> outs;
  ReduceCompute(input_str, outs, rows, cols, context);
  log_debug << "----------------------2: rows:" << rows << ",cols:" << cols
            << ",outs.size():" << outs.size() << "--------" ;

  // re-assign
  for (int64_t i = 0; i < tmpout_flat.size(); i++) {
    tmpout_flat(i) = std::move(outs[i]);
  }

  ////////////////////////
  // Set the real output using the contents of the reduction but the
  // real expected output shape.  The number of elements should
  // match between the two shapes.
  Tensor out;
  if (!out.CopyFrom(tmp_out, helper.out_shape())) {
    context->SetStatus(errors::Internal("Error during reduction copy."));
  }
  log_debug << "out:" << out.DebugString(9);
  context->set_output(0, out);
}

} // namespace tensorflow
