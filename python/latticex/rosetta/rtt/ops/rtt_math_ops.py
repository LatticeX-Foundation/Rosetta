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
from tensorflow.python.framework import dtypes
from latticex.rosetta.rtt.framework import rtt_tensor as rtt_ts
from tensorflow.python.ops import gen_math_ops
from tensorflow.python.util import deprecation
import os



def rtt_neg(x, name=None):
    """Computes numerical negative value element-wise."""
    x = rtt_ts.convert_to_rtttensor(x)
    _result = rtt_ts.rtt_ops.rtt_negative(x._raw, name=name)
    return rtt_ts.RttTensor(_result)


def rtt_add(x, y, name=None):
    """Returns x + y element-wise."""
    x = rtt_ts.convert_to_rtttensor(x)
    y = rtt_ts.convert_to_rtttensor(y)
    _result = rtt_ts.rtt_ops.rtt_add(x._raw, y._raw, name=name)
    return rtt_ts.RttTensor(_result)


def rtt_sub(x, y, name=None):
    """Returns x - y element-wise."""
    x = rtt_ts.convert_to_rtttensor(x)
    y = rtt_ts.convert_to_rtttensor(y)
    _result = rtt_ts.rtt_ops.rtt_sub(x._raw, y._raw, name=name)
    return rtt_ts.RttTensor(_result)


def rtt_mul(x, y, name=None):
    """Returns x * y element-wise."""
    x = rtt_ts.convert_to_rtttensor(x)
    y = rtt_ts.convert_to_rtttensor(y)
    _result = rtt_ts.rtt_ops.rtt_mul(x._raw, y._raw, name=name)
    return rtt_ts.RttTensor(_result)


def rtt_div(x, y, name=None):
    """Divides x / y elementwise (using Python 2 division operator semantics."""
    x = rtt_ts.convert_to_rtttensor(x)
    y = rtt_ts.convert_to_rtttensor(y)
    _result = rtt_ts.rtt_ops.rtt_div(x._raw, y._raw, name=name)
    return rtt_ts.RttTensor(_result)


def rtt_floordiv(x, y, name=None):
    """Divides `x / y` elementwise, rounding toward the most negative integer."""
    x = rtt_ts.convert_to_rtttensor(x)
    y = rtt_ts.convert_to_rtttensor(y)
    _result = rtt_ts.rtt_ops.rtt_floordiv(x._raw, y._raw, name=name)
    return rtt_ts.RttTensor(_result)


def rtt_truediv(x, y, name=None):
    """Divides x / y elementwise (using Python 3 division operator semantics)."""
    x = rtt_ts.convert_to_rtttensor(x)
    y = rtt_ts.convert_to_rtttensor(y)
    _result = rtt_ts.rtt_ops.rtt_truediv(x._raw, y._raw, name=name)
    return rtt_ts.RttTensor(_result)


def rtt_realdiv(x, y, name=None):
    """Returns x / y element-wise for real types."""
    x = rtt_ts.convert_to_rtttensor(x)
    y = rtt_ts.convert_to_rtttensor(y)
    _result = rtt_ts.rtt_ops.rtt_realdiv(x._raw, y._raw, name=name)
    return rtt_ts.RttTensor(_result)


def rtt_reciprocaldiv(x, y, name=None):
    """Divides x / y elementwise (using Python 2 division operator semantics."""
    x = rtt_ts.convert_to_rtttensor(x)
    y = rtt_ts.convert_to_rtttensor(y)
    _result = rtt_ts.rtt_ops.rtt_reciprocaldiv(x._raw, y._raw, name=name)
    return rtt_ts.RttTensor(_result)


def rtt_equal(x, y, name=None):
    """Returns the truth value of (x == y) element-wise."""
    x = rtt_ts.convert_to_rtttensor(x)
    y = rtt_ts.convert_to_rtttensor(y)
    _result = rtt_ts.rtt_ops.rtt_equal(x._raw, y._raw, name=name)
    return rtt_ts.RttTensor(_result)


