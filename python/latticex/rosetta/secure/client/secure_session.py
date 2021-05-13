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
from tensorflow.python.framework import ops
from tensorflow.python.framework import sparse_tensor
from tensorflow.python.client import session
from latticex.rosetta.controller.protocol_api import default_run
from latticex.rosetta.secure.spass.static_replace_pass import replace_tf_subgraph_with_secure_subgraph


# List of extensions supported to convert run arguments into actual 
# fetches and feeds.
_MPC_REGISTERED_EXPANSIONS = [
    # SparseTensors are fetched as SparseTensorValues. They can be fed
    # SparseTensorValues or normal tuples.
    (sparse_tensor.SparseTensor,
     lambda fetch: (
         [fetch.indices, fetch.values, fetch.dense_shape],
         lambda fetched_vals: sparse_tensor.SparseTensorValue(*fetched_vals)),
     lambda feed, feed_val: list(zip(
         [feed.indices, feed.values, feed.dense_shape], feed_val)),
     lambda feed: [feed.indices, feed.values, feed.dense_shape]),
    # IndexedSlices are fetched as IndexedSlicesValues. They can be fed
    # IndexedSlicesValues or normal tuples.
    (ops.IndexedSlices,
     lambda fetch: (
         [fetch.values, fetch.indices] if fetch.dense_shape is None
         else [fetch.values, fetch.indices, fetch.dense_shape],
         session._get_indexed_slices_value_from_fetches),
     session._get_feeds_for_indexed_slices,
     lambda feed: [feed.values, feed.indices] if feed.dense_shape is None
     else [feed.values, feed.indices, feed.dense_shape]),
    # The default catches all other types and performs no expansions.
    (object,
     lambda fetch: ([replace_tf_subgraph_with_secure_subgraph(fetch)], lambda fetched_vals: fetched_vals[0]),
     lambda feed, feed_val: [(feed, feed_val)],
     lambda feed: [feed])]

# assign _MPC_REGISTERED_EXPANSIONS to session._REGISTERED_EXPANSIONS
session._REGISTERED_EXPANSIONS = _MPC_REGISTERED_EXPANSIONS


class SecureSession(tf.compat.v1.Session):
    """A class for running TensorFlow operations, it derived tf.session for MPC.

      A `Session` object encapsulates the environment in which `Operation`
      objects are executed, and `Tensor` or 'RttTensor' objects are evaluated.
    """

    def __init__(self, target='', graph=None, config=None):
        """
        # Construct a new SecureSession object.
        """
        super().__init__(target, graph, config)

    
    def run(self, fetches, feed_dict=None, options=None, run_metadata=None):
        """
        Runs operations and evaluates tensors in `fetches`.
        """
        # Init backend for compatible with previous versions
        default_run()

        return super().run(fetches, feed_dict, options, run_metadata)
        

    def partial_run(self, handle, fetches, feed_dict=None):
        """
        Continues the execution with more feeds and fetches.
        """
        # Init backend for compatible with previous versions
        default_run()

        return super().partial_run(handle, fetches, feed_dict)



# assign SecureSession to tf.session
tf.Session = SecureSession
tf.compat.v1.Session = SecureSession
