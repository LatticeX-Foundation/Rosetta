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
from latticex.rosetta.secure import MsgIdGenerator
from latticex.rosetta.secure import StaticReplacePass
from latticex.rosetta.secure import SecureApplyGradientDescent
from latticex.rosetta.secure.utils.common import is_secure_compare_tensor
from latticex.rosetta.rtt.framework import rtt_tensor as rtt_ts
from latticex.rosetta.controller.common_util import rtt_get_logger


class SecureGradientDescentOptimizer(tf.compat.v1.train.GradientDescentOptimizer):
    """
    # Secure GradientDescent optimizer, only override __init__ & minimize functions
    """

    def __init__(self, learning_rate, use_locking=False, name="GradientDescent"):
        """
        # Construct a new secure GradientDescent optimizer.
        """
        super(tf.train.GradientDescentOptimizer, self).__init__(learning_rate, use_locking, name)


    def _apply_dense(self, grad, var):
        return SecureApplyGradientDescent(
            var,
            math_ops.cast(self._learning_rate_tensor, tf.float64),
            grad,
            use_locking=self._use_locking).op


    def _assert_valid_dtypes(self, tensors):
        """Asserts tensors are all valid types (see `_valid_dtypes`).

        Args:
            tensors: Tensors to check.

        Raises:
            ValueError: If any tensor is not a valid type.
        """

        valid_dtypes = self._valid_dtypes()
        valid_dtypes.add(tf.string)
        for t in tensors:
            if (is_secure_compare_tensor(t)):
                raise ValueError(
                    "Invalid type %r for %s, not expected secure compare op." % (
                        dtype, t.name))

            dtype = t.dtype.base_dtype
            if dtype not in valid_dtypes:
                raise ValueError(
                    "Invalid type %r for %s, expected: %s." % (
                        dtype, t.name, [v for v in valid_dtypes]))


    def minimize(self, loss, global_step=None, var_list=None,
                 gate_gradients=1, aggregation_method=None,
                 colocate_gradients_with_ops=False, name=None,
                 grad_loss=None):
        rtt_get_logger().debug('begin to run StaticReplacePass...')

        # Create StaticReplacePass
        PassObj = StaticReplacePass()

        # Run the pass, and return new loss
        if isinstance(loss, rtt_ts.RttTensor):
            loss = PassObj.run(loss._raw)
        else:
            loss = PassObj.run(loss)
        rtt_get_logger().debug('end to run StaticReplacePass.')

        # generate secure gradient graph
        train_op = super(tf.train.GradientDescentOptimizer, self).minimize(loss, global_step, var_list,
                                                            gate_gradients, aggregation_method,
                                                            colocate_gradients_with_ops, name,
                                                            grad_loss)

        # generate message id
        MsgIdGenerator(regenerate=True).gen_msgid_and_notified(loss)

        return train_op


# assign secure gradient descent optimizer to gradient descent optimizer,
# now, the tf default gradient descent optimizer is secure gradient descent optimizer
tf.train.GradientDescentOptimizer = SecureGradientDescentOptimizer



