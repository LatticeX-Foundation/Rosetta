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
from latticex.rosetta.controller.backend_handler import py_io_handler
from latticex.rosetta.controller.io_api import party_id_to_node_id
from latticex.rosetta.controller.io_api import get_current_node_id
from latticex.rosetta.controller.io_api import get_data_node_ids
from latticex.rosetta.controller.common_util import rtt_get_logger
from latticex.rosetta.controller.controller_base_ import _rtt

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


def __check(task_id):
    if not py_protocol_handler.is_activated(task_id):
        errmsg = "Protocol have not activated. See rtt.activate()."
        rtt_get_logger().error(errmsg)
        raise Exception(errmsg)


def __input(task_id, is_private_input: bool, node_id: str, inp):
    __check(task_id)
    is_single = False
    if isinstance(inp, float) or isinstance(inp, int):
        is_single = True
        inp = np.array([float(inp)])
    elif isinstance(inp, np.ndarray):
        pass
    elif isinstance(inp, list):
        inp = np.array(inp)
    else:
        raise Exception("unsupported type~")
    if isinstance(node_id, int):
        node_id = party_id_to_node_id(node_id, task_id = task_id)
    if node_id not in get_data_node_ids(task_id = task_id):
        raise Exception("Node " + node_id + " is not valid data node!")
    if is_private_input:
        ret = _rtt.input.PrivateInput(task_id).input(node_id, inp)
    else:
        ret = _rtt.input.PublicInput(task_id).input(node_id, inp)
    return ret


def __console_input(task_id, is_private_input: bool, node_id: str, shape: tuple = None):
    __check(task_id)
    current_node_id = get_current_node_id(task_id = task_id)

    org_shape = shape
    if shape is None:
        shape = (1,)

    total_inputs = 1
    for i in range(len(shape)):
        if not (i < 2 and isinstance(shape[i], int) and shape[i] > 0):
            raise Exception(
                "shape, only supports None, (m,), (m,n), which m, n is integer and greater 0. row first.")
        total_inputs *= shape[i]

    if total_inputs == 1:
        prompt = "please input the private data (float or integer): "
    else:
        prompt = "please input the private data (float or integer, {} items, separated by space): ".format(
            total_inputs)

    inp = []
    if node_id == current_node_id:
        tries = 3
        while tries > 0:
            inp_ok = True
            inp = []
            instr = input(prompt)
            instr = instr.split(' ')
            instr = [x for x in instr if len(x) > 0]
            if len(instr) < total_inputs:
                print("too few inputs, need {} item(s)".format(total_inputs))
                inp_ok = False
            else:
                for i in range(total_inputs):
                    try:
                        inp.append(float(instr[i]))
                    except Exception as e:
                        print(e)
                        print(
                            "{}, unsupported type. only supports float or integer.".format(instr[i]))
                        inp_ok = False
                        break

            if inp_ok:
                break

            tries -= 1
            if tries == 0:
                raise Exception("invalid inputs")
    else:
        inp = [0] * total_inputs

    arr = __input(task_id, is_private_input, node_id, inp)
    lst = arr.reshape(shape)
    # if org_shape is None:
    #    return lst[0]
    return lst


def private_input(node_id: str, inp, task_id = None):
    """
    One party set its private input value to be shared among multi-parties.

    Args:
        node_id: the input of which node_id will be shared.
        input_value: the input values. ONLY the inputs of the party that
        has the same role as the `node_id` will be processed.

    Return: 
        the local shared piece of the real value. 
        Note that each party will has different cipher value that returned.
    """
    if task_id == None:
        task_id = ''
    if isinstance(node_id, int):
        node_id = party_id_to_node_id(node_id, task_id = task_id)
    return __input(task_id, True, node_id, inp)


def private_console_input(node_id: str, shape: tuple = None, task_id = None):
    """
    Just the same as private_input while the values will be fetched from console.

    Args:
        node_id: which party provide the private data
        shape: only supports None, (m,), (m,n), which m, n is integer and greater 0. row first.

    Usage:
        >> rtt.private_console_input(0)
        >> 1.2
        will return 1.2
        >> rtt.private_console_input(0, (2, 3))
        >> 1e2 2 .3   4 5    6.6
        will return [[100.0, 2.0, 0.3], [4.0, 5.0, 6.6]]
    """
    if task_id == None:
        task_id = ''
    if isinstance(node_id, int):
        node_id = party_id_to_node_id(node_id, task_id = task_id)
    return __console_input(task_id, True, node_id, shape)


def public_input(node_id: str, inp, task_id = None):
    if task_id == None:
        task_id = ''
    if isinstance(node_id, int):
        node_id = party_id_to_node_id(node_id, task_id = task_id)
    return __input(task_id, False, node_id, inp)


def public_console_input(node_id: str, shape: tuple = None, task_id = None):
    if task_id == None:
        task_id = ''
    if isinstance(node_id, int):
        node_id = party_id_to_node_id(node_id, task_id = task_id)
    return __console_input(task_id, False, node_id, shape)


if __name__ == "__main__":
    print(private_console_input(0))
    print(private_console_input(0, None))
    print(private_console_input(0, (1,)))
    print(private_console_input(0, (2,)))
    print(private_console_input(0, (1, 2)))
    print(private_console_input(0, (2, 3)))

    print(public_console_input(0))
    print(public_console_input(0, None))
    print(public_console_input(0, (1,)))
    print(public_console_input(0, (2,)))
    print(public_console_input(0, (1, 2)))
    print(public_console_input(0, (2, 3)))
