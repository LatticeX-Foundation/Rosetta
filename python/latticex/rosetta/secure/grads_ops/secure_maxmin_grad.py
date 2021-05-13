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
from latticex.rosetta.secure.decorator import SecureEqual, SecureMul, SecureTruediv, SecureSum
from tensorflow.python.framework import ops
from tensorflow.python.ops import array_ops
from tensorflow.python.ops import math_ops



def _SecureMinOrMaxGrad(op, grad):
  """Gradient for SecureMin or SecureMax."""
  input_shape = array_ops.shape(op.inputs[0])
  output_shape_kept_dims = math_ops.reduced_shape(input_shape, op.inputs[1])
  y = op.outputs[0]
  y = array_ops.reshape(y, output_shape_kept_dims)
  grad = array_ops.reshape(grad, output_shape_kept_dims)

  # Compute the number of selected (maximum or minimum) elements in each
  # reduction dimension. If there are multiple minimum or maximum elements
  # then the gradient will be divided between them.
  indicators = SecureEqual(y, op.inputs[0])
  num_selected = array_ops.reshape(SecureSum(indicators, op.inputs[1]), output_shape_kept_dims)

  return [SecureMul(SecureTruediv(indicators, num_selected), grad), None]


@ops.RegisterGradient("SecureReduceMax")
def SecureMaxGrad(op, grad):
    """The gradient for the Secure Max operator."""
    return _SecureMinOrMaxGrad(op, grad)


@ops.RegisterGradient("SecureReduceMin")
def SecureMinGrad(op, grad):
    """The gradient for the Secure Min operator."""
    return _SecureMinOrMaxGrad(op, grad)

