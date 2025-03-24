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

#include "tensorflow/core/framework/common_shape_fns.h"
#include "tensorflow/core/framework/op.h"
#include "tensorflow/core/framework/shape_inference.h"

/// Note[georgeshi]: for now, please only use INT32 or INT64!
REGISTER_OP("TfToSecure")
  .Attr("dtype: {int32, int64, float, double, string}")
  .Input("input: dtype")
  .Output("output: string");

/// Note[georgeshi]: we can NOT use string in native TF op
REGISTER_OP("SecureToTf")
  .Attr("dtype: {float, double, int32, int64, string}")
  .Input("input: string")
  .Output("output: dtype");

REGISTER_OP("PrivateInput")
  .Attr("dtype: {int32, int64, float, double, string}")
  .Input("input: dtype")
  .Input("data_owner: string")
  .Output("output: string");

