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
from latticex.rosetta.secure.decorator import SecureMatMul
from tensorflow.python.framework import ops



@ops.RegisterGradient("SecureMatmul")
def SecureMatMulGrad(op, grad):
    """The gradient for the Secure MatMul operator."""
    t_a = op.get_attr("transpose_a")
    t_b = op.get_attr("transpose_b")

    a = op.inputs[0]
    b = op.inputs[1]

    if not t_a and not t_b:
        grad_a = SecureMatMul(grad, b, transpose_b=True)
        grad_b = SecureMatMul(a, grad, transpose_a=True)
    elif not t_a and t_b:
        grad_a = SecureMatMul(grad, b)
        grad_b = SecureMatMul(grad, a, transpose_a=True)
    elif t_a and not t_b:
        grad_a = SecureMatMul(b, grad, transpose_b=True)
        grad_b = SecureMatMul(a, grad)
    elif t_a and t_b:
        grad_a = SecureMatMul(b, grad, transpose_a=True, transpose_b=True)
        grad_b = SecureMatMul(grad, a, transpose_a=True, transpose_b=True)

    return grad_a, grad_b

