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
import json

from latticex.rosetta.controller.common_util import *
from latticex.rosetta.controller import backend_handler
from latticex.rosetta.controller.backend_handler import py_io_handler
from latticex.rosetta.controller.controller_base_ import _rtt

def __process(load_dict):
    mpc = load_dict.get('MPC')
    ret = {}
    ret['NODE_INFO'] = []
    if mpc != None:
        for i in range(3):
            pi = mpc['P' + str(i)]
            pi['NODE_ID'] = 'P' + str(i)
            ret['NODE_INFO'].append(pi)
        ret['DATA_NODES'] = ['P0', 'P1', 'P2']
        ret['COMPUTATION_NODES'] = {'P0':0, 'P1':1, 'P2':2}
        ret['RESULT_NODES'] = ['P0', 'P1', 'P2']
        return ret
    return load_dict
    
def __create_io(task_id, io_config_str=None):
    if io_config_str is None:
        config_json_str = None
        try:
            with open(backend_handler._cfgfile, 'r') as load_f:
                load_dict = json.load(load_f)
                load_dict = __process(load_dict)
                rtt_get_logger().debug("original config file: %s" % str(load_dict))

                if backend_handler._party_id != -1:
                    computation_nodes = load_dict["COMPUTATION_NODES"]
                    party_id = backend_handler._party_id
                    for name in computation_nodes.keys():
                        value = computation_nodes[name]
                        if value == party_id:
                            node_id = name
                if backend_handler._node_id != '':
                    node_id = backend_handler._node_id
                config_json_str = json.dumps(load_dict, indent=4)
                #print('task id:', task_id, 'node id:', node_id, ', config:', config_json_str)
                io_config_str = config_json_str
        except Exception as e:
            rtt_get_logger().error("Fail to load or parse your config:[" + str(e) +
                                   "]. Please make sure your provide the correct Config string parameter " +
                                   "or CONFIG.json file in local directory.")
            return
    res = py_io_handler.create_io(task_id, node_id, io_config_str)
    return res

def __has_io_wrapper(task_id = None):
    if task_id == None:
        task_id = ''
    return py_io_handler.has_io_wrapper(task_id)

def _check_io(task_id):
    if task_id == None:
        task_id = ''
    if __has_io_wrapper(task_id):
        print('io wrapper with task id[' + task_id + '] already exists')
        return
    if __create_io(task_id):
        print('Create IO ok, task id:', task_id)
    else:
        print('IO already exists, task id:', task_id)

def __get_io_wrapper(task_id = None):
    if task_id == None:
        task_id = ''
    return py_io_handler.get_io_wrapper(task_id)

def set_channel(task_id, io):
    py_io_handler.set_channel(task_id, io)

def party_id_to_node_id(party_id, task_id = None):
    return __get_io_wrapper(task_id).party_id_to_node_id(party_id)

def node_id_to_party_id(node_id, task_id = None):
    return __get_io_wrapper(task_id).node_id_to_party_id(node_id)

def get_current_node_id(task_id = None):
    return __get_io_wrapper(task_id).get_current_node_id()

def get_current_party_id(task_id = None):
    return __get_io_wrapper(task_id).get_current_party_id()

def get_data_node_ids(task_id = None):
    return __get_io_wrapper(task_id).get_data_node_ids()

def get_computation_node_ids(task_id = None):
    return __get_io_wrapper(task_id).get_computation_node_ids()

def get_result_node_ids(task_id = None):
    return __get_io_wrapper(task_id).get_result_node_ids()

def get_connected_node_ids(task_id = None):
    return __get_io_wrapper(task_id).get_connected_node_ids()

def recv_msg(node_id, msg_id, msg_len, task_id = None):
    return __get_io_wrapper(task_id).recv_msg(node_id, msg_id, msg_len)

def send_msg(node_id, msg_id, msg, task_id = None):
    __get_io_wrapper(task_id).send_msg(node_id, msg_id, msg)
