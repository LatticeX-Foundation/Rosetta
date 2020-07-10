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
from tensorflow.python.ops import array_ops
from tensorflow import dtypes, placeholder, zeros, ones, zeros_like, ones_like
from latticex.rosetta.rtt.utils.common import dtype_check_and_set
from latticex.rosetta.rtt.framework import rtt_tensor as rtt_ts
import tensorflow as tf


def RttPlaceholder(dtype, shape=None, name=None):
    dtype = dtype_check_and_set(dtype)
    return rtt_ts.placeholder(array_ops.placeholder(dtype, shape, name))


# override placeholder functions for RTT
tf.placeholder = RttPlaceholder

