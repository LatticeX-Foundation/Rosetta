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

import sys
import os
import tensorflow as tf
import getopt

# set output buffer to zero
class ZeroBufferOut(object):
    def __init__(self, stream):
       self.stream = stream
    def write(self, data):
        self.stream.write(data)
        self.stream.flush()
    def writelines(self, datas):
        self.stream.writelines(datas)
        self.stream.flush()
    def __getattr__(self, attr):
        return getattr(self.stream, attr)
sys.stdout = ZeroBufferOut(sys.stdout)


tf_mpcop_lib = os.path.dirname(__file__) + '/../libtf-mpcop.so'
tf_dpass_lib = os.path.dirname(__file__) + '/../libtf-dpass.so'


def debug_print(info):
    # if os.environ.has_key('ROSETTA_DEBUG') and os.environ['ROSETTA_DEBUG'] == 'ON':
    #     print(info)
    print(info)


dpass = None
if 'ROSETTA_DPASS' in os.environ and os.environ['ROSETTA_DPASS'] == 'OFF':
    debug_print('NOT load library: {}, disable dynamic pass.'.format(tf_dpass_lib))
else:
    dpass = tf.load_op_library(tf_dpass_lib)
    debug_print('load library: {}'.format(tf_dpass_lib))

__doc__ = """
    This is LatticeX Rosetta.
"""

# MPC
from latticex.rosetta.mpc import *
# ZK for the future
# HE for the future

# A simple way to cope commandline arguments
# ########################### commandline arguments
import argparse
parser = argparse.ArgumentParser(description="LatticeX Rosetta")
parser.add_argument('--party_id', type=int, help="Party ID",
                    required=True, choices=[0, 1, 2])
parser.add_argument('--cfgfile', type=str, help="Config File",
                    default=os.path.abspath('.') + "/CONFIG.json")
args, unparsed = parser.parse_known_args()
# ###########################
party_id = args.party_id
cfgfile = args.cfgfile
print('party id: {} with config json file: {}'.format(party_id, cfgfile))

# The users must provide a 'CONFIG.json' in the run-directory
# We will read the config json file, named CONFIG.json, in the current directory
mpc_player.init_mpc(party_id, cfgfile)
