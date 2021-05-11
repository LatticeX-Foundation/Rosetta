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
from tensorflow.python.ops import array_ops
from tensorflow import dtypes, zeros, ones, zeros_like, ones_like
from latticex.rosetta.rtt.utils.common import dtype_check_and_set
from latticex.rosetta.rtt.framework import rtt_tensor as rtt_ts
from tensorflow.python.framework import constant_op
from tensorflow.python.framework import ops
from tensorflow.python.framework import tensor_shape
from tensorflow.python.ops import gen_array_ops
import tensorflow as tf


def RttPlaceholder(dtype, shape=None, name=None):
    dtype = dtype_check_and_set(dtype)
    return rtt_ts.placeholder(array_ops.placeholder(dtype, shape, name))


def rtt_ones(shape, dtype=dtypes.float32, name=None):
  """Creates a tensor with all elements set to 1."""

  dtype = dtypes.as_dtype(dtype).base_dtype
  with ops.name_scope(name, "ones", [shape]) as name:
    one = True if dtype == dtypes.bool else 1
    if not isinstance(shape, ops.Tensor):
      try:
        # Create a constant if it won't be very big. Otherwise create a fill op
        # to prevent serialized GraphDefs from becoming too large.
        output = array_ops._constant_if_small(one, shape, dtype, name)
        if output is not None:
          return output

        # Go through tensor shapes to get int64-if-needed semantics
        shape = constant_op._tensor_shape_tensor_conversion_function(
            tensor_shape.TensorShape(shape))
      except (TypeError, ValueError):
        # Happens when shape is a list with tensor elements
        shape = ops.convert_to_tensor(shape, dtype=dtypes.int32)
    if not shape._shape_tuple():
      shape = tf.reshape(shape, [-1])  # Ensure it's a vector

    if (dtype == dtypes.string):
        output = tf.fill(shape, tf.constant('1', dtype=dtype), name=name)
    else:
        output = tf.fill(shape, tf.constant(one, dtype=dtype), name=name)
#   assert output.dtype.base_dtype == dtype
  return output


def rtt_zeros(shape, dtype=dtypes.float32, name=None):
  """Creates a tensor with all elements set to zero."""

  dtype = dtypes.as_dtype(dtype).base_dtype
  with ops.name_scope(name, "zeros", [shape]) as name:
    if dtype == dtypes.bool:
      zero = False
    elif dtype == dtypes.string:
      zero = ""
    else:
      zero = 0

    if not isinstance(shape, ops.Tensor):
      try:
        # Create a constant if it won't be very big. Otherwise create a fill op
        # to prevent serialized GraphDefs from becoming too large.
        output = array_ops._constant_if_small(zero, shape, dtype, name)
        if output is not None:
          return output

        # Go through tensor shapes to get int64-if-needed semantics
        shape = constant_op._tensor_shape_tensor_conversion_function(
            tensor_shape.TensorShape(shape))
      except (TypeError, ValueError):
        # Happens when shape is a list with tensor elements
        shape = ops.convert_to_tensor(shape, dtype=dtypes.int32)
    if not shape._shape_tuple():
      shape = tf.reshape(shape, [-1])  # Ensure it's a vector
    
    if (dtype == dtypes.string):
        output = tf.fill(shape, tf.constant('0', dtype=dtype), name=name)
    else:
        output = tf.fill(shape, tf.constant(zero, dtype=dtype), name=name)
#   assert output.dtype.base_dtype == dtype
  return output


# override placeholder/ones/zeors functions for RTT
tf.placeholder = RttPlaceholder
tf.ones = rtt_ones
tf.zeros = rtt_zeros
array_ops.ones = rtt_ones
array_ops.zeros = rtt_zeros

