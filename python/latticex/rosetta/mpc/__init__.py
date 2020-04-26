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
# ops
from latticex.rosetta.mpc.ops.array_ops import *
from latticex.rosetta.mpc.ops.random_ops import *
from latticex.rosetta.mpc.ops.resource_variable_ops import *
from latticex.rosetta.mpc.ops.variables import *
from latticex.rosetta.mpc.ops.gradients_util import *
from latticex.rosetta.mpc.ops.math_ops import *
from latticex.rosetta.mpc.ops.nn_util import *

# decorator of mpc operators
from latticex.rosetta.mpc.decorator import *
# nn_utils depends on decorator
from latticex.rosetta.mpc.ops.nn_util import *
# for static replacement of Saver
from latticex.rosetta.mpc.ops.training.io_saver import *
# for static replacement of apply_gradient_descent
from latticex.rosetta.mpc.ops.training.mpc_apply_gradient_descent import *
# utils
from latticex.rosetta.mpc.utils.common import *
from latticex.rosetta.mpc.utils.dataset import *
from latticex.rosetta.mpc.utils.input import *

# client
from latticex.rosetta.mpc.client.mpcsession import *
from latticex.rosetta.mpc.client.mpcplayer import *

# framework
from latticex.rosetta.mpc.framework.constant_op import *
from latticex.rosetta.mpc.framework.random_seed import *

# gradient
from latticex.rosetta.mpc.grads_ops.mpcadd_grad import *
from latticex.rosetta.mpc.grads_ops.mpcsub_grad import *
from latticex.rosetta.mpc.grads_ops.mpcdiv_grad import *
from latticex.rosetta.mpc.grads_ops.mpclog_grad import *
from latticex.rosetta.mpc.grads_ops.mpclog1p_grad import *
from latticex.rosetta.mpc.grads_ops.mpcmatmul_grad import *
from latticex.rosetta.mpc.grads_ops.mpcmul_grad import *
from latticex.rosetta.mpc.grads_ops.mpcmaxmin_grad import *
from latticex.rosetta.mpc.grads_ops.mpcmean_grad import *
from latticex.rosetta.mpc.grads_ops.mpcpow_grad import *
from latticex.rosetta.mpc.grads_ops.mpcsigmoid_grad import *
from latticex.rosetta.mpc.grads_ops.mpc_nn_grad import *

# static pass, replace 
from latticex.rosetta.mpc.spass.CopyAndRepMpcOpPass import *
from latticex.rosetta.mpc.spass.MpcAdadeltaOptimizer import *
from latticex.rosetta.mpc.spass.MpcAdagradDAOptimizer import *
from latticex.rosetta.mpc.spass.MpcAdagradOptimizer import *
from latticex.rosetta.mpc.spass.MpcAdamOptimizer import *
from latticex.rosetta.mpc.spass.MpcFtrlOptimizer import *
from latticex.rosetta.mpc.spass.MpcGradientDescentOptimizer import *
from latticex.rosetta.mpc.spass.MpcMomentumOptimizer import *
from latticex.rosetta.mpc.spass.MpcProximalAdagradOptimizer import *
from latticex.rosetta.mpc.spass.MpcProximalGradientDescentOptimizer import *
from latticex.rosetta.mpc.spass.MpcRMSPropOptimizer import *


