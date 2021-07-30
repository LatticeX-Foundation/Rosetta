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
from latticex.rosetta.secure.decorator import SecurePow, SecureMul
from tensorflow.python.framework import constant_op
from tensorflow.python.framework import ops



@ops.RegisterGradient("SecureRsqrt")
def SecureRsqrtGrad(op, grad):
    """The gradient for the SecureRsqrt operator."""
    """Returns -0.5 * grad * y^3."""
    y = op.outputs[0]  #y = x^(-1/2)
    with ops.control_dependencies([grad]):
        half_neg_one = constant_op.constant("-0.5")
        three = constant_op.constant("3")
        y_3 = SecurePow(y, three)
        return SecureMul(SecureMul(grad, half_neg_one), y_3)
