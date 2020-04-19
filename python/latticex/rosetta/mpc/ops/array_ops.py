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
from latticex.rosetta.mpc.utils.common import dtype_check_and_set
import tensorflow as tf

def MpcPlaceholder(dtype, shape=None, name=None):
    dtype = dtype_check_and_set(dtype)
    return array_ops.placeholder(dtype, shape, name)


def MpcZeros(shape, dtype=dtypes.float64, name=None):
    dtype = dtype_check_and_set(dtype)
    return array_ops.zeros(shape, dtype, name)


def MpcOnes(shape, dtype=dtypes.float64, name=None):
    dtype = dtype_check_and_set(dtype)
    return array_ops.ones(shape, dtype, name)


def MpcZeros_like(tensor, dtype=None, name=None, optimize=True):
    dtype = dtype_check_and_set(dtype)
    return array_ops.zeros_like(tensor, dtype, name)


def MpcOnes_like(tensor, dtype=None, name=None, optimize=True):
    dtype = dtype_check_and_set(dtype)
    return array_ops.ones_like(tensor, dtype, name)

# override placeholder functions for MPC
tf.placeholder = MpcPlaceholder

# override zeros functions for MPC
tf.zeros = MpcZeros

# override ones functions for MPC
tf.ones = MpcOnes

# override zeros_like functions for MPC
tf.zeros_like = MpcZeros_like

# override ones_like functions for MPC
tf.ones_like = MpcOnes_like