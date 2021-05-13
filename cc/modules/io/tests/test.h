#pragma once
#include <catch.hpp>

#include <vector>
#include <random>
using namespace std;

// net internal use
template <typename T>
static inline void rand_vec(vector<T>& vec, int length) {
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