def rtt_notequal(x, y, name=None):
    """Returns the truth value of (x != y) element-wise."""
    x = rtt_ts.convert_to_rtttensor(x)
    y = rtt_ts.convert_to_rtttensor(y)
    _result = rtt_ts.rtt_ops.rtt_not_equal(x._raw, y._raw, name=name)
    return rtt_ts.RttTensor(_result)


def rtt_greater(x, y, name=None):
    """Returns the truth value of (x > y) element-wise."""
    x = rtt_ts.convert_to_rtttensor(x)
    y = rtt_ts.convert_to_rtttensor(y)
    _result = rtt_ts.rtt_ops.rtt_greater(x._raw, y._raw, name=name)
    return rtt_ts.RttTensor(_result)


def rtt_greaterequal(x, y, name=None):
    """Returns the truth value of (x >= y) element-wise."""
    x = rtt_ts.convert_to_rtttensor(x)
    y = rtt_ts.convert_to_rtttensor(y)
    _result = rtt_ts.rtt_ops.rtt_greater_equal(x._raw, y._raw, name=name)
    return rtt_ts.RttTensor(_result)


def rtt_less(x, y, name=None):
    """Returns the truth value of (x < y) element-wise."""
    x = rtt_ts.convert_to_rtttensor(x)
    y = rtt_ts.convert_to_rtttensor(y)
    _result = rtt_ts.rtt_ops.rtt_less(x._raw, y._raw, name=name)
    return rtt_ts.RttTensor(_result)


def rtt_lessequal(x, y, name=None):
    """Returns the truth value of (x <= y) element-wise."""
    x = rtt_ts.convert_to_rtttensor(x)
    y = rtt_ts.convert_to_rtttensor(y)
    _result = rtt_ts.rtt_ops.rtt_less_equal(x._raw, y._raw, name=name)
    return rtt_ts.RttTensor(_result)


def rtt_logical_and(x, y, name=None):
    """Returns the truth value of (x & y) element-wise.."""
    x = rtt_ts.convert_to_rtttensor(x)
    y = rtt_ts.convert_to_rtttensor(y)
    _result = rtt_ts.rtt_ops.rtt_logical_and(x._raw, y._raw, name=name)
    return rtt_ts.RttTensor(_result)


def rtt_logical_or(x, y, name=None):
    """Returns the truth value of (x | y) element-wise."""
    x = rtt_ts.convert_to_rtttensor(x)
    y = rtt_ts.convert_to_rtttensor(y)
    _result = rtt_ts.rtt_ops.rtt_logical_or(x._raw, y._raw, name=name)
    return rtt_ts.RttTensor(_result)


def rtt_logical_xor(x, y, name=None):
    """Returns the truth value of (x ^ y) element-wise."""
    x = rtt_ts.convert_to_rtttensor(x)
    y = rtt_ts.convert_to_rtttensor(y)
    _result = rtt_ts.rtt_ops.rtt_logical_xor(x._raw, y._raw, name=name)
    return rtt_ts.RttTensor(_result)


def rtt_logical_not(x, name=None):
    """Returns the truth value of (!x) element-wise."""
    x = rtt_ts.convert_to_rtttensor(x)
    _result = rtt_ts.rtt_ops.rtt_logical_not(x._raw, name=name)
    return rtt_ts.RttTensor(_result)


def rtt_matmul(x, y, transpose_a=False, transpose_b=False, name=None):
    """Multiplies matrix `a` by matrix `b`, producing `a` * `b`."""
    x = rtt_ts.convert_to_rtttensor(x)
    y = rtt_ts.convert_to_rtttensor(y)
    _result = rtt_ts.rtt_ops.rtt_matmul(x._raw, y._raw, transpose_a=transpose_a, transpose_b=transpose_b, name=name)
    return rtt_ts.RttTensor(_result)


def rtt_square(x, name=None):
    """Computes square of x element-wise."""
    x = rtt_ts.convert_to_rtttensor(x)
    _result = rtt_ts.rtt_ops.rtt_square(x._raw, name=name)
    return rtt_ts.RttTensor(_result)


