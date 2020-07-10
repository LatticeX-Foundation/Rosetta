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
#include <cstring>
#include <vector>
#include <array>
using namespace std;
using std::vector;

namespace rosetta {
namespace convert {

////////////////////////////////////////////////
// convert T type to hex-formatted string
// T should be POD type
template <typename T, typename std::enable_if<std::is_pod<T>::value, int>::type = 0>
string to_hex_str(const T& t) {
  int len = sizeof(t) + 1;
  char* buf = new char[len];
  memset(buf, 0, len);

  char* p = (char*)&t;
  string s;
  for (int i = 0; i < sizeof(T); i++) {
    sprintf(buf, "%02x", p[i] & 0xFF);
    s.append(buf);
  }

  delete[] buf;
  return s;
};

template <typename T>
void to_hex_str(const T& t, string& s) {
  int len = sizeof(t) + 1;
  char* buf = new char[len];
  memset(buf, 0, len);

  char* p = (char*)&t;
  for (int i = 0; i < sizeof(T); i++) {
    sprintf(buf, "%02x", p[i] & 0xFF);
    s.append(buf);
  }

  delete[] buf;
};

static void to_hex_str_copy(const void* ptr, int size, string& s) {
  int len = size + 1;
  char* buf = new char[len];
  memset(buf, 0, len);

  const char* p = (const char*)ptr;
  for (int i = 0; i < size; i++) {
    sprintf(buf, "%02x", p[i] & 0xFF);
    s.append(buf);
  }

  delete[] buf;
};

template <typename T>
void to_hex_str(const vector<T>& ss, vector<string>& ts) {
  int len = sizeof(T) + 1;
  char* buf = new char[len];
  memset(buf, 0, len);

  ts.resize(ss.size());
  for (int i = 0; i < ss.size(); ++i) {
    char* p = (char*)&ss[i];
    string s;
    for (int j = 0; j < sizeof(T); j++) {
      sprintf(buf, "%02x", p[j] & 0xFF);
      s.append(buf);
    }

    ts[i] = s;
  }

  delete[] buf;
};

// convert from value encoded with hex-formatted string to specify T type
// T should be POD type
template <typename T>
T from_hex_str(const string& s) {
  assert(s.size() == sizeof(T) * 2 && "string size and T are not compatiable");

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

template <typename T>
void from_hex_str(const string& s, T& t) {
  t = from_hex_str<T>(s);
}

template <typename T>
void from_hex_str(const vector<string>& s, vector<T>& t) {
  t.resize(s.size());
  for (auto i = 0; i < s.size(); ++i)
    from_hex_str(s[i], t[i]);
}

template <typename T>
vector<T> from_hex_str(const vector<string>& s) {
  vector<T> t(s.size());
  from_hex_str(s, t);
  return t;
}

static vector<double> from_double_str(const vector<string>& s) {
  vector<double> t(s.size());
  for (int i = 0; i < s.size(); i++)
    t[i] = std::stod(s[i]);

  return t;
}

static void from_double_str(const vector<string>& s, vector<double>& t) {
  t.resize(s.size());
  for (int i = 0; i < s.size(); i++)
    t[i] = std::stod(s[i]);
}

static vector<int64_t> from_int_str(const vector<string>& s) {
  vector<int64_t> t(s.size());
  for (int i = 0; i < s.size(); i++)
    t[i] = std::stod(s[i]);

  return t;
}

static void from_int_str(const vector<string>& s, vector<int64_t>& t) {
  t.resize(s.size());
  for (int i = 0; i < s.size(); i++)
    t[i] = std::stoll(s[i]);
}

} // namespace convert
} // namespace rosetta
