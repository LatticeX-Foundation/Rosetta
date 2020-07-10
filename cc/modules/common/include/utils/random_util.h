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

static vector<double> generate_random(int n, int seed = -1) {
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

static default_random_engine ee(time(0));
static normal_distribution<double> nnd(0, 10);
static MatrixXd get_random(int c, int r) {
  return MatrixXd::Zero(c, r).unaryExpr([](double dummy) { return nnd(ee); });
}

static MatrixXd get_random2(int c, int r) {
  return MatrixXd::Zero(c, r).unaryExpr([](double dummy) { return (ee() % 10000) / 10000.0; });
}

static MatrixXd get_random3(int c, int r) {
  return MatrixXd::Zero(c, r).unaryExpr(
    [](double dummy) { return (ee() & 0xFFFF) * (0.5 / 0xFFFF); });
}

// generate [0, max_size), return the first 'front_size' elements
static void gen_random_index0(vector<int>& indexs_r, int max_size, int front_size = -1) {
  if (front_size < 0)
    front_size = max_size;

  indexs_r.clear();
  indexs_r.resize(front_size);
  for (int i = 0; i < max_size; i++)
    indexs_r[i] = i;

  std::random_shuffle(indexs_r.begin(), indexs_r.end());
}

static void gen_random_index(vector<int>& indexs_r, int max_size, int front_size = -1) {
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

static void gen_random_index2(
  vector<int>& indexs_r,
  int max_size,
  int front_size = -1,
  uint32_t seed = 0) {
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

static void standardization(MatrixXd& X) {
  for (int j = 0; j < X.cols(); j++) {
    // MatrixXd feature = X.col(j);
  }
}

static void xavier_uniform(MATRIX& m, size_t fan_in, size_t fan_out, int64_t seed = -1) {
  throw;
  // double d = sqrt(6.0 / (fan_in + fan_out));
  // std::uniform_real_distribution<> dist{-d, d};
  // std::random_device rd;
  // std::default_random_engine e{rd()};
  // e.seed((uint32_t)seed);
  // for (int r = 0; r < m.rows(); r++) {
  //   for (int c = 0; c < m.cols(); c++) {
  //     m(r, c) = floatToDType(dist(e));
  //   }
  // }
}

static void xavier_uniform2(vector<float>& v, size_t fan_in, size_t fan_out, int64_t seed = -1) {
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

static void uniform2(vector<float>& v, float low, float up, int64_t seed = -1) {
  std::uniform_real_distribution<> dist{low, up};
  std::random_device rd;
  std::default_random_engine e{rd()};
  if (seed != -1)
    e.seed((uint32_t)seed);
  for (int i = 0; i < v.size(); i++) {
    v[i] = dist(e);
  }
}

static void uniform2(MatrixXd& m, float low, float up, int64_t seed = -1) {
  std::uniform_real_distribution<> dist{low, up};
  std::random_device rd;
  std::default_random_engine e{rd()};
  if (seed != -1)
    e.seed((uint32_t)seed);
  for (int r = 0; r < m.rows(); r++) {
    for (int c = 0; c < m.cols(); c++) {
      m(r, c) = dist(e);
    }
  }
}

static void uniform2(MatrixXui& m, int32_t low, int32_t up, int64_t seed = -1) {
  std::uniform_int_distribution<int32_t> dist{low, up};
  std::random_device rd;
  std::default_random_engine e{rd()};
  if (seed != -1)
    e.seed((uint32_t)seed);
  for (int r = 0; r < m.rows(); r++) {
    for (int c = 0; c < m.cols(); c++) {
      m(r, c) = dist(e);
    }
  }
}

static void uniform2(MatrixXul& m, int64_t low, int64_t up, int64_t seed = -1) {
  std::uniform_int_distribution<int64_t> dist{low, up};
  std::random_device rd;
  std::default_random_engine e{rd()};
  if (seed != -1)
    e.seed((uint32_t)seed);
  for (int r = 0; r < m.rows(); r++) {
    for (int c = 0; c < m.cols(); c++) {
      m(r, c) = dist(e);
    }
  }
}

static void uniform2(MatrixXl& m, int64_t low, int64_t up, int64_t seed = -1) {
  std::uniform_int_distribution<int64_t> dist{low, up};
  std::random_device rd;
  std::default_random_engine e{rd()};
  if (seed != -1)
    e.seed((uint32_t)seed);
  for (int r = 0; r < m.rows(); r++) {
    for (int c = 0; c < m.cols(); c++) {
      m(r, c) = dist(e);
    }
  }
}

static void uniform2(vector<vector<uint64_t>>& m, int64_t low, int64_t up, int64_t seed = -1) {
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
static void uniform2(vector<vector<int64_t>>& m, int64_t low, int64_t up, int64_t seed = -1) {
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
static void uniform2(vector<uint64_t>& m, int64_t low, int64_t up, int64_t seed = -1) {
  std::uniform_int_distribution<int64_t> dist{low, up};
  std::random_device rd;
  std::default_random_engine e{rd()};
  if (seed != -1)
    e.seed((uint32_t)seed);
  for (int r = 0; r < m.size(); r++) {
    m[r] = dist(e);
  }
}

static void uniform2(vector<int64_t>& m, int64_t low, int64_t up, int64_t seed = -1) {
  std::uniform_int_distribution<int64_t> dist{low, up};
  std::random_device rd;
  std::default_random_engine e{rd()};
  if (seed != -1)
    e.seed((uint32_t)seed);
  for (int r = 0; r < m.size(); r++) {
    m[r] = dist(e);
  }
}
