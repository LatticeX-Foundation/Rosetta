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
from latticex.rosetta.controller.controller_base_ import _rtt
from latticex.rosetta.controller.common_util import rtt_get_logger

# A simple way to cope commandline arguments
# ########################### commandline arguments
import argparse
import os

_parser = argparse.ArgumentParser(description="LatticeX Rosetta")
_parser.add_argument('--party_id', type=int, help="Party ID",
                     required=False, default=-1, choices=[0, 1, 2])
_parser.add_argument('--cfgfile', type=str, help="Config File",
                     default=os.path.abspath('.') + "/CONFIG.json")
_args, _unparsed = _parser.parse_known_args()
# ###########################
# the party id can also be configed in CONFIG.json file.
# If you config it by commandline, this will override the one in config file.
_party_id = _args.party_id
_cfgfile = _args.cfgfile

rtt_get_logger().info('party id: {} with config json file: {}'.format(_party_id, _cfgfile))

py_protocol_handler = _rtt.protocol.ProtocolHandler()
