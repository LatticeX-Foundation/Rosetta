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
#include "cc/modules/io/include/internal/msg_id.h"

#if 0
// the following variables are used in the map version
std::mutex map_msgid_mtx_;
std::map<std::string, id_type> map_msgid_;
id_type id_counter_;
std::deque<id_type> reuseid_;
#endif

#include <openssl/sha.h>

namespace {
std::string to_hex(const char* data, int size) {
  std::string str("");
  char tmp[3] = {0};
  for (int i = 0; i < size; i++) {
    sprintf(tmp, "%02x", data[i] & 0xFF);
    str += std::string(tmp, 2);
  }
  return str;
}
} // namespace

void msg_id_t::hash() {
  char bin[33] = {0};
  ::SHA256((const unsigned char*)src_.data(), src_.size(), (unsigned char*)bin);
  memcpy(bin_, bin, 16); // get the first 16 bytes
  str_.assign(to_hex(bin_, 16));
}

std::ostream& operator<<(std::ostream& os, const msg_id_t& id) {
  std::string str(id.str_);
  if (str.empty()) {
    str.assign(to_hex(id.bin_, 16));
  }
  os << str << "[" << id.src_ << "]";
  return os;
}