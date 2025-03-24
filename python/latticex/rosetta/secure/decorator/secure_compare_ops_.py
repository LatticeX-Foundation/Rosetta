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


# -----------------------------
# secure compare ops
# -----------------------------
def SecureEqual(x, y, name=None, lh_is_const=False, rh_is_const=False):
    return _secure_ops.secure_equal(x, y, name=name,
                            lh_is_const=lh_is_const, rh_is_const=rh_is_const)


def SecureNotEqual(x, y, name=None, lh_is_const=False, rh_is_const=False):
    return _secure_ops.secure_not_equal(x, y, name=name,
                                lh_is_const=lh_is_const, rh_is_const=rh_is_const)


def SecureGreater(x, y, name=None, lh_is_const=False, rh_is_const=False):
    return _secure_ops.secure_greater(x, y, name=name,
                              lh_is_const=lh_is_const, rh_is_const=rh_is_const)


def SecureGreaterEqual(x, y, name=None, lh_is_const=False, rh_is_const=False):
    return _secure_ops.secure_greater_equal(x, y, name=name,
                                    lh_is_const=lh_is_const, rh_is_const=rh_is_const)


def SecureLess(x, y, name=None, lh_is_const=False, rh_is_const=False):
    return _secure_ops.secure_less(x, y, name=name,
                           lh_is_const=lh_is_const, rh_is_const=rh_is_const)


def SecureLessEqual(x, y, name=None, lh_is_const=False, rh_is_const=False):
    return _secure_ops.secure_less_equal(x, y, name=name,
                                 lh_is_const=lh_is_const, rh_is_const=rh_is_const)