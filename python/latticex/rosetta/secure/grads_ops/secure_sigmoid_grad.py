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
from latticex.rosetta.secure.decorator import SecureMul, SecureSub
from tensorflow.python.framework import constant_op
from tensorflow.python.framework import ops



@ops.RegisterGradient("SecureSigmoid")
def SecureSigmoidGrad(op, grad):
    """The gradient for the Secure Sigmoid operator."""
    one = constant_op.constant("1")
    y = op.outputs[0]  # y = sigmoid(x)
    with ops.control_dependencies([grad]):
        return SecureMul(grad, SecureMul(y, SecureSub(one, y, lh_is_const=True, rh_is_const=False)))
