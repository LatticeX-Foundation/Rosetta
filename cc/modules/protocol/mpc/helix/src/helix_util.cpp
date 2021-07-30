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
#include "cc/modules/common/include/utils/perf_stats.h"
#include "cc/modules/protocol/mpc/helix/include/helix_util.h"
#include "cc/modules/protocol/mpc/helix/include/helix_ops_impl.h"

namespace rosetta {
// step1 define the global(static) timer variables
DEFINE_GLOBAL_TIMER_COUNTER(convert_string_to_share_timer)
DEFINE_GLOBAL_TIMER_COUNTER(convert_share_to_string_timer)
DEFINE_GLOBAL_TIMER_COUNTER(convert_string_to_double_timer)

// step2 define an atexit function(static)
DEFINE_AT_EXIT_FUNCTION_BEG()
DEFINE_AT_EXIT_FUNCTION_BODY(convert_string_to_share_timer)
DEFINE_AT_EXIT_FUNCTION_BODY(convert_share_to_string_timer)
DEFINE_AT_EXIT_FUNCTION_BODY(convert_string_to_double_timer)
DEFINE_AT_EXIT_FUNCTION_END()

// step3 use ELAPSED_STATISTIC_BEG/ELAPSED_STATISTIC_END in the program
} // namespace rosetta

//! @todo optimized
namespace rosetta {
namespace helix {

// operator+
template std::vector<mpc_t> operator+(const std::vector<mpc_t>&, const std::vector<mpc_t>&);
template std::vector<double> operator+(const std::vector<double>&, const std::vector<double>&);
// operator-
template std::vector<mpc_t> operator-(const std::vector<mpc_t>&, const std::vector<mpc_t>&);
template std::vector<double> operator-(const std::vector<double>&, const std::vector<double>&);
// operator/
template std::vector<mpc_t> operator/(const std::vector<mpc_t>&, const int);
template std::vector<double> operator/(const std::vector<double>&, const int);
template std::vector<bit_t> operator/(const std::vector<bit_t>&, const int);
// flatten
template void flatten<double>(const std::vector<std::vector<double>>&, std::vector<double>&);
// flatten_r
template void flatten_r<Share>(std::vector<std::vector<Share>>&, const std::vector<Share>&, int, int);

void convert_string_to_share(
  const vector<std::string>& a,
  vector<Share>& b,
  bool human,
  void* phi) {
  ELAPSED_STATISTIC_BEG(convert_string_to_share_timer);

  size_t size = a.size();
  b.clear();
  b.resize(size);

  if (size == 0)
    return;
  string v0 = a[0];

  bool mpc_encoded = false;
  bool double_encoded = false;
  if (human) {
    if (v0.length() == sizeof(mpc_t) * 2 * 2 + 1) {
      if (v0.back() == '#') {
        mpc_encoded = true;
      }
    }
  } else {
    if (v0.length() == sizeof(mpc_t) * 2 + 1) {
      if (v0.back() == '#') {
        mpc_encoded = true;
      }
    }
  }
  if (v0.length() == sizeof(double) + 1) {
    if (v0.back() == '$') {
      double_encoded = true;
    }
  }

  if (mpc_encoded) {
    for (int i = 0; i < size; i++) {
      b[i].input(a[i], human);
    }
    ELAPSED_STATISTIC_END(convert_string_to_share_timer);
  } else {
    ELAPSED_STATISTIC_END(convert_string_to_share_timer);
    
    vector<double> fb(size);
    if (double_encoded) {
      for (int i = 0; i < a.size(); i++) {
        memcpy(&fb[i], a[i].data(), sizeof(double));
      }
    } else {
      helix_plain_string_to_double(a, fb);
    }

    //! @todo optimized, use HelixImpl*
    HelixInternal* hi = (HelixInternal*)phi;
    /**
     * Logically, arriving here means both sides are constant plaintext value.
     * It's best to call the native plaintext operation. But it will be much complex
     * in software architecture. And considering that this case rarely occurs, we handle
     * it as a constant input and convert it to ciphertext ('Share') uniformly.
    */
    // hi->Input(node_id, 0, fb, b);
    hi->ConstCommonInput(fb, b);
  }
}

void convert_share_to_string(const vector<Share>& a, vector<std::string>& b, bool human) {
  ELAPSED_STATISTIC_BEG(convert_share_to_string_timer);

  size_t size = a.size();
  b.resize(size);
  for (int i = 0; i < size; i++) {
    a[i].output(b[i], human);
    b[i].push_back('#');
  }
  ELAPSED_STATISTIC_END(convert_share_to_string_timer);
}

void helix_plain_string_to_double(const std::vector<std::string>& a, std::vector<double>& b) {
  ELAPSED_STATISTIC_BEG(convert_string_to_double_timer);
  size_t size = a.size();
  b.resize(size);
  for (int i = 0; i < size; i++) {
    //2// b[i] = std::stod(a[i]);  // 3.5~4
    b[i] = to_double(a[i].c_str()); // 2.0
    ///1//  b[i] = std::strtod(a[i].c_str(), nullptr); // 3.7
  }

  ELAPSED_STATISTIC_END(convert_string_to_double_timer);
}
void helix_double_to_plain_string(const std::vector<double>& a, std::vector<std::string>& b) {
  size_t size = a.size();
  b.resize(size);
  for (int i = 0; i < size; i++) {
    b[i] = std::to_string(a[i]);
  }
}

vector<mpc_t> operator-(const vector<bit_t>& v1, const vector<mpc_t>& v2) {
  assert(v1.size() == v2.size());
  vector<mpc_t> v(v1.size());
  for (int i = 0; i < v.size(); i++) {
    v[i] = (mpc_t)((mpc_t)v1[i] - v2[i]);
  }
  return v;
}

BitShare operator^(const BitShare& v1, const BitShare& v2) {
  BitShare v;
  v.s0.A0 = v1.s0.A0 ^ v2.s0.A0;
  v.s1.A1 = v1.s1.A1 ^ v2.s1.A1;
  return v;
}

vector<BitShare> operator^(const vector<BitShare>& v1, const vector<BitShare>& v2) {
  assert(v1.size() == v2.size());
  vector<BitShare> v(v1.size());
  for (int i = 0; i < v.size(); i++) {
    //v[i] = v1[i] ^ v2[i];
    v[i].s0.A0 = v1[i].s0.A0 ^ v2[i].s0.A0;
    v[i].s1.A1 = v1[i].s1.A1 ^ v2[i].s1.A1;
  }
  return v;
}

int adversary(int p) {
  if (p == PARTY_0)
    return PARTY_1;
  if (p == PARTY_1)
    return PARTY_0;
  assert(false && "not supports p==PARTY_2 in 3PC");
  return p;
}

} // namespace helix
} // namespace rosetta
