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
#include "reduce_op.h"

// Reduce OP: Mean/Max/Min/Sum/...
namespace tensorflow {

namespace shape_inference {

template <typename T>
Status ReductionShapeHelper2(
  const Tensor* reduction_indices_t, const int32 input_rank, std::set<int64>* true_indices,
  int64_t* rindex) {
  auto reduction_indices = reduction_indices_t->flat<T>();
  for (int i = 0; i < reduction_indices_t->NumElements(); ++i) {
    const T reduction_index = reduction_indices(i);
    if (reduction_index == -1) {
      *rindex = reduction_index;
      true_indices->insert(0);
      continue;
      //return Status::OK();
    }
    if (reduction_index < -input_rank || reduction_index >= input_rank) {
      return errors::InvalidArgument(
        "Invalid reduction dimension ", reduction_index, " for input with ", input_rank,
        " dimensions.");
    }

    auto wrapped_index = reduction_index;
    if (wrapped_index < 0) {
      wrapped_index += input_rank;
    }

    true_indices->insert(wrapped_index);
  }
  return Status::OK();
}

Status ReductionShape2(InferenceContext* c) {
  ShapeHandle input = c->input(0);

  ShapeHandle indices;
  // Older versions of TensorFlow accidentally allowed higher rank tensors like
  // [[1,2]] or [[1],[2]] to represent axis=[1,2].
  if (c->graph_def_version() < 21) {
    indices = c->input(1);
  } else {
    TF_RETURN_IF_ERROR(c->WithRankAtMost(c->input(1), 1, &indices));
  }

  bool keep_dims;
  TF_RETURN_IF_ERROR(c->GetAttr("keep_dims", &keep_dims));

  const Tensor* reduction_indices_t = c->input_tensor(1);
  if (reduction_indices_t == nullptr || !c->RankKnown(input)) {
    // If we do not have the reduction values at runtime, or the
    // rank of the input, we don't know the output shape.

    if (keep_dims && c->RankKnown(input)) {
      // output rank matches input input if <keep_dims>.
      c->set_output(0, c->UnknownShapeOfRank(c->Rank(input)));
      return Status::OK();
    } else {
      return shape_inference::UnknownShape(c);
    }
  }

  const int32 input_rank = c->Rank(input);
  std::set<int64> true_indices;
  int64_t rindex = 0;
  if (reduction_indices_t->dtype() == DataType::DT_INT32) {
    TF_RETURN_IF_ERROR(
      ReductionShapeHelper2<int32>(reduction_indices_t, input_rank, &true_indices, &rindex));
  } else if (reduction_indices_t->dtype() == DataType::DT_INT64) {
    TF_RETURN_IF_ERROR(
      ReductionShapeHelper2<int64>(reduction_indices_t, input_rank, &true_indices, &rindex));
  } else {
    return errors::InvalidArgument("reduction_indices can only be int32 or int64");
  }

  std::vector<DimensionHandle> dims;
  if (rindex == -1) {
    if (input_rank == 2) {
      c->set_output(0, c->MakeShape(dims));
      return Status::OK();
    }
  }

  for (int i = 0; i < input_rank; ++i) {
    if (true_indices.count(i) > 0) {
      if (keep_dims) {
        dims.emplace_back(c->MakeDim(1));
      }
    } else {
      dims.emplace_back(c->Dim(input, i));
    }
  }

  c->set_output(0, c->MakeShape(dims));
  return Status::OK();
}
} // namespace shape_inference

#define REGISTER_MPC_REDUCE_OP(name)         \
  REGISTER_OP(#name)                         \
    .Input("input: T")                       \
    .Input("reduction_indices: Tidx")        \
    .Output("output: T")                     \
    .Attr("keep_dims: bool = false")         \
    .Attr("T: numbertype")                   \
    .Attr("Tidx: {int32, int64} = DT_INT32") \
    // .SetShapeFn(shape_inference::ReductionShape2)

REGISTER_MPC_REDUCE_OP(MpcMean).Doc(R"doc(
    MpcMean
)doc");

REGISTER_MPC_REDUCE_OP(MpcMax).Doc(R"doc(
    MpcMax
)doc");

REGISTER_MPC_REDUCE_OP(MpcMin).Doc(R"doc(
    MpcMin
)doc");

REGISTER_MPC_REDUCE_OP(MpcSum).Doc(R"doc(
    MpcSum
)doc");

#define REGISTER_MPCOP_KERNELS_ALL_TYPES_REDUCEOP REGISTER_MPCOP_KERNELS_ALL_TYPES_BINARYOP
REGISTER_MPCOP_KERNELS_ALL_TYPES_REDUCEOP(MpcMean, MpcReduceOp, MpcMeanOpFunctor)
REGISTER_MPCOP_KERNELS_ALL_TYPES_REDUCEOP(MpcMax, MpcReduceOp, MpcMaxOpFunctor)
REGISTER_MPCOP_KERNELS_ALL_TYPES_REDUCEOP(MpcMin, MpcReduceOp, MpcMinOpFunctor)
REGISTER_MPCOP_KERNELS_ALL_TYPES_REDUCEOP(MpcSum, MpcReduceOp, MpcSumOpFunctor)
} // namespace tensorflow