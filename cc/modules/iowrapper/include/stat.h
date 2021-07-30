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

#include <sstream>
#include <string>
#include <atomic>

namespace rosetta {

struct NetStat_st {
  /**
   * statistics
   */
  std::atomic<uint64_t> bytes_sent{0};
  std::atomic<uint64_t> bytes_received{0};
  std::atomic<uint64_t> message_sent{0};
  std::atomic<uint64_t> message_received{0};
  void reset();
};

/**
 * Network statistics
 */
class NetStat {
 public:
  NetStat() = default;
  NetStat(const NetStat_st& ns_st);

  friend NetStat operator-(const NetStat& ns1, const NetStat& ns2);
  friend NetStat operator+(const NetStat& ns1, const NetStat& ns2);
  friend std::ostream& operator<<(std::ostream& os, const NetStat& ns);
  std::string fmt_string() const;
  void print(std::string str = "") const;

 public:
  uint64_t bytes_sent() { return bytes_sent_; }
  uint64_t bytes_received() { return bytes_received_; }
  uint64_t message_sent() { return message_sent_; }
  uint64_t message_received() { return message_received_; }

 private:
  uint64_t bytes_sent_ = 0;
  uint64_t bytes_received_ = 0;
  uint64_t message_sent_ = 0;
  uint64_t message_received_ = 0;
};

} // namespace rosetta
