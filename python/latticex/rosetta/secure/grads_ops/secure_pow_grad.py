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
from latticex.rosetta.secure.decorator import SecurePow, SecureMul, SecureSub
from tensorflow.python.framework import constant_op
from tensorflow.python.framework import ops
from tensorflow.python.ops import array_ops
from tensorflow.python.ops import string_ops
from tensorflow.python.ops import gen_math_ops
from tensorflow.python.framework import dtypes



@ops.RegisterGradient("SecurePow")
def SecurePowGrad(op, grad):
    """The gradient for the Secure Pow operator.
    Currently, only y is supported as a constant.
    """
    x = op.inputs[0]
    y = op.inputs[1]
    sx = array_ops.shape(x)
    sy = array_ops.shape(y)
    y_dec = string_ops.as_string(gen_math_ops.sub(string_ops.string_to_number(y),
                                constant_op.constant(1.0, dtype=dtypes.float32)))
    # one = constant_op.constant("1")
    # y_dec = SecureSub(y, one, lh_is_const=True, rh_is_const=True)
    temp = SecureMul(y, SecurePow(x, y_dec, lh_is_const=False, rh_is_const=True), 
                  lh_is_const=True, rh_is_const=False)
    zero = constant_op.constant("0")
    dX = array_ops.reshape(SecureMul(grad, temp), sx)
    dY = array_ops.reshape(zero, sy)

    return dX, dY
    
