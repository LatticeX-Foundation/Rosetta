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
"""
    This module provides the functional APIs to control the backend cryptographic protocol.

"""
import json

from latticex.rosetta.controller.common_util import *
from latticex.rosetta.controller import backend_handler
from latticex.rosetta.controller.backend_handler import py_protocol_handler
from latticex.rosetta.controller.io_api import _check_io


def get_supported_protocols():
    """ Get list of all the backend cryptographic protocols.

    You can activate one of the protocols as your backend Ops.

    If you are a protocol developer, you can implement and register
    your own protocols in the backend, and you will see and use them
    here just as the native ones.

    Returns:
        The list of the names of the supported secure protocols.
    """
    return py_protocol_handler.get_supported_protocols()


def get_default_protocol_name():
    """ Get the name of the default protocol that will be used if none is set."""
    return py_protocol_handler.get_default_protocol_name()


def activate(protocol_name=None, protocol_config_str=None, task_id=None):
    """ Activate the specific protocol to carry out your subsequent Tensorflow 
    Operations.

    It is highly recommended that you use this interface explicitly to choose your
    backend protocol before calling `run` in the Tensorflow session.
    Besides, do NOT call this while the graph is running.

    Args:
        protocol_name: name of the protocol, which MUST be one of the supported
        protocols. If this parameter is not provided, the default protocol will 
        be used.

        protocol_config_str: the config JSON string that is compatible with your
        protocol.[deprecated]
    """
    # if it is already been activated, we should deactivate it first, this action
    # will be carried by PM internally.
    # step 1: fill all as default if parameter is none
    _check_io(task_id)
    if protocol_name is None:
        protocol_name = get_default_protocol_name()

    # step 2: check parameter 'protocol_name'
    # TODO: check parameter 'protocol_config_str'
    legal_set = get_supported_protocols()
    if protocol_name not in legal_set:
        rtt_get_logger().error("Protocol, `" + protocol_name + "`, is not supported!")
        raise ValueError("The protocol you chose: '" + \
            protocol_name + "', is not supported yet!")

    # step3: call backend entry, and then check result.
    if task_id == None:
        task_id = ""
    res = py_protocol_handler.activate(protocol_name, task_id)
    if res == 0:
        rtt_get_logger().info("Your protocol, `" + protocol_name +
                              "`, has been activated successfully!")
    else:
        rtt_get_logger().error("Your protocol, `" + protocol_name +
                               "`, failed to be activated!")

def get_protocol_name(task_id=None):
    """Get the name of the currently activated protocol."""
    if task_id == None:
        task_id = ""
    return py_protocol_handler.get_protocol_name(task_id)

def deactivate(task_id=None):
    """ Deactivate your current backend protocol.

    All the resources related, such as the network connections and local cache, will be released.

    please DO NOT call this while your TF graph is running.
    """
    if task_id == None:
        task_id = ""
    prtc = get_protocol_name(task_id)
    res = py_protocol_handler.deactivate(task_id)
    if res == 0:
        rtt_get_logger().info("The current protocol, `" + prtc +
                              "`, has been deactivated successfully!")
    else:
        rtt_get_logger().error("Your protocol, `" + prtc +
                               "`, failed to be deactivated! You may try again later.")

def mapping_id(unique_id, task_id=None):
    """ Create session unique id to task id.
    """
    if task_id == None:
        task_id = ""
    py_protocol_handler.mapping_id(unique_id=unique_id, task_id=task_id)

def unmapping_id(unique_id):
    """ Remove session unique id to task id.
    """
    py_protocol_handler.unmapping_id(unique_id=unique_id)

def query_mapping_id(unique_id):
    """ Query task id with session unique id.
    """
    return py_protocol_handler.query_mapping_id(unique_id)
    
def default_run(task_id=None):
    """Used by the SPASS or other internal modules to adapt to ProtocolManager[PM]

        Note that this should NOT be used by user directly.
    """
    if task_id == None:
        task_id = ""
    # step 1: check whether the backend has been inited
    curr_prtc_name = get_protocol_name(task_id)
    if (curr_prtc_name is None) or (curr_prtc_name == ""):
        # step 2: use the default protocol to run.
        activate(task_id=task_id)
    else:
        # arriving here means the PM has already been activated, so we do nothing.
        pass


