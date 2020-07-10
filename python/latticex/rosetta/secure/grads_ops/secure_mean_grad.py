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
from latticex.rosetta.secure.decorator import SecureTruediv
from tensorflow.python.framework import constant_op
from tensorflow.python.framework import ops
from tensorflow.python.ops import array_ops
from tensorflow.python.ops import math_ops
from tensorflow.python.ops import math_grad



@ops.RegisterGradient("SecureReduceMean")
def SecureMeanGrad(op, grad):
    """The gradient for the Secure Mean operator."""
    sum_grad = math_grad._SumGrad(op, grad)[0]
    input_shape = op.inputs[0]._shape_tuple()
    output_shape = op.outputs[0]._shape_tuple()

    if (input_shape is not None 
        and output_shape is not None 
        and None not in input_shape 
        and None not in output_shape
    ):
        input_size = np.prod(input_shape)
        output_size = np.prod(output_shape)
        factor = input_size // max(output_size, 1)
        factor = constant_op.constant(factor, dtype=tf.int32)
    else:
        input_shape = array_ops.shape(op.inputs[0])
        output_shape = array_ops.shape(op.outputs[0])
        factor = math_grad._safe_shape_div(math_ops.reduce_prod(input_shape), math_ops.reduce_prod(output_shape))

    return SecureTruediv(sum_grad, tf.as_string(factor), lh_is_const=False, rh_is_const=True), None

