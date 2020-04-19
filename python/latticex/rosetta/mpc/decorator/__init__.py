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
# import mpc ops
from latticex.rosetta.mpc.decorator.mpc_reduce_ops_ import *
from latticex.rosetta.mpc.decorator.mpc_binary_ops_ import *
from latticex.rosetta.mpc.decorator.mpc_ops_ import *

# All MPC OPs are list here just for convenience
# For example, you can simply use MpcAdd instead of mpcop.mpc_add
# More useage details see python/MpcOps/test_cases


# otheres
from latticex.rosetta.mpc.decorator.mpc_others_ops_ import *

#MpcConv1d = mpcop.mpc_conv1d
#MpcConv2d = mpcop.mpc_conv2d
#MpcConv3d = mpcop.mpc_conv3d