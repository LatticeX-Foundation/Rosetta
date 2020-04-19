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
from latticex.rosetta.mpc.client.mpcplayer import mpc_player
import latticex._rosetta as _rtt
import numpy as np
import pandas as pd
from enum import Enum, unique

"""
A simple way to cope private input.
"""


class MpcPrivateInput(object):
    """
    todo
    """
    pass


def private_input(party_id: int, inp):
    if isinstance(inp, float) or isinstance(inp, int):
        inp = float(inp)
        # return mpc_player.private_input(party_id, inp)
        return _rtt.input.Input(mpc_player).private_input(party_id, inp)
    elif isinstance(inp, np.ndarray):
        return _rtt.input.Input(mpc_player).private_input(party_id, inp)
    else:
        raise Exception("unsupported type~")


def private_console_input(party_id: int):
    if party_id == mpc_player.id:
        instr = input("please input the private data (float or integer): ")
        inp = float(instr)
    else:
        inp = 0
    return private_input(party_id, inp)
