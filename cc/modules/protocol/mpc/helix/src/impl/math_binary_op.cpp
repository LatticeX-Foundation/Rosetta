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
#include "helix_impl_util.h"

/**
 * Binary OP(s)
 * Add/Sub/Mul/Div/Compare(s)
 * 
 * \param scale, true, if op is compare operation
 */
#define HELIX_PROTOCOL_BINARY_OP_(op, scale)                                                      \
  int HelixOpsImpl::op(                                                                           \
    const vector<string>& a, const vector<string>& b, vector<string>& c, const attr_type* attr) { \
    bool lh_is_const = get_attr_value(attr, "lh_is_const", 0) == 1;                               \
    bool rh_is_const = get_attr_value(attr, "rh_is_const", 0) == 1;                               \
    assert(!(lh_is_const && rh_is_const));                                                        \
                                                                                                  \
    vector<Share> shareA, shareB, shareC;                                                         \
    vector<double> doubleA, doubleB;                                                              \
                                                                                                  \
    if (lh_is_const) {                                                                            \
      helix_plain_string_to_double(a, doubleA);                                                   \
      helix_convert_string_to_share(b, shareB);                                                   \
      hi->op(doubleA, shareB, shareC);                                                            \
    } else if (rh_is_const) {                                                                     \
      helix_convert_string_to_share(a, shareA);                                                   \
      helix_plain_string_to_double(b, doubleB);                                                   \
      hi->op(shareA, doubleB, shareC);                                                            \
    } else {                                                                                      \
      helix_convert_string_to_share(a, shareA);                                                   \
      helix_convert_string_to_share(b, shareB);                                                   \
      hi->op(shareA, shareB, shareC);                                                             \
    }                                                                                             \
                                                                                                  \
    if (scale) {                                                                                  \
      hi->Scale(shareC, hi->context_->FLOAT_PRECISION);                                    \
    }                                                                                             \
    helix_convert_share_to_string(shareC, c);                                                     \
    return 0;                                                                                     \
  }

#define HELIX_PROTOCOL_BINARY_OP(op) HELIX_PROTOCOL_BINARY_OP_(op, false)
#define HELIX_PROTOCOL_COMPARE_OP(op) HELIX_PROTOCOL_BINARY_OP_(op, true)
#define HELIX_PROTOCOL_LOGICAL_OP(op) HELIX_PROTOCOL_BINARY_OP_(op, false)

namespace rosetta {

HELIX_PROTOCOL_BINARY_OP(Add)
HELIX_PROTOCOL_BINARY_OP(Sub)
HELIX_PROTOCOL_BINARY_OP(Mul)
HELIX_PROTOCOL_BINARY_OP(Div)
HELIX_PROTOCOL_BINARY_OP(Truediv)
HELIX_PROTOCOL_BINARY_OP(Floordiv)
HELIX_PROTOCOL_BINARY_OP(Reciprocaldiv)

HELIX_PROTOCOL_COMPARE_OP(Less)
HELIX_PROTOCOL_COMPARE_OP(LessEqual)
HELIX_PROTOCOL_COMPARE_OP(Equal)
HELIX_PROTOCOL_COMPARE_OP(NotEqual)
HELIX_PROTOCOL_COMPARE_OP(Greater)
HELIX_PROTOCOL_COMPARE_OP(GreaterEqual)

HELIX_PROTOCOL_LOGICAL_OP(AND)
HELIX_PROTOCOL_LOGICAL_OP(OR)
HELIX_PROTOCOL_LOGICAL_OP(XOR)

} // namespace rosetta
