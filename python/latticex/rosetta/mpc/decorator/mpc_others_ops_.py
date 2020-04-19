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

_tf_mpcop_lib = os.path.dirname(__file__) + '/../../../libtf-mpcop.so'
_mpcops = tf.load_op_library(_tf_mpcop_lib)


# print(__file__, tf_mpcop_lib, mpcop_)
# print('tf-mpcop:  {}'.format(tf_mpcop_lib))

# ops
def MpcSigmoid(x, name=None):
    return _mpcops.mpc_sigmoid(x, name=name)


def MpcRelu(x, name=None):
    return _mpcops.mpc_relu(x, name=name)


MpcReluPrime = _mpcops.mpc_relu_prime
MpcAbs = _mpcops.mpc_abs
MpcAbsPrime = _mpcops.mpc_abs_prime

MpcSaveV2 = _mpcops.mpc_save_v2
MpcRestoreV2 = _mpcops.mpc_restore_v2
MpcApplyGradientDescent = _mpcops.mpc_apply_gradient_descent
MpcSigmoidCrossEntropy = _mpcops.mpc_sigmoid_cross_entropy
MpcReveal = _mpcops.mpc_reveal
