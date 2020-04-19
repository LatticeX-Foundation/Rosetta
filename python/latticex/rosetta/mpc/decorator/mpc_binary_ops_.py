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


# print(__file__, tf__mpcoplib, mpcop)

def MpcAdd(x, y, name=None, lh_is_const=False, rh_is_const=False):
    return _mpcops.mpc_add(x, y, name=name,
                          lh_is_const=lh_is_const, rh_is_const=rh_is_const)


def MpcSub(x, y, name=None, lh_is_const=False, rh_is_const=False):
    return _mpcops.mpc_sub(x, y, name=name,
                          lh_is_const=lh_is_const, rh_is_const=rh_is_const)


def MpcMul(x, y, name=None, lh_is_const=False, rh_is_const=False):
    return _mpcops.mpc_mul(x, y, name=name,
                          lh_is_const=lh_is_const, rh_is_const=rh_is_const)


def MpcDiv(x, y, name=None, lh_is_const=False, rh_is_const=False):
    return _mpcops.mpc_div(x, y, name=name,
                          lh_is_const=lh_is_const, rh_is_const=rh_is_const)

#TODO: to align with TwnsorFlow naming. Should change this to backend implementation in the future 
MpcFloorDiv = MpcDiv

def MpcTruediv(x, y, name=None, lh_is_const=False, rh_is_const=False):
    return _mpcops.mpc_truediv(x, y, name=name,
                              lh_is_const=lh_is_const, rh_is_const=rh_is_const)

#TODO: to align with TwnsorFlow naming. Should change this to backend implementation in the future
MpcDivide = MpcTruediv

def MpcRealDiv(x, y, name=None, lh_is_const=False, rh_is_const=False):
    return _mpcops.mpc_realdiv(x, y, name=name,
                              lh_is_const=lh_is_const, rh_is_const=rh_is_const)


def MpcEqual(x, y, name=None, lh_is_const=False, rh_is_const=False):
    return _mpcops.mpc_equal(x, y, name=name,
                            lh_is_const=lh_is_const, rh_is_const=rh_is_const)


def MpcGreater(x, y, name=None, lh_is_const=False, rh_is_const=False):
    return _mpcops.mpc_greater(x, y, name=name,
                              lh_is_const=lh_is_const, rh_is_const=rh_is_const)


def MpcGreaterEqual(x, y, name=None, lh_is_const=False, rh_is_const=False):
    return _mpcops.mpc_greater_equal(x, y, name=name,
                                    lh_is_const=lh_is_const, rh_is_const=rh_is_const)


def MpcLess(x, y, name=None, lh_is_const=False, rh_is_const=False):
    return _mpcops.mpc_less(x, y, name=name,
                           lh_is_const=lh_is_const, rh_is_const=rh_is_const)


def MpcLessEqual(x, y, name=None, lh_is_const=False, rh_is_const=False):
    return _mpcops.mpc_less_equal(x, y, name=name,
                                 lh_is_const=lh_is_const, rh_is_const=rh_is_const)
