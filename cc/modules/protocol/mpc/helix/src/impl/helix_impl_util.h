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
#include "cc/modules/protocol/mpc/helix/include/helix_ops_impl.h"
#include "cc/modules/protocol/utility/include/prg.h"
#include "cc/modules/common/include/utils/rtt_logger.h"

#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>
using namespace std;

namespace rosetta {

// todo, remove this to common header
namespace {
static inline string get_attr_value(const attr_type* attr, string tag, string default_value) {
  if (attr && attr->count(tag) > 0)
    default_value = attr->at(tag);
  return default_value;
}

static inline int get_attr_value(const attr_type* attr, string tag, int default_value) {
  string val = get_attr_value(attr, tag, to_string(default_value));
  return std::stoi(val);
}

} // namespace

} // namespace rosetta
