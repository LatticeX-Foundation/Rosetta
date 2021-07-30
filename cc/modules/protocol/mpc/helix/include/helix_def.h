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
#include "cc/modules/protocol/mpc/comm/include/mpc_common.h"
#include "cc/modules/common/include/utils/helper.h"

#include <vector>
#include <string>
#include <iostream>
#include <iomanip>

namespace rosetta {
namespace helix {

using namespace rosetta;

/**
 * sharing data type
 *
 * for each real data X (single one), its sharing:
 * P0 holds (deltaX, A0)    assign ---> {.s0=deltaX, .s1=A0}
 * P1 holds (deltaX, A1)    assign ---> {.s0=deltaX, .s1=A1}
 * P2 holds (A0, A1)        assign ---> {.s0=A0,     .s1=A1}
 * deltaX = X - A0 - A1, A = A0 + A1
 *  
 * s0 --> P0:deltaX P1:deltaX P2:A0
 * s1 --> P0:A0     P1:A1     P2:A1
 *
 * note that for bitshare, everything still hold except the '-'operation will be XOR.
 */
template <typename T>
class ShareT {
 public:
  union {
    T delta = 0;
    T A0;
    T B0;
  } s0;
  union {
    T A0 = 0;
    T A1;
    T B0;
    T B1;
  } s1;
  //! @todo will be replaced `s0,s1` as `vector<mpc_t> s`;

 public:
  /**
  * output Share to string
  * \param human if human readable
  */
  void output(std::string& s, bool human = false) const {
    s.clear();
    if (human) {
      s += get_hex_str(s0.A0);
      s += get_hex_str(s1.A1);
    } else {
      char* p = (char*)&s0.A0;
#if 0
      s.append(p, p + sizeof(T));
      p = (char*)&s1.A1;
      s.append(p, p + sizeof(T));
#else
      s.resize(sizeof(T) * 2);
      memcpy((char*)s.data(), p, sizeof(T) * 2);
#endif
    }
  }
  /**
  * input Share from string
  * \param human if human readable
  */
  void input(const std::string& s, bool human = false) {
    if (human) {
      // Note[George]: the const '2' is due to hex coding
      s0.A0 = from_hex_str<T>(std::string(s.begin(), s.begin() + 2 * sizeof(T)));
      s1.A1 = from_hex_str<T>(std::string(s.begin() + 2 * sizeof(T), s.begin() + 4 * sizeof(T)));
    } else {
      char* p = (char*)&s0.A0;
#if 0
      memcpy(p, s.data(), sizeof(T));
      p = (char*)&s1.A1;
      memcpy(p, s.data() + sizeof(T), sizeof(T));
#else
      memcpy(p, s.data(), sizeof(T) * 2);
#endif
    }
  }

  friend std::ostream& operator<<(std::ostream& os, const ShareT& share) {
    std::string s;
    share.output(s, true);
    os << s;
    return os;
  }

 public:
  //! @todo some basic operations(add/sub/mul/...) here
};

#if 0
// why not?
template <typename T>
inline std::ostream& operator<<(std::ostream& os, const vector<rosetta::helix::ShareT<T>>& shares) {
  for (int i = 0; i < shares.size(); i++) {
    os << shares[i] << " ";
    if ((i & 0x7) == 0x7)
      os << endl;
  }
  return os;
}
#endif

class Share : public ShareT<mpc_t> {};
class BitShare : public ShareT<bit_t> {};

} // namespace helix
} // namespace rosetta

inline std::ostream& operator<<(std::ostream& os, const vector<rosetta::helix::Share>& shares) {
  for (int i = 0; i < shares.size(); i++) {
    if ((i & 0x3) == 0)
      os << std::setw(4) << i << " ";
    os << shares[i] << " ";
    if ((i & 0x3) == 0x3)
      os << endl;
  }
  return os;
}
