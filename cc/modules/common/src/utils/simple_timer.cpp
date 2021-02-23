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
#include "cc/modules/common/include/utils/simple_timer.h"

#include <chrono>
using namespace std::chrono;

void SimpleTimer::start() {
  begin = steady_clock::now();
  end = begin;
  stoped = false;
}

double SimpleTimer::stop() {
  end = steady_clock::now();
  stoped = true;
  duration<double> elapsed_seconds = end - begin;
  double costTime = elapsed_seconds.count();
  return costTime;
}

double SimpleTimer::elapse() const {
  auto ending = steady_clock::now();
  if (stoped)
    ending = end;
  duration<double> elapsed_seconds = ending - begin;
  return elapsed_seconds.count();
}

long long int SimpleTimer::ms_elapse() const {
  auto ending = steady_clock::now();
  if (stoped)
    ending = end;
  duration<long long int, std::milli> elapsed_milliseconds =
    duration_cast<duration<long long int, std::milli>>(ending - begin);
  return elapsed_milliseconds.count();
}

long long int SimpleTimer::us_elapse() const {
  auto ending = steady_clock::now();
  if (stoped)
    ending = end;
  duration<long long int, std::micro> elapsed_microseconds =
    duration_cast<duration<long long int, std::micro>>(ending - begin);
  return elapsed_microseconds.count();
}

long long int SimpleTimer::ns_elapse() const {
  auto ending = steady_clock::now();
  if (stoped)
    ending = end;
  duration<long long int, std::nano> elapsed_nanoseconds =
    duration_cast<duration<long long int, std::nano>>(ending - begin);
  return elapsed_nanoseconds.count();
}
