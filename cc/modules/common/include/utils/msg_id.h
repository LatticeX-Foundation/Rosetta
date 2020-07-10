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
#include <stdio.h>
#include <string.h>
#include <memory>

#define MID_LEN 2

namespace rosetta {

class MsgId {
 public:
  explicit MsgId() {}
  explicit MsgId(const MsgId& mid) {
    _strsrc = mid._strsrc;
    memcpy(_bydest, mid._bydest, MID_LEN);
  }

  explicit MsgId(const std::string& strnum) {
    _strsrc = strnum;
    unsigned int nid = strtoul(strnum.c_str(), nullptr, 10);
    memcpy(_bydest, (unsigned char*)&nid, MID_LEN);
  }

  explicit MsgId(unsigned int id) {
    _strsrc = std::to_string(id);
    memcpy(_bydest, &id, MID_LEN);
  }

  MsgId& operator=(const MsgId& id) {
    _strsrc = id._strsrc;
    memcpy(_bydest, id._bydest, MID_LEN);
    return *this;
  }

 public:
  size_t sizes() { return MID_LEN; }
  const std::string& str() const { return _strsrc; }
  const char* data() const { return (const char*)_bydest; }

 private:
  char _bydest[MID_LEN] = {0};
  std::string _strsrc = "";
};
} // namespace rosetta
