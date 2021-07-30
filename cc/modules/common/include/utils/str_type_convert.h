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
#include "cc/modules/protocol/mpc/comm/include/mpc_common.h"
using namespace std;
using std::vector;

namespace rosetta {
namespace convert {

template <typename T>
void to_binary_str(const T& t, string& s) {
  s.resize(sizeof(t));
  memcpy((void*)s.data(), &t, sizeof(t));
};

static inline void to_binary_str(const void* ptr, int size, string& s) {
  s.resize(size);
  memcpy((void*)s.data(), ptr, size);
};

template <typename T>
void to_binary_str(const vector<T>& ss, vector<string>& ts) {
  ts.resize(ss.size());
  for (int i = 0; i < ss.size(); ++i) {
    ts[i].resize(sizeof(T));
    memcpy((void*)ts[i].data(), (void*)&ss[i], sizeof(ss[i]));
  }
};

// convert from value encoded with hex-formatted string to specify T type
// T should be POD type
template <typename T>
T from_binary_str(const string& s) {
  T t;
  memcpy((void*)&t, s.data(), sizeof(t)); //cns.size());
  return t;
}

template <typename T>
void from_binary_str(const string& s, T& t) {
  memcpy((void*)&t, s.data(), sizeof(t));
}

template <typename T>
void from_binary_str(const vector<string>& s, vector<T>& t) {
  t.resize(s.size());
  for (auto i = 0; i < s.size(); ++i)
    from_binary_str(s[i], t[i]);
}

template <typename T>
vector<T> from_binary_str(const vector<string>& s) {
  vector<T> t(s.size());
  from_binary_str(s, t);
  return t;
}

vector<double> from_double_str(const vector<string>& s);
void from_double_str(const vector<string>& s, vector<double>& t);

vector<int64_t> from_int_str(const vector<string>& s);
void from_int_str(const vector<string>& s, vector<int64_t>& t);

extern template void to_binary_str<double>(const double&, std::string&);
extern template void to_binary_str<mpc_t>(const mpc_t&, std::string&);
extern template void to_binary_str<double>(const std::vector<double>&, std::vector<std::string>&);
extern template void to_binary_str<mpc_t> (const std::vector<mpc_t>&,  std::vector<std::string>&);

extern template double from_binary_str<double>(const std::string&);
extern template mpc_t from_binary_str<mpc_t>(const std::string&);
// const std::string&, T&
extern template void from_binary_str<double>(const std::string&, double&);
extern template void from_binary_str<mpc_t>(const std::string&, mpc_t&);
// std::vector<std::string>&, std::vector<T>&
extern template void from_binary_str<double>(const std::vector<std::string>&, std::vector<double>&);
extern template void from_binary_str<mpc_t>(const std::vector<std::string>&, std::vector<mpc_t>&);
// std::vector<std::string>&
extern template std::vector<double> from_binary_str<double>(const std::vector<std::string>&);
extern template std::vector<mpc_t> from_binary_str<mpc_t>(const std::vector<std::string>&);


} // namespace convert
} // namespace rosetta
