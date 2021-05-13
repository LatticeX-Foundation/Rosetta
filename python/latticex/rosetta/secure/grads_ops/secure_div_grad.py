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
from latticex.rosetta.secure.decorator import SecureTruediv, SecureMul, SecureSum, SecureNeg
from tensorflow.python.framework import ops
from tensorflow.python.ops import array_ops
from tensorflow.python.ops import gen_array_ops



@ops.RegisterGradient("SecureDiv")
def SecureDivGrad(op, grad):
    """The gradient for the Secure Div operator."""
    return _SecureDivideGrad(op, grad)


@ops.RegisterGradient("SecureTruediv")
def SecureTruedivGrad(op, grad):
    """The gradient for the Secure Truediv operator."""
    return _SecureDivideGrad(op, grad)


@ops.RegisterGradient("SecureRealdiv")
def SecureRealDivGrad(op, grad):
    """The gradient for the Secure RealDiv operator."""
    return _SecureDivideGrad(op, grad)


@ops.RegisterGradient("SecureFloordiv")
def SecureFloorDivGrad(_, unused_grad):
  """The gradient for the SecureFloorDiv operator."""
  return None, None


def _SecureDivideGrad(op, grad):
    x = op.inputs[0]
    y = op.inputs[1]
    sx = array_ops.shape(x)
    sy = array_ops.shape(y)
    rx, ry = gen_array_ops.broadcast_gradient_args(sx, sy)
    dX = array_ops.reshape(SecureSum(SecureTruediv(grad, y), rx), sx)
    temp = SecureTruediv(SecureTruediv(SecureNeg(x), y), y)
    dY = array_ops.reshape(SecureSum(SecureMul(grad, temp), ry), sy)
    return (dX, dY)

