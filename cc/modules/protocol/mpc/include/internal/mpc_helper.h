// ==============================================================================
// Copyright 2020 The LatticeX Foundation
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
#pragma once

#include "mpc_defines.h"
#include "ex.h"
#include "simple_timer.h"
#include "params.h"

int initialize_mpc(const Params_Fields& params);
int uninitialize_mpc();
int partyid();

void convert_mytype_to_double(const vector<mpc_t>& vec1, vector<double>& vec2);
void convert_double_to_mytype(const vector<double>& vec1, vector<mpc_t>& vec2);
void convert_mytype_to_double_bc(const vector<mpc_t>& vec1, vector<double>& vec2);
void convert_double_to_mytype_bc(const vector<double>& vec1, vector<mpc_t>& vec2);

extern void synchronize(const msg_id_t& msg_id);
extern void synchronize(int length);
extern void synchronize();

#ifndef tf_convert_double_to_mpctype
#define tf_convert_double_to_mpctype convert_double_to_mytype_bc
#define tf_convert_mpctype_to_double convert_mytype_to_double_bc
#endif
