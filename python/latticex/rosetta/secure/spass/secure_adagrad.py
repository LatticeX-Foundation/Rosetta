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


class SecureAdagradOptimizer(tf.compat.v1.train.AdagradOptimizer):
    """
    # secure Adagrad optimizer, only override __init__ & minimize functions
    """

    def __init__(self, learning_rate, initial_accumulator_value=0.1, use_locking=False, name="Adagrad"):
        """
        # Construct a new secure Adagrad optimizer.
        """
        super(tf.train.AdagradOptimizer, self).__init__(learning_rate, initial_accumulator_value, use_locking, name)


    def minimize(self, loss, global_step=None, var_list=None,
                 gate_gradients=1, aggregation_method=None,
                 colocate_gradients_with_ops=False, name=None,
                 grad_loss=None):

        raise NotImplementedError("Now, Adagrad optimizer is not supported!")


# assign secure adagrad optimizer to adagrad optimizer,
# now, the tf default adagrad optimizer is secure adagrad optimizer
tf.train.AdagradOptimizer = SecureAdagradOptimizer


