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
import os

_tf_mpcop_lib = os.path.dirname(__file__) + '/../../../libtf-mpcop.so'
_mpcops = tf.load_op_library(_tf_mpcop_lib)

# print(__file__, tf_mpcop_lib, _mpcops)


def MpcMax(input_tensor,
           axis=None,
           keepdims=None,
           name=None,
           reduction_indices=None,
           keep_dims=None):
    # if axis is None:
    #     axis = reduction_indices
    # if axis is None:
    #     axis = -1
    
    keepdims = False if keepdims is None else keepdims
    axis = math_ops._ReductionDims(input_tensor, axis)

    return _mpcops.mpc_max(input_tensor, reduction_indices=axis, name=name, keep_dims=keepdims)

MpcMin = _mpcops.mpc_min

def MpcMean(input_tensor,
            axis=None,
            keepdims=None,
            name=None,
            reduction_indices=None,
            keep_dims=None):
    # if axis is None:
    #     axis = reduction_indices
    # if axis is None:
    #     axis = -1

    keepdims = False if keepdims is None else keepdims
    axis = math_ops._ReductionDims(input_tensor, axis)

    return _mpcops.mpc_mean(input_tensor, reduction_indices=axis, name=name, keep_dims=keepdims)
