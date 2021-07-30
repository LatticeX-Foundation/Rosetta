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
#include "cc/modules/common/include/utils/msg_id.h"
#include "cc/modules/common/include/utils/msg_id_mgr.h"
#include "cc/modules/common/include/utils/rtt_logger.h"

#include <map>
#include <set>
#include <vector>
#include <string>
#include <atomic>
#include <mutex>
#include <deque>
#include <iostream>
#include <cassert>
using namespace std;

#if USE_SHA256_ID
#include <openssl/sha.h>
namespace {
inline std::string to_hex(const char* data, int size) {
  /**
   * for high-performance, no need hex-string
   * if you want debug msgid, comment the following line
   */
  //return std::string("");

  std::string str("");
  char tmp[3] = {0};
  for (int i = 0; i < size; i++) {
    sprintf(tmp, "%02x", data[i] & 0xFF);
    str += std::string(tmp, 2);
  }
  return str;
}
} // namespace
#endif

msg_id_t::msg_id_t() {
  str_ = "";
  src_ = "";
  memset(bin_, 0, BIN_SIZE);
}
msg_id_t::msg_id_t(const msg_id_t& id) {
  src_.assign(id.src_);
  str_.assign(id.str_);
  memcpy(bin_, id.bin_, BIN_SIZE);
}
msg_id_t& msg_id_t::operator=(const msg_id_t& id) {
  src_.assign(id.src_);
  str_.assign(id.str_);
  memcpy(bin_, id.bin_, BIN_SIZE);
  return *this;
}

msg_id_t::msg_id_t(const char* bin, int len) {
  memcpy(bin_, bin, BIN_SIZE);
#if USE_SHA256_ID
  str_.assign(to_hex(bin_, BIN_SIZE));
#else
  str_.assign(std::to_string(*(id_type*)bin));
#endif
  src_ = "";
}
msg_id_t::msg_id_t(id_type id, const std::string& src_msgid) {
#if USE_SHA256_ID
  src_.assign(src_msgid);
  hash();
#else
  src_.assign(std::to_string(id));
  str_.assign(src_msgid);
  memcpy(bin_, &id, BIN_SIZE);
#endif
}
msg_id_t::msg_id_t(const std::string& src_msgid) {
  auto& id = get_msgid(src_msgid);
  src_.assign(id.src_);
  str_.assign(id.str_);
  memcpy(bin_, id.bin_, BIN_SIZE);
}

std::ostream& operator<<(std::ostream& os, const msg_id_t& id) {
  os << id.str() << "[" << id.get_hex() << "]";
  return os;
}

bool msg_id_t::operator<(const msg_id_t& id) const {
#if (BIN_SIZE == 2)
  return *(uint16_t*)bin_ < *(uint16_t*)id.bin_;
#elif (BIN_SIZE == 4)
  return *(uint32_t*)bin_ < *(uint32_t*)id.bin_;
#elif (BIN_SIZE == 8)
  return *(uint64_t*)bin_ < *(uint64_t*)id.bin_;
#elif (BIN_SIZE == 12)
  char *p0 = (char*)bin_, *p1 = (char*)id.bin_;
  __uint128_t p0_a = *(uint64_t*)p0;
  __uint128_t p1_a = *(uint64_t*)p1;
  p0 += 8, p1 += 8;
  __uint128_t p0_b = *(uint32_t*)p0;
  __uint128_t p1_b = *(uint32_t*)p1;
  return ((p0_a << 64) | p0_b) < ((p1_a << 64) | p1_b);
#elif (BIN_SIZE == 16)
  char *p0 = (char*)bin_, *p1 = (char*)id.bin_;
  __uint128_t p0_a = *(uint64_t*)p0;
  __uint128_t p1_a = *(uint64_t*)p1;
  p0 += 8, p1 += 8;
  __uint128_t p0_b = *(uint64_t*)p0;
  __uint128_t p1_b = *(uint64_t*)p1;
  return ((p0_a << 64) | p0_b) < ((p1_a << 64) | p1_b);
#endif
  return (strncmp(bin_, id.bin_, BIN_SIZE) < 0);
}

bool msg_id_t::operator==(const msg_id_t& id) const {
#if (BIN_SIZE == 2)
  return *(uint16_t*)bin_ == *(uint16_t*)id.bin_;
#elif (BIN_SIZE == 4)
  return *(uint32_t*)bin_ == *(uint32_t*)id.bin_;
#elif (BIN_SIZE == 8)
  return *(uint64_t*)bin_ == *(uint64_t*)id.bin_;
#elif (BIN_SIZE == 12)
  char *p0 = (char*)bin_, *p1 = (char*)id.bin_;
  __uint128_t p0_a = *(uint64_t*)p0;
  __uint128_t p1_a = *(uint64_t*)p1;
  p0 += 8, p1 += 8;
  __uint128_t p0_b = *(uint32_t*)p0;
  __uint128_t p1_b = *(uint32_t*)p1;
  return ((p0_a << 64) | p0_b) == ((p1_a << 64) | p1_b);
#elif (BIN_SIZE == 16)
  char *p0 = (char*)bin_, *p1 = (char*)id.bin_;
  __uint128_t p0_a = *(uint64_t*)p0;
  __uint128_t p1_a = *(uint64_t*)p1;
  p0 += 8, p1 += 8;
  __uint128_t p0_b = *(uint64_t*)p0;
  __uint128_t p1_b = *(uint64_t*)p1;
  return ((p0_a << 64) | p0_b) == ((p1_a << 64) | p1_b);
#endif
  return (strncmp(bin_, id.bin_, BIN_SIZE) == 0);
}

#if USE_SHA256_ID
void msg_id_t::hash() {
  char bin[33] = {0};
  ::SHA256((const unsigned char*)src_.data(), src_.size(), (unsigned char*)bin);
  memcpy(bin_, bin, BIN_SIZE); // get the first BIN_SIZE bytes
  str_.assign(to_hex(bin_, BIN_SIZE));
}
#endif
