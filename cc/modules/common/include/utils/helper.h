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
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using std::vector;

#include "cc/modules/common/include/utils/logger.h"

////////////////////////////////////////////////
static inline std::vector<std::string> split(const std::string& src, char delim) {
  std::stringstream ss(src);
  std::string item;
  std::vector<std::string> elems;
  while (std::getline(ss, item, delim)) {
    elems.push_back(std::move(item));
  }
  return elems;
}

////////////////////////////////////////////////
template <typename... T>
size_t ARG_COUNT(T... args) {
  return sizeof...(args);
}

template <typename T>
void print(T v) {
  cout << fixed;
  cout << v << endl;
}

template <typename T, typename... ARG>
void print(T v, ARG... args) {
  cout << v << " ";
  print(args...);
}

////////////////////////////////////////////////
inline void rand_vec(vector<int64_t>& vec, int vec_size, int bit_size) {
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
static inline void rand_vec2(vector<T>& vec, int length) {
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
static inline void rand_vec3(vector<T>& vec, int length) {
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

inline void rand_vec_30bit(vector<int64_t>& rand_vec, int length) {
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

inline void rand_vec_60bit(vector<int64_t>& rand_vec, int length) {
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

inline void random_vector(vector<double>& v, size_t size, double low = -2.0, double high = 2.0) {
  vector<uint64_t> vv;
  rand_vec2(vv, size);
  v.resize(size);
  for (int i = 0; i < size; i++) {
    v[i] = (((int64_t)vv[i]) % 0xff) / 128.0;
    v[i] = (high - low) * v[i] + low;
  }
}
inline void random_vector(vector<uint64_t>& v, size_t size) { rand_vec2(v, size); }
inline void random_vector(vector<uint8_t>& v, size_t size) {
  rand_vec2(v, size);
  for (int i = 0; i < size; i++) {
    v[i] = v[i] & 1;
  }
}
////////////////////////////////////////////////

template <class T>
inline void print_vec(const vector<T>& a, int length = -1, string msg = "") {
  if (length < 0 || length > a.size())
    length = a.size();
  cout << msg << ": size [" << a.size() << "]" << endl;
  for (int i = 0; i < length; i++) {
    cout << a[i] << endl;
    // cout << setw(22) << a[i];
    // if (i > 0 && (((i + 1) & 0x7) == 0))
    //   cout << endl;
  }
  cout << endl;
}
template <>
inline void print_vec(const vector<string>& a, int length, string msg) {
  if (length < 0 || length > a.size())
    length = a.size();
  cout << msg << ": size [" << a.size() << "]" << endl;
  for (int i = 0; i < length; i++) {
    cout << a[i] << endl;
  }
  cout << endl;
}

template <>
inline void print_vec(const vector<uint8_t>& a, int length, string msg) {
  if (length < 0 || length > a.size())
    length = a.size();
  cout << msg << ": size [" << a.size() << "]" << endl;
  for (int i = 0; i < length; i++) {
    // cout << setw(22) << (myType2)a[i];
    cout << setw(22) << std::to_string(a[i]);
    if (i > 0 && ((i & 0x7) == 0))
      cout << endl;
  }
  cout << endl;
}

template <class T>
inline void print_vec(const vector<vector<T>>& a, int r = 7, int c = 7, string msg = "") {
  if (r < 0 || r > a.size())
    r = a.size();

  if (!msg.empty())
    cout << msg << ":" << endl;
  for (int i = 0; i < r; i++) {
    if (c < 0 || c > a[r].size())
      c = a[r].size();

    cout << "i:" << i << " ";
    for (int j = 0; j < c; j++) {
      // cout << setw(22) << (myType2)a[i][j];
      cout << setw(22) << a[i][j];
      if (j > 0 && ((j & 0x7) == 0))
        cout << endl;
    }
    cout << endl;
  }
  cout << endl;
}

////////////////////////////////////////////////
template <class T>
inline void add_vec(vector<T>& out, vector<T> in_1, vector<T> in_2) {
  if (in_1.size() != in_2.size()) {
    cerr << "Error: the size should be the same!" << endl;
    return;
  }
  for (int i = 0; i < in_1.size(); i++) {
    out.push_back(in_1[i] + in_2[i]);
  }
}

template <class T>
inline void sub_vec(
  vector<T>& out,
  const vector<T>& in_1,
  const vector<T>& in_2) // out = in_1 - in_2
{
  assert(in_1.size() == in_2.size());
  out.clear();
  out.resize(in_1.size());

  for (int i = 0; i < in_1.size(); i++) {
    T v = in_1[i] - in_2[i];
    out[i] = v;
  }
}

template <class T>
inline void sub_vec(
  vector<vector<T>>& out,
  const vector<vector<T>>& in_1,
  const vector<vector<T>>& in_2) // out = in_1 - in_2
{
  assert(in_1.size() == in_2.size());
  out.clear();
  out.resize(in_1.size());

  for (int i = 0; i < in_1.size(); i++) {
    assert(in_1[i].size() == in_2[i].size());
    out[i].resize(in_1[i].size());
    for (int j = 0; j < in_1[i].size(); j++) {
      T v = in_1[i][j] - in_2[i][j];
      out[i][j] = v;
    }
  }
}
template <class T>
inline void multiply_vec(vector<T>& out, vector<T> in_1, vector<T> in_2) {
  if (in_1.size() != in_2.size()) {
    cerr << "Error: the size should be the same!" << endl;
    return;
  }
  for (int i = 0; i < in_1.size(); i++) {
    out.push_back(in_1[i] * in_2[i]);
  }
}

////////////////////////////////////////////////
//! @todo optimized
template <typename T>
inline string get_hex_str(T t) {
  char buf[8] = {0};
  char* p = (char*)&t;
  string s;
  for (int i = 0; i < sizeof(T); i++) {
    sprintf(buf, "%02x", p[i] & 0xFF);
    s.append(buf);
  }
  return s;
};

//! @todo optimized
template <typename T>
inline T from_hex_str(string s) {
  T t;
  unsigned char* p = (unsigned char*)&t;
  for (int i = 0; i < sizeof(T); i++) {
    char c = toupper(s[2 * i]);
    if (c >= '0' && c <= '9') {
      p[i] = ((c - '0') << 4);
    } else if (c >= 'A' && c <= 'F') {
      p[i] = ((c - 'A' + 10) << 4);
    }

    c = toupper(s[2 * i + 1]);
    if (c >= '0' && c <= '9') {
      p[i] |= (c - '0');
    } else if (c >= 'A' && c <= 'F') {
      p[i] |= (c - 'A' + 10);
    }
  }
  return t;
}

////////////////////////////////////////////////
template <typename T>
static void zero_vec2d(vector<vector<T>>& v, int r, int c) {
  v.clear();
  v.resize(r);
  for (int i = 0; i < r; i++) {
    v[i].resize(c, 0);
  }
}

template <typename T>
static void clear_vec2d(vector<vector<T>>& v) {
  v.clear();
  v.resize(0);
  vector<vector<T>>().swap(v);
}

template <typename T>
static void clear_vec1d(vector<T>& v) {
  v.clear();
  v.resize(0);
  vector<T>().swap(v);
}

//////////////////////////

/**
 * size: bytes
 */
static string fmt_mem_size(uint64_t size) {
  stringstream sss;
  if (size > 1024 * 1024 * 1024) // G
  {
    sss << 1.0 * size / (1024 * 1024 * 1024) << "G ";
  }
  if (size > 1024 * 1024) // M
  {
    sss << 1.0 * size / (1024 * 1024) << "M ";
  }
  if (size > 1024) // K
  {
    sss << 1.0 * size / 1024 << "K ";
  }
  sss << size << "B";
  return sss.str();
}

/**
 * us: microseconds
 */
static string fmt_time(int64_t us) {
  stringstream sss;
  sss << setw(11) << us << "(us) [ ";
  if (us > 3600 * 1000 * 1000L) // Hour
  {
    sss << us / (3600 * 1000 * 1000L) << " h ";
    us %= (3600 * 1000 * 1000L);
  }
  if (us > 60 * 1000 * 1000) // minutes
  {
    sss << us / (60 * 1000 * 1000) << " m ";
    us %= (60 * 1000 * 1000);
  }
  if (us > 1000 * 1000) // second
  {
    sss << us / (1000 * 1000) << " s ";
    us %= (1000 * 1000);
  }
  if (us > 1000) // millisecond
  {
    sss << us / 1000 << " ms ";
    us %= 1000;
  }
  if (us > 0) // microsecond
  {
    sss << us << " us ";
  }
  sss << "]";
  return sss.str();
}