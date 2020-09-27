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

# temporary way in V0.2.0.
_curr_config_json_str = ""


def get_supported_protocols():
    """ Get list of all the backend cryptographic protocols.

    You can activte one of the protocols as your backend Ops.

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


def activate(protocol_name=None, protocol_config_str=None):
    """ Activate the specific protocol to carry out your subsequent Tensorflow 
    Operations.

    It is highly recomended that you use this interface explicitly to choose your
    backend protocol before calling `run` in the Tensorflow session.
    Besides, do NOT call this while the graph is running.

    Args:
        protocol_name: name of the protocol, which MUST be one of the supported
        protocols. If this parameter is not provided, the default protocol will 
        be used.

        protocol_config_str: the config JSON string that is compatible with your
        protocol.
    """
    # if it is alrady been activated, we should deactivate it first, this action
    # will be carried by PM internally.
    # step 1: fill all as default if parameter is none
    global _curr_config_json_str
    if protocol_name is None:
        protocol_name = get_default_protocol_name()
    if protocol_config_str is None:
        config_json_str = None
        try:
            with open(backend_handler._cfgfile, 'r') as load_f:
                load_dict = json.load(load_f)
                rtt_get_logger().debug("original config file: %s" % str(load_dict))
                party_id = load_dict["PARTY_ID"]
                # cmdline 'party_id' option can override PARTY_ID in config file
                if backend_handler._party_id != -1:
                    party_id = backend_handler._party_id
                load_dict["PARTY_ID"] = party_id
                config_json_str = json.dumps(load_dict, indent=4)
                protocol_config_str = config_json_str
        except Exception as e:
            rtt_get_logger().error("Fail to load or parse your config:[" + str(e) +
                                   "]. Please make sure your provide the correct Config string parameter " +
                                   "or CONFIG.json file in local directory.")
            return
    # step 2: check parameter 'protocol_name'
    # TODO: check parameter 'protocol_config_str'
    legal_set = get_supported_protocols()
    if protocol_name not in legal_set:
        rtt_get_logger().error("Protocol, `" + protocol_name + "`, is not supported!")
        return
        # raise ValueError("Error! The protocol you chose,'" + \
        #     protocol_name + "', is not supported yet!")

    # step3: call backend entry, and then check result.
    res = py_protocol_handler.activate(protocol_name, protocol_config_str)
    if res == 0:
        rtt_get_logger().info("Your protocol, `" + protocol_name +
                              "`, has been activated successfully!")
        _curr_config_json_str = protocol_config_str
    else:
        rtt_get_logger().error("Your protocol, `" + protocol_name +
                               "`, failed to be activated! Please check your config!")


def get_protocol_name():
    """Get the protocol name currently are activated."""
    return py_protocol_handler.get_protocol_name()


def deactivate():
    """ Deactivate your current backend protocol.

    All the resources related, such as the network connections and local cache, will be released.

    please DO NOT call this while your TF graph is runnning.
    """
    prtc = get_protocol_name()
    res = py_protocol_handler.deactivate()
    if res == 0:
        rtt_get_logger().info("The current protocol, `" + prtc +
                              "`, has been deactivated successfully!")
    else:
        rtt_get_logger().error("Your protocol, `" + prtc +
                               "`, failed to be deactivated! You may try again later.")


def default_run():
    """Used by the SPASS or other internal modules to adapt to ProtocolManager[PM]

        Note that this should NOT be used by user directly.
    """
    # step 1: checkout whether the backend has been inited
    curr_prtc_name = get_protocol_name()
    if (curr_prtc_name is None) or (curr_prtc_name == ""):
        # step 2: use the default protocol to run.
        activate()
    else:
        # arriving here means the PM has already been activated, so we do nothing.
        pass


def get_protocol_config(keyword=""):
    """Get all the config info you are using now.

    Note that the parameter is not supported yet.
    """
    global _curr_config_json_str
    if not py_protocol_handler.is_activated():
        rtt_get_logger().error("This API can only be called after some protocol is activated!")
        return -1
    return _curr_config_json_str


# def set_protocol_config():
#     "TODO"
#     pass


# def set_protocol_config_item(keyword="", value=""):
#     "TODO"
#     pass


def get_party_id():
    """Get your party id."""
    if not py_protocol_handler.is_activated():
        rtt_get_logger().error("This API can only be called after some protocol is activated!")
        return -1
    return py_protocol_handler.get_party_id()


def backend_log_to_stdout(flag: bool):
    """ Set C++ log output to stdout or not"""
    py_protocol_handler.log_to_stdout(flag)


def set_backend_logfile(logfile: str):
    """ Set C++ log output filepath"""
    py_protocol_handler.set_logfile(logfile)


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


def start_perf_stats():
    py_protocol_handler.start_perf_stats()


def get_perf_stats(pretty: bool = False):
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
    return py_protocol_handler.get_perf_stats(pretty)
