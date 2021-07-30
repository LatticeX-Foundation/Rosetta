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
#include "cc/modules/common/include/utils/helper.h"
#include "cc/modules/common/include/utils/rtt_logger.h"

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

////////////////////////////////////////////////

template std::string get_hex_str<uintlong>(uintlong);
template uintlong from_hex_str<uintlong>(std::string);

string to_readable_dec(const uintlong& v) {
  if (v == 0)
    return "0";

  uintlong value = (uintlong)v;
  string hex;
  while (value) {
    hex.insert(0, 1, value % 10 + '0');
    value = value / 10;
  }

  return hex;
}
////////////////////////////////////////////////

void print_vec(const vector<uint8_t>& a, int length, string msg) {
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

void c_print_vec(const vector<double>& a, int length, string msg) {
  if (length < 0 || length > a.size())
    length = a.size();
  printf("%s : size [%zu] ------\n", msg.data(), a.size());
  for (int i = 0; i < length; i++) {
    // cout << setw(22) << (myType2)a[i];
    cout << setw(22) << std::to_string(a[i]);
    printf("%.6lf\t", a[i]);
  }
  printf("\n------\n");
}

void print_vec(const vector<int>& a, int length, string msg) {
  if (length < 0 || length > a.size())
    length = a.size();
  cout << msg << ": size [" << a.size() << "]" << endl;
  for (int i = 0; i < length; i++) {
    cout << a[i] << endl;
  }
  cout << endl;
}

void print_vec(const vector<uint64_t>& a, int length, string msg) {
  if (length < 0 || length > a.size())
    length = a.size();
  cout << msg << ": size [" << a.size() << "]" << endl;
  for (int i = 0; i < length; i++) {
    cout << a[i] << endl;
  }
  cout << endl;
}

void print_vec(const vector<unsigned __int128>& a, int length, string msg) {
  if (length < 0 || length > a.size())
    length = a.size();
  cout << msg << ": size [" << a.size() << "]" << endl;
  for (int i = 0; i < length; i++) {
    cout << to_readable_hex(a[i]) << endl;
  }
  cout << endl;
}

/*
Helper function: Prints a vector of floating-point values.
*/
template <typename T>
inline void print_vector_(vector<T> vec, string msg, size_t print_size, int precision) {
  /*
    Save the formatting information for cout.
    */
  std::ios old_fmt(nullptr);
  old_fmt.copyfmt(cout);

  size_t slot_count = vec.size();

  cout << msg << ": size [" << vec.size() << "]" << endl;
  cout << std::fixed << std::setprecision(precision);
  //cout << endl;
  if (slot_count <= 2 * print_size) {
    cout << "  [";
    for (size_t i = 0; i < slot_count; i++) {
      cout << " " << vec[i] << ((i != slot_count - 1) ? "," : " ]\n");
    }
  } else {
    vec.resize(std::max(vec.size(), 2 * print_size));
    cout << "  [";
    for (size_t i = 0; i < print_size; i++) {
      cout << " " << vec[i] << ",";
    }
    if (vec.size() > 2 * print_size) {
      cout << " ...,";
    }
    for (size_t i = slot_count - print_size; i < slot_count; i++) {
      cout << " " << vec[i] << ((i != slot_count - 1) ? "," : " ]\n");
    }
  }
  cout << endl;

  /*
    Restore the old cout formatting.
    */
  cout.copyfmt(old_fmt);
}

void print_vector(std::vector<double>& vec, std::string msg, size_t print_size, int precision) {
  print_vector_(vec, msg, print_size, precision);
}

/*
Helper function: Prints a matrix of values.
*/
template <typename T>
inline void print_matrix(vector<T> matrix, size_t row_size) {
  /*
    We're not going to print every column of the matrix (there are 2048). Instead
    print this many slots from beginning and end of the matrix.
    */
  size_t print_size = 5;

  cout << endl;
  cout << "    [";
  for (size_t i = 0; i < print_size; i++) {
    cout << setw(3) << std::right << matrix[i] << ",";
  }
  cout << setw(3) << " ...,";
  for (size_t i = row_size - print_size; i < row_size; i++) {
    cout << setw(3) << matrix[i] << ((i != row_size - 1) ? "," : " ]\n");
  }
  cout << "    [";
  for (size_t i = row_size; i < row_size + print_size; i++) {
    cout << setw(3) << matrix[i] << ",";
  }
  cout << setw(3) << " ...,";
  for (size_t i = 2 * row_size - print_size; i < 2 * row_size; i++) {
    cout << setw(3) << matrix[i] << ((i != 2 * row_size - 1) ? "," : " ]\n");
  }
  cout << endl;
};

void print_vec(const vector<double>& a, int length, string msg) {
  if (length < 0 || length > a.size())
    length = a.size();
  // log_info << msg << ": size [" << a.size() << "]";
  // for (int i = 0; i < length; i++) {
  //   log_info << std::fixed << std::setprecision(10) << a[i];
  // }
  cout << msg << ": [plain] size [" << a.size() << "] ";
  for (int i = 0; i < length; i++) {
    cout << a[i] << " ";
  }
  cout << endl;
}

void print_vec(const vector<string>& a, int length, string msg) {
  if (length < 0 || length > a.size())
    length = a.size();
  log_info << msg << ": size [" << a.size() << "]";
  for (int i = 0; i < length; i++) {
    log_info << a[i];
  }
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

std::vector<std::string> split(const std::string& src, char delim) {
  std::stringstream ss(src);
  std::string item;
  std::vector<std::string> elems;
  while (std::getline(ss, item, delim)) {
    elems.push_back(std::move(item));
  }
  return elems;
}

string fmt_mem_size(uint64_t size) {
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

string fmt_time(int64_t us) {
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

//! ref https://tinodidriksen.com/2011/05/cpp-convert-string-to-double-speed/
double to_double(const char* p) {
  double r = 0.0;
  bool neg = false;
  if (*p == '-') {
    neg = true;
    ++p;
  }
  while (*p >= '0' && *p <= '9') {
    r = (r * 10.0) + (*p - '0');
    ++p;
  }
  if (*p == '.') {
    double f = 0.0;
    int n = 0;
    ++p;
    while (*p >= '0' && *p <= '9') {
      f = (f * 10.0) + (*p - '0');
      ++p;
      ++n;
    }
    r += f / std::pow(10.0, n);
  }
  if (neg) {
    r = -r;
  }
  return r;
}

static const int tab64[64] = {63, 0,  58, 1,  59, 47, 53, 2,  60, 39, 48, 27, 54, 33, 42, 3,
                              61, 51, 37, 40, 49, 18, 28, 20, 55, 30, 34, 11, 43, 14, 22, 4,
                              62, 57, 46, 52, 38, 26, 32, 41, 50, 36, 17, 19, 29, 10, 13, 21,
                              56, 45, 25, 31, 35, 16, 9,  12, 44, 24, 15, 8,  23, 7,  6,  5};

uint64_t log2floor(uint64_t value) {
  value |= value >> 1;
  value |= value >> 2;
  value |= value >> 4;
  value |= value >> 8;
  value |= value >> 16;
  value |= value >> 32;
  return tab64[((uint64_t)((value - (value >> 1)) * 0x07EDD5E59A4E28C2)) >> 58];
}

uint64_t log2ceil(uint64_t value) {
  auto floor = log2floor(value);
  return floor + (value > (1ull << floor));
}

double operator-(const timespec& end, const timespec& start) {
  timespec temp;
  if ((end.tv_nsec - start.tv_nsec) < 0) {
    temp.tv_sec = end.tv_sec - start.tv_sec - 1;
    temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
  } else {
    temp.tv_sec = end.tv_sec - start.tv_sec;
    temp.tv_nsec = end.tv_nsec - start.tv_nsec;
  }
  return temp.tv_sec + (double)temp.tv_nsec / 1e9;
}
