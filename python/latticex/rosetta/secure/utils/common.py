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
import re
import numpy as np


def is_secure_compare_tensor(tensor):
    """Whether the tensor is the secure compare tensor.

    Args:
        tensors: Tensors to check.

    Return:
        True: mean the tensorf is secure compare tensor, otherwise is not.
    """

    secure_compare_op_name = ("secureequal", "securenotequal", "securegreater", "securegreaterequal", "securelessequal", "secureless")
    if (tensor != None):
        op_def_name = tensor.op.op_def.name.lower()
        if (op_def_name in secure_compare_op_name) :
            return True
            
    return False


