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

#include <set>
#include <string>
using namespace std;

namespace rosetta {

class MpcPRGKeys {
 public:
  static MpcPRGKeys keys; // store initialized keys, init in `init_aeskeys()`
  // clang-format off
  std::string key_prg   = "FFFF0000000000000000000000000000";
  std::string key_prg01 = "01000000000000000000000000000000";
  std::string key_prg02 = "02000000000000000000000000000000";
  std::string key_prg12 = "12000000000000000000000000000000";
  std::string key_prg0  = "FF000000000000000000000000000000";
  std::string key_prg1  = "FF010000000000000000000000000000";
  std::string key_prg2  = "FF020000000000000000000000000000";
  // clang-format on
  void reset();
  void fmt_print();
};

//! @todo remove to public, for Helix and SNN
class MpcPRGObjs {
  static std::set<msg_id_t> msig_objs_;
  static std::mutex msgid_mpc_prgs_mtx_;
  static std::map<msg_id_t, std::shared_ptr<MpcPRGObjs>> msgid_mpc_prgs_;

 public:
  /**
   * prg   // P0 and P1 and P2
   * prg01 // P0 and P1
   * prg02 // P0 and P2
   * prg12 // P1 and P2
   * prg0  // P0
   * prg1  // P1
   * prg2  // P2
   */
  std::shared_ptr<RttPRG> prg = nullptr;
  std::shared_ptr<RttPRG> prg01 = nullptr;
  std::shared_ptr<RttPRG> prg02 = nullptr;
  std::shared_ptr<RttPRG> prg12 = nullptr;
  std::shared_ptr<RttPRG> prg0 = nullptr;
  std::shared_ptr<RttPRG> prg1 = nullptr;
  std::shared_ptr<RttPRG> prg2 = nullptr;

 public:
  static std::shared_ptr<MpcPRGObjs> Get(int player, const msg_id_t& msg_id);

 private:
  int init(int player, const msg_id_t& msg_id);
};
} // namespace rosetta
