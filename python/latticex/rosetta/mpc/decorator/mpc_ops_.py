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
mpcops = tf.load_op_library(_tf_mpcop_lib)


# print(__file__, tf_mpcop_lib, mpcops)

def MpcMatMul(a,
              b,
              transpose_a=False,
              transpose_b=False,
              adjoint_a=False,
              adjoint_b=False,
              a_is_sparse=False,
              b_is_sparse=False,
              name=None):
    # do other things
    return mpcops.mpc_mat_mul(a, b, transpose_a=transpose_a, transpose_b=transpose_b, name=name)


def MpcPow(x, y, name=None, lh_is_const=False, rh_is_const=True):
    return mpcops.mpc_pow(x, y, name=name, lh_is_const=lh_is_const, rh_is_const=rh_is_const)


def MpcLog(x, name=None):
    return mpcops.mpc_log(x, name=name)


def MpcLog1p(x, name=None):
    return mpcops.mpc_log1p(x, name=name)

# high-precision Log
def MpcHLog(x, name=None):
    return mpcops.mpc_h_log(x, name=name)
