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
from tensorflow.python.framework import constant_op
import tensorflow as tf
from latticex.rosetta.rtt.utils.common import dtype_check_and_set
from latticex.rosetta.rtt.framework import rtt_tensor as rtt_ts
import numpy as np

def _element_is_str(value):
    if (isinstance(value, (list, tuple))):
        return _element_is_str(value[0])
    elif (isinstance(value, (str,))):
        return True
    else:
        return False


def _const_convert_to_str(value):
    if _element_is_str(value):
        return value

    if (isinstance(value, (list, tuple))):
        result = []
        for item in value:
            if (isinstance(item, (list, tuple))):
                it = [str(x) for x in item]
                result.append(it)
            else:
                result.append(str(item))
        return result
    elif isinstance(value, np.ndarray):
        elems = [str(x) for x in np.nditer(value)]
        conv_value = np.array(elems)
        return conv_value.reshape(value.shape)
    else:
        return str(value)


def RttConstant(value, dtype=None, shape=None, name="Const", verify_shape=False):
    dtype = dtype_check_and_set(dtype)
    value = _const_convert_to_str(value)
    return rtt_ts.constant(constant_op.constant_v1(value, dtype, shape, name, verify_shape))


# override tensorflow constant functions for RTT constant
tf.constant = RttConstant
