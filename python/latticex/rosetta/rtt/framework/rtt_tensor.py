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
from tensorflow.python.keras.utils import tf_utils
from tensorflow.python.client import session as tf_session
from tensorflow.python.framework import ops as tf_ops
from tensorflow.python.ops import array_ops as tf_array_ops
from latticex.rosetta.secure.spass.static_replace_pass import replace_tf_subgraph_with_secure_subgraph
import numpy as np
import re
import os
from latticex.rosetta.controller.common_util import rtt_get_logger

# load librtt_ops.so
if 'ROSETTA_MPC_128' in os.environ and os.environ['ROSETTA_MPC_128'] == 'ON':
    _rtt_ops_lib = os.path.dirname(__file__) + '/../../../lib128/librtt-ops.so'
else:
    _rtt_ops_lib = os.path.dirname(__file__) + '/../../../librtt-ops.so'
rtt_ops = tf.load_op_library(_rtt_ops_lib)


class RttTensor(object):
    """
    Implementation of rosetta custom tensor type. 
    Represents one of the outputs of an `Operation`.
    """

    # List of Python operators that we allow to override.
    OVERLOADABLE_OPERATORS = {
      # Binary.
      "__add__",
      "__radd__",
      "__sub__",
      "__rsub__",
      "__mul__",
      "__rmul__",
      "__div__",
      "__rdiv__",
      "__truediv__",
      "__rtruediv__",
      "__floordiv__",
      "__rfloordiv__",
      "__lt__",
      "__le__",
      "__gt__",
      "__ge__",
      "__eq__",
      "__ne__",
      "__getitem__",
      "__pow__",
      "__rpow__",
      "__matmul__",
      "__rmatmul__",

      # Unary.
      "__neg__",
      "__abs__"
    }

    def __init__(self, value):
        assert isinstance(value, tf.Tensor), type(value)
        assert value.dtype is tf.string, value.dtype
        self._raw = value
        # print(self._raw.dtype)

    @property
    def shape(self):
        return self._raw.shape

    @property
    def name(self):
        return self._raw.name

    @property
    def dtype(self):
        return tf.string

    def __hash__(self):
        # Necessary to support Python's collection membership operators
        return id(self)

    def eval(self, session=None, dtype=None):
        tf_tensor = convert_from_rtttensor(self, dtype=dtype)
        evaluated = tf_tensor.eval(session=session)
        if tf_tensor.dtype is tf.string:
            return evaluated.astype(str)
        return evaluated

    def __add__(self, other):
        """ self + other """
        other = convert_to_rtttensor(other)
        res = rtt_ops.rtt_add(self._raw, other._raw)
        return RttTensor(res)

    def __radd__(self, other):
        """ other + self """
        other = convert_to_rtttensor(other)
        res = rtt_ops.rtt_add(self._raw, other._raw)
        return RttTensor(res)

    def __sub__(self, other):
        """ self - other """
        other = convert_to_rtttensor(other)
        res = rtt_ops.rtt_sub(self._raw, other._raw)
        return RttTensor(res)

    def __rsub__(self, other):
        """ other - self """
        other = convert_to_rtttensor(other)
        res = rtt_ops.rtt_sub(other._raw, self._raw)
        return RttTensor(res)

    def __mul__(self, other):
        """ self * other """
        other = convert_to_rtttensor(other)
        res = rtt_ops.rtt_mul(self._raw, other._raw)
        return RttTensor(res)

    def __rmul__(self, other):
        """ other * self """
        other = convert_to_rtttensor(other)
        res = rtt_ops.rtt_mul(self._raw, other._raw)
        return RttTensor(res)

    def __div__(self, other):
        """ self / other """
        other = convert_to_rtttensor(other)
        res = rtt_ops.rtt_div(self._raw, other._raw)
        return RttTensor(res)

    def __rdiv__(self, other):
        """ other / div """
        other = convert_to_rtttensor(other)
        res = rtt_ops.rtt_div(other._raw, self._raw)
        return RttTensor(res)

    def __truediv__(self, other):
        """ self truediv other """
        other = convert_to_rtttensor(other)
        res = rtt_ops.rtt_truediv(self._raw, other._raw)
        return RttTensor(res)

    def __rtruediv__(self, other):
        """ other truediv self """
        other = convert_to_rtttensor(other)
        res = rtt_ops.rtt_truediv(other._raw, self._raw)
        return RttTensor(res)

    def __floordiv__(self, other):
        """ self // other """
        other = convert_to_rtttensor(other)
        res = rtt_ops.rtt_floordiv(self._raw, other._raw)
        return RttTensor(res)

    def __rfloordiv__(self, other):
        """ other // self """
        other = convert_to_rtttensor(other)
        res = rtt_ops.rtt_floordiv(other._raw, self._raw)
        return RttTensor(res)

    def __lt__(self, other):
        """ self < other """
        other = convert_to_rtttensor(other)
        res = rtt_ops.rtt_less(self._raw, other._raw)
        return RttTensor(res)

    def __le__(self, other):
        """ self <= other """
        other = convert_to_rtttensor(other)
        res = rtt_ops.rtt_less_equal(self._raw, other._raw)
        return RttTensor(res)

    def __gt__(self, other):
        """ self > other """
        other = convert_to_rtttensor(other)
        res = rtt_ops.rtt_greater(self._raw, other._raw)
        return RttTensor(res)

    def __ge__(self, other):
        """ self >= other """
        other = convert_to_rtttensor(other)
        res = rtt_ops.rtt_greater_equal(self._raw, other._raw)
        return RttTensor(res)

    def __eq__(self, other):
        """ self == other """
        other = convert_to_rtttensor(other)
        res = rtt_ops.rtt_equal(self._raw, other._raw)
        return RttTensor(res)
    
    def __ne__(self, other):
        """ self != other """
        other = convert_to_rtttensor(other)
        res = rtt_ops.rtt_not_equal(self._raw, other._raw)
        return RttTensor(res)

    def __and__(self, other):
        """ self & other """
        other = convert_to_rtttensor(other)
        res = rtt_ops.rtt_logical_and(self._raw, other._raw)
        return RttTensor(res)

    def __or__(self, other):
        """ self | other """
        other = convert_to_rtttensor(other)
        res = rtt_ops.rtt_logical_or(self._raw, other._raw)
        return RttTensor(res)
    
    def __xor__(self, other):
        """ self ^ other """
        other = convert_to_rtttensor(other)
        res = rtt_ops.rtt_logical_xor(self._raw, other._raw)
        return RttTensor(res)

    def __invert__(self):
        """ !self """
        res = rtt_ops.rtt_logical_not(self._raw)
        return RttTensor(res)

    def __getitem__(self, slice_spec):
        """ override [] """
        res = tf_array_ops._slice_helper(self._raw, slice_spec)
        return RttTensor(res)

    def __pow__(self, other):
        """ self ** other """
        other = convert_to_rtttensor(other)
        res = rtt_ops.rtt_pow(self._raw, other._raw)
        return RttTensor(res)

    def __rpow__(self, other):
        """ other ** self """
        other = convert_to_rtttensor(other)
        res = rtt_ops.rtt_pow(other._raw, self._raw)
        return RttTensor(res)

    def __matmul__(self, other):
        """ self @ other """
        other = convert_to_rtttensor(other)
        res = rtt_ops.rtt_mat_mul(self._raw, other._raw)
        return RttTensor(res)

    def __rmatmul__(self, other):
        """ other @ self """
        other = convert_to_rtttensor(other)
        res = rtt_ops.rtt_matmul(other._raw, self._raw)
        return RttTensor(res)

    def __neg__(self):
        """ -self """
        res = rtt_ops.rtt_negative(self._raw)
        return RttTensor(res)

    def __abs__(self):
        """ abs(self) """
        res = rtt_ops.rtt_abs(self._raw)
        return RttTensor(res)


