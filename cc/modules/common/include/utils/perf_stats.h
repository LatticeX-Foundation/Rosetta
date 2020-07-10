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
#include <sstream>
#include <vector>
#include <iostream>
#include <cstring>
#include <cstdlib>
using namespace std;

/**
 * Performance Statitistic. Time, Network, CPU, Memory, etc.
 */
namespace rosetta {
class PerfStats {
 public:
  string vmpeak = "";
  struct __stat {
    // io
    int64_t bytes_sent = 0;
    int64_t bytes_recv = 0;
    int64_t msg_sent = 0;
    int64_t msg_recv = 0;
    double elapsed_sent = 0;
    double elapsed_recv = 0;
    // time
    double clock_seconds = 0;
    double cpu_seconds = 0;
    clock_t start = 0;
    struct timespec req_start, req_end;
  } p, r, a; // prepare,run,all(p+r)
  friend __stat operator+(const __stat& l, const __stat& r) {
    __stat t;
#define _add(v) t.v = l.v + r.v
    _add(bytes_sent);
    _add(bytes_recv);
    _add(msg_sent);
    _add(msg_recv);
    _add(elapsed_sent);
    _add(elapsed_recv);

    _add(clock_seconds);
    _add(cpu_seconds);
#undef _add
    return t;
  }
  void reset() {
    memset(&p, 0, sizeof(__stat));
    memset(&r, 0, sizeof(__stat));
    memset(&a, 0, sizeof(__stat));
    vmpeak = "";
  }
  std::string stats(int party);
  std::string stats2(int party);
};
} // namespace rosetta
