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

#include <string>
#include <vector>
#include <cstdint>
using namespace std;

#if __SIZEOF_INT128__
typedef unsigned __int128 uintlong;
typedef __int128 intlong;
#else
typedef uint64_t uintlong;
typedef int64_t intlong;
#endif

////////////////////////////////////////////////
template <typename T>
string to_readable_hex(const T& v) {
  int len = sizeof(T);
  string hex(2 * len, 0);
  unsigned char* p = (unsigned char*)&v;
  unsigned char p0;
  unsigned char p1;
  for (int i = 0; i < len; ++i) {
    p0 = p[len - 1 - i] & 0x0000000F;
    p1 = p[len - 1 - i] >> 4 & 0x0000000F;
    hex[2 * i + 1] = p0 >= 0 && p0 <= 9 ? p0 + '0' : p0 - 10 + 'A';
    hex[2 * i] = p1 >= 0 && p1 <= 9 ? p1 + '0' : p1 - 10 + 'A';
  }
  return hex;
}

string to_readable_dec(const uintlong& v);

////////////////////////////////////////////////
void print_vector(vector<double>& vec, string msg, size_t print_size = 4, int precision = 8);
////////////////////////////////////////////////
void print_vec(const vector<uint8_t>& a, int length = -1, string msg = "");
void print_vec(const vector<int>& a, int length = -1, string msg = "");
void print_vec(const vector<uint64_t>& a, int length = -1, string msg = "");
void print_vec(const vector<unsigned __int128>& a, int length = -1, string msg = "");
void print_vec(const vector<double>& a, int length = -1, string msg = "");
void print_vec(const vector<string>& a, int length = -1, string msg = "");

void c_print_vec(const vector<double>& a, int length, string msg);

////////////////////////////////////////////////
//! @todo optimized
template <typename T>
string get_hex_str(T t) {
  char buf[8] = {0};
  char* p = (char*)&t;
  string s(sizeof(T)*2, 0);
  for (int i = 0; i < sizeof(T); i++) {
    sprintf(buf, "%02x", p[i] & 0xFF);
    // s.append(buf);
    s[2*i] = buf[0];
    s[2*i+1] = buf[1];
  }
  return s;
};
extern template std::string get_hex_str<uintlong>(uintlong);

//! @todo optimized
template <typename T>
T from_hex_str(string s) {
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
extern template uintlong from_hex_str<uintlong>(std::string);

inline string get_hex_buffer(const void* buf, size_t size) {
  char tmp[8] = {0};
  char* p = (char*)buf;
  string s;
  for (size_t i = 0; i < size; i++) {
    sprintf(tmp, "%02x", p[i] & 0xFF);
    s.append(tmp);
  }
  return s;
}

//////////////////////////
/**
 * split src by delim
 */
std::vector<std::string> split(const std::string& src, char delim);

/**
 * size: bytes
 */
string fmt_mem_size(uint64_t size);
/**
 * us: microseconds
 */
string fmt_time(int64_t us);

/**
 * c_string to double
 */
double to_double(const char* p);

/**
 * log_2(integer)
 */
uint64_t log2floor(uint64_t value);
uint64_t log2ceil(uint64_t value);

//! return end - start
double operator-(const timespec& end, const timespec& start);