# implement rtt tensor fetch function
def _fetch_function(rtt_tensor):
    # NOTE(George): calling `replace_tf_subgraph_with_secure_subgraph` to get SecureOp
    unwrapped = [replace_tf_subgraph_with_secure_subgraph(convert_from_rtttensor(rtt_tensor, dtype=tf.string))]
    rewrapper = lambda components_fetched: components_fetched[0]
    return unwrapped, rewrapper

# implement rtt tensor feed function
def _feed_function(rtt_tensor, feed_value):
    return [(rtt_tensor._raw, feed_value)]

# implement rtt tensor fetch function for partial run
def _feed_function_for_partial_run(rtt_tensor):
    return [rtt_tensor._raw]  

# this allows rtt.RttTensor to be passed directly to tf.Session.run,
# unwrapping and converting the result as needed
tf_session.register_session_run_conversion_functions(
    tensor_type=RttTensor,
    fetch_function=_fetch_function,
    feed_function=_feed_function,
    feed_function_for_partial_run=_feed_function_for_partial_run,
)


def _tensor_conversion_function(tensor, dtype=None, name=None, as_ref=False):
    # assert name is None, "Not implemented, name='{}'".format(name)
    # assert not as_ref, "Not implemented, as_ref={}".format(as_ref)
    # assert dtype in [tf.int32, None], dtype
    return convert_from_rtttensor(tensor, dtype=dtype)


