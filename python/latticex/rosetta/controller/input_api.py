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
from latticex.rosetta.controller.backend_handler import py_protocol_handler
from latticex.rosetta.controller.common_util import rtt_get_logger
import latticex._rosetta as _rtt

import numpy as np
import pandas as pd
from enum import Enum, unique

"""
A simple way to cope private input.
"""


class SecurePrivateInput(object):
    """
    Not implemented yet, please use the functional API now.
    """
    pass


def __check():
    if not py_protocol_handler.is_activated():
        errmsg = "Protocol have not activated. See rtt.activate()."
        rtt_get_logger().error(errmsg)
        raise Exception(errmsg)


def private_input(party_id: int, inp):
    """
    One party set its private input value to be shared among multi-parties.

    Args:
        party_id: the input of which party_id will be shared.
        inp: the input values. ONLY the inputs of the party that
        has the same role as the `party_id` will be processed.

    Return: 
        the local shared piece of the real value. 
        Note that each party will has different cipher value that returned.
    """
    __check()
    if isinstance(inp, float) or isinstance(inp, int):
        inp = float(inp)
    elif isinstance(inp, np.ndarray):
        pass
    elif isinstance(inp, list):
        inp = np.array(inp)
    else:
        raise Exception("unsupported type~")
    return _rtt.input.Input().private_input(party_id, inp)


def private_console_input(party_id: int):
    """
    Just the same as private_input while the values will be fetched from console.
    """
    __check()
    partyid = py_protocol_handler.get_party_id()
    if party_id == partyid:
        instr = input("please input the private data (float or integer): ")
        if isinstance(inp, float) or isinstance(inp, int):
            inp = float(instr)
        else:
            raise Exception("unsupported type~")
    else:
        inp = 0
    return private_input(party_id, inp)
