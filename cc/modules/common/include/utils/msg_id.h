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
#include <cstdlib>
#include <cstring>
#include <ostream>
#include <memory>
#include <string>

#define USE_SHA256_ID 1 // Hash version. Use OpenSSL::SHA256.

#if USE_SHA256_ID
// 8, 12, 16
#define BIN_SIZE 8
#else
#define BIN_SIZE 2
#endif

using id_type = uint16_t;

typedef char msgid_array[BIN_SIZE];
class msg_id_t {
 public:
  explicit msg_id_t();
  explicit msg_id_t(const msg_id_t& id);
  msg_id_t& operator=(const msg_id_t& id);

  explicit msg_id_t(const char* bin, int len);
  explicit msg_id_t(id_type id, const std::string& src_msgid);
  explicit msg_id_t(const std::string& src_msgid);

 public:
  friend std::ostream& operator<<(std::ostream& os, const msg_id_t& id);
  bool operator<(const msg_id_t& id) const;
  bool operator==(const msg_id_t& id) const;
  static constexpr size_t Size() { return BIN_SIZE; }
  const std::string& str() const { return src_; }
  const char* data() const { return (const char*)bin_; }
  uint64_t to_uint64() const { return *(uint64_t*)bin_; }
  const char* get_hex(void) const {
	  return str_.c_str();
  }

 private:
#if USE_SHA256_ID
  void hash();
#endif

 private:
  char bin_[BIN_SIZE]; //! the binary digest, use the first BIN_SIZE bytes of sha256
  std::string str_ = ""; //! the hash hex-string (unique for each src_)
  std::string src_ = ""; //! source string
};

/**
 * Get the message id
 */
const msg_id_t& get_msgid(const std::string& str_msgid);

