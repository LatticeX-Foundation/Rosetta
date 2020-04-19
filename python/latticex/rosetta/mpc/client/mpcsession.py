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
from latticex.rosetta.mpc.spass import CopyAndRepMpcOpPass as mpcspass
import re


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
     lambda fetch: ([ReplaceTensorWithMpcTensor(fetch)], lambda fetched_vals: fetched_vals[0]),
     lambda feed, feed_val: [(feed, feed_val)],
     lambda feed: [feed])]

# assign _MPC_REGISTERED_EXPANSIONS to session._REGISTERED_EXPANSIONS
session._REGISTERED_EXPANSIONS = _MPC_REGISTERED_EXPANSIONS


def ReplaceTensorWithMpcTensor(fetch):
    """Replace tensor with mpc tensor.

      fetch: A tensor from run param
      return: return mpc fetch tensor

    Attention: It is not reasonable to check whether this is a gradient graph, 
    but the real usage scenario will not be a problem
    """

    def is_gradients_graph(grad_scope):
        if grad_scope.startswith('gradients/') or re.match("gradients_[0-9]+/", grad_scope) != None:
            return True
        return False


    if isinstance(fetch, ops.Tensor):
        # Check the subgraph exist mpc op,  
        # if exist mpc op, so we known the subgraph has be replaced, do nothing;
        # otherwise we must use CopyAndRepMpcOpPass obj to replace the tf static graph 
        # to mpc static graph
        if not mpcspass.CopyAndRepMpcOpPass.is_exist_mpc_op(fetch.op) and not is_gradients_graph(fetch.name):
            # Create CopyAndRepMpcOpPass object
            PassObj = mpcspass.CopyAndRepMpcOpPass()

            # run static pass
            fetch = PassObj.run(fetch)

    return fetch


# class MpcSession(tf.Session):
#     """A class for running TensorFlow operations, it derived tf.session for MPC.

#       A `Session` object encapsulates the environment in which `Operation`
#       objects are executed, and `Tensor` objects are evaluated.
#     """

#     def __init__(self, target='', graph=None, config=None):
#         """
#         # Construct a new MpcSession object.
#         """
#         print("used mpc session object")
#         super(tf.Session, self).__init__(target, graph, config)


# assign MpcSession to tf.session,
# now, the tf.session is MpcSession
# tf.Session = MpcSession

