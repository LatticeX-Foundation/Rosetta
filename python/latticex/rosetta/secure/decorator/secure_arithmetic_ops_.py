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
from tensorflow.python.ops import array_ops
from tensorflow.python.framework import ops
from latticex.rosetta.secure.decorator.secure_base_ import _secure_ops


# -----------------------------
# Secure arithmetic binary ops
# -----------------------------
def SecureAdd(x, y, name=None, lh_is_const=False, rh_is_const=False):
    return _secure_ops.secure_add(x, y, name=name,
                                  lh_is_const=lh_is_const, rh_is_const=rh_is_const)


def SecureSub(x, y, name=None, lh_is_const=False, rh_is_const=False):
    return _secure_ops.secure_sub(x, y, name=name,
                                  lh_is_const=lh_is_const, rh_is_const=rh_is_const)


def SecureMul(x, y, name=None, lh_is_const=False, rh_is_const=False):
    return _secure_ops.secure_mul(x, y, name=name,
                                  lh_is_const=lh_is_const, rh_is_const=rh_is_const)


def SecureDiv(x, y, name=None, lh_is_const=False, rh_is_const=False):
    return _secure_ops.secure_div(x, y, name=name,
                                  lh_is_const=lh_is_const, rh_is_const=rh_is_const)


def SecureRealDiv(x, y, name=None, lh_is_const=False, rh_is_const=False):
    return _secure_ops.secure_realdiv(x, y, name=name,
                                      lh_is_const=lh_is_const, rh_is_const=rh_is_const)


def SecureFloorDiv(x, y, name=None, lh_is_const=False, rh_is_const=False):
    return _secure_ops.secure_floordiv(x, y, name=name,
                                       lh_is_const=lh_is_const, rh_is_const=rh_is_const)


def SecureTruediv(x, y, name=None, lh_is_const=False, rh_is_const=False):
    return _secure_ops.secure_truediv(x, y, name=name,
                                      lh_is_const=lh_is_const, rh_is_const=rh_is_const)


# TODO: to align with TwnsorFlow naming. Should change this to backend implementation in the future
SecureDivide = SecureDiv


def SecurePow(x, y, name=None, lh_is_const=False, rh_is_const=True):
    return _secure_ops.secure_pow(x, y, name=name, lh_is_const=lh_is_const, rh_is_const=rh_is_const)


# -----------------------------
# secure arithmetic unary ops
# -----------------------------
def SecureNeg(x, name=None):
    return _secure_ops.secure_negative(x, name)


def SecureSquare(x, name=None):
    return _secure_ops.secure_square(x, name)


def SecureLog(x, name=None):
    return _secure_ops.secure_log(x, name=name)


def SecureLog1p(x, name=None):
    return _secure_ops.secure_log1p(x, name=name)


# high-precision Log
def SecureHLog(x, name=None):
    return _secure_ops.secure_h_log(x, name=name)


def SecureAbs(x, name=None):
    return _secure_ops.secure_abs(x, name=name)


def SecureAbsPrime(x, name=None):
    return _secure_ops.secure_abs_prime(x, name=name)


def SecureAddN(inputs, name=None):
    if not inputs or not isinstance(inputs, (list, tuple)):
        raise ValueError("inputs must be a list of at least one "
                         "Tensor/IndexedSlices with the same dtype and shape")
    inputs = ops.convert_n_to_tensor_or_indexed_slices(inputs)
    if not all(isinstance(x, (ops.Tensor, ops.IndexedSlices)) for x in inputs):
        raise ValueError("inputs must be a list of at least one "
                         "Tensor/IndexedSlices with the same dtype and shape")

    if len(inputs) == 1:
        if isinstance(inputs[0], ops.IndexedSlices):
            values = ops.convert_to_tensor(inputs[0])
        else:
            values = inputs[0]

        if name:
            return array_ops.identity(values, name=name)
        return values

    return _secure_ops.secure_add_n(inputs, name=name)
