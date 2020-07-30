# ==============================================================================
# Copyright 2020 The LatticeX Foundation
# This file is part of the Rosetta library.
#
# The Rosetta library is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The Rosetta library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with the Rosetta library. If not, see <http://www.gnu.org/licenses/>.
# =============================================================================="
import tensorflow as tf
from tensorflow.python.ops import math_ops
from latticex.rosetta.secure.decorator.secure_base_ import _secure_ops


# -----------------------------
# secure reduction ops
# -----------------------------
def SecureMax(input_tensor,
           axis=None,
           keepdims=None,
           name=None,
           reduction_indices=None,
           keep_dims=None):
    keepdims = False if keepdims is None else keepdims
    axis = math_ops._ReductionDims(input_tensor, axis)

    return _secure_ops.secure_reduce_max(input_tensor, reduction_indices=axis, name=name, keep_dims=keepdims)


def SecureMin(input_tensor,
           axis=None,
           keepdims=None,
           name=None,
           reduction_indices=None,
           keep_dims=None):
    keepdims = False if keepdims is None else keepdims
    axis = math_ops._ReductionDims(input_tensor, axis)

    return _secure_ops.secure_reduce_min(input_tensor, reduction_indices=axis, name=name, keep_dims=keepdims)


def SecureMean(input_tensor,
            axis=None,
            keepdims=None,
            name=None,
            reduction_indices=None,
            keep_dims=None):
    keepdims = False if keepdims is None else keepdims
    axis = math_ops._ReductionDims(input_tensor, axis)

    return _secure_ops.secure_reduce_mean(input_tensor, reduction_indices=axis, name=name, keep_dims=keepdims)


def SecureSum(input_tensor,
            axis=None,
            keepdims=None,
            name=None,
            reduction_indices=None,
            keep_dims=None):
    keepdims = False if keepdims is None else keepdims
    axis = math_ops._ReductionDims(input_tensor, axis)

    return _secure_ops.secure_reduce_sum(input_tensor, reduction_indices=axis, name=name, keep_dims=keepdims)