def rtt_exp(x, name=None):
    """Computes square of x element-wise."""
    x = rtt_ts.convert_to_rtttensor(x)
    _result = rtt_ts.rtt_ops.rtt_exp(x._raw, name=name)
    return rtt_ts.RttTensor(_result)


def rtt_rsqrt(x, name=None):
    """Computes 1/sqrt(x) of x element-wise."""
    x = rtt_ts.convert_to_rtttensor(x)
    _result = rtt_ts.rtt_ops.rtt_rsqrt(x._raw, name=name)
    return rtt_ts.RttTensor(_result)


def rtt_sqrt(x, name=None):
    """Computes square of x element-wise."""
    x = rtt_ts.convert_to_rtttensor(x)
    _result = rtt_ts.rtt_ops.rtt_sqrt(x._raw, name=name)
    return rtt_ts.RttTensor(_result)


def rtt_pow(x, y, name=None): 
    """Computes the power of one value to another."""
    x = rtt_ts.convert_to_rtttensor(x)
    y = rtt_ts.convert_to_rtttensor(y)
    _result = rtt_ts.rtt_ops.rtt_pow(x._raw, y._raw, name=name)
    return rtt_ts.RttTensor(_result)


def rtt_log(x, name=None):
    """Computes natural logarithm of x element-wise."""
    x = rtt_ts.convert_to_rtttensor(x)
    _result = rtt_ts.rtt_ops.rtt_log(x._raw, name=name)
    return rtt_ts.RttTensor(_result)


def rtt_log1p(x, name=None):
    """Computes natural logarithm of (1 + x) element-wise."""
    x = rtt_ts.convert_to_rtttensor(x)
    _result = rtt_ts.rtt_ops.rtt_log1p(x._raw, name=name)
    return rtt_ts.RttTensor(_result)


def rtt_abs(x, name=None):
    """Computes the absolute value of a tensor."""
    x = rtt_ts.convert_to_rtttensor(x)
    _result = rtt_ts.rtt_ops.rtt_abs(x._raw, name=name)
    return rtt_ts.RttTensor(_result)


def rtt_max(
    input_tensor,
    axis=None,
    keepdims=None,
    name=None,
    reduction_indices=None,
    keep_dims=None,
):
    """Computes the maximum of elements across dimensions of a tensor."""

    keepdims = deprecation.deprecated_argument_lookup("keepdims", keepdims,
                                                      "keep_dims", keep_dims)
    keepdims = False if keepdims is None else keepdims
    axis = math_ops._ReductionDims(input_tensor, axis)
    input_tensor = rtt_ts.convert_to_rtttensor(input_tensor)
    _result = rtt_ts.rtt_ops.rtt_reduce_max(
        input_tensor, reduction_indices=axis, name=name, keep_dims=keepdims
    )
    return rtt_ts.RttTensor(_result)


def rtt_min(
    input_tensor,
    axis=None,
    keepdims=None,
    name=None,
    reduction_indices=None,
    keep_dims=None,
):
    """Computes the minimum of elements across dimensions of a tensor."""

    keepdims = deprecation.deprecated_argument_lookup("keepdims", keepdims,
                                                      "keep_dims", keep_dims)
    keepdims = False if keepdims is None else keepdims
    axis = math_ops._ReductionDims(input_tensor, axis)
    input_tensor = rtt_ts.convert_to_rtttensor(input_tensor)
    _result = rtt_ts.rtt_ops.rtt_reduce_min(
        input_tensor, reduction_indices=axis, name=name, keep_dims=keepdims
    )
    return rtt_ts.RttTensor(_result)


def rtt_sum(
    input_tensor,
    axis=None,
    keepdims=None,
    name=None,
    reduction_indices=None,
    keep_dims=None,
):
    """Computes the sum of elements across dimensions of a tensor."""

    keepdims = deprecation.deprecated_argument_lookup("keepdims", keepdims,
                                                      "keep_dims", keep_dims)
    keepdims = False if keepdims is None else keepdims
    axis = math_ops._ReductionDims(input_tensor, axis)
    input_tensor = rtt_ts.convert_to_rtttensor(input_tensor)
    _result = rtt_ts.rtt_ops.rtt_reduce_sum(
        input_tensor, reduction_indices=axis, name=name, keep_dims=keepdims
    )
    return rtt_ts.RttTensor(_result)


