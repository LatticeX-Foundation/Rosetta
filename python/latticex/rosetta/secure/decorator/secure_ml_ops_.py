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
from latticex.rosetta.secure.decorator.secure_base_ import _secure_ops
import numpy as np

# --------------------------------
# secure machine learning related ops
# --------------------------------


def SecureSigmoid(x, name=None):
    return _secure_ops.secure_sigmoid(x, name=name)


def SecureSigmoidV2(x, name=None):
    """secure sigmoid approximated with 6-piece-wise polynomial
        sigmoid(x) equals:
        0, for x in range (-inf, -4];
        0.0484792 * x + 0.1998976, for x in range (-4, -2]; 
        0.1928931 * x + 0.4761351, for x in range (-2, 0];
        0.1928931 * x + 0.5238649, for x in range (0, 2];
        0.0484792 * x + 0.8001024, for x in range (2,4];
        1, for x in range (4, +inf) 
    """
    # 0, for x in range (-inf, -4)
    # a1 = np.full(np.double(x).shape, 0.0484792, dtype=np.float64)
    # a2 = np.full(np.double(x).shape, 0.1928931, dtype=np.float64)
    # a3 = np.full(np.double(x).shape, 0.1928931, dtype=np.float64)
    # a4 = np.full(np.double(x).shape, 0.0484792, dtype=np.float64)

    cmp_neg_4 = tf.greater_equal(tf.constant("-4.0"), x)
    cmp_neg_2 = tf.greater_equal(tf.constant("-2.0"), x)
    cmp_0 = tf.greater_equal(tf.constant("0.0"), x)
    cmp_pos_2 = tf.greater_equal(tf.constant("2.0"), x)
    cmp_pos_4 = tf.greater_equal(tf.constant("4.0"), x)

    # 0
    y0 = tf.constant("0.0")
    # 0.0484792 * x + 0.1998976, for x in range (-4, -2];
    y1 = tf.add(tf.multiply(x, tf.constant("0.0484792")), tf.constant("0.1998976"))
    y_delta_0 = tf.subtract(y0, y1)
    # 0.1928931 * x + 0.4761351, for x in range (-2, 0];
    y2 = tf.add(tf.multiply(x, tf.constant("0.1928931")), tf.constant("0.4761351"))
    y_delta_1 = tf.subtract(y1, y2)
    # 0.1928931 * x + 0.5238649, for x in range (0, 2];
    y3 = tf.add(tf.multiply(x, tf.constant("0.1928931")), tf.constant("0.5238649"))
    y_delta_2 = tf.subtract(y2, y3)
    # 0.0484792 * x + 0.8001024, for x in range (2,4];
    y4 = tf.add(tf.multiply(x, tf.constant("0.0484792")), tf.constant("0.8001024"))
    y_delta_3 = tf.subtract(y3, y4)
    # 1
    y5 = tf.constant("1.0")
    y_delta_4 = tf.subtract(y4, y5)

    # cmp(-4, x)*(0 - y1) + cmp(-2,x)*(y1-y2)+ ... + cmp(4, x)*(y4-1) + 1
    result0 = tf.add(tf.multiply(cmp_neg_4, y_delta_0), tf.multiply(cmp_neg_2, y_delta_1))
    result1 = tf.add(result0, tf.multiply(cmp_0, y_delta_2))
    result2 = tf.add(result1, tf.multiply(cmp_pos_2, y_delta_3))
    result3 = tf.add(result2, tf.multiply(cmp_pos_4, y_delta_4))
    # const is necessary ?
    return tf.add(result3, y5)


def SecureSigmoidV3(x, name=None):
    """secure sigmoid approximated with 3-piece-wise polynomial
        sigmoid(x) equals:
        0, for x in range (-inf, -3.1];
        0.161*x + 0.5, for x in range (-3.1, 3.1];
        1, for x in range (3.1, +inf) 
    """
    cmp_neg_3 = tf.greater_equal(tf.constant("-3.1"), x)
    cmp_pos_3 = tf.greater_equal(tf.constant("3.1"), x)

    # 0
    y0 = tf.constant("0")
    # 0.161*x+0.5, x in (-3,1, 3,1]
    y1 = tf.add(tf.multiply(x, tf.constant("0.161")), tf.constant("0.5"))
    y_delta_0 = tf.subtract(y0, y1)
    # 1
    y2 = tf.constant("1")
    y_delta_1 = tf.subtract(y1, y2)

    # cmp(-3.1, x)*(0 - y1) + cmp(3.1, x)*(y1-1) + 1
    result = tf.add(tf.multiply(
        cmp_neg_3, y_delta_0), tf.multiply(cmp_pos_3, y_delta_1))
    return tf.add(result, y2)


def SecureSigmoidChebyshev(x, name=None):
    """Sigmoid implemented with ChebyshevNominal
    sigmoid(x) = 0.5 + 0.2159198015 * x -0.0082176259 * x^3 + 0.0001825597 * x^5 - 0.0000018848 * x^7 + 0.0000000072 * x^9,  x in [-8,8] is preferable
    """
    b0 = tf.constant("0.5")
    a1 = tf.constant("0.2159198015")
    a3 = tf.constant("-0.0082176259")
    a5 = tf.constant("0.0001825597")
    a7 = tf.constant("-0.0000018848")
    a9 = tf.constant("0.0000000072")
    
    # x2 = x^2
    x2 = tf.square(x)
    # x3 = x^3
    x3 = tf.multiply(x, x2)
    # x5 = x^5, x5 = x2 * x3
    x5 = tf.multiply(x2, x3)
    # x7 = x^7,  x7 = x2 * x5
    x7 = tf.multiply(x2, x5)
    # x9 = x2 * x7
    x9 = tf.multiply(x2, x7)

    # y = y9 + y7 + y5 + y3 + y1 + w0
    y1 = tf.add(b0, tf.multiply(x, a1))
    y2 = tf.add(tf.multiply(x3, a3), tf.multiply(x5, a5))
    y3 = tf.add(tf.multiply(x7, a7), tf.multiply(x9, a9))
    return tf.add(tf.add(y1, y2), y3)


def SecureRelu(x, name=None):
    return _secure_ops.secure_relu(x, name=name)


def SecureReluPrime(x, name=None):
    return _secure_ops.secure_relu_prime(x, name=name)


SecureSaveV2 = _secure_ops.secure_save_v2
# SecureRestoreV2 = _secure_ops.secure_restore_v2
SecureApplyGradientDescent = _secure_ops.secure_apply_gradient_descent
SecureSigmoidCrossEntropy = _secure_ops.secure_sigmoid_cross_entropy
SecureReveal = _secure_ops.secure_reveal
SecureAssign = _secure_ops.secure_assign
