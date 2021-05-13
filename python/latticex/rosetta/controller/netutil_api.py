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
from typing import List
from latticex.rosetta.controller.controller_base_ import _rtt
vecstrs = List[str]


class Netutil(object):
    @staticmethod
    def enable_ssl_socket(v: bool):
        _rtt.netutil.enable_ssl_socket(v)

    @staticmethod
    def is_enable_ssl_socket(v: bool):
        _rtt.netutil.is_enable_ssl_socket(v)
