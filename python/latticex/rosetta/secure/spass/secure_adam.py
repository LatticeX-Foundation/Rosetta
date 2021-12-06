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
from tensorflow.python.eager import context
from tensorflow.python.distribute import distribution_strategy_context as distribute_ctx
from tensorflow.python.ops import variable_scope
from tensorflow.python.ops import resource_variable_ops
from tensorflow.python.ops import control_flow_ops
from tensorflow.python.framework import ops
from tensorflow.python.ops import math_ops
from latticex.rosetta.secure import MsgIdGenerator
from latticex.rosetta.secure import StaticReplacePass
from latticex.rosetta.secure import SecureApplyAdam
from latticex.rosetta.secure.utils.common import is_secure_compare_tensor
from latticex.rosetta.rtt.framework import rtt_tensor as rtt_ts
from latticex.rosetta.rtt.ops.rtt_state_ops import get_rtt_var_from_rtt_tensor
from latticex.rosetta.controller.common_util import rtt_get_logger


class SecureAdamOptimizer(tf.compat.v1.train.AdamOptimizer):
    """
    # secure Adam optimizer, only override __init__ & minimize functions
    """

    def __init__(self, 
                learning_rate=0.001,
                beta1=0.9,
                beta2=0.999,
                epsilon=1e-8,
                use_locking=False, 
                name="Adam"):
        """
        # Construct a new secure Adam optimizer.
        """
        super(tf.train.AdamOptimizer, self).__init__(learning_rate, beta1, beta2, epsilon, use_locking, name)

    
    def _prepare(self):
        lr = self._call_if_callable(self._lr)
        beta1 = self._call_if_callable(self._beta1)
        beta2 = self._call_if_callable(self._beta2)
        epsilon = self._call_if_callable(self._epsilon)

        self._lr_t = ops.convert_to_tensor(lr, name="learning_rate")
        self._beta1_t = ops.convert_to_tensor(beta1, name="beta1")
        self._beta2_t = ops.convert_to_tensor(beta2, name="beta2")
        self._epsilon_t = ops.convert_to_tensor(epsilon, name="epsilon")
        self._str_beta1_t = ops.convert_to_tensor(str(beta1), name="str_beta1")
        self._str_beta2_t = ops.convert_to_tensor(str(beta2), name="str_beta2")


    def _apply_dense(self, grad, var):
        m = self.get_slot(var, "m")
        v = self.get_slot(var, "v")
        beta1_power, beta2_power = self._get_beta_accumulators()
        return SecureApplyAdam(
                var,
                get_rtt_var_from_rtt_tensor(m),
                get_rtt_var_from_rtt_tensor(v),
                math_ops.cast(beta1_power, var.dtype.base_dtype),
                math_ops.cast(beta2_power, var.dtype.base_dtype),
                math_ops.cast(self._lr_t, tf.float64),
                math_ops.cast(self._beta1_t, tf.float64),
                math_ops.cast(self._beta2_t, tf.float64),
                math_ops.cast(self._epsilon_t, tf.float64),
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


    def _create_non_slot_variable(self, initial_value, name, colocate_with):
        """Add an extra variable, not associated with a slot."""
        # Recommendation: Use OptimizerV2 if your optimizer uses non-slot variables.
        eager = context.executing_eagerly()
        graph = None if eager else colocate_with.graph

        key = (name, graph)
        v = self._non_slot_dict.get(key, None)
        if v is None:
            self._maybe_initialize_trackable()
            distribution_strategy = distribute_ctx.get_strategy()
            with distribution_strategy.extended.colocate_vars_with(colocate_with):
                if eager:
                    restored_initial_value = self._preload_simple_restoration(
                        name=name, shape=None)
                    if restored_initial_value is not None:
                        initial_value = restored_initial_value
                v = variable_scope.variable(
                    initial_value, name=name, trainable=False,
                    use_resource=resource_variable_ops.is_resource_variable(
                        colocate_with))
            # Restore this variable by name if necessary, but don't add a
            # Trackable dependency. Optimizers return the current graph's
            # non-slot variables from _checkpoint_dependencies explicitly rather
            # than unconditionally adding dependencies (since there may be multiple
            # non-slot variables with the same name in different graphs, trying to
            # save all of them would result in errors).
            # self._handle_deferred_dependencies(name=name, trackable=get_rtt_var_from_rtt_tensor(v))
            self._non_slot_dict[key] = v

        return v


    def _finish(self, update_ops, name_scope):
        # Update the power accumulators.
        with ops.control_dependencies(update_ops):
            beta1_power, beta2_power = self._get_beta_accumulators()
            with ops.colocate_with(beta1_power):
                update_beta1 = tf.assign(beta1_power, (beta1_power * self._str_beta1_t), use_locking=self._use_locking)
                update_beta2 = tf.assign(beta2_power, (beta2_power * self._str_beta2_t), use_locking=self._use_locking)
        return control_flow_ops.group(
            *update_ops + [update_beta1, update_beta2], name=name_scope)


    def minimize(self, loss, global_step=None, var_list=None,
                 gate_gradients=1, aggregation_method=None,
                 colocate_gradients_with_ops=False, name=None,
                 grad_loss=None):

        rtt_get_logger().debug('prepare to run StaticReplacePass...')

        # Create StaticReplacePass
        PassObj = StaticReplacePass()

        # Run the pass, and return new loss
        if isinstance(loss, rtt_ts.RttTensor):
            loss = PassObj.run(loss._raw)
        else:
            loss = PassObj.run(loss)
        rtt_get_logger().debug('run StaticReplacePass success.')

        # generate secure gradient graph
        train_op = super(tf.train.AdamOptimizer, self).minimize(loss, global_step, var_list,
                                                            gate_gradients, aggregation_method,
                                                            colocate_gradients_with_ops, name,
                                                            grad_loss)

        # generate message id
        MsgIdGenerator(regenerate=True).gen_msgid_and_notified(loss)

        return train_op


# assign secure adam optimizer to adam optimizer,
# now, the tf default adam optimizer is secure adam optimizer
tf.train.AdamOptimizer = SecureAdamOptimizer


