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
from tensorflow.python.framework import random_seed
from tensorflow.python.framework import ops
from tensorflow.python.eager import context
from latticex.rosetta.controller.backend_handler import py_protocol_handler
import tensorflow as tf

# base on random_seed.get_seed of TensorFlow
def SecureGetRandomSeed(op_seed=None):
    """Returns the local seeds an operation should use given an op-specific seed.
    Args:
      op_seed: integer.

    Returns:
      A tuple of two integers that should be used for the local seed of this
      operation.
    """
    eager = context.executing_eagerly()

    if eager:
        global_seed = context.global_seed()
    else:
        global_seed = ops.get_default_graph().seed

    seeds = (0, 0)
    if global_seed is not None:
        if op_seed is None:
            # pylint: disable=protected-access
            if hasattr(ops.get_default_graph(), '_seed_used'):
                ops.get_default_graph()._seed_used = True
            if eager:
                op_seed = context.internal_operation_seed()
            else:
                op_seed = ops.get_default_graph()._last_id

        seeds = random_seed._truncate_seed(
            global_seed), random_seed._truncate_seed(op_seed)
    else:
        if op_seed is not None:
            seeds = random_seed.DEFAULT_GRAPH_SEED, random_seed._truncate_seed(
                op_seed)
        else:
            if py_protocol_handler.is_activated():
                seeds = py_protocol_handler.rand_seed(
                    0), py_protocol_handler.rand_seed(0)

    # Avoid (0, 0) as the C++ ops interpret it as nondeterminism, which would
    # random_seedbe unexpected since Python docs say nondeterminism is (None, None).
    if seeds == (0, 0):
        return (0, random_seed._MAXINT32)

    return seeds


# override tensorflow random seed functions for MPC
random_seed.get_seed = SecureGetRandomSeed
tf.random.get_seed = SecureGetRandomSeed
