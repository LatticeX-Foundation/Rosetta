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
import numpy as np
from latticex.rosetta.mpc.decorator import MpcMul

from tensorflow.python.compat import compat
from tensorflow.python.eager import context
from tensorflow.python.framework import constant_op
from tensorflow.python.framework import dtypes
from tensorflow.python.framework import ops
from tensorflow.python.framework import tensor_util
from tensorflow.python.ops import array_ops
from tensorflow.python.ops import gen_array_ops
from tensorflow.python.ops import gen_math_ops
from tensorflow.python.ops import math_ops
from tensorflow.python.ops import math_grad


@ops.RegisterGradient("MpcMul")
def MpcMulGrad(op, grad):
  """The gradient of mpc multiplication."""
  x = op.inputs[0]
  y = op.inputs[1]

  if (isinstance(grad, ops.Tensor) and
        math_grad._ShapesFullySpecifiedAndEqual(x, y, grad) and
        grad.dtype in (dtypes.int32, dtypes.float32, dtypes.float64)):
    return (MpcMul(grad, y),
            MpcMul(grad, x))

  assert x.dtype.base_dtype == y.dtype.base_dtype, (x.dtype, " mpc_vs. ", y.dtype)
  sx = array_ops.shape(x)
  sy = array_ops.shape(y)
  rx, ry = gen_array_ops.broadcast_gradient_args(sx, sy)
  x = math_ops.conj(x)
  y = math_ops.conj(y)
  return (array_ops.reshape(math_ops.reduce_sum(MpcMul(grad, y), rx), sx),
          array_ops.reshape(math_ops.reduce_sum(MpcMul(x, grad), ry), sy))

