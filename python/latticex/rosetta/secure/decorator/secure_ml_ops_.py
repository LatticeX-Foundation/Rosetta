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
import os


_secureop_lib = os.path.dirname(__file__) + '/../../../libsecure-ops.so'
_secure_ops = tf.load_op_library(_secureop_lib)


# --------------------------------
# secure machine learning related ops
# --------------------------------
def SecureSigmoid(x, name=None):
    return _secure_ops.secure_sigmoid(x, name=name)

def SecureRelu(x, name=None):
    return _secure_ops.secure_relu(x, name=name)

def SecureReluPrime(x, name=None):
    return _secure_ops.secure_relu_prime(x, name=name)

SecureSaveV2 = _secure_ops.secure_save_v2
# SecureRestoreV2 = _secure_ops.secure_restore_v2
SecureApplyGradientDescent = _secure_ops.secure_apply_gradient_descent
SecureSigmoidCrossEntropy = _secure_ops.secure_sigmoid_cross_entropy
SecureReveal = _secure_ops.secure_reveal
