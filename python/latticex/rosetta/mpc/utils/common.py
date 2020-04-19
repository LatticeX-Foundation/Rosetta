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
from tensorflow import dtypes
import numpy as np

def dtype_check_and_set(t):
    new_dtype = t
    if t in [None, float, dtypes.float16, dtypes.float32]:
        new_dtype = dtypes.float64
    return new_dtype


def check_mpc_op_grads(out_g, out_mpc_g):
    PRECISION = 5.0/100
    limit_precision = np.full(np.array(out_g).shape, PRECISION, np.float)
    for i in range(len(out_mpc_g)):
        if ((abs(np.array(out_g[i]) - np.array(out_mpc_g[i])) > limit_precision[i]).all()):
            return False
    
    return True


def is_mpc_compare_tensor(tensor):
    """Whether the tensor is the mpc compare tensor.

    Args:
        tensors: Tensors to check.

    Return:
        True: mean the tensorf is mpc compare tensor, otherwise is not.
    """

    mpc_compare_op_name = ("mpcequal", "mpcgreater", "mpcgreaterequal", "mpclessequal", "mpcless")
    if (tensor != None):
        op_def_name = tensor.op.op_def.name.lower()
        if (op_def_name in mpc_compare_op_name) :
            return True
            
    return False

