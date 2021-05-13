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
from tensorflow.python.framework import ops


@ops.RegisterGradient("SecureEqual")
def SecureEqualGrad(_, unused_grad):
  """The gradient for the SecureEqual operator."""
  return None, None


@ops.RegisterGradient("SecureNotEqual")
def SecureNotEqualGrad(_, unused_grad):
  """The gradient for the SecureNotEqual operator."""
  return None, None


@ops.RegisterGradient("SecureGreater")
def SecureGreaterGrad(_, unused_grad):
  """The gradient for the SecureGreater operator."""
  return None, None


@ops.RegisterGradient("SecureGreaterEqual")
def SecureGreaterEqualGrad(_, unused_grad):
  """The gradient for the SecureGreaterEqual operator."""
  return None, None


@ops.RegisterGradient("SecureLess")
def SecureLessGrad(_, unused_grad):
  """The gradient for the SecureLess operator."""
  return None, None


@ops.RegisterGradient("SecureLessEqual")
def SecureLessEqualGrad(_, unused_grad):
  """The gradient for the SecureLessEqual operator."""
  return None, None

