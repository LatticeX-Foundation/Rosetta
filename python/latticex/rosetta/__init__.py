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

from latticex.rosetta.controller.common_util import rtt_get_logger

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


if 'ROSETTA_MPC_128' in os.environ and os.environ['ROSETTA_MPC_128'] == 'ON':
    tf_dpass_lib = os.path.dirname(__file__) + '/../lib128/libtf-dpass.so'
else:
    tf_dpass_lib = os.path.dirname(__file__) + '/../libtf-dpass.so'
tf_dpass_lib = os.path.dirname(__file__) + '/../libtf-dpass.so'

dpass = None
if 'ROSETTA_DPASS' in os.environ and os.environ['ROSETTA_DPASS'] == 'OFF':
    rtt_get_logger().debug('NOT load library: {}, disable dynamic pass.'.format(tf_dpass_lib))
else:
    dpass = tf.load_op_library(tf_dpass_lib)
    rtt_get_logger().debug('load library: {}'.format(tf_dpass_lib))

__doc__ = """
    This is LatticeX Rosetta.
"""

# RTT
from latticex.rosetta.rtt import *
# MPC
from latticex.rosetta.secure import *
# ZK for the future
# HE for the future

from latticex.rosetta.controller import *

from latticex.rosetta.controller import backend_handler

