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


class MpcAdadeltaOptimizer(tf.train.AdadeltaOptimizer):
    """
    # mpc Adadelta optimizer, only override __init__ & minimize functions
    """

    def __init__(self, learning_rate, use_locking=False, name="Adadelta"):
        """
        # Construct a new mpc Adadelta optimizer.
        """
        super(tf.train.AdadeltaOptimizer, self).__init__(learning_rate, use_locking, name)


    def minimize(self, loss, global_step=None, var_list=None,
                 gate_gradients=1, aggregation_method=None,
                 colocate_gradients_with_ops=False, name=None,
                 grad_loss=None):

        raise NotImplementedError("Now, adadelta optimizer is not supported!")


# assign mpc adadelta optimizer to adadelta optimizer,
# now, the tf default adadelta optimizer is mpc adadelta optimizer
tf.train.AdadeltaOptimizer = MpcAdadeltaOptimizer


# for testing
if __name__ == "__main__":
    # linear model
    X = tf.Variable(1.0, name='x')
    W = tf.Variable(2.0, name='weight')
    b = tf.Variable(3.0, name='bias')
    Loss = tf.multiply(X, W) + b
    train_step = tf.train.AdadeltaOptimizer(0.01).minimize(Loss)