def rtt_mean(
    input_tensor,
    axis=None,
    keepdims=None,
    name=None,
    reduction_indices=None,
    keep_dims=None,
):
    """Computes the mean of elements across dimensions of a tensor."""

    keepdims = deprecation.deprecated_argument_lookup("keepdims", keepdims,
                                                      "keep_dims", keep_dims)
    keepdims = False if keepdims is None else keepdims
    axis = math_ops._ReductionDims(input_tensor, axis)
    input_tensor = rtt_ts.convert_to_rtttensor(input_tensor)
    _result = rtt_ts.rtt_ops.rtt_reduce_mean(
        input_tensor, reduction_indices=axis, name=name, keep_dims=keepdims
    )
    return rtt_ts.RttTensor(_result)


def rtt_arg_max(input, dimension=None, name=None, output_type=dtypes.string):
    if dimension is None:
        dimension = 0
    input = rtt_ts.convert_to_rtttensor(input)
    _result = rtt_ts.rtt_ops.rtt_arg_max(input, dimension=dimension, name=name, output_type=output_type)
    return rtt_ts.RttTensor(_result)



def rtt_cast(x, dtype, name=None):
  """Casts a tensor from tf numeric to a rtt string type.

  Args:
    x: A `Tensor` or `SparseTensor` or `IndexedSlices` of numeric type. It could
      be `uint8`, `uint16`, `uint32`, `uint64`, `int8`, `int16`, `int32`,
      `int64`, `float16`, `float32`, `float64`, `complex64`, `complex128`,
      `bfloat16`.
    dtype: The destination type. The list of supported dtypes is the same as
      `x`.
    name: A name for the operation (optional).

  Returns:
    A `RTTTensor`  with same shape as `x` and type as rtt `string`.
  """
  if (isinstance(x, rtt_ts.RttTensor)):
      return x

  base_type = dtypes.as_dtype(dtype).base_dtype
  if base_type in (tf.int16, tf.int32, tf.int64, tf.float16, tf.float32, tf.float64):
      return tf.as_string(x)
  else:
      return x



#---------------------------------------------------------
# Static override tensorflow math ops to rosetta native ops
def static_override_tf_ops_to_rtt_ops():
    tf.negative = rtt_neg
    tf.add = rtt_add
    tf.subtract = rtt_sub
    tf.multiply = rtt_mul
    tf.div = rtt_div
    tf.reciprocaldiv = rtt_reciprocaldiv
    tf.floordiv = rtt_floordiv
    tf.truediv = rtt_truediv
    tf.realdiv = rtt_realdiv
    tf.equal = rtt_equal
    tf.not_equal = rtt_notequal
    tf.greater = rtt_greater
    tf.greater_equal = rtt_greaterequal
    tf.less = rtt_less
    tf.less_equal = rtt_lessequal
    tf.logical_and = rtt_logical_and
    tf.logical_or = rtt_logical_or
    tf.logical_xor = rtt_logical_xor
    tf.logical_not = rtt_logical_not
    tf.matmul = rtt_matmul
    tf.square = rtt_square
    tf.exp = rtt_exp
    tf.sqrt = rtt_sqrt
    tf.rsqrt = rtt_rsqrt
    tf.pow = rtt_pow
    tf.log = rtt_log
    tf.log1p = rtt_log1p
    tf.abs = rtt_abs
    tf.reduce_max = rtt_max
    tf.reduce_min = rtt_min
    tf.reduce_mean = rtt_mean
    tf.reduce_sum = rtt_sum
    tf.cast = rtt_cast
    tf.argmax = rtt_arg_max

    #-------------------------------
    gen_math_ops.mat_mul = rtt_matmul

    

# run static override
static_override_tf_ops_to_rtt_ops()

