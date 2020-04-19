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
from latticex.rosetta.mpc.decorator import MpcMatMul

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


@ops.RegisterGradient("MpcMatMul")
def MpcMatMulGrad(op, grad):
    """The gradient for the Mpc MatMul operator."""
    t_a = op.get_attr("transpose_a")
    t_b = op.get_attr("transpose_b")

    a = op.inputs[0]
    b = op.inputs[1]

    if not t_a and not t_b:
        grad_a = MpcMatMul(grad, b, transpose_b=True)
        grad_b = MpcMatMul(a, grad, transpose_a=True)
    elif not t_a and t_b:
        grad_a = MpcMatMul(grad, b)
        grad_b = MpcMatMul(grad, a, transpose_a=True)
    elif t_a and not t_b:
        grad_a = MpcMatMul(b, grad, transpose_b=True)
        grad_b = MpcMatMul(a, grad)
    elif t_a and t_b:
        grad_a = MpcMatMul(b, grad, transpose_a=True, transpose_b=True)
        grad_b = MpcMatMul(grad, a, transpose_a=True, transpose_b=True)

    return grad_a, grad_b

