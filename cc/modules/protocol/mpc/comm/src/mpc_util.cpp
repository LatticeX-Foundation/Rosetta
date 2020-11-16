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
#include "cc/modules/protocol/mpc/comm/include/mpc_util.h"
#include "cc/modules/protocol/mpc/comm/include/mpc_helper.h"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include <chrono>
#include <random>
using namespace std;

namespace rosetta {
static std::streambuf* cout_buf = nullptr;
static std::ofstream of;
static bool redirect_io = false;

// redirect stdout to external specified log file
void redirect_stdout(const std::string& logfile) {
  cout_buf = cout.rdbuf();
  of.open(logfile);
  streambuf* fileBuf = of.rdbuf();
  cout.rdbuf(fileBuf);
  redirect_io = true;
}

void restore_stdout() {
  if (!redirect_io)
    return;
  return;
  of.flush();
  of.close();
  cout.rdbuf(cout_buf);
}
} // namespace rosetta

void convert_mpctype_to_double(const vector<mpc_t>& a, vector<double>& b) {
  b.resize(a.size());
  for (int i = 0; i < a.size(); i++)
    b[i] = MpcTypeToFloat(a[i]);
}
void convert_double_to_mpctype(const vector<double>& a, vector<mpc_t>& b) {
  b.resize(a.size());
  for (int i = 0; i < a.size(); i++)
    b[i] = FloatToMpcType(a[i]);
}

void convert_string_to_mpctype(const vector<std::string>& a, vector<mpc_t>& b, bool human) {
  size_t size = a.size();
  b.resize(size);
  if (human) {
    for (int i = 0; i < size; i++) {
      b[i] = from_hex_str<mpc_t>(a[i]);
    }
  } else {
    for (int i = 0; i < size; i++) {
      char* p = (char*)&b[i];
      memcpy(p, a[i].data(), sizeof(mpc_t));
    }
  }
}

/**
 * convert_mpctype_to_string
 * 
 * if human == true:
 * a[i].size() == sizeof(mpc_t)
 * a[i] = 0x1234567887654321UL
 * b[i] = "1234567887654321"
 * b[i].size() == 2*sizeof(mpc_t)
 * 
 * if human == false:
 * a[i].size() == sizeof(mpc_t)
 * a[i] = 0x1234567887654321UL
 * b[i].size() == sizeof(mpc_t)
 */
void convert_mpctype_to_string(const vector<mpc_t>& a, vector<std::string>& b, bool human) {
  size_t size = a.size();
  b.resize(size);
  if (human) {
    for (int i = 0; i < size; i++) {
      b[i] = get_hex_str(a[i]);
    }
  } else {
    for (int i = 0; i < size; i++) {
      char* p = (char*)&a[i];
      b[i].assign(p, p + sizeof(mpc_t));
    }
  }
}


/**
 * get_mpc_peers
 */
vector<int> get_mpc_peers(int party) {
  static vector<int> mpc_a_peers{PARTY_B, PARTY_C};
  static vector<int> mpc_b_peers{PARTY_A, PARTY_C};
  static vector<int> mpc_c_peers{PARTY_B, PARTY_A};
  
  switch (party)
  {
  case PARTY_A:
    return mpc_a_peers;
  case PARTY_B:
    return mpc_b_peers;
  case PARTY_C:
    return mpc_c_peers;
  default:
    throw std::runtime_error(string("party id invalid"));
  }
}
