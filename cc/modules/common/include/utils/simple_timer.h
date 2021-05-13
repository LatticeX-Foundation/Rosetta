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

#include <chrono>

class SimpleTimer {
 public:
  SimpleTimer() { start(); }
  virtual ~SimpleTimer() {}

  void start();
  double stop();

  double elapse() const;
  long long int ms_elapse() const;
  long long int us_elapse() const;
  long long int ns_elapse() const;

 private:
  bool stoped;
  std::chrono::time_point<std::chrono::steady_clock> begin;
  std::chrono::time_point<std::chrono::steady_clock> end;
};
