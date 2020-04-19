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

class MpcUnaryOpFunctor {
 public:
  virtual void operator()(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size) = 0;
};

namespace tensorflow {

template <typename Device, typename T, typename Funtor>
class MpcUnaryOp : public MpcOpKernel {
 public:
  explicit MpcUnaryOp(OpKernelConstruction* context) : MpcOpKernel(context) {}
  void MpcCompute(OpKernelContext* context) {
    const Tensor& inp = context->input(0);
    Tensor* out = nullptr;
    OP_REQUIRES_OK(context, context->allocate_output(0, inp.shape(), &out));
    // todo
  }
};

} // namespace tensorflow