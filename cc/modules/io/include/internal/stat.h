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

#include "cc/modules/io/include/internal/comm.h"

namespace rosetta {
namespace io {

struct NetStat_st {
  /**
   * statistics
   */
  atomic<uint64_t> bytes_sent{0};
  atomic<uint64_t> bytes_received{0};
  atomic<uint64_t> message_sent{0};
  atomic<uint64_t> message_received{0};

  void reset() {
    bytes_sent.store(0);
    bytes_received.store(0);
    message_sent.store(0);
    message_received.store(0);
  }
};

/**
 * Network statistics
 */
class NetStat {
 public:
  NetStat() = default;
  NetStat(const NetStat_st& ns_st) {
    bytes_sent = ns_st.bytes_sent.load();
    bytes_received = ns_st.bytes_received.load();
    message_sent = ns_st.message_sent.load();
    message_received = ns_st.message_received.load();
  }

  friend NetStat operator-(const NetStat& ns1, const NetStat& ns2) {
    NetStat ns;
    // clang-format off
    ns.bytes_sent       = ns1.bytes_sent        - ns2.bytes_sent;
    ns.bytes_received   = ns1.bytes_received    - ns2.bytes_received;
    ns.message_sent     = ns1.message_sent      - ns2.message_sent;
    ns.message_received = ns1.message_received  - ns2.message_received;
    // clang-format on
    return ns;
  }

  friend NetStat operator+(const NetStat& ns1, const NetStat& ns2) {
    NetStat ns;
    // clang-format off
    ns.bytes_sent       = ns1.bytes_sent        + ns2.bytes_sent;
    ns.bytes_received   = ns1.bytes_received    + ns2.bytes_received;
    ns.message_sent     = ns1.message_sent      + ns2.message_sent;
    ns.message_received = ns1.message_received  + ns2.message_received;
    // clang-format on
    return ns;
  }

  friend ostream& operator<<(ostream& os, const NetStat& ns) {
    os << ns.fmt_string();
    return os;
  }

  string fmt_string() const {
    stringstream sss;
    sss << " bytes sent:" << setw(15) << bytes_sent;
    sss << " bytes recv:" << setw(15) << bytes_received;
    sss << " msges sent:" << setw(06) << message_sent;
    sss << " msges recv:" << setw(06) << message_received;
    return sss.str();
  }

  void print(string str = "") const { cout << str << fmt_string() << endl; }

 private:
  uint64_t bytes_sent = 0;
  uint64_t bytes_received = 0;
  uint64_t message_sent = 0;
  uint64_t message_received = 0;
};

} // namespace io
} // namespace rosetta