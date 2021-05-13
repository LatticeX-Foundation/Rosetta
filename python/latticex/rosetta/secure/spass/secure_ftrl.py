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


class SecureFtrlOptimizer(tf.compat.v1.train.FtrlOptimizer):
    """
    # Secure Ftrl optimizer, only override __init__ & minimize functions
    """

    def __init__(self, learning_rate, learning_rate_power=-0.5, initial_accumulator_value=0.1,
                 l1_regularization_strength=0.0, l2_regularization_strength=0.0, use_locking=False, name="Ftrl"):
        """
        # Construct a new secure Ftrl optimizer.
        """
        super(tf.train.FtrlOptimizer, self).__init__(learning_rate, learning_rate_power,
                                                    initial_accumulator_value, l1_regularization_strength,
                                                    l2_regularization_strength, use_locking, name)


    def minimize(self, loss, global_step=None, var_list=None,
                 gate_gradients=1, aggregation_method=None,
                 colocate_gradients_with_ops=False, name=None,
                 grad_loss=None):

        raise NotImplementedError("Now, Ftrl optimizer is not supported!")


# assign secure ftrl optimizer to ftrl optimizer,
# now, the tf default ftrl optimizer is secure ftrl optimizer
tf.train.FtrlOptimizer = SecureFtrlOptimizer

