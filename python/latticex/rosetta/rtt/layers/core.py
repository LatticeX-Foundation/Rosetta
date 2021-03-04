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
from tensorflow.python.layers import base
from tensorflow.python.ops import init_ops
from latticex.rosetta.rtt.keras.layers.core import RttKerasDense



class RttDense(RttKerasDense, base.Layer):
  """Densely-connected layer class."""

  def __init__(self, units,
               activation=None,
               use_bias=True,
               kernel_initializer=None,
               bias_initializer=init_ops.zeros_initializer(),
               kernel_regularizer=None,
               bias_regularizer=None,
               activity_regularizer=None,
               kernel_constraint=None,
               bias_constraint=None,
               trainable=True,
               name=None,
               **kwargs):
    super(RttDense, self).__init__(units=units,
                                activation=activation,
                                use_bias=use_bias,
                                kernel_initializer=kernel_initializer,
                                bias_initializer=bias_initializer,
                                kernel_regularizer=kernel_regularizer,
                                bias_regularizer=bias_regularizer,
                                activity_regularizer=activity_regularizer,
                                kernel_constraint=kernel_constraint,
                                bias_constraint=bias_constraint,
                                trainable=trainable,
                                name=name,
                                **kwargs)


def rtt_dense(
    inputs, units,
    activation=None,
    use_bias=True,
    kernel_initializer=None,
    bias_initializer=init_ops.zeros_initializer(),
    kernel_regularizer=None,
    bias_regularizer=None,
    activity_regularizer=None,
    kernel_constraint=None,
    bias_constraint=None,
    trainable=True,
    name=None,
    reuse=None):
  """Functional interface for the densely-connected layer.

  This layer implements the operation:
  `outputs = activation(inputs * kernel + bias)`
  where `activation` is the activation function passed as the `activation`
  argument (if not `None`), `kernel` is a weights matrix created by the layer,
  and `bias` is a bias vector created by the layer
  (only if `use_bias` is `True`).

  Arguments:
    inputs: Tensor input.
    units: Integer or Long, dimensionality of the output space.
    activation: Activation function (callable). Set it to None to maintain a
      linear activation.
    use_bias: Boolean, whether the layer uses a bias.
    kernel_initializer: Initializer function for the weight matrix.
      If `None` (default), weights are initialized using the default
      initializer used by `tf.compat.v1.get_variable`.
    bias_initializer: Initializer function for the bias.
    kernel_regularizer: Regularizer function for the weight matrix.
    bias_regularizer: Regularizer function for the bias.
    activity_regularizer: Regularizer function for the output.
    kernel_constraint: An optional projection function to be applied to the
        kernel after being updated by an `Optimizer` (e.g. used to implement
        norm constraints or value constraints for layer weights). The function
        must take as input the unprojected variable and must return the
        projected variable (which must have the same shape). Constraints are
        not safe to use when doing asynchronous distributed training.
    bias_constraint: An optional projection function to be applied to the
        bias after being updated by an `Optimizer`.
    trainable: Boolean, if `True` also add variables to the graph collection
      `GraphKeys.TRAINABLE_VARIABLES` (see `tf.Variable`).
    name: String, the name of the layer.
    reuse: Boolean, whether to reuse the weights of a previous layer
      by the same name.

  Returns:
    Output tensor the same shape as `inputs` except the last dimension is of
    size `units`.

  Raises:
    ValueError: if eager execution is enabled.
  """
  layer = RttDense(units,
                activation=activation,
                use_bias=use_bias,
                kernel_initializer=kernel_initializer,
                bias_initializer=bias_initializer,
                kernel_regularizer=kernel_regularizer,
                bias_regularizer=bias_regularizer,
                activity_regularizer=activity_regularizer,
                kernel_constraint=kernel_constraint,
                bias_constraint=bias_constraint,
                trainable=trainable,
                name=name,
                _scope=name,
                _reuse=reuse)
  return layer.apply(inputs)


# override tensorflow tf.layers.dense functions for RTT dense
tf.layers.dense = rtt_dense

