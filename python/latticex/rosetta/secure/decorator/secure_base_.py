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
import struct

if 'ROSETTA_MPC_128' in os.environ and os.environ['ROSETTA_MPC_128'] == 'ON':
    _secureop_lib = os.path.dirname(__file__) + '/../../../lib128/libsecure-ops.so'
else:
    _secureop_lib = os.path.dirname(__file__) + '/../../../libsecure-ops.so'
_secure_ops = tf.load_op_library(_secureop_lib)

def _encode_party_id(party):
    if isinstance(party, list) or isinstance(party, tuple):
        encoded_bytes = struct.pack("@i", len(party))
        node_id_flag = 'N'.encode('utf-8')
        node_id_encoded_bytes = struct.pack("%ds" % len(node_id_flag), node_id_flag)
        party_id_flag = 'P'.encode('utf-8')
        party_id_encoded_bytes = struct.pack("%ds" % len(party_id_flag), party_id_flag)
        for item in party:
            if isinstance(item, str):
                encoded_bytes += node_id_encoded_bytes
                item_encoded_bytes = item.encode('utf-8')
                encoded_bytes += struct.pack("@i%ds" % len(item_encoded_bytes), len(item_encoded_bytes), item_encoded_bytes)
            elif isinstance(item, int):
                encoded_bytes += party_id_encoded_bytes
                encoded_bytes += struct.pack("@i", item)
            else:
                raise Exception("unsupport " + type(item) + "!")
    elif isinstance(party, int) or party == None:
        if party == None:
            party = 0
        elif party < 1:
            raise Exception("at least one node should be specified to reveal result!")
        else:
            party *= -1
        encoded_bytes = struct.pack("@i", party)
    else:
        raise Exception("unsupport " + type(party) + "!")
    return encoded_bytes
