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
#include "cc/modules/protocol/mpc/helix/include/helix_internal.h"

#include <iostream>
#include <vector>
#include <cassert>
using namespace std;

namespace rosetta {
namespace helix {

/**
 * RevealAndPrint*, only for test & debug
 */
void HelixInternal::RevealAndPrint(const vector<Share>& X, std::string msg) {
  vector<double> plain(X.size());
  Reveal(X, plain, encode_reveal_mask(7));
  if (party_id() == PARTY_0)
    print_vec(plain, 16, msg);
}
void HelixInternal::RevealAndPrint2(const vector<Share>& X, std::string msg) {
  vector<mpc_t> plain(X.size());
  Reveal(X, plain, encode_reveal_mask(7));
  if (party_id() == PARTY_0)
    print_vec(plain, 16, msg);
}
void HelixInternal::RevealAndPrint(const vector<BitShare>& X, std::string msg) {
  vector<bit_t> plain(X.size());
  Reveal(X, plain, encode_reveal_mask(7));
  if (party_id() != PARTY_0)
    return;
  cout << msg << " ";
  for (int i = 0; i < 80; i++) {
    if (i == plain.size())
      break;
    if (i > 0) {
      if (i % 8 == 0)
        cout << "  ";
      else if (i % 4 == 0)
        cout << " ";
    }
    cout << to_string(plain[i]);
  }
  cout << endl;
}
void HelixInternal::RevealAndPrint(const Share& X, std::string msg) {
  vector<mpc_t> plain(1);
  Reveal({X}, plain, encode_reveal_mask(7));
  cout << msg << " " << to_readable_dec(plain[0]) << endl;
}
void HelixInternal::RevealAndPrint(const BitShare& X, std::string msg) {
  vector<bit_t> plain(1);
  Reveal({X}, plain, encode_reveal_mask(7));
  cout << msg << " " << to_string(plain[0]) << endl;
}

} // namespace helix
} // namespace rosetta