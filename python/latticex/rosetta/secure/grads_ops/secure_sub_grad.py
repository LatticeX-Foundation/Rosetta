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
from latticex.rosetta.secure.decorator import SecureNeg, SecureSum
from tensorflow.python.framework import ops
from tensorflow.python.ops import array_ops
from tensorflow.python.ops import gen_array_ops
from tensorflow.python.ops import math_grad


@ops.RegisterGradient("SecureSub")
def SecureSubGrad(op, grad):
    """Gradient for Secure Sub."""
    x = op.inputs[0]
    y = op.inputs[1]

    if (isinstance(grad, ops.Tensor) and  math_grad._ShapesFullySpecifiedAndEqual(x, y, grad)):
        return grad, SecureNeg(grad)

    sx = array_ops.shape(x)
    sy = array_ops.shape(y)
    rx, ry = gen_array_ops.broadcast_gradient_args(sx, sy)
    return (array_ops.reshape(SecureSum(grad, rx), sx),
            array_ops.reshape(SecureNeg(SecureSum(grad, ry)), sy))

