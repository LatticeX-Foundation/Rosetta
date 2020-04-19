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
# from tensorflow.python.ops import variables
# from tensorflow.python.ops import resource_variable_ops
# from tensorflow.python.ops import array_ops
# from tensorflow.python.ops import random_ops
# from tensorflow.python.ops import gradients_util


# # override RefVariable & ResourceVariable class for MPC
# variables.RefVariable = MPCRefVariable
# resource_variable_ops.ResourceVariable = MPCResourceVariable


# # override placeholder functions for MPC
# tf.placeholder = MpcPlaceholder


# # override random_normal functions for MPC
# tf.random_normal = MpcRandom_normal

# # override random_uniform functions for MPC
# tf.random_uniform = MpcRandom_uniform

# # override truncated_normal functions for MPC
# tf.truncated_normal = MpcTruncated_normal

# # override random_gamma functions for MPC
# tf.random_gamma = MpcRandom_gamma

# # override random_poisson functions for MPC
# tf.random_poisson = MpcRandom_poisson


# # override zeros functions for MPC
# tf.zeros = MpcZeros

# # override ones functions for MPC
# tf.ones = MpcOnes

# # override zeros_like functions for MPC
# tf.zeros_like = MpcZeros_like

# # override ones_like functions for MPC
# tf.ones_like = MpcOnes_like


# override gradients_util._DefaultGradYs functions for MPC
# gradients_util._DefaultGradYs = MpcDefaultGradYs
