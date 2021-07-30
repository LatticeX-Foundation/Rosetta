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

#include <cassert>
#include <vector>
#include <string>

#include "cc/modules/common/include/utils/helper.h"
#include "cc/modules/common/include/utils/model_tool.h"
#include "cc/modules/common/include/utils/simple_timer.h"
#include "cc/modules/protocol/utility/include/util.h"
#include "cc/modules/protocol/mpc/helix/include/helix_def.h"
#include "cc/modules/protocol/mpc/comm/include/mpc_common.h"
#include "cc/modules/protocol/mpc/comm/include/mpc_helper.h"
using namespace rosetta;

namespace rosetta {
namespace helix {

//////////////////////////////////////////////////
//////////////////////////////////////////////////
//! @todo this block will remove to common defines header
// some macros, const
const int PARTY_0 = 0;
const int PARTY_1 = 1;
const int PARTY_2 = 2;

#define convert_plain_to_fixpoint convert_double_to_mpctype
#define convert_fixpoint_to_plain convert_mpctype_to_double
//////////////////////////////////////////////////
//////////////////////////////////////////////////

/**
 * type conversion series
 */
/**
 * @note[GeorgeShi]: this is for converting uniform interface input string to internal
 *                Helix-specific data type 'Share'
 * @param: 
 *        @a, input string
 *        @b, decoded according to a's format
 *        @human, whether 'a' is 'readable', currently this is always true.
 *        @phi, pointer to outer HelixInternal class, to call its private_input if needed.
 */
void convert_string_to_share(
  const std::vector<std::string>& a,
  std::vector<Share>& b,
  bool human = false,
  void* phi = nullptr);

void convert_share_to_string(
  const std::vector<Share>& a,
  std::vector<std::string>& b,
  bool human = false);

void helix_plain_string_to_double(const std::vector<std::string>& a, std::vector<double>& b);
void helix_double_to_plain_string(const std::vector<double>& a, std::vector<std::string>& b);

#define helix_convert_string_to_share(a, b) \
  rosetta::helix::convert_string_to_share(a, b, false, (void*)hi.get())
#define helix_convert_share_to_string(a, b) rosetta::helix::convert_share_to_string(a, b, false)

/**
 * for convenience
 * operator overloading
 */
template <typename T>
vector<T> operator+(const vector<T>& v1, const vector<T>& v2) {
  assert(v1.size() == v2.size());
  vector<T> v(v1.size());
  for (int i = 0; i < v.size(); i++) {
    v[i] = v1[i] + v2[i];
  }
  return v;
}
extern template std::vector<mpc_t> operator+(const std::vector<mpc_t>&, const std::vector<mpc_t>&);
extern template std::vector<double> operator+(const std::vector<double>&, const std::vector<double>&);

template <>
inline vector<bit_t> operator+(const vector<bit_t>& v1, const vector<bit_t>& v2) {
  assert(v1.size() == v2.size());
  vector<bit_t> v(v1.size());
  for (int i = 0; i < v.size(); i++) {
    v[i] = v1[i] ^ v2[i];
  }
  return v;
}

template <typename T>
vector<T> operator-(const vector<T>& v1, const vector<T>& v2) {
  assert(v1.size() == v2.size());
  vector<T> v(v1.size());
  for (int i = 0; i < v.size(); i++) {
    v[i] = v1[i] - v2[i];
  }
  return v;
}
extern template std::vector<mpc_t> operator-(const std::vector<mpc_t>&, const std::vector<mpc_t>&);
extern template std::vector<double> operator-(const std::vector<double>&, const std::vector<double>&);

template <>
inline vector<bit_t> operator-(const vector<bit_t>& v1, const vector<bit_t>& v2) {
  assert(v1.size() == v2.size());
  vector<bit_t> v(v1.size());
  for (int i = 0; i < v.size(); i++) {
    v[i] = v1[i] ^ v2[i];
  }
  return v;
}

template <typename T>
vector<T> operator/(const vector<T>& v1, const int N) {
  vector<T> v(v1.size());
  for (int i = 0; i < v.size(); i++) {
    v[i] = v1[i] / N;
  }
  return v;
}
extern template std::vector<mpc_t> operator/(const std::vector<mpc_t>&, const int);
extern template std::vector<double> operator/(const std::vector<double>&, const int);
extern template std::vector<bit_t> operator/(const std::vector<bit_t>&, const int);

vector<mpc_t> operator-(const vector<bit_t>& v1, const vector<mpc_t>& v2);
BitShare operator^(const BitShare& v1, const BitShare& v2);
vector<BitShare> operator^(const vector<BitShare>& v1, const vector<BitShare>& v2);

/**
 * 
 * @todo the following helper functions will remove to public headers soon
 * 
 */

/**
 * simulation, helper, ..., functions
 */
int adversary(int p);

/**
 * get vector<vector<>> rows and cols
 * return rows*cols
 */
template <typename T>
inline size_t get_m_n(const vector<vector<T>>& v, int& m, int& n) {
  m = v.size();
  n = (m > 0) ? v[0].size() : 0;
  return m * n;
}

/**
 * row first
 * (m, n) ---> vector with size m*n
 */
template <typename T>
void flatten(const vector<vector<T>>& vv, vector<T>& v) {
  int m, n;
  size_t size = get_m_n(vv, m, n);
  v.resize(size);
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
      v[i * n + j] = vv[i][j];
    }
  }
}
extern template void flatten<double>(const std::vector<std::vector<double>>&, std::vector<double>&);

template <typename T>
void flatten_r(vector<vector<T>>& vv, const vector<T>& v, int m, int n) {
  if (m <= 0)
    return;
  if (n <= 0)
    return;

  vv.resize(m, vector<T>(n));
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
      vv[i][j] = v[i * n + j];
    }
  }
}
extern template void flatten_r<Share>(std::vector<std::vector<Share>>&, const std::vector<Share>&, int, int);

template <typename T>
inline void print_shape_(vector<vector<T>>& v, string name = "") {
  int m, n;
  size_t size = get_m_n(v, m, n);
  cout << name << " size: " << size << " shape: (" << m << "," << n << ")" << endl;
}
#define print_shape(v) print_shape_(v, #v)

} // namespace helix
} // namespace rosetta
