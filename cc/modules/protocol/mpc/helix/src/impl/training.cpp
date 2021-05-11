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

#include <iostream>
#include <string>
#include <vector>

using namespace std;
namespace rosetta {
int HelixOpsImpl::Reveal(const vector<string>& a, vector<string>& c, const attr_type* attr_info) {
  int p = get_attr_value(attr_info, "receive_party", 7);
  vector<Share> shareA;
  helix_convert_string_to_share(a, shareA);
  vector<double> fc;
  hi->Reveal(shareA, fc, p);
  helix_double_to_plain_string(fc, c);
  return 0;
}

int HelixOpsImpl::Reveal(const vector<string>& a, vector<double>& c, const attr_type* attr_info) {
  int p = get_attr_value(attr_info, "receive_party", 7);
  vector<Share> shareA;
  helix_convert_string_to_share(a, shareA);
  hi->Reveal(shareA, c, p);
  return 0;
}

/**
 * @desc: This is for Tensorflow's SaveV2 Op.
 */
int HelixOpsImpl::ConditionalReveal(
  vector<string>& in_vec,
  vector<string>& out_cipher_vec,
  vector<double>& out_plain_vec) {
  // not to reveal plaintext by default
  int save_mode = 0;
  if (op_config_map.find("save_mode") != op_config_map.end()) {
    log_debug << op_config_map["save_mode"] << endl;
    save_mode = stoi(op_config_map["save_mode"]);
  }
  // all ciphertext
  if (0 == save_mode) {
    out_cipher_vec = in_vec;
    out_plain_vec.clear();
    return 0;
  }
  // only plaintext for selected parties
  vector<Share> shareA;
  helix_convert_string_to_share(in_vec, shareA);
  vector<double> c;
  hi->Reveal(shareA, c, save_mode);
  int my_party_id = hi->party_id();
  if (
    ((save_mode & 1) && my_party_id == 0) || ((save_mode & 2) && my_party_id == 1) ||
    ((save_mode & 4) && my_party_id == 2)) {
    out_plain_vec.swap(c);
  } else {
    out_plain_vec.clear();
  }
  out_cipher_vec.clear();

  return 0;
}

} // namespace rosetta
