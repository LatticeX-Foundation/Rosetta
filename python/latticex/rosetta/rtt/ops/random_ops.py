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
from tensorflow.python.framework import dtypes
from tensorflow.python.framework import ops
from tensorflow.python.ops import random_ops
from tensorflow.python.framework import random_seed
from tensorflow.python.ops import gen_random_ops
from tensorflow.python.ops import math_ops


def rtt_random_uniform(shape,
                    minval=0,
                    maxval=None,
                    dtype=dtypes.float32,
                    seed=None,
                    name=None):
  """Outputs random values from a uniform distribution."""

  
  dtype = dtypes.as_dtype(dtype)
  if dtype not in (dtypes.float16, dtypes.bfloat16, dtypes.float32,
                   dtypes.float64, dtypes.int32, dtypes.int64, dtypes.string):
    raise ValueError("Invalid dtype %r" % dtype)

  bk_dtype = dtype
  if (dtype == dtypes.string):
    dtype = dtypes.float32

  if maxval is None:
    if dtype.is_integer:
      raise ValueError("Must specify maxval for integer dtype %r" % dtype)
    maxval = 1
  with ops.name_scope(name, "random_uniform", [shape, minval, maxval]) as name:
    shape = random_ops._ShapeTensor(shape)
    minval = ops.convert_to_tensor(minval, dtype=dtype, name="min")
    maxval = ops.convert_to_tensor(maxval, dtype=dtype, name="max")
    seed1, seed2 = random_seed.get_seed(seed)
    if dtype.is_integer:
      rv = gen_random_ops.random_uniform_int(
          shape, minval, maxval, seed=seed1, seed2=seed2, name=name)
    else:
      rnd = gen_random_ops.random_uniform(shape, dtypes.float32, seed=seed1, seed2=seed2)
      rv = math_ops.add(rnd * (maxval - minval), minval, name=name)

    if (bk_dtype == dtypes.string):
        return tf.as_string(rv)
    else:
        return rv


# override rtt_random_uniform functions for RTT
random_ops.random_uniform = rtt_random_uniform
tf.random_shuffle = rtt_random_uniform

