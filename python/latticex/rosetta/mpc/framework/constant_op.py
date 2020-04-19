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
from tensorflow.python.framework import constant_op
import tensorflow as tf
from latticex.rosetta.mpc.utils.common import dtype_check_and_set

def MpcConstant(value, dtype=None, shape=None, name="Const", verify_shape=False):
    dtype = dtype_check_and_set(dtype)
    return constant_op.constant_v1(value, dtype, shape, name, verify_shape)

# override tensorflow constant functions for MPC
tf.constant = MpcConstant
