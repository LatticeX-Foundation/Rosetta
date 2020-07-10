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
from latticex.rosetta.secure.decorator import SecureMul
from tensorflow.python.framework import constant_op
from tensorflow.python.framework import ops



@ops.RegisterGradient("SecureSquare")
def SecureSquareGrad(op, grad):
    """ Gradient for Secure Square. """
    x = op.inputs[0]

    # Added control dependencies to prevent 2*x from being computed too early.
    with ops.control_dependencies([grad]):
        y = constant_op.constant("2")
        return SecureMul(grad, SecureMul(x, y))
