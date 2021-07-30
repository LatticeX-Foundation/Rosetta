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
// Template implementations

#pragma once
template <typename T>
void addVectors(const vector<T>& a, const vector<T>& b, vector<T>& c, size_t size) {
  for (size_t i = 0; i < size; ++i)
    c[i] = a[i] + b[i];
}
extern template void addVectors<mpc_t>(const std::vector<mpc_t>&, const std::vector<mpc_t>&, std::vector<mpc_t>&, size_t);
extern template void addVectors<small_mpc_t>(const std::vector<small_mpc_t>&, const std::vector<small_mpc_t>&, std::vector<small_mpc_t>&, size_t);

template <typename T>
void subtractVectors(const vector<T>& a, const vector<T>& b, vector<T>& c, size_t size) {
  for (size_t i = 0; i < size; ++i)
    c[i] = a[i] - b[i];
}
extern template void subtractVectors<mpc_t>(const std::vector<mpc_t>&, const std::vector<mpc_t>&, std::vector<mpc_t>&, size_t);
extern template void subtractVectors<small_mpc_t>(const std::vector<small_mpc_t>&, const std::vector<small_mpc_t>&, std::vector<small_mpc_t>&, size_t);

template <typename T>
void copyVectors(const vector<T>& a, vector<T>& b, size_t size) {
  for (size_t i = 0; i < size; ++i)
    b[i] = a[i];
}
extern template void copyVectors<mpc_t>(const std::vector<mpc_t>&, std::vector<mpc_t>&, size_t);
extern template void copyVectors<small_mpc_t>(const std::vector<small_mpc_t>&, std::vector<small_mpc_t>&, size_t);

template <typename T1, typename T2>
void addModuloOdd(const vector<T1>& a, const vector<T2>& b, vector<mpc_t>& c, size_t size) {
  assert(
    (sizeof(T1) == sizeof(mpc_t) or sizeof(T2) == sizeof(mpc_t)) &&
    "At least one type should be mpc_t for typecast to work");
  for (size_t i = 0; i < size; ++i) {
    if (a[i] == MINUS_ONE and b[i] == MINUS_ONE)
      c[i] = 0;
    else
      c[i] = (a[i] + b[i] + wrapAround(a[i], b[i])) % MINUS_ONE;
  }
}
extern template void addModuloOdd<mpc_t, mpc_t>(const std::vector<mpc_t>&, const std::vector<mpc_t>&, std::vector<mpc_t>&, size_t);
extern template void addModuloOdd<mpc_t, small_mpc_t>(const std::vector<mpc_t>&, const std::vector<small_mpc_t>&, std::vector<mpc_t>&, size_t);
extern template void addModuloOdd<small_mpc_t, small_mpc_t>(const std::vector<small_mpc_t>&, const std::vector<small_mpc_t>&, std::vector<mpc_t>&, size_t);
extern template void addModuloOdd<small_mpc_t, mpc_t>(const std::vector<small_mpc_t>&, const std::vector<mpc_t>&, std::vector<mpc_t>&, size_t);

template <typename T1, typename T2>
mpc_t addModuloOdd(T1 a, T2 b) {
  assert(
    (sizeof(T1) == sizeof(mpc_t) or sizeof(T2) == sizeof(mpc_t)) &&
    "At least one type should be mpc_t for typecast to work");

  if (a == MINUS_ONE and b == MINUS_ONE)
    return 0;
  else
    return (a + b + wrapAround(a, b)) % MINUS_ONE;
}
extern template mpc_t addModuloOdd<mpc_t, mpc_t>(mpc_t, mpc_t);
extern template mpc_t addModuloOdd<mpc_t, small_mpc_t>(mpc_t, small_mpc_t);
extern template mpc_t addModuloOdd<small_mpc_t, small_mpc_t>(small_mpc_t, small_mpc_t);
extern template mpc_t addModuloOdd<small_mpc_t, mpc_t>(small_mpc_t, mpc_t);

template <typename T1, typename T2>
void subtractModuloOdd(const vector<T1>& a, const vector<T2>& b, vector<mpc_t>& c, size_t size) {
  vector<mpc_t> temp(size);
  for (size_t i = 0; i < size; ++i)
    temp[i] = MINUS_ONE - b[i];

  addModuloOdd<T1, mpc_t>(a, temp, c, size);
}
extern template void subtractModuloOdd<mpc_t, mpc_t>(const std::vector<mpc_t>&, const std::vector<mpc_t>&, std::vector<mpc_t>&, size_t);
extern template void subtractModuloOdd<mpc_t, small_mpc_t>(const std::vector<mpc_t>&, const std::vector<small_mpc_t>&, std::vector<mpc_t>&, size_t);
extern template void subtractModuloOdd<small_mpc_t, small_mpc_t>(const std::vector<small_mpc_t>&, const std::vector<small_mpc_t>&, std::vector<mpc_t>&, size_t);
extern template void subtractModuloOdd<small_mpc_t, mpc_t>(const std::vector<small_mpc_t>&, const std::vector<mpc_t>&, std::vector<mpc_t>&, size_t);

template <typename T1, typename T2>
mpc_t subtractModuloOdd(T1 a, T2 b) {
  mpc_t temp = MINUS_ONE - b;
  return addModuloOdd<T1, mpc_t>(a, temp);
}
extern template mpc_t subtractModuloOdd<mpc_t, mpc_t>(mpc_t, mpc_t);
extern template mpc_t subtractModuloOdd<mpc_t, small_mpc_t>(mpc_t, small_mpc_t);
extern template mpc_t subtractModuloOdd<small_mpc_t, small_mpc_t>(small_mpc_t, small_mpc_t);
extern template mpc_t subtractModuloOdd<small_mpc_t, mpc_t>(small_mpc_t, mpc_t);
