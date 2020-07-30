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

# -----------------------------
# Import TensorFlow secure binary ops
# -----------------------------
import tensorflow as tf
import os

if 'ROSETTA_MPC_128' in os.environ and os.environ['ROSETTA_MPC_128'] == 'ON':
    _secureop_lib = os.path.dirname(__file__) + '/../../../lib128/libsecure-ops.so'
else:
    _secureop_lib = os.path.dirname(__file__) + '/../../../libsecure-ops.so'
_secure_ops = tf.load_op_library(_secureop_lib)
