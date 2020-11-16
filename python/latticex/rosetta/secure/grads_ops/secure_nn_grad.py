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
from latticex.rosetta.secure import SecureMul, SecureSub, SecureSigmoid
from tensorflow.python.framework import constant_op
from tensorflow.python.framework import ops
from tensorflow.python.ops import array_ops
from tensorflow.python.ops import gen_array_ops



@ops.RegisterGradient("SecureSigmoidCrossEntropy")
def SecureSigmoidCrossEntropyGrad(op, grad):
    """ The customized gradient for SecureSigmoidCrossEntropy Operator that is used
        in function sigmoid_cross_entropy_with_logits.
        @note : the function is we use in the forward pass is 
        max(logit, 0) - logit * label + log(1 + exp(-abs(logits))
        just as it is in Tendorflow.
        In the backward pass here, we use  - logits * labels + log(1 + exp(logits).
        So the grad of X is grad * (-Lables + 1/(1+e^(-logits))) 
    """
    return __SecureSigmoidCrossEntropyGrad(op, grad)

def __SecureSigmoidCrossEntropyGrad(op, grad):
    x_logits = op.inputs[0]
    x_labels = op.inputs[1]
    #shape_logits = array_ops.shape(x_logits)
    #shape_labels = array_ops.shape(x_labels)
    #inter_sa, internal_sb = gen_array_ops.broadcast_gradient_args(shape_logits, shape_labels)
    d_logits = SecureMul(grad, SecureSub(SecureSigmoid(x_logits), x_labels))
    zeros = constant_op.constant("0")
    d_labels = SecureMul(grad, SecureSub(zeros, x_logits, lh_is_const=True, rh_is_const=False))
    return (d_logits, d_labels)

