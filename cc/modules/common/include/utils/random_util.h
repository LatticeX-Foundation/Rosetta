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
#include <vector>
#include <cstdint>
using namespace std;

////////////////////////////////////////////////
vector<double> generate_random(int n, int seed = -1);
void gen_random_index0(vector<int>& indexs_r, int max_size, int front_size = -1);
void gen_random_index(vector<int>& indexs_r, int max_size, int front_size = -1);
void gen_random_index2(vector<int>& indexs_r, int max_size, int front_size = -1, uint32_t seed = 0);

////////////////////////////////////////////////
void xavier_uniform2(vector<double>& v, size_t fan_in, size_t fan_out, int64_t seed = -1);
void uniform2(vector<double>& v, double low, double up, int64_t seed = -1);
void uniform2(vector<vector<uint64_t>>& m, int64_t low, int64_t up, int64_t seed = -1);
void uniform2(vector<vector<int64_t>>& m, int64_t low, int64_t up, int64_t seed = -1);
void uniform2(vector<uint64_t>& m, int64_t low, int64_t up, int64_t seed = -1);
void uniform2(vector<int64_t>& m, int64_t low, int64_t up, int64_t seed = -1);

////////////////////////////////////////////////
void rand_vec(vector<int64_t>& vec, int vec_size, int bit_size);
void rand_vec_30bit(vector<int64_t>& rand_vec, int length);
void rand_vec_60bit(vector<int64_t>& rand_vec, int length);
void random_vector(vector<double>& v, size_t size, double low = -2.0, double high = 2.0, int64_t seed=-1);
void random_vector(vector<uint64_t>& v, size_t size);
void random_vector(vector<uint8_t>& v, size_t size);

////////////////////////////////////////////////