# this allows implicit convertion of rtt.RttTensor to tf.Tensor,
# but since the output dtype is determined by the outer context
# we essentially have to export with the implied risk of data loss
tf_ops.register_tensor_conversion_function(RttTensor, _tensor_conversion_function)


# this allows RttTensor to pass the tf.is_tensor test
tf_ops.register_dense_tensor_like_type(RttTensor)


# this allows rtt.RttTensor to be plumbed through Keras layers
# but seems only truly useful when used in conjunction with
# `register_tensor_conversion_function`
tf_utils.register_symbolic_tensor_type(RttTensor)


def _convert_numpy_tensor(tensor):
    """ convert numpy tensor to rtt tensor """

    if len(tensor.shape) > 2:
        raise ValueError("Only matrices are supported for now.")

    if (
        np.issubdtype(tensor.dtype, np.int16)
        or np.issubdtype(tensor.dtype, np.int32)
        or np.issubdtype(tensor.dtype, np.int64)
        or np.issubdtype(tensor.dtype, np.float)
        or np.issubdtype(tensor.dtype, np.double)
        or np.issubdtype(tensor.dtype, np.unicode_)
        or np.issubdtype(tensor.dtype, np.object_)
    ):
        # supported as strings
        tensor = tensor.astype(np.string_)
        return RttTensor(rtt_ops.tf_to_rtt(tensor))

    if np.issubdtype(tensor.dtype, np.string_):
        return RttTensor(rtt_ops.tf_to_rtt(tensor))

    raise ValueError(
        "Don't know how to convert NumPy tensor with dtype '{}'".format(tensor.dtype)
    )


def _convert_tensorflow_tensor(tensor):
    """ convert tensorflow native tensor to rtt tensor """

    if tensor.dtype in (tf.string,):
        # supported as-is
        rtt_get_logger().debug("debug in _convert_tensorflow_tensor:" + str(tensor))
        return RttTensor(rtt_ops.tf_to_rtt(tensor))

    if tensor.dtype in (tf.int32, tf.int64, tf.float16, tf.float32, tf.float64):
        # supported as strings
        tensor = tf.as_string(tensor)
        return RttTensor(rtt_ops.tf_to_rtt(tensor))

    raise ValueError(
        "Don't know how to convert TensorFlow tensor with dtype '{}'".format(
            tensor.dtype
        )
    )


def convert_to_rtttensor(tensor):
    """ convert to rtt tensor """

    if tensor is None:
        return None

    if isinstance(tensor, RttTensor):
        return tensor

    if isinstance(tensor, (int, float, str)):
        return _convert_numpy_tensor(np.array([tensor]))

    if isinstance(tensor, (list, tuple)):
        return _convert_numpy_tensor(np.array(tensor))

    if isinstance(tensor, np.ndarray):
        return _convert_numpy_tensor(tensor)

    if isinstance(tensor, tf.Tensor):
        return _convert_tensorflow_tensor(tensor)

    if isinstance(tensor, tf.Variable):
        return _convert_tensorflow_tensor(tf_ops.convert_to_tensor_v2(tensor))

    raise ValueError("Don't know how to convert value of type {}".format(type(tensor)))


def convert_from_rtttensor(value, dtype=None):
    """ convert from rtt tensor """

    assert isinstance(value, RttTensor), type(value)

    if dtype is None:
        dtype = tf.string

    if dtype in [tf.int32, tf.string]:
        return rtt_ops.rtt_to_tf(value._raw, dtype=dtype)

    raise ValueError("Don't know how to evaluate to dtype '{}'".format(dtype))


def constant(tensor):
    """Creates a constant rrt tensor. """
    return convert_to_rtttensor(tensor)


def placeholder(tensor):
    """Inserts a placeholder for a rtt tensor that will be always fed."""
    return convert_to_rtttensor(tensor)

