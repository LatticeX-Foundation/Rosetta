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
#include "cc/modules/common/include/utils/random_util.h"

#include <cassert>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>
#include <cstdint>
#include <algorithm>
using namespace std;

vector<double> generate_random(int n, int seed) {
  static default_random_engine e;
  if (seed > 0) {
    e.seed(seed);
  } else {
    e.seed(time(0));
  }
  static normal_distribution<> dist;

  vector<double> v;
  for (int i = 0; i < n; i++) {
    v.push_back(dist(e));
  }
  return v;
}

// generate [0, max_size), return the first 'front_size' elements
void gen_random_index0(vector<int>& indexs_r, int max_size, int front_size) {
  if (front_size < 0)
    front_size = max_size;

  indexs_r.clear();
  indexs_r.resize(front_size);
  for (int i = 0; i < max_size; i++)
    indexs_r[i] = i;

  std::random_shuffle(indexs_r.begin(), indexs_r.end());
}

void gen_random_index(vector<int>& indexs_r, int max_size, int front_size) {
  if (front_size < 0)
    front_size = max_size;

  vector<int> indexs(max_size);
  for (int i = 0; i < max_size; i++)
    indexs[i] = i;

  std::random_device rd;
  std::mt19937 mt(rd());
  indexs_r.clear();
  indexs_r.resize(front_size);
  for (int i = 0; i < front_size; i++) {
    int j = mt() % indexs.size();
    indexs_r[i] = indexs[j];
    indexs[j] = indexs[indexs.size() - 1];
    indexs.pop_back();
  }
}

void gen_random_index2(vector<int>& indexs_r, int max_size, int front_size, uint32_t seed) {
  if (front_size < 0)
    front_size = max_size;

  vector<int> indexs(max_size);
  for (int i = 0; i < max_size; i++)
    indexs[i] = i;

  if (seed != 0)
    srand(seed);

  indexs_r.clear();
  indexs_r.resize(front_size);
  for (int i = 0; i < front_size; i++) {
    int j = rand() * (seed + 1) % indexs.size();
    indexs_r[i] = indexs[j];
    indexs[j] = indexs[indexs.size() - 1];
    indexs.pop_back();
  }
}

void xavier_uniform2(vector<double>& v, size_t fan_in, size_t fan_out, int64_t seed) {
  double d = sqrt(6.0 / (fan_in + fan_out));
  std::uniform_real_distribution<> dist{-d, d};
  std::random_device rd;
  std::default_random_engine e{rd()};
  if (seed != -1)
    e.seed((uint32_t)seed);
  for (int i = 0; i < v.size(); i++) {
    v[i] = dist(e);
  }
}

void uniform2(vector<double>& v, double low, double up, int64_t seed) {
  std::uniform_real_distribution<> dist{low, up};
  std::random_device rd;
  std::default_random_engine e{rd()};
  if (seed != -1)
    e.seed((uint32_t)seed);
  for (int i = 0; i < v.size(); i++) {
    v[i] = dist(e);
  }
}

void uniform2(vector<vector<uint64_t>>& m, int64_t low, int64_t up, int64_t seed) {
  std::uniform_int_distribution<int64_t> dist{low, up};
  std::random_device rd;
  std::default_random_engine e{rd()};
  if (seed != -1)
    e.seed((uint32_t)seed);
  for (int r = 0; r < m.size(); r++) {
    for (int c = 0; c < m[r].size(); c++) {
      m[r][c] = dist(e);
    }
  }
}

void uniform2(vector<vector<int64_t>>& m, int64_t low, int64_t up, int64_t seed) {
  std::uniform_int_distribution<int64_t> dist{low, up};
  std::random_device rd;
  std::default_random_engine e{rd()};
  if (seed != -1)
    e.seed((uint32_t)seed);
  for (int r = 0; r < m.size(); r++) {
    for (int c = 0; c < m[r].size(); c++) {
      m[r][c] = dist(e);
    }
  }
}

void uniform2(vector<uint64_t>& m, int64_t low, int64_t up, int64_t seed) {
  std::uniform_int_distribution<int64_t> dist{low, up};
  std::random_device rd;
  std::default_random_engine e{rd()};
  if (seed != -1)
    e.seed((uint32_t)seed);
  for (int r = 0; r < m.size(); r++) {
    m[r] = dist(e);
  }
}

void uniform2(vector<int64_t>& m, int64_t low, int64_t up, int64_t seed) {
  std::uniform_int_distribution<int64_t> dist{low, up};
  std::random_device rd;
  std::default_random_engine e{rd()};
  if (seed != -1)
    e.seed((uint32_t)seed);
  for (int r = 0; r < m.size(); r++) {
    m[r] = dist(e);
  }
}

////////////////////////////////////////////////
void rand_vec(vector<int64_t>& vec, int vec_size, int bit_size) {
  vec.clear();
  vec.resize(vec_size);

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<uint> dis(0, (1L << (bit_size - 1)) - 1);

  for (int i = 0; i < vec_size; i++) {
    vec[i] = dis(gen) - (1L << (bit_size - 2));
  }
}

template <typename T>
inline void rand_vec2(vector<T>& vec, int length) {
  vec.clear();
  vec.resize(length);

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<T> dis(
    std::numeric_limits<T>::min(), std::numeric_limits<T>::max());

  for (int i = 0; i < length; i++) {
    vec[i] = dis(gen);
  }
}

template <typename T>
inline void rand_vec3(vector<T>& vec, int length) {
  vec.clear();
  vec.resize(length);

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<T> dis(
    std::numeric_limits<T>::min(), std::numeric_limits<T>::max());

  for (int i = 0; i < length; i++) {
    vec[i] = dis(gen);
  }
}

void rand_vec_30bit(vector<int64_t>& rand_vec, int length) {
  rand_vec.clear();
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<uint> dis(0, (1 << 29) - 1);
  //    cout<<dis.max()<<endl;
  //    cout<<dis.min()<<endl;
  int tmp;
  for (int i = 0; i < length; i++) {
    tmp = dis(gen) - (1 << 28);
    rand_vec.push_back(tmp);
  }
}

void rand_vec_60bit(vector<int64_t>& rand_vec, int length) {
  rand_vec.clear();
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<uint64_t> dis(0, (1ULL << 59) - 1);

  int64_t tmp;
  for (int i = 0; i < length; i++) {
    tmp = dis(gen) - (1ULL << 58);
    rand_vec.push_back(tmp);
  }
}

void random_vector(vector<double>& v, size_t size, double low, double high, int64_t seed/*=-1*/) {
  v.resize(size, 0);
  uniform2(v, low, high, seed);
}

void random_vector(vector<uint64_t>& v, size_t size) { rand_vec2(v, size); }
void random_vector(vector<uint8_t>& v, size_t size) {
  rand_vec2(v, size);
  for (int i = 0; i < size; i++) {
    v[i] = v[i] & 1;
  }
}
////////////////////////////////////////////////
