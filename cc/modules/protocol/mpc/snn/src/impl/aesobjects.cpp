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
#include "cc/modules/protocol/mpc/snn/include/opsets_base.h"

namespace rosetta {
namespace mpc {
AESKeyStrings AESKeyStrings::keys; // global aes keys
void AESKeyStrings::print() {
  cout << " key_0:" << key_0 << endl;
  cout << " key_a:" << key_a << endl;
  cout << " key_b:" << key_b << endl;
  cout << " key_c:" << key_c << endl;
  cout << "key_ab:" << key_ab << endl;
  cout << "key_ac:" << key_ac << endl;
  cout << "key_bc:" << key_bc << endl;
  cout << "key_cd:" << key_cd << endl;
}

std::set<msg_id_t> AESObjects::msig_objs_;
mutex AESObjects::msgid_aesobjs_mtx_;
map<msg_id_t, std::shared_ptr<AESObjects>> AESObjects::msgid_aesobjs_;

int AESObjects::init_aes(int pid, const msg_id_t& msg_id) {
  ///! xor local key string with global key and msg id
  string skey = msg_id.str(); // msg id
  auto fxor = [skey](const std::string& gstr) -> std::string {
    std::string tkey = gstr; // temp key
    tkey.resize(32);
#if MPC_DEBUG_USE_FIXED_AESKEY
    return tkey;
#endif
    // todo: optimize, this way not unique
    for (int i = 0; i < (int)skey.length(); i++) {
      int index = i % (tkey.length());
      int a = (int)tkey[index];
      int b = (int)skey[i];
      int c = (a ^ b) % 16;
      if (c < 10)
        tkey[index] = (char)c + '0';
      else
        tkey[index] = (char)(c - 10) + 'a';
    }
    return tkey;
  };
  std::string k0 = fxor(AESKeyStrings::keys.key_0);
  std::string ka = fxor(AESKeyStrings::keys.key_a);
  std::string kb = fxor(AESKeyStrings::keys.key_b);
  std::string kc = fxor(AESKeyStrings::keys.key_c);
  std::string kab = fxor(AESKeyStrings::keys.key_ab);
  std::string kac = fxor(AESKeyStrings::keys.key_ac);
  std::string kbc = fxor(AESKeyStrings::keys.key_bc);
  std::string kcd = fxor(AESKeyStrings::keys.key_cd);

  /*********************** AES SETUP and SYNC *************************/
  // P0 --> "KEYS":["keyA","keyAB","keyAC","null"]
  // P1 --> "KEYS":["keyB","keyAB","null","keyBC"]
  // P2 --> "KEYS":["keyC","keyCD","keyAC","keyBC"]
  // clang-format off
    vector<vector<string>> KEY = {
      {ka, kab, kac, ""}, 
      {kb, kab, "", kbc},
      {kc, kcd, kac, kbc}
    };
    vector<vector<string>> KSTR = {
      {"A", "AB", "AC", "NUL"}, 
      {"B", "AB", "NUL", "BC"},
      {"C", "CD", "AC", "BC"}
    };
  // clang-format on

#define INIT_AES_OBJECT(obj, i)                                                       \
  if (false) {                                                                         \
    cout << "P" << pid << " key[" << i << "]: " << KEY[pid][i] << " of " #obj " key(" \
         << KSTR[pid][i] << ")" << endl;                                              \
  }                                                                                   \
  obj = std::make_shared<AESObject>();                                                \
  obj->Init(KEY[pid][i])

  INIT_AES_OBJECT(aes_indep, 0);
  INIT_AES_OBJECT(aes_common, 1);
  INIT_AES_OBJECT(aes_a_1, 2);
  INIT_AES_OBJECT(aes_b_1, 2);
  INIT_AES_OBJECT(aes_c_1, 2);
  INIT_AES_OBJECT(aes_a_2, 3);
  INIT_AES_OBJECT(aes_b_2, 3);
#undef INIT_AES_OBJECT

  aes_randseed = std::make_shared<AESObject>();
  aes_randseed->Init(k0);

  return 0;
}

} // namespace mpc
} // namespace rosetta
