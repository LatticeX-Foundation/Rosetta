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
#include "cc/modules/io/include/internal/cycle_buffer.h"

#include <cstring>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <assert.h>
using namespace std;

namespace rosetta {
namespace io {

int cycle_buffer::peek(char* data, int32_t length) {
  timer_.start();
  {
    unique_lock<mutex> lck(mtx_);
    cv_.wait(lck, [&]() {
      if (n_ - remain_space_ >= length)
        return true;
      cout << "can not peek" << endl;
      return false;
    });
  }
  timer_.start();
  {
    unique_lock<mutex> lck(mtx_);
    if (r_pos_ >= w_pos_) {
      if (r_pos_ <= n_ - length) {
        memcpy(data, buffer_ + r_pos_, length);
      } else {
        int first_n = n_ - r_pos_;
        memcpy(data, buffer_ + r_pos_, first_n);
        memcpy(data + first_n, buffer_, length - first_n);
      }
    } else {
      memcpy(data, buffer_ + r_pos_, length);
    }
    cv_.notify_all();
  }
  return length;
}
int cycle_buffer::read(char* data, int32_t length) {
  timer_.start();
  {
    do {
      unique_lock<mutex> lck(mtx_);
      cv_.wait_for(lck, std::chrono::milliseconds(1000), [&]() {
        if (n_ - remain_space_ >= length)
          return true;
        if (verbose_ > 1)
          cout << "can not read. expected:" << length << ",actual:" << size() << endl;
        return false;
      });
      if (n_ - remain_space_ >= length)
        break;
    } while (true);
  }
  timer_.start();
  {
    unique_lock<mutex> lck(mtx_);
    if (r_pos_ >= w_pos_) {
      if (r_pos_ <= n_ - length) {
        memcpy(data, buffer_ + r_pos_, length);
        r_pos_ += length;
      } else {
        int first_n = n_ - r_pos_;
        memcpy(data, buffer_ + r_pos_, first_n);
        memcpy(data + first_n, buffer_, length - first_n);
        r_pos_ = length - first_n;
      }
    } else {
      memcpy(data, buffer_ + r_pos_, length);
      r_pos_ += length;
    }
    remain_space_ += length;
    cv_.notify_all();
  }
  return length;
}
void cycle_buffer::realloc(int32_t length) {
  unique_lock<mutex> lck(mtx_);
  if (remain_space_ < length) {
    int32_t new_n = n_ * ((length / n_) + 2); // at least 2x
    cout << "can not write. expected:" << length << ", actual:" << remain_space_
         << ". will expand from " << n_ << " to " << new_n << endl;

    char* newbuffer_ = new char[new_n];
    int32_t havesize = size();
    if (w_pos_ > r_pos_) {
      memcpy(newbuffer_, buffer_ + r_pos_, havesize);
    } else {
      int first_n = n_ - r_pos_;
      memcpy(newbuffer_, buffer_ + r_pos_, first_n);
      if (havesize - first_n > 0) {
        memcpy(newbuffer_ + first_n, buffer_, havesize - first_n);
      }
    }
    n_ = new_n;
    remain_space_ = n_ - havesize;
    r_pos_ = 0;
    w_pos_ = havesize;
    delete[] buffer_;
    buffer_ = newbuffer_;
    newbuffer_ = nullptr;
  }
}

// data --> buffer_
int cycle_buffer::write(const char* data, int32_t length) {
  timer_.start();
  {
    realloc(length);
    unique_lock<mutex> lck(mtx_);
    cv_.wait(lck, [&]() {
      if (remain_space_ >= length)
        return true;
      cout << "never enter here. can not write" << endl;
      return false;
    });
  }
  timer_.start();
  {
    unique_lock<mutex> lck(mtx_);
    if (w_pos_ >= r_pos_) {
      if (w_pos_ <= n_ - length) {
        memcpy(buffer_ + w_pos_, data, length);
        w_pos_ += length;
      } else {
        int first_n = n_ - w_pos_;
        memcpy(buffer_ + w_pos_, data, first_n);
        memcpy(buffer_, data + first_n, length - first_n);
        w_pos_ = length - first_n;
      }
    } else {
      memcpy(buffer_ + w_pos_, data, length);
      w_pos_ += length;
    }
    remain_space_ -= length;
    cv_.notify_all();
  }
  return length;
}

} // namespace io
} // namespace rosetta