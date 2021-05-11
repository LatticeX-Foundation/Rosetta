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
# matrix arithmetic ops
# -----------------------------
def SecureMatMul(a,
              b,
              transpose_a=False,
              transpose_b=False,
              adjoint_a=False,
              adjoint_b=False,
              a_is_sparse=False,
              b_is_sparse=False,
              name=None):
    return _secure_ops.secure_matmul(a, b, transpose_a=transpose_a, transpose_b=transpose_b, name=name)