def get_party_id(task_id=None):
    """Get your party id."""
    if task_id == None:
        task_id = ""
    if not py_protocol_handler.is_activated(task_id):
        rtt_get_logger().error("This API can only be called after some protocol is activated!")
        return -1
    return py_protocol_handler.get_party_id(task_id)

def backend_log_to_stdout(flag: bool):
    """ Set C++ log output to stdout or not"""
    py_protocol_handler.log_to_stdout(flag)

def set_backend_logfile(logfile: str, task_id=None):
    """ Set C++ log output filepath"""
    if task_id == None:
        task_id = ""
    py_protocol_handler.set_logfile(logfile, task_id)

def set_backend_logpattern(pattern: str):
    """ Set C++ log output filepath"""
    py_protocol_handler.set_logpattern(pattern)

def set_backend_loglevel(loglevel: int):
    """ Set C++ log output level.

    Args:
        loglevel should be as follows ('info' by default),
        0: all;
        1: Trace;
        2: Debug;
        3: Info;
        4: Warn; 
        5: Error;
        6: Fatal
    """
    py_protocol_handler.set_loglevel(loglevel)

def set_backend_logpattern(pattern: str):
    """ Set C++ log output filepath"""
    py_protocol_handler.set_logpattern(pattern)

def set_float_precision(float_precision: int, task_id=None):
    """ Set floating point precision.

    Args:
        float_precision:  number of floating point precision bits in fixed point representation, with default value is 13.
        task_id: task ID for the specified protocol. 
    """
    if task_id == None:
        task_id = ""
    py_protocol_handler.set_float_precision(float_precision, task_id)

def set_saver_model(model_nodes: list, task_id=None):
    """ Specify which nodes act as model savers.

    Args:
        model_nodes:  model saver nodes.
        task_id: task ID for the specified protocol.    
    """
    if task_id == None:
        task_id = ""
    py_protocol_handler.set_saver_model(model_nodes, task_id)

def set_restore_model(model_nodes: list, task_id=None):
    """ Set nodes to restore model.

    Args:
        model_nodes:  nodes to restore model.
        task_id: task ID for the specified protocol.    
    """
    if task_id == None:
        task_id = ""
    py_protocol_handler.set_float_precision(model_nodes, task_id)

def get_float_precision(task_id=None):
    """ Get floating point precision. 
    Args:
        task_id: task ID for the specified protocol.    
    """
    if task_id == None:
        task_id = ""
    py_protocol_handler.set_float_precision(float_precision, task_id)

def get_saver_model(task_id=None):
    """ Get nodes that act as model savers.
    Args:
        task_id: task ID for the specified protocol.    
    """
    if task_id == None:
        task_id = ""
    py_protocol_handler.set_saver_model(model_nodes, task_id)

def get_restore_model(model_nodes: list, task_id=None):
    """ Get nodes to restore model.
    Args:
        task_id: task ID for the specified protocol.    
    """
    if task_id == None:
        task_id = ""
    py_protocol_handler.set_float_precision(model_nodes, task_id)

def start_perf_stats(task_id=None):
    if task_id == None:
        task_id = ""
    py_protocol_handler.start_perf_stats(task_id)


def get_perf_stats(pretty: bool = False, task_id=None):
    """
    return the communications and elapsed between .activate()/.start_perf_stats() and this call.
    Return json string, eg:
    {
      "name": "default",
      "elapsed(s)": {
        "clock": 0.0,
        "cpu": 0.0,
        ""elapse": 1.785008878
      },
      "communication(B)": {
        "bytes-sent": 968974,
        "bytes-recv": 245330,
        "msg-sent": 595,
        "msg-recv": 432
      }
    }
    """
    if task_id == None:
        task_id = ""
    return py_protocol_handler.get_perf_stats(pretty, task_id)
