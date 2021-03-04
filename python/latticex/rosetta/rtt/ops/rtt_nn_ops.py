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
from tensorflow.python.framework import dtypes
from tensorflow.python.framework import tensor_util
from tensorflow.python.framework import ops
from tensorflow.python.framework import errors_impl
from tensorflow.python.ops import array_ops
from tensorflow.python.ops import math_ops
from latticex.rosetta.rtt.framework import rtt_tensor as rtt_ts
from tensorflow.python.ops import nn
from tensorflow.python.ops import gen_nn_ops
import os


#---------------------------------------------------------
def rtt_sigmoid(x, name=None):
    """Computes sigmoid of `x` element-wise.
    Specifically, `y = 1 / (1 + exp(-x))`."""
    x = rtt_ts.convert_to_rtttensor(x)
    _result = rtt_ts.rtt_ops.rtt_sigmoid(x._raw, name=name)
    return rtt_ts.RttTensor(_result)


def rtt_relu(x, name=None):
    """Computes rectified linear: `max(features, 0)`."""
    x = rtt_ts.convert_to_rtttensor(x)
    _result = rtt_ts.rtt_ops.rtt_relu(x._raw, name=name)
    return rtt_ts.RttTensor(_result)


def rtt_conv2d(input, filter, strides=None, padding=None, use_cudnn_on_gpu=False, 
            explicit_paddings=[], data_format="NHWC", dilations=[1, 1, 1, 1], name=None):
    input = rtt_ts.convert_to_rtttensor(input)
    filter = rtt_ts.convert_to_rtttensor(filter)
    _result = rtt_ts.rtt_ops.rtt_conv2d(input._raw, filter._raw, strides=strides, padding=padding, 
                        use_cudnn_on_gpu=use_cudnn_on_gpu, explicit_paddings=explicit_paddings, 
                        data_format=data_format, dilations=None, name=name)
    return rtt_ts.RttTensor(_result)


def rtt_bias_add(value, bias, data_format="NHWC", name=None):
    value = rtt_ts.convert_to_rtttensor(value)
    bias = rtt_ts.convert_to_rtttensor(bias)
    _result = rtt_ts.rtt_ops.rtt_bias_add(value._raw, bias._raw, data_format=data_format, name=name)
    return rtt_ts.RttTensor(_result)


def rtt_l2_loss(x, name=None):
    x = rtt_ts.convert_to_rtttensor(x)
    _result = rtt_ts.rtt_ops.rtt_l2_loss(x._raw, name=name)
    return rtt_ts.RttTensor(_result)


def rtt_fused_batch_norm(x, scale, offset, mean, variance, epsilon=0.0001, data_format="NHWC", is_training=True, name=None):
    x = rtt_ts.convert_to_rtttensor(x)
    scale = rtt_ts.convert_to_rtttensor(scale)
    offset = rtt_ts.convert_to_rtttensor(offset)
    mean = rtt_ts.convert_to_rtttensor(mean)
    variance = rtt_ts.convert_to_rtttensor(variance)
    y, batch_mean, batch_var, _, _ = rtt_ts.rtt_ops.rtt_fused_batch_norm(x, scale, offset, mean, variance, epsilon=epsilon,
                                                                data_format=data_format, is_training=is_training, name=name)
    return  rtt_ts.RttTensor(y), rtt_ts.RttTensor(batch_mean), rtt_ts.RttTensor(batch_var), _, _


def rtt_avg_pool(value, ksize, strides, padding, data_format="NHWC", name=None):
    value = rtt_ts.convert_to_rtttensor(value)
    _result = rtt_ts.rtt_ops.rtt_avg_pool(value, ksize=ksize, strides=strides, padding=padding, 
                                        data_format=data_format, name=name)
    return  rtt_ts.RttTensor(_result)


