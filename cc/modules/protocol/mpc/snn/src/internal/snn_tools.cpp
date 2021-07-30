// ==============================================================================
// Copyright 2021 The LatticeX Foundation
// This file is part of the Rosetta library.
//
// The Rosetta library is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// The Rosetta library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with the Rosetta library. If not, see <http://www.gnu.org/licenses/>.
// ==============================================================================
#include"cc/modules/protocol/mpc/snn/include/snn_tools.h"
// addVectors
template void addVectors<mpc_t>(const std::vector<mpc_t>&, const std::vector<mpc_t>&, std::vector<mpc_t>&, size_t);
template void addVectors<small_mpc_t>(const std::vector<small_mpc_t>&, const std::vector<small_mpc_t>&, std::vector<small_mpc_t>&, size_t);
// subtractVectors
template void subtractVectors<mpc_t>(const std::vector<mpc_t>&, const std::vector<mpc_t>&, std::vector<mpc_t>&, size_t);
template void subtractVectors<small_mpc_t>(const std::vector<small_mpc_t>&, const std::vector<small_mpc_t>&, std::vector<small_mpc_t>&, size_t);
// copyVectors
template void copyVectors<mpc_t>(const std::vector<mpc_t>&, std::vector<mpc_t>&, size_t);
template void copyVectors<small_mpc_t>(const std::vector<small_mpc_t>&, std::vector<small_mpc_t>&, size_t);
// addModuloOdd
template void addModuloOdd<mpc_t, mpc_t>(const std::vector<mpc_t>&, const std::vector<mpc_t>&, std::vector<mpc_t>&, size_t);
template void addModuloOdd<mpc_t, small_mpc_t>(const std::vector<mpc_t>&, const std::vector<small_mpc_t>&, std::vector<mpc_t>&, size_t);
template void addModuloOdd<small_mpc_t, small_mpc_t>(const std::vector<small_mpc_t>&, const std::vector<small_mpc_t>&, std::vector<mpc_t>&, size_t);
template void addModuloOdd<small_mpc_t, mpc_t>(const std::vector<small_mpc_t>&, const std::vector<mpc_t>&, std::vector<mpc_t>&, size_t);

template mpc_t addModuloOdd<mpc_t, mpc_t>(mpc_t, mpc_t);
template mpc_t addModuloOdd<mpc_t, small_mpc_t>(mpc_t, small_mpc_t);
template mpc_t addModuloOdd<small_mpc_t, small_mpc_t>(small_mpc_t, small_mpc_t);
template mpc_t addModuloOdd<small_mpc_t, mpc_t>(small_mpc_t, mpc_t);

template void subtractModuloOdd<mpc_t, mpc_t>(const std::vector<mpc_t>&, const std::vector<mpc_t>&, std::vector<mpc_t>&, size_t);
template void subtractModuloOdd<mpc_t, small_mpc_t>(const std::vector<mpc_t>&, const std::vector<small_mpc_t>&, std::vector<mpc_t>&, size_t);
template void subtractModuloOdd<small_mpc_t, small_mpc_t>(const std::vector<small_mpc_t>&, const std::vector<small_mpc_t>&, std::vector<mpc_t>&, size_t);
template void subtractModuloOdd<small_mpc_t, mpc_t>(const std::vector<small_mpc_t>&, const std::vector<mpc_t>&, std::vector<mpc_t>&, size_t);

template mpc_t subtractModuloOdd<mpc_t, mpc_t>(mpc_t, mpc_t);
template mpc_t subtractModuloOdd<mpc_t, small_mpc_t>(mpc_t, small_mpc_t);
template mpc_t subtractModuloOdd<small_mpc_t, small_mpc_t>(small_mpc_t, small_mpc_t);
template mpc_t subtractModuloOdd<small_mpc_t, mpc_t>(small_mpc_t, mpc_t);
