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

#include <map>
#include <vector>
#include <string>
#include <atomic>
#include <mutex>
#include <deque>
#include <iostream>
#include <cstring>
#include <cstdlib>

using id_type = uint16_t; // will used in the future

/**
 * Hash version. \n
 * Use OpenSSL::SHA256. \n
 */
class msg_id_t {
 public:
  explicit msg_id_t() { memset(bin_, 0, 16); };
  explicit msg_id_t(const msg_id_t& id) {
    src_.assign(id.src_);
    str_.assign(id.str_);
    memcpy(bin_, id.bin_, 16);
  }
  explicit msg_id_t(const std::string& key) {
    src_.assign(key);
    hash();
  }
  msg_id_t& operator=(const msg_id_t& id) {
    src_.assign(id.src_);
    str_.assign(id.str_);
    memcpy(bin_, id.bin_, 16);
    return *this;
  }

 public:
  friend std::ostream& operator<<(std::ostream& os, const msg_id_t& id);
  bool operator<(const msg_id_t& id) const { return (strncmp(bin_, id.bin_, 16) < 0); }
  bool operator==(const msg_id_t& id) const { return (strncmp(bin_, id.bin_, 16) == 0); }
  static size_t Size() { return 16; }
  const std::string& str() const { return str_; }
  const char* data() const { return (const char*)bin_; }
  char* data() { return bin_; }

 private:
  void hash();

 private:
  char bin_[16]; // the binary digest, use the first 16 bytes of sha256
  std::string str_ = ""; // the hash hex-string (unique for each src_)
  std::string src_ = ""; // source string
};
