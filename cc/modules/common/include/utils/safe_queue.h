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

/*
** thread safe queue based on <queue>
** header-only
*/

#include <chrono>
#include <queue>
#include <mutex>
#include <condition_variable>

template <typename T>
class safe_queue {
 public:
  safe_queue(int64_t max_size = -1L) : max_size_(max_size) {}
  ~safe_queue() {}
  bool push(const T& v, int32_t timeout = -1);
  bool push(T&& v, int32_t timeout = -1);
  bool pop(T& v, int32_t timeout = -1);
  size_t size() {
    return queue_.size();
  }
  bool empty() {
    return queue_.empty();
  }

 private:
  std::mutex mtx_;
  std::condition_variable empty_cv_;
  std::condition_variable full_cv_;
  std::queue<T> queue_;
  size_t max_size_;
};

template <typename T>
bool safe_queue<T>::push(const T& v, int32_t timeout) {
  std::unique_lock<std::mutex> lock(mtx_);

  while (true) {
    if (size() < max_size_) {
      queue_.push(v);
      empty_cv_.notify_one();
      return true;
    } else {
      if (timeout < 0) {
        full_cv_.wait(lock);
      } else {
        full_cv_.wait_for(lock, std::chrono::milliseconds(timeout));
        if (size() >= max_size_)
          return false;
      }
    }
  }
  return false;
}

template <typename T>
bool safe_queue<T>::push(T&& v, int32_t timeout) {
  std::unique_lock<std::mutex> lock(mtx_);

  while (true) {
    if (size() < max_size_) {
      queue_.push(std::move(v));
      empty_cv_.notify_one();
      return true;
    } else {
      if (timeout < 0) {
        full_cv_.wait(lock);
      } else {
        full_cv_.wait_for(lock, std::chrono::milliseconds(timeout));
        if (size() >= max_size_)
          return false;
      }
    }
  }
  return false;
}

template <typename T>
bool safe_queue<T>::pop(T& v, int32_t timeout) {
  std::unique_lock<std::mutex> lock(mtx_);

  while (true) {
    if (!queue_.empty()) {
      v = queue_.front();
      queue_.pop();
      full_cv_.notify_one();
      return true;
    } else {
      if (timeout < 0) {
        empty_cv_.wait(lock);
      } else {
        empty_cv_.wait_for(lock, std::chrono::milliseconds(timeout));
        if (queue_.empty())
          return false;
      }
    }
  }
  return false;
}
