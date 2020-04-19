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

from latticex.rosetta.mpc import CopyAndRepMpcOpPass
from latticex.rosetta.mpc.utils.common import is_mpc_compare_tensor


import logging
logging.basicConfig(format='%(asctime)s - %(pathname)s[line:%(lineno)d] - %(levelname)s: %(message)s', level=logging.DEBUG)


class MpcGradientDescentOptimizer(tf.train.GradientDescentOptimizer):
    """
    # mpc GradientDescent optimizer, only override __init__ & minimize functions
    """

    def __init__(self, learning_rate, use_locking=False, name="GradientDescent"):
        """
        # Construct a new mpc GradientDescent optimizer.
        """
        super(tf.train.GradientDescentOptimizer, self).__init__(learning_rate, use_locking, name)


    def _assert_valid_dtypes(self, tensors):
        """Asserts tensors are all valid types (see `_valid_dtypes`).

        Args:
            tensors: Tensors to check.

        Raises:
            ValueError: If any tensor is not a valid type.
        """

        valid_dtypes = self._valid_dtypes()
        for t in tensors:
            if (is_mpc_compare_tensor(t)):
                raise ValueError(
                    "Invalid type %r for %s, not expected mpc compare op." % (
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
        logging.debug('begin to run CopyAndRepMpcOpPass...')

        # Create CopyAndRepMpcOpPass
        PassObj = CopyAndRepMpcOpPass()

        # Run the pass, and return new loss
        loss = PassObj.run(loss)

        logging.debug('end to run CopyAndRepMpcOpPass.')

        return super(tf.train.GradientDescentOptimizer, self).minimize(loss, global_step, var_list,
                                                                       gate_gradients, aggregation_method,
                                                                       colocate_gradients_with_ops, name,
                                                                       grad_loss)


# assign mpc gradient descent optimizer to gradient descent optimizer,
# now, the tf default gradient descent optimizer is mpc gradient descent optimizer
tf.train.GradientDescentOptimizer = MpcGradientDescentOptimizer



# for testing
if __name__ == "__main__":
    # linear model
    X = tf.Variable(1.0, name='x')
    W = tf.Variable(2.0, name='weight')
    b = tf.Variable(3.0, name='bias')
    Loss = tf.multiply(X, W) + b
    train_step = tf.train.GradientDescentOptimizer(0.01).minimize(Loss)

    Writer = tf.summary.FileWriter("log", tf.get_default_graph())
    Writer.close()
