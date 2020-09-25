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
from latticex.rosetta.rtt.framework import rtt_tensor as rtt_ts
from tensorflow.python.ops import gen_state_ops


def _get_rtt_var(refVar):
        """get rtt variable from rtt tensor"""
        rtt_var_op_def_name  = ("VariableV2", )
        
        if (isinstance(refVar, rtt_ts.RttTensor)):
            dest_tensor = refVar._raw
            while (dest_tensor.op.op_def.name not in rtt_var_op_def_name):
                assert len(dest_tensor.op.inputs) > 0, "input parameters 'ref' is incorrect!"
                dest_tensor = dest_tensor.op.inputs[0]
            return dest_tensor
        else:
            return refVar



def RttAssign(ref, value, validate_shape=None, use_locking=None, name=None):
    """Update `ref` by assigning `value` to it."""
    value = rtt_ts.convert_to_rtttensor(value)
    ref = _get_rtt_var(ref)

    if ref.dtype._is_ref_dtype:
        return gen_state_ops.assign(
            ref, value._raw, use_locking=use_locking, name=name,
            validate_shape=validate_shape)
    return ref.assign(value._raw, name=name)


# override tensorflow constant functions for RTT constant
tf.assign = RttAssign
