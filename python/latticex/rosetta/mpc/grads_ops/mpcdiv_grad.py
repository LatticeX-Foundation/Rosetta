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
from latticex.rosetta.mpc.decorator import MpcTruediv, MpcMul

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


@ops.RegisterGradient("MpcDiv")
def MpcDivGrad(op, grad):
    """The gradient for the Mpc Div operator."""
    return _MpcDivideGrad(op, grad)

@ops.RegisterGradient("MpcTruediv")
def MpcTruedivGrad(op, grad):
    """The gradient for the Mpc Truediv operator."""
    return _MpcDivideGrad(op, grad)

@ops.RegisterGradient("MpcRealDiv")
def MpcRealDivGrad(op, grad):
    """The gradient for the Mpc RealDiv operator."""
    return _MpcDivideGrad(op, grad)


def _MpcDivideGrad(op, grad):
    x = op.inputs[0]
    y = op.inputs[1]
    sx = array_ops.shape(x)
    sy = array_ops.shape(y)
    rx, ry = gen_array_ops.broadcast_gradient_args(sx, sy)
    x = math_ops.conj(x)
    y = math_ops.conj(y)
    dX = array_ops.reshape(math_ops.reduce_sum(MpcTruediv(grad, y), rx), sx)
    temp = MpcTruediv(MpcTruediv(-x, y), y)
    dY = array_ops.reshape(math_ops.reduce_sum(MpcMul(grad, temp), ry), sy)
    return (dX, dY)