def rtt_max_pool(value, ksize, strides, padding, data_format="NHWC", name=None):
    value = rtt_ts.convert_to_rtttensor(value)
    _result = rtt_ts.rtt_ops.rtt_max_pool(value, ksize=ksize, strides=strides, padding=padding, 
                                        data_format=data_format, name=name)
    return  rtt_ts.RttTensor(_result)


def rtt_softmax(logits, axis=None, name=None):
    if axis is None:
        axis = -1

    def _softmax(logits, compute_op, dim=-1, name=None):
        logits = rtt_ts.convert_to_rtttensor(logits)

        def _swap_axis(logits, dim_index, last_index, name=None):
            """Swaps logits's dim_index and last_index."""
            return array_ops.transpose(
                logits,
                array_ops.concat([
                    math_ops.range(dim_index), [last_index],
                    math_ops.range(dim_index + 1, last_index), [dim_index]
                ], 0),
                name=name)

        # We need its original shape for shape inference.
        shape = logits._raw.get_shape()
        is_last_dim = (dim == -1) or (dim == shape.ndims - 1)
        if is_last_dim:
            _result = compute_op(logits, name=name)
            return  rtt_ts.RttTensor(_result)

        dim_val = dim
        if isinstance(dim, ops.Tensor):
            dim_val = tensor_util.constant_value(dim)
        elif isinstance(dim, rtt_ts.RttTensor):
            dim_val = tensor_util.constant_value(dim._raw)
        if dim_val is not None and not -shape.ndims <= dim_val < shape.ndims:
            raise errors_impl.InvalidArgumentError(
                None, None,
                "Dimension (%d) must be in the range [%d, %d) where %d is the number of"
                " dimensions in the input." % (dim_val, -shape.ndims, shape.ndims,
                                            shape.ndims))

        # In case dim is negative (and is not last dimension -1), add shape.ndims
        ndims = array_ops.rank(logits._raw)
        if not isinstance(dim, ops.Tensor):
            if dim < 0:
                dim += ndims
        else:
            dim = array_ops.where(math_ops.less(dim, 0), dim + ndims, dim)

        # Swap logits' dimension of dim and its last dimension.
        input_rank = array_ops.rank(logits._raw)
        dim_axis = dim % shape.ndims
        logits = _swap_axis(logits._raw, dim_axis, math_ops.subtract(input_rank, 1))

        # Do the actual softmax on its last dimension.
        _result = compute_op(logits, name=name)

        # If dim is not the last dimension, we have to do a transpose so that we can
        # still perform softmax on its last dimension.
        _result = _swap_axis(
            _result, dim_axis, math_ops.subtract(input_rank, 1), name=name)

        # Make shape inference work since transpose may erase its static shape.
        _result.set_shape(shape)
        return  rtt_ts.RttTensor(_result)

    return _softmax(logits, rtt_ts.rtt_ops.rtt_softmax, axis, name)


#---------------------------------------------------------
# Static override tensorflow nn ops to rosetta nn ops
def static_override_tf_nn_ops_to_rtt_nn_ops():
    tf.sigmoid = rtt_sigmoid
    tf.nn.sigmoid = rtt_sigmoid
    tf.nn.relu = rtt_relu
    tf.nn.softmax = rtt_softmax

    gen_nn_ops.conv2d = rtt_conv2d
    gen_nn_ops.bias_add = rtt_bias_add
    gen_nn_ops._fused_batch_norm = rtt_fused_batch_norm
    tf.nn.conv2d = rtt_conv2d
    tf.nn.bias_add = rtt_bias_add
    tf.nn.l2_loss = rtt_l2_loss
    tf.nn.l2_loss = rtt_l2_loss
    tf.nn.max_pool = rtt_max_pool
    tf.nn.avg_pool = rtt_avg_pool
    nn.l2_loss = rtt_l2_loss
    nn.max_pool = rtt_max_pool
    nn.avg_pool = rtt_avg_pool


# run static override
static_override_tf_nn_ops_to_rtt_nn_ops()