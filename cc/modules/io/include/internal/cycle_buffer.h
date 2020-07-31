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

#include "cc/modules/common/include/utils/simple_timer.h"

#include <mutex>
#include <condition_variable>

namespace rosetta {
namespace io {

struct cycle_buffer {
  bool is_full_ = false;
  bool is_empty_ = true;
  int32_t r_pos_ = 0; // read position
  int32_t w_pos_ = 0; // write position
  int32_t n_ = 0; // buffer size
  int32_t remain_space_ = 0;
  char* buffer_ = nullptr;
  std::mutex mtx_;
  std::condition_variable cv_;
  int verbose_ = 0;

  /// a timer for rm empty <msgid -> buffer>
  SimpleTimer timer_;

  int32_t size() {
    return n_ - remain_space_;
  }
  int32_t remain_space() const {
    return remain_space_;
  }

 public:
  ~cycle_buffer() {
    delete[] buffer_;
    buffer_ = nullptr;
  }
  cycle_buffer(int32_t n) : n_(n), remain_space_(n) {
    buffer_ = new char[n_];
  }
  void empty() {
    is_full_ = false;
    is_empty_ = true;
    r_pos_ = 0;
    w_pos_ = 0;
    remain_space_ = n_;
  }

 public:
  // if i can read length size buffer
  bool can_read(int32_t length) {
    std::unique_lock<std::mutex> lck(mtx_);
    return (n_ - remain_space_ >= length);
  }
  bool can_remove(double t) {
    if (
      (timer_.elapse() > t) // no visits in t seconds
      && (remain_space_ == n_) // no datas
    ) {
      return true;
    }
    return false;
  }

  /**
   * The real data will not be deleted. \n
   * The caller must make sure that can read length size bytes data
   */
  int peek(char* data, int32_t length);
  int read(char* data, int32_t length);
  void realloc(int32_t length);
  int write(const char* data, int32_t length);
};
} // namespace io
} // namespace rosetta