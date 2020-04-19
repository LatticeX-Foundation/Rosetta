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

#include "logger.h"

////////////////////////////////////////////////
static uint32_t byteswap_uint32(uint32_t a) {
  return (
      (((a >> 24) & 0xff) << 0) | (((a >> 16) & 0xff) << 8) | (((a >> 8) & 0xff) << 16) |
      (((a >> 0) & 0xff) << 24));
}

static uint8_t* read_file(const char* szFile) {
  ifstream file(szFile, ios::binary | ios::ate);
  if (!file.is_open())
    printf("can't open file %s\n", szFile);
  streamsize size = file.tellg();
  file.seekg(0, ios::beg);

  if (size == -1)
    return nullptr;

  uint8_t* buffer = new uint8_t[size];
  file.read((char*)buffer, size);
  return buffer;
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

inline void rand_vec(vector<int64_t>& vec, int length, int bit_size) {
  vec.clear();
  vec.resize(length);

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<uint> dis(0, (1L << (bit_size - 1)) - 1);

  for (int i = 0; i < length; i++) {
    vec[i] = dis(gen) - (1L << (bit_size - 2));
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
////////////////////////////////////////////////

template <class T>
inline void print_vec(const vector<T>& a, int length = -1, string msg = "") {
  if (length < 0 || length > a.size())
    length = a.size();
  cout << msg << ": size [" << a.size() << "]" << endl;
  for (int i = 0; i < length; i++) {
    // cout << setw(22) << (myType2)a[i];
    cout << setw(22) << a[i];
    if ((i & 0x7) == 0x7)
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
      if ((j & 0x7) == 0x7)
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
    vector<T>& out, const vector<T>& in_1,
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
    vector<vector<T>>& out, const vector<vector<T>>& in_1,
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
