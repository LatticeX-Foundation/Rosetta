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
from tensorflow.python.ops import random_ops
from tensorflow import dtypes
from latticex.rosetta.mpc.utils.common import dtype_check_and_set

def MpcRandom_normal(shape,
                  mean=0.0,
                  stddev=1.0,
                  dtype=dtypes.float64,
                  seed=None,
                  name=None):
    dtype = dtype_check_and_set(dtype)
    return random_ops.random_normal(shape, mean, stddev, dtype, seed, name)


def MpcRandom_uniform(shape,
                   minval=0,
                   maxval=None,
                   dtype=dtypes.float32,
                   seed=None,
                   name=None):
    dtype = dtype_check_and_set(dtype)
    return random_ops.random_uniform(shape, minval, maxval, dtype, seed, name)


def MpcTruncated_normal(shape,
                     mean=0.0,
                     stddev=1.0,
                     dtype=dtypes.float32,
                     seed=None,
                     name=None):
    dtype = dtype_check_and_set(dtype)
    return random_ops.truncated_normal(shape, mean, stddev, dtype, seed, name)
   

def MpcRandom_gamma(shape,
                 alpha,
                 beta=None,
                 dtype=dtypes.float32,
                 seed=None,
                 name=None):
    dtype = dtype_check_and_set(dtype)
    return random_ops.random_gamma(shape, alpha, beta, dtype, seed, name)


def MpcRandom_poisson(lam, shape, dtype=dtypes.float32, seed=None, name=None):
    dtype = dtype_check_and_set(dtype)
    #return random_ops.random_poisson(shape, mean, stddev, dtype, seed, name)
    return random_ops.random_poisson(lam, shape, dtype, seed, name)

# override random_normal functions for MPC
tf.random_normal = MpcRandom_normal

# override random_uniform functions for MPC
tf.random_uniform = MpcRandom_uniform

# override truncated_normal functions for MPC
tf.truncated_normal = MpcTruncated_normal

# override random_gamma functions for MPC
tf.random_gamma = MpcRandom_gamma

# override random_poisson functions for MPC
tf.random_poisson = MpcRandom_poisson