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
#include <cmath>
#include <thread>

using namespace std;

namespace rosetta {
namespace helix {
// send
template int HelixInternal::send<mpc_t>(int, const std::vector<mpc_t>&, size_t); 
template int HelixInternal::send<unsigned char>(int, const std::vector<unsigned char>&, size_t); 
template int HelixInternal::send<unsigned char>(const std::string&, const std::vector<unsigned char>&, size_t); 
// recv
template int HelixInternal::recv<mpc_t>(int, std::vector<mpc_t>&, size_t); 
template int HelixInternal::recv<unsigned char>(int, std::vector<unsigned char>&, size_t); 
template int HelixInternal::recv<unsigned char>(const std::string&, std::vector<unsigned char>&, size_t); 
// Reveal_
template void HelixInternal::Reveal_<Share, mpc_t>(const std::vector<Share>&, vector<mpc_t>& , const std::vector<std::string>&); 
template void HelixInternal::Reveal_<Share, mpc_t>(const std::vector<Share>&, vector<mpc_t>& , const std::string&); 
template void HelixInternal::Reveal_<BitShare, mpc_t>(const std::vector<BitShare>&, vector<mpc_t>& , const std::vector<std::string>&); 
template void HelixInternal::Reveal_<BitShare, mpc_t>(const std::vector<BitShare>&, vector<mpc_t>& , const std::string&); 
// Input 
template void HelixInternal::Input_<mpc_t, Share>(const std::string& , const std::vector<mpc_t>& X, std::vector<Share>&);
template void HelixInternal::Input_<bit_t, BitShare>(const std::string& , const std::vector<bit_t>& X, std::vector<BitShare>&);
// Mul_
template void HelixInternal::Mul_<bit_t, BitShare>(const std::vector<BitShare>& , const std::vector<BitShare>&, std::vector<BitShare>& , int , bool); 
template void HelixInternal::Mul_<mpc_t, Share>(const std::vector<Share>& , const std::vector<Share>&, std::vector<Share>& , int , bool); 
/**
 * Trunc, locally
 */
void HelixInternal::Trunc(vector<mpc_t>& X, size_t size, size_t power) {
  if (player == PARTY_0) {
    for (size_t i = 0; i < size; ++i)
      X[i] = static_cast<mpc_t>(static_cast<signed_mpc_t>(X[i]) >> power);
  }

  if (player == PARTY_1) {
    for (size_t i = 0; i < size; ++i)
      X[i] = -static_cast<mpc_t>(static_cast<signed_mpc_t>(-X[i]) >> power);
  }
  AUDIT("id:{}, P{} Trunc output(mpc_t){}",       msgid.get_hex(), player, Vector<mpc_t>(X));
}

void HelixInternal::Trunc_many(vector<mpc_t>& X, size_t size, vector<size_t> power_list) {
  assert(X.size() == power_list.size());
  if (player == PARTY_0) {
    for (size_t i = 0; i < size; ++i)
      X[i] = static_cast<mpc_t>(static_cast<signed_mpc_t>(X[i]) >> power_list[i]);
  }

  if (player == PARTY_1) {
    for (size_t i = 0; i < size; ++i)
      X[i] = -static_cast<mpc_t>(static_cast<signed_mpc_t>(-X[i]) >> power_list[i]);
  }
  AUDIT("id:{}, P{} Trunc_many output(mpc_t){}",       msgid.get_hex(), player, Vector<mpc_t>(X));
}

/**
 * Trunc Share
 *
 * Rounds: 1
 * Communication: 2*size*\ell
 */
void HelixInternal::Trunc_many(vector<Share>& X, size_t size, vector<size_t> power_list) {
  assert(X.size() == power_list.size());
#if FIX_SHARE_TRUNCATION_PROBABILISTIC_ERROR
  // Fix the serious truncation error by restricting any shares <X_0, X_1>
  // to <X-r, r> s.t r \in [2^{-62}, 2^{62}]. 
  //  In this way, the serious error that MSB(X) = b and MSB<X-r> == MSB<r> == (1-b)
  // can be avoided! 
  // check section5.1.1 in ABY3 paper for detailed discussion.
  // cout << "New trunc!" << endl;
  vector<mpc_t> tmp_rand_share(size, 0);
  vector<mpc_t> Z0(size, 0), Z1(size, 0);
  if (is_helper()) {
    PRF2(tmp_rand_share, size);
    AUDIT("id:{}, P{} Trunc_many PRF2, tmp_rand_share(mpc_t){}",       msgid.get_hex(), player, Vector<mpc_t>(tmp_rand_share));
    PRF12(Z1, size);
    AUDIT("id:{}, P{} Trunc_many, P1 and P2 generat Z1(mpc_t){}", msgid.get_hex(), player, Vector<mpc_t>(Z1));
    vector<mpc_t> A0_minus_r(size, 0);
    for (int i = 0; i < size; ++i) {
      // make sure r \in [2^{-62}, 2^{62}
      tmp_rand_share[i] = (mpc_t)((signed_mpc_t)tmp_rand_share[i] >> 1);
      A0_minus_r[i] = X[i].s0.A0 - tmp_rand_share[i];
    }
    
    
    send(PARTY_1, A0_minus_r, size);
    // Trunc(tmp_rand_share, size, power);
    for (size_t i = 0; i < size; ++i) {
      tmp_rand_share[i] = static_cast<mpc_t>(static_cast<signed_mpc_t>(tmp_rand_share[i]) >> power_list[i]);
    }
    Z0 = tmp_rand_share - Z1;
    AUDIT("id:{}, P{} Trunc_many compute Z0=tmp_rand_share-Z1, Z0(mpc_t){}", msgid.get_hex(), player, Vector<mpc_t>(Z0));

    send(PARTY_0, Z0, size);
    AUDIT("id:{}, P{} Trunc_many SEND to P{}, Z0(mpc_t){}", msgid.get_hex(), player, PARTY_0, Vector<mpc_t>(Z0));
    for (int i = 0; i < size; i++) {
      X[i].s0.A0 = Z0[i];
      X[i].s1.A1 = Z1[i];
    }
  }
  
  if (player == PARTY_1) {
    PRF12(Z1, size);
    AUDIT("id:{}, P{} Trunc_many generat Z1(mpc_t){}", msgid.get_hex(), player, Vector<mpc_t>(Z1));

    recv(PARTY_2, tmp_rand_share, size);
    AUDIT("id:{}, P{} Trunc_many RECV from P{}, tmp_rand_share(mpc_t){}", msgid.get_hex(), player, PARTY_2, Vector<mpc_t>(tmp_rand_share));

    vector<mpc_t> X_minus_rand(size, 0);
    for (size_t i = 0; i < size; ++i) {
      X_minus_rand[i] = X[i].s0.delta + X[i].s1.A1;
      X_minus_rand[i] = X_minus_rand[i] + tmp_rand_share[i];
    }

    // Trunc(X_minus_rand, size, power);
    for (size_t i = 0; i < size; ++i) {
      X_minus_rand[i] = -static_cast<mpc_t>(static_cast<signed_mpc_t>(-X_minus_rand[i]) >> power_list[i]);
    }
    send(PARTY_0, X_minus_rand, size);
    AUDIT("id:{}, P{} Trunc_many SEND to P{}, X_minus_rand(mpc_t){}", msgid.get_hex(), player, PARTY_2, Vector<mpc_t>(X_minus_rand));

    for (int i = 0; i < size; i++) {
      X[i].s0.delta = X_minus_rand[i];
      X[i].s1.A1 = Z1[i];
    }
  }

  if (player == PARTY_0) {
    recv(PARTY_1, tmp_rand_share, size);
    AUDIT("id:{}, P{} Trunc_many RECV from P{}, tmp_rand_share(mpc_t){}", msgid.get_hex(), player, PARTY_1, Vector<mpc_t>(tmp_rand_share));

    recv(PARTY_2, Z0, size);
    AUDIT("id:{}, P{} Trunc_many RECV from P{}, Z0(mpc_t){}", msgid.get_hex(), player, PARTY_2, Vector<mpc_t>(Z0));
    for (int i = 0; i < size; i++) {
      X[i].s0.delta = tmp_rand_share[i];
      X[i].s1.A0 = Z0[i];
    }
  }
  return;
#else
  vector<mpc_t> T0(size, 0), T1(size, 0); // X = T0 + T1
  if (player == PARTY_0) {
    for (size_t i = 0; i < size; ++i) {
      T0[i] = X[i].s0.delta + X[i].s1.A0;
    }
    Trunc_many(T0, size, power_list);
  }

  if (player == PARTY_1) {
    for (size_t i = 0; i < size; ++i) {
      T1[i] = X[i].s1.A1;
    }
    Trunc_many(T1, size, power_list);
  }

  // Zi
  vector<mpc_t> Z0(size, 0), Z1(size, 0);
  PRF02(Z0, size);
  if ((player == PARTY_0) || (player == PARTY_2)) {
    AUDIT("id:{}, P{} Trunc_many, P0 and P2 generat Z0(mpc_t){}", msgid.get_hex(), player, Vector<mpc_t>(Z0));
  }
  PRF12(Z1, size);
  if ((player == PARTY_1) || (player == PARTY_2)) {
    AUDIT("id:{}, P{} Trunc_many, P1 and P2 generat Z1(mpc_t){}", msgid.get_hex(), player, Vector<mpc_t>(Z1));
  }

  vector<mpc_t> X0(size, 0), X1(size, 0);
  if (is_primary()) {
    if (player == PARTY_0) {
      X0 = T0 - Z0;
      send(PARTY_1, X0, size);
      AUDIT("id:{}, P{} Trunc_many SEND to P{}, X0(mpc_t){}", msgid.get_hex(), player, PARTY_1, Vector<mpc_t>(X0));

      recv(PARTY_1, X1, size);
      AUDIT("id:{}, P{} Trunc_many RECV from P{}, X1(mpc_t){}", msgid.get_hex(), player, PARTY_1, Vector<mpc_t>(X1));
    } else {
      X1 = T1 - Z1;
      recv(PARTY_0, X0, size);
      AUDIT("id:{}, P{} Trunc_many RECV from P{}, X0(mpc_t){}", msgid.get_hex(), player, PARTY_0, Vector<mpc_t>(X0));

      send(PARTY_0, X1, size);
      AUDIT("id:{}, P{} Trunc_many RECV from P{}, X1(mpc_t){}", msgid.get_hex(), player, PARTY_0, Vector<mpc_t>(X1));
    }
  }

  vector<mpc_t> deltaX = X0 + X1;
  for (int i = 0; i < size; i++) {
    if (is_helper()) {
      X[i].s0.A0 = Z0[i];
      X[i].s1.A1 = Z1[i];
    } else {
      X[i].s0.delta = deltaX[i];
      if (player == PARTY_0)
        X[i].s1.A0 = Z0[i];
      else
        X[i].s1.A1 = Z1[i];
    }
  }
#endif
  AUDIT("id:{}, P{} Trunc_many output(Share){}",       msgid.get_hex(), player, Vector<Share>(X));
}

/**
 * Trunc Share
 *
 * Rounds: 1
 * Communication: 2*size*\ell
 */
void HelixInternal::Trunc(vector<Share>& X, size_t size, size_t power) {
  // no need to trunc
  if (power <= 0) {
    return;
  }

  vector<size_t> power_list(size, power);
  Trunc_many(X, size, power_list);
}
/**
 * dummy, for template use
 */
void HelixInternal::Trunc(vector<bit_t>& X, size_t size, size_t d) {}
void HelixInternal::Trunc(vector<BitShare>& X, size_t size, size_t d) {}

/**
 * Reveal, arithmetic
 */
void HelixInternal::Reveal(const vector<Share>& X, vector<mpc_t>& plain, const string& nodes) {
  AUDIT("id:{}, P{} Reveal, input(Share){}",       msgid.get_hex(), player, Vector<Share>(X));
  Reveal_(X, plain, nodes);
  AUDIT("id:{}, P{} Reveal, output(mpc_t, plain){}", msgid.get_hex(), player, Vector<mpc_t>(plain));
}
void HelixInternal::Reveal(const vector<Share>& X, vector<double>& plain, const string& nodes) {
  AUDIT("id:{}, P{} Reveal, input(Share){}",                 msgid.get_hex(), player, Vector<Share>(X));
  vector<mpc_t> mpc_plain;
  Reveal_(X, mpc_plain, nodes);
  convert_fixpoint_to_plain(mpc_plain, plain, GetMpcContext()->FLOAT_PRECISION);
  AUDIT("id:{}, P{} Reveal, output(double, plain){}", msgid.get_hex(), player, Vector<double>(plain));
}
void HelixInternal::Reveal(const vector<Share>& X, vector<mpc_t>& plain, const vector<string>& nodes) {
  AUDIT("id:{}, P{} Reveal, input(Share){}",                 msgid.get_hex(), player, Vector<Share>(X));
  Reveal_(X, plain, nodes);
  AUDIT("id:{}, P{} Reveal, output(mpc_t, plain){}", msgid.get_hex(), player, Vector<mpc_t>(plain));
}

void HelixInternal::Reveal(const vector<Share>& X, vector<double>& plain, const vector<string>& nodes) {
  AUDIT("id:{}, P{} Reveal, input(Share){}",                 msgid.get_hex(), player, Vector<Share>(X));
  vector<mpc_t> mpc_plain;
  Reveal_(X, mpc_plain, nodes);
  convert_fixpoint_to_plain(mpc_plain, plain, GetMpcContext()->FLOAT_PRECISION);
  AUDIT("id:{}, P{} Reveal, output(double, plain){}", msgid.get_hex(), player, Vector<double>(plain));
}

/**
 * Reveal, binary
 */
void HelixInternal::Reveal(const vector<BitShare>& X, vector<bit_t>& plain, const string& nodes) {
  AUDIT("id:{}, P{} Reveal, input(BitShare){}",         msgid.get_hex(), player, Vector<BitShare>(X));
  Reveal_(X, plain, nodes);
  AUDIT("id:{}, P{} Reveal, output(bit_t){}", msgid.get_hex(), player, Vector<bit_t>(plain));
}

void HelixInternal::SyncCiphertext(const vector<Share>& in_vec, vector<Share>& out_vec, const map<string, int>& ciphertext_nodes) {
  string current_node_id = io->GetCurrentNodeId();
  vector<mpc_t> in_vec_s(2 * in_vec.size(), 0);
  int j = 0;
  for (int i = 0; i < in_vec.size(); i++) {
    in_vec_s[j++] = in_vec[i].s0.A0;
    in_vec_s[j++] = in_vec[i].s1.A1;
  }
  vector<mpc_t> out_vec_s(2 * out_vec.size(), 0);
   bool recv_flag = false;
  for (auto iter = ciphertext_nodes.begin(); iter != ciphertext_nodes.end(); iter++) {
    if (player == iter->second && iter->first != current_node_id) {
      send(iter->first, in_vec_s, in_vec_s.size());
    } else if (iter->first == current_node_id && player != iter->second) {
      recv(iter->second, out_vec_s, out_vec_s.size());
      recv_flag = true;
    }
  }
  if (recv_flag) {
    int j = 0;
    for (int i = 0; i < out_vec.size(); i++) {
      out_vec[i].s0.A0 = out_vec_s[j++];
      out_vec[i].s1.A1 = out_vec_s[j++];
    }
  }
}

/**
 * Input, binary
 */
void HelixInternal::Input(const string& node_id,  const vector<bit_t>& X, vector<BitShare>& shareX) {
  Input_(node_id, X, shareX);
}

/**
 * Input, arithmetic
 *
 * @see Input_
 */
void HelixInternal::Input(const string& node_id, const vector<mpc_t>& X, vector<Share>& shareX) {
  Input_(node_id, X, shareX);
}
void HelixInternal::Input(const string& node_id, const vector<double>& X, vector<Share>& shareX) {
  vector<mpc_t> mpcX;
  convert_plain_to_fixpoint(X, mpcX, GetMpcContext()->FLOAT_PRECISION);
  Input(node_id, mpcX, shareX);
}

/** 
 * @note: this input X should be a common SAME value for each party!
 * To convert a constant input X to 'Share' format,
 * we present it in the following legal way, locally:
 * P0: (X, 0)
 * P1: (X, 0)
 * P2: (0, 0) 
 */
void HelixInternal::ConstCommonInput(const vector<mpc_t>& X, vector<Share>& shareX) {
  AUDIT("id:{}, P{} ConstCommonInput input X(mpc_t){}", msgid.get_hex(), player, Vector<mpc_t>(X));
  int ele_size = X.size();
  shareX.resize(ele_size);
  // each party sets his share
  for (int i = 0; i < ele_size; i++) {
    if (is_helper()) {
      shareX[i].s0.A0 = 0;
      shareX[i].s1.A1 = 0;
    } else {
      shareX[i].s0.delta = X[i];
      if (player == PARTY_0)
        shareX[i].s1.A0 = 0;
      else
        shareX[i].s1.A1 = 0;
    }
  }

  AUDIT("id:{}, P{} ConstCommonInput output shareX(Share){}", msgid.get_hex(), player, Vector<Share>(shareX));
}

void HelixInternal::ConstCommonInput(const vector<double>& X, vector<Share>& shareX) {
  AUDIT("id:{}, P{} ConstCommonInput input X(double){}", msgid.get_hex(), player, Vector<double>(X));
  vector<mpc_t> mpcX;
  // print_vec(X, 10, "debug calling ConstCommonInput");
  convert_plain_to_fixpoint(X, mpcX, GetMpcContext()->FLOAT_PRECISION);
  ConstCommonInput(mpcX, shareX);

  AUDIT("id:{}, P{} ConstCommonInput output shareX(Share){}", msgid.get_hex(), player, Vector<Share>(shareX));
}

/**
 * Broadcast: broadcast public values to peers
 * now, only supports P0 or P1 hold the msg
 */
void HelixInternal::Broadcast(const string& from_node, const string& msg, string& result) {
  DEB("helix public input msg, size: {}", msg.size());
  
  if (msg.size() != result.size())
    result.resize(msg.size());
  Broadcast(from_node, msg.data(), &result[0], msg.size());
}
void HelixInternal::Broadcast(const string& from_node, const char* msg, char* result, size_t size) {
  map<string, int> computation_nodes = io->GetComputationNodes();
  vector<string> non_computation_nodes = io->GetNonComputationNodes();
  string current_node = io->GetCurrentNodeId();
  string node_c = io->GetNodeId(PARTY_2);
  if (from_node == current_node) {
    for (auto iter = computation_nodes.begin(); iter != computation_nodes.end(); iter++) {
      if (current_node != iter->first) {
        send(iter->first, msg, size);
        memcpy(result, msg, size);
        AUDIT("id:{}, P{} Broadcast SEND to P{}{}", msgid.get_hex(), player, iter->first, CStr(msg, size));
      }
    }
  } else if (computation_nodes.find(current_node) != computation_nodes.end()) {
    recv(from_node, result, size);
    AUDIT("id:{}, P{} Broadcast RECV from P{}{}", msgid.get_hex(), player, from_node, CStr(result, size));
  } else if (std::find(non_computation_nodes.begin(), non_computation_nodes.end(), current_node) != non_computation_nodes.end()) {
    recv(node_c, result, size);
    AUDIT("id:{}, P{} Broadcast RECV from P{}{}", msgid.get_hex(), player, node_c, CStr(result, size));
  }

  if (current_node == node_c) {
    for (auto iter = non_computation_nodes.begin(); iter != non_computation_nodes.end(); iter++) {
      if (*iter != from_node) {
        send(*iter, result, size);
        AUDIT("id:{}, P{} Broadcast SEND to P{}{}", msgid.get_hex(), player, *iter, CStr(msg, size));
      }
    }
  }
}

/**
 * \param[in] X, its shape = (m,n)
 * \param[out] shareX, its shape = X.shape
 *
 * @see Input_
 */
void HelixInternal::Input(const string& node_id, const vector<vector<double>>& X, vector<vector<Share>>& shareX) {
  int m, n;
  size_t size = get_m_n(X, m, n);

  // 1. flatten X
  vector<double> flattenX;
  flatten(X, flattenX);

  // 2. double --> fixpoint
  vector<mpc_t> fixpointX;
  convert_plain_to_fixpoint(flattenX, fixpointX, GetMpcContext()->FLOAT_PRECISION);

  // 3. get input
  vector<Share> flattenShareX;
  Input(node_id, fixpointX, flattenShareX);

  // 4. back to (m,n)
  flatten_r(shareX, flattenShareX, m, n);
}

/**
 * Combine inputs in vertical
 */
void HelixInternal::CombineInputInVertical(
  const vector<vector<Share>>& shareX0,
  const vector<vector<Share>>& shareX1,
  const vector<vector<Share>>& shareX2,
  vector<vector<Share>>& shareX) {
  // valid check
  int m0, n0, m1, n1, m2, n2;
  get_m_n(shareX0, m0, n0);
  get_m_n(shareX1, m1, n1);
  get_m_n(shareX2, m2, n2);
  assert(m0 == m1);
  assert(m0 == m2);
  assert(m0 > 0);
  assert(n0 + n1 + n2 > 0);

  shareX.clear();
  shareX.resize(m0, vector<Share>(n0 + n1 + n2));

  int index = 0;
  for (int i = 0; i < m0; i++) {
    index = 0;
    for (int j = 0; j < n0; j++) {
      shareX[i][index++] = shareX0[i][j];
    }
    for (int j = 0; j < n1; j++) {
      shareX[i][index++] = shareX1[i][j];
    }
    for (int j = 0; j < n2; j++) {
      shareX[i][index++] = shareX2[i][j];
    }
  }
}

/**
 * Combine inputs in horizontal
 */
void HelixInternal::CombineInputInHorizontal(
  const vector<vector<Share>>& shareX0,
  const vector<vector<Share>>& shareX1,
  const vector<vector<Share>>& shareX2,
  vector<vector<Share>>& shareX) {
  for (int i=0; i<shareX0.size(); ++i) {
    AUDIT("id:{}, P{} CombineInputInHorizontal, input shareX0[{}](Share){}", msgid.get_hex(), player, i, Vector<Share>(shareX0[i]));
  }

  for (int i=0; i<shareX1.size(); ++i) {
    AUDIT("id:{}, P{} CombineInputInHorizontal, input shareX1[{}](Share){}", msgid.get_hex(), player, i, Vector<Share>(shareX1[i]));
  }

  for (int i=0; i<shareX0.size(); ++i) {
    AUDIT("id:{}, P{} CombineInputInHorizontal, input shareX2[{}](Share){}", msgid.get_hex(), player, i, Vector<Share>(shareX2[i]));
  }
  // valid check
  int m0, n0, m1, n1, m2, n2;
  get_m_n(shareX0, m0, n0);
  get_m_n(shareX1, m1, n1);
  get_m_n(shareX2, m2, n2);
  assert(n0 == n1);
  assert(n0 == n2);
  assert(n0 > 0);
  assert(m0 + m1 + m2 > 0);

  shareX.clear();
  shareX.resize(m0 + m1 + m2, vector<Share>(n0));

  int index = 0;
  for (int j = 0; j < n0; j++) {
    index = 0;
    for (int i = 0; i < m0; i++) {
      shareX[index++][j] = shareX0[i][j];
    }
    for (int i = 0; i < m1; i++) {
      shareX[index++][j] = shareX1[i][j];
    }
    for (int i = 0; i < m2; i++) {
      shareX[index++][j] = shareX2[i][j];
    }
  }

  for (int i=0; i<shareX.size(); ++i) {
    AUDIT("id:{}, P{} CombineInputInHorizontal, input shareX[{}](Share){}", msgid.get_hex(), player, i, Vector<Share>(shareX[i]));
  }
}

/**
 * Mul, multiply with constant, locally (and trunc, if C has scaled)
 *
 * Z = X * C
 *
 * \param C constants
 * \param c_has_scaled indicates whether C is scaled. If true, will truncate Z by call Trunc.
 */
void HelixInternal::Mul(
  const vector<Share>& X,
  const vector<mpc_t>& C,
  vector<Share>& Z,
  bool c_has_scaled) {
  AUDIT("id:{}, P{} Mul(S, M), input X(Share){}", msgid.get_hex(), player, Vector<Share>(X));
  AUDIT("id:{}, P{} Mul(S, M), input Y(mpc_t){}", msgid.get_hex(), player, Vector<mpc_t>(C));
  assert(X.size() == C.size());
  size_t size = X.size();
  resize_vector(Z, size);

  for (int i = 0; i < size; i++) {
    Z[i].s0.A0 = X[i].s0.A0 * C[i];
    Z[i].s1.A1 = X[i].s1.A1 * C[i];
  }

  if (c_has_scaled)
    Trunc(Z, size, GetMpcContext()->FLOAT_PRECISION);

  AUDIT("id:{}, P{} Mul(S, M), output Z(Share){}", msgid.get_hex(), player, Vector<Share>(Z));
}

/**
 * Mul, multiply with constant, float version
 *
 * Z = X * C
 *
 * \param C constants
 */
void HelixInternal::Mul(const vector<Share>& X, const vector<double>& C, vector<Share>& Z) {
  vector<mpc_t> fpC;
  convert_plain_to_fixpoint(C, fpC, GetMpcContext()->FLOAT_PRECISION);
  Mul(X, fpC, Z, true);
}
void HelixInternal::Mul(const vector<double>& C, const vector<Share>& X, vector<Share>& Z) {
  Mul(X, C, Z);
}

// This is the so-called FPMul if need_trunc is true.
void HelixInternal::Mul(
  const vector<Share>& X,
  const vector<Share>& Y,
  vector<Share>& Z,
  bool need_trunc) {
  Mul_<mpc_t, Share>(X, Y, Z, GetMpcContext()->FLOAT_PRECISION, need_trunc);
}

void HelixInternal::Mul(const vector<BitShare>& X, const vector<BitShare>& Y, vector<BitShare>& Z) {
  Mul_<bit_t, BitShare>(X, Y, Z, GetMpcContext()->FLOAT_PRECISION);
}

/**
 * Mean
 *
 * row first
 */
void HelixInternal::Mean(const vector<Share>& X, vector<Share>& Y, int rows, int cols) {
  assert(X.size() == rows * cols);
  if (rows == 0)
    return;

  AUDIT("id:{}, Mean({},{}), P{} input X(Share){}", msgid.get_hex(), rows, cols, player, Vector<Share>(X));
  vector<Share> sumY(rows);
  for (int i = 0; i < rows; i++) {
    vector<Share> rowX(X.begin() + i * cols, X.begin() + (i + 1) * cols);
    Sum(rowX, sumY[i]);
  }
  vector<double> vfscale(rows, 1.0 / cols);
  Mul(sumY, vfscale, Y);
  AUDIT("id:{}, Mean({},{}), P{} output(Share){}", msgid.get_hex(), rows, cols, player, Vector<Share>(Y));
}

void HelixInternal::Sum(const vector<Share>& X, vector<Share>& Y, int rows, int cols) {
  assert(X.size() == rows * cols);
  if (rows == 0)
    return;

  AUDIT("id:{}, Sum({},{}), P{} input X(Share){}", msgid.get_hex(), rows, cols, player, Vector<Share>(X));
  resize_vector(Y, rows);
  for (int i = 0; i < rows; i++) {
    vector<Share> rowX(X.begin() + i * cols, X.begin() + (i + 1) * cols);
    Sum(rowX, Y[i]);
  }
  AUDIT("id:{}, Sum({},{}), P{} output(Share){}", msgid.get_hex(), rows, cols, player, Vector<Share>(Y));
}

void HelixInternal::Max(const vector<Share>& X, vector<Share>& Y, int rows, int cols) {
  assert(X.size() == rows * cols);
  if (rows == 0)
    return;

  AUDIT("id:{}, Max({},{}), P{} input X(Share){}", msgid.get_hex(), rows, cols, player, Vector<Share>(X));
  resize_vector(Y, rows);
  // initiate
  for (size_t i = 0; i < rows; ++i) {
    Y[i] = X[i * cols];
  }
  vector<Share>& max = Y;

  // RevealAndPrint(X, " input X:");
  int depth = ceil(log2(cols));
  int curr_L = cols;
  vector<Share> level_data = X, first_v, second_v, diff_v, comp_res;
  bool need_pad = false;
  for (size_t i = 0; i < depth; ++i) {
    // cout << "curr level: " << i << endl;
    // cout << "curr level length:" << curr_L << endl;
    int next_L = ceil( curr_L / 2.0);
    vector<Share> new_data;
    first_v.clear();
    second_v.clear();
    diff_v.clear();
    comp_res.clear();
    // If the numbers in a row is odd, there is no need to comapre the last element. 
    if (curr_L % 2 == 1) {
      need_pad = true;
    }
    // pack for batch comparison.
    Share tmp_v;
    for (int j = 0; j < rows; ++j) {
      for (int k = 0; k < curr_L - 1; k = k + 2) {
        first_v.push_back(level_data[j*curr_L + k]);
        second_v.push_back(level_data[j*curr_L + k + 1]);
      }
    }
    Sub(first_v, second_v, diff_v);

    // batch comparison
    int comp_size = diff_v.size();
    comp_res.resize(comp_size);
    new_data.resize(comp_size);
    DReLU(diff_v, comp_res);
		Select1Of2(first_v, second_v, comp_res, new_data);
    
    // unpack for next layer.
    int comp_res_col = comp_size / rows;
    vector<Share> next_level_data;
    for (int j = 0; j < rows; ++j) {
      for (int k = 0; k <= next_L - 1; ++k) {
        if (k == next_L - 1) {
          // reserve the last element if needed.
          if(need_pad && (next_L != 1)) {
            next_level_data.push_back(level_data[j * curr_L + curr_L - 1]);
          } else {
            next_level_data.push_back(new_data[j * comp_res_col + k]);
          }
        } else {
          next_level_data.push_back(new_data[j * comp_res_col + k]);
        }
      }
    }
    level_data = next_level_data;
    curr_L = next_L;
  }
  max = level_data;
  AUDIT("id:{}, Max({},{}), P{} output(Share){}", msgid.get_hex(), rows, cols, player, Vector<Share>(Y));
}

void HelixInternal::Min(const vector<Share>& X, vector<Share>& Y, int rows, int cols) {
  assert(X.size() == rows * cols);
  if (rows == 0)
    return;

  AUDIT("id:{}, Min({},{}), P{} input X(Share){}", msgid.get_hex(), rows, cols, player, Vector<Share>(X));
  resize_vector(Y, rows);
  // initiate
  for (size_t i = 0; i < rows; ++i) {
    Y[i] = X[i * cols];
  }
  vector<Share>& min = Y;

  // RevealAndPrint(X, " input X:");
  int depth = ceil(log2(cols));
  int curr_L = cols;
  vector<Share> level_data = X, first_v, second_v, diff_v, comp_res;
  bool need_pad = false;
  for (size_t i = 0; i < depth; ++i) {
    // cout << "curr level: " << i << endl;
    // cout << "curr level length:" << curr_L << endl;
    int next_L = ceil( curr_L / 2.0);
    vector<Share> new_data;
    first_v.clear();
    second_v.clear();
    diff_v.clear();
    comp_res.clear();
    // If the numbers in a row is odd, there is no need to comapre the last element. 
    if (curr_L % 2 == 1) {
      need_pad = true;
    }
    // pack for batch Relu.
    Share tmp_v;
    for (int j = 0; j < rows; ++j) {
      for (int k = 0; k < curr_L - 1; k = k + 2) {
        first_v.push_back(level_data[j*curr_L + k]);
        second_v.push_back(level_data[j*curr_L + k + 1]);
      }
    }
    Sub(second_v, first_v, diff_v);

    int comp_size = diff_v.size();
    comp_res.resize(comp_size);
    new_data.resize(comp_size);
    DReLU(diff_v, comp_res);
		Select1Of2(first_v, second_v, comp_res, new_data);
    
    // unpack for next layer.
    int comp_res_col = comp_size / rows;
    vector<Share> next_level_data;
    for (int j = 0; j < rows; ++j) {
      for (int k = 0; k <= next_L - 1; ++k) {
        if (k == next_L - 1) {
          // reserve the last element if needed.
          if(need_pad && (next_L != 1)) {
            next_level_data.push_back(level_data[j * curr_L + curr_L - 1]);
          } else {
            next_level_data.push_back(new_data[j * comp_res_col + k]);
          }
        } else {
          next_level_data.push_back(new_data[j * comp_res_col + k]);
        }
      }
    }
    level_data = next_level_data;
    curr_L = next_L;
  }
  min = level_data;
  AUDIT("id:{}, Min({},{}), P{} output(Share){}", msgid.get_hex(), rows, cols, player, Vector<Share>(Y));
}

void HelixInternal::AddN(const vector<Share>& X, vector<Share>& Y, int rows, int cols) {
  assert(X.size() == rows * cols && "rows*cols not equal to vector size");
  AUDIT("id:{}, AddN({},{}), P{} input X(Share){}", msgid.get_hex(), rows, cols, player, Vector<Share>(X));

  if (rows == 0 || cols == 0)
    return;
  if (rows == 1) { //copy back
    Y.assign(X.begin(), X.end());
    return;
  }

  resize_vector(Y, cols);
  memset(Y.data(), 0, sizeof(Y[0])*Y.size());
  for (int i = 0; i < cols; i++) {
    for (int j = 0; j < rows; j++) {
      auto index = j * cols + i;
      Y[i].s0.A0 += X[index].s0.A0;
      Y[i].s1.A1 += X[index].s1.A1;
    }
  }
  AUDIT("id:{}, AddN({},{}), P{} output(Share){}", msgid.get_hex(), rows, cols, player, Vector<Share>(Y));
}

/**
 * Square
 *
 * Y = X * X
 */
void HelixInternal::Square(const vector<Share>& X, vector<Share>& Y) { Mul(X, X, Y); }

/**
 * Z = X * Y
 * shape: (m,k) = (m,n) x (n,k)
 */
void HelixInternal::MatMul(
  const vector<Share>& X,
  const vector<Share>& Y,
  vector<Share>& Z,
  size_t m,
  size_t n,
  size_t k,
  bool t_a,
  bool t_b) {
  if (m * n * k > 10000) {
    //MatMul2(X, Y, Z, m, n, k, t_a, t_b);
    //return;
  }
  size_t size_a = m * n;
  size_t size_b = n * k;
  size_t size_c = m * k;
  assert(size_a > 0);
  assert(size_b > 0);
  assert(size_c > 0);
  resize_vector(Z, size_c);

  AUDIT("id:{}, MatMul({},{},{}), P{} input X(Share){}", msgid.get_hex(), m, n, k, player, Vector<Share>(X));
  AUDIT("id:{}, MatMul({},{},{}), P{} input Y(Share){}", msgid.get_hex(), m, n, k, player, Vector<Share>(Y));

  // step 2 Ci
  vector<mpc_t> C0(size_c, 0), C1(size_c, 0);
  PRF02(C0, size_c);
  if ((player == PARTY_0) || (player == PARTY_2)) {
    AUDIT("id:{}, MatMul({},{},{}), P{} generator C0(mpc_t){}", msgid.get_hex(), m, n, k, player, Vector<mpc_t>(C0));
  }

  if ((player == PARTY_1) || (player == PARTY_2)) {
    if (player == PARTY_2) {
      // step 3 (A0+A1)*(B0+B1)-C0
      vector<mpc_t> A(size_a, 0), B(size_b, 0), C(size_c, 0);
      for (int i = 0; i < size_a; i++) {
        A[i] = X[i].s0.A0 + X[i].s1.A1;
      }
      for (int i = 0; i < size_b; i++) {
        B[i] = Y[i].s0.B0 + Y[i].s1.B1;
      }

      EigenMatMul(A, B, C, m, n, k, t_a, t_b);
      C1 = C - C0;
      AUDIT("id:{}, MatMul({},{},{}), P{} locally computes C1(=(A0+A1)*(B0+b1)-C0)(mpc_t){}", msgid.get_hex(), m, n, k, player, Vector<mpc_t>(C1));

      send(PARTY_1, C1, size_c); // size_c == m*k*sizeof(mpc_t)
      AUDIT("id:{}, MatMul({},{},{}), P{} SEND to P{} C1(mpc_t){}", msgid.get_hex(), m, n, k, player, PARTY_1, Vector<mpc_t>(C1));
    } else {
      recv(PARTY_2, C1, size_c);
      AUDIT("id:{}, MatMul({},{},{}), P{} RECV from P{} C1(mpc)t{}", msgid.get_hex(), m, n, k, player, PARTY_2, Vector<mpc_t>(C1));
    }
  }

  // step 4 Zi
  vector<mpc_t> Z0(size_c, 0), Z1(size_c, 0);
  PRF02(Z0, size_c);
  if ((player == PARTY_0) || (player == PARTY_2)) {
    AUDIT("id:{}, Mul({},{},{}), P0 and P2 generat Z0(mpc_t){}", msgid.get_hex(), m, n, k, Vector<mpc_t>(Z0));
  }

  PRF12(Z1, size_c);
  if ((player == PARTY_1) || (player == PARTY_2)) {
    AUDIT("id:{}, Mul({},{},{}), P1 and P2 generat Z1(mpc_t){}", msgid.get_hex(), m, n, k, Vector<mpc_t>(Z1));
  }

  vector<mpc_t> tildeZ(size_c, 0), tildeZ0(size_c, 0), tildeZ1(size_c, 0);
  vector<mpc_t> hatZ(size_c, 0), hatZ0(size_c, 0), hatZ1(size_c, 0);
  if (is_primary()) {
    if (player == PARTY_0) {
      // step 5 deltaX*B0 + A0*deltaY + C0
      vector<mpc_t> A(size_a, 0), B(size_b, 0);
      for (int i = 0; i < size_a; i++) {
        A[i] = X[i].s0.delta;
      }
      for (int i = 0; i < size_b; i++) {
        B[i] = Y[i].s1.B0;
      }
      EigenMatMul(A, B, tildeZ0, m, n, k, t_a, t_b);

      for (int i = 0; i < size_a; i++) {
        A[i] = X[i].s1.A0;
      }
      for (int i = 0; i < size_b; i++) {
        B[i] = Y[i].s0.delta;
      }
      EigenMatMul(A, B, tildeZ1, m, n, k, t_a, t_b);

      tildeZ0 = tildeZ0 + tildeZ1 + C0;
      AUDIT("id:{}, MatMul({},{},{}), P{} locally computes tildeZ0(=deltaX*B0+A0*deltaY+C0)(mpc_t){}", msgid.get_hex(), m, n, k, player, Vector<mpc_t>(tildeZ0));

      // step 6,7
      Trunc(tildeZ0, size_c, GetMpcContext()->FLOAT_PRECISION);
      AUDIT("id:{}, MatMul({},{},{}), P{} locally tuncates tildeZ0(mpc_t){}", msgid.get_hex(), m, n, k, player, Vector<mpc_t>(tildeZ0));

      hatZ0 = tildeZ0 - Z0;
      AUDIT("id:{}, MatMul({},{},{}), P{} locally computes hatZ0(=tildeZ0-Z0)(mpc_t){}", msgid.get_hex(), m, n, k, player, Vector<mpc_t>(hatZ0));

      send(PARTY_1, hatZ0, size_c);
      AUDIT("id:{}, MatMul({},{},{}), P{} SEND to P{} hatZ0(mpc_t){}", msgid.get_hex(), m, n, k, player, PARTY_1, Vector<mpc_t>(hatZ0));

      recv(PARTY_1, hatZ1, size_c);
      AUDIT("id:{}, MatMul({},{},{}), P{} RECV from P{} hatZ1(mpc_t){}", msgid.get_hex(), m, n, k, player, PARTY_1, Vector<mpc_t>(hatZ1));
    } else {
      // step 5 deltaX*deltaY + deltaX*B1 + A1*deltaY + C1
      vector<mpc_t> A(size_a, 0), B(size_b, 0), AB(size_c, 0);
      for (int i = 0; i < size_a; i++) {
        A[i] = X[i].s0.delta;
      }
      for (int i = 0; i < size_b; i++) {
        B[i] = Y[i].s0.delta;
      }
      EigenMatMul(A, B, AB, m, n, k, t_a, t_b);

      for (int i = 0; i < size_a; i++) {
        A[i] = X[i].s0.delta;
      }
      for (int i = 0; i < size_b; i++) {
        B[i] = Y[i].s1.B1;
      }
      EigenMatMul(A, B, tildeZ0, m, n, k, t_a, t_b);

      for (int i = 0; i < size_a; i++) {
        A[i] = X[i].s1.A1;
      }
      for (int i = 0; i < size_b; i++) {
        B[i] = Y[i].s0.delta;
      }
      EigenMatMul(A, B, tildeZ1, m, n, k, t_a, t_b);

      tildeZ1 = AB + tildeZ0 + tildeZ1 + C1;
      AUDIT("id:{}, MatMul({},{},{}), P{} locally compute tildeZ1(tildwZ1=deltaX*deltaY+deltaX*B1+A1*deltaY+C1)(mpc_t){}", msgid.get_hex(), m, n, k, player, Vector<mpc_t>(tildeZ1));

      // step 6,7
      Trunc(tildeZ1, size_c, GetMpcContext()->FLOAT_PRECISION);
      AUDIT("id:{}, MatMul({},{},{}), P{} locally tuncates tildeZ1(mpc_t){}", msgid.get_hex(), m, n, k, player, Vector<mpc_t>(tildeZ1));

      hatZ1 = tildeZ1 - Z1;
      AUDIT("id:{}, MatMul({},{},{}), P{} locally compute hatZ1(=tildeZ1-Z1)(mpc_t){}", msgid.get_hex(), m, n, k, player, Vector<mpc_t>(hatZ1));

      recv(PARTY_0, hatZ0, size_c);
      AUDIT("id:{}, MatMul({},{},{}), P{} RECV from P{} hatZ0(mpc_t){}", msgid.get_hex(), m, n, k, player, PARTY_0, Vector<mpc_t>(hatZ0));

      send(PARTY_0, hatZ1, size_c);
      AUDIT("id:{}, MatMul({},{},{}), P{} SEND to P{} hatZ1(mpc_t){}", msgid.get_hex(), m, n, k, player, PARTY_0, Vector<mpc_t>(hatZ1));
    }

    // step 8 reveal hatZ
    hatZ = hatZ0 + hatZ1;
    AUDIT("id:{}, MatMul({},{},{}), P{} locally compute hatZ(=hatZ0+hatZ1)(mpc_t){}", msgid.get_hex(), m, n, k, player, Vector<mpc_t>(hatZ));
  }

  // each party sets the share
  for (int i = 0; i < size_c; i++) {
    if (is_helper()) {
      Z[i].s0.A0 = Z0[i];
      Z[i].s1.A1 = Z1[i];
    } else {
      Z[i].s0.delta = hatZ[i];
      if (player == PARTY_0)
        Z[i].s1.A0 = Z0[i];
      else
        Z[i].s1.A1 = Z1[i];
    }
  }

  AUDIT("id:{}, MatMul({},{},{}), P{} output(Share){}", msgid.get_hex(), m, n, k, player, Vector<Share>(Z));
}

void HelixInternal::MatMul2(
  const vector<Share>& X,
  const vector<Share>& Y,
  vector<Share>& Z,
  size_t m,
  size_t n,
  size_t k,
  bool t_a,
  bool t_b) {
  size_t size_a = m * n;
  size_t size_b = n * k;
  size_t size_c = m * k;
  assert(size_a > 0);
  assert(size_b > 0);
  assert(size_c > 0);
  resize_vector(Z, size_c);

  AUDIT("id:{}, MatMul2({},{},{}), P{} input X(Share){}", msgid.get_hex(), m, n, k, player, Vector<Share>(X));
  AUDIT("id:{}, MatMul2({},{},{}), P{} input Y(Share){}", msgid.get_hex(), m, n, k, player, Vector<Share>(Y));

  // step 2 Ci
  vector<mpc_t> C0(size_c, 0), C1(size_c, 0);
  PRF02(C0, size_c);
  if ((player == PARTY_0) || (player == PARTY_2)) {
    AUDIT("id:{}, MatMul2({},{},{}), P{} generator C0(mpc_t){}", msgid.get_hex(), m, n, k, player, Vector<mpc_t>(C0));
  }

  if ((player == PARTY_1) || (player == PARTY_2)) {
    if (player == PARTY_2) {
      // step 3 (A0+A1)*(B0+B1)-C0
      vector<mpc_t> A(size_a, 0), B(size_b, 0), C(size_c, 0);
      for (int i = 0; i < size_a; i++) {
        A[i] = X[i].s0.A0 + X[i].s1.A1;
      }
      for (int i = 0; i < size_b; i++) {
        B[i] = Y[i].s0.B0 + Y[i].s1.B1;
      }

      EigenMatMul(A, B, C, m, n, k, t_a, t_b);
      Sub(C, C0, C1);
      AUDIT("id:{}, MatMul2({},{},{}), P{} locally computes C1((A0+A1)*(B0+b1)-C0)(mpc_t){}", msgid.get_hex(), m, n, k, player, Vector<mpc_t>(C1));

      send(PARTY_1, C1, size_c); // size_c == m*k*sizeof(mpc_t)
      AUDIT("id:{}, MatMul2({},{},{}), P{} SEND to P{} C1(mpc_t){}", msgid.get_hex(), m, n, k, player, PARTY_1, Vector<mpc_t>(C1));
    } else {
      recv(PARTY_2, C1, size_c);
      AUDIT("id:{}, MatMul2({},{},{}), P{} RECV from P{} C1(mpc_t){}", msgid.get_hex(), m, n, k, player, PARTY_2, Vector<mpc_t>(C1));
    }
  }

  // step 4 Zi
  vector<mpc_t> Z0(size_c, 0), Z1(size_c, 0);
  PRF02(Z0, size_c);
  if ((player == PARTY_0) || (player == PARTY_2)) {
    AUDIT("id:{}, MatMul2({},{},{}), P0 and P2 generat Z0(mpc_t){}", msgid.get_hex(), m, n, k, Vector<mpc_t>(Z0));
  }

  PRF12(Z1, size_c);
  if ((player == PARTY_1) || (player == PARTY_2)) {
    AUDIT("id:{}, MatMul2({},{},{}), P1 and P2 generat Z1(mpc_t){}", msgid.get_hex(), m, n, k, Vector<mpc_t>(Z1));
  }

  vector<mpc_t> tildeZ(size_c, 0), tildeZ0(size_c, 0), tildeZ1(size_c, 0);
  vector<mpc_t> hatZ(size_c, 0), hatZ0(size_c, 0), hatZ1(size_c, 0);
  if (is_primary()) {
    if (player == PARTY_0) {
      // step 5 deltaX*B0 + A0*deltaY + C0

      auto f0 = [&]() {
        vector<mpc_t> A(size_a, 0), B(size_b, 0);
        for (int i = 0; i < size_a; i++) {
          A[i] = X[i].s0.delta;
        }
        for (int i = 0; i < size_b; i++) {
          B[i] = Y[i].s1.B0;
        }
        EigenMatMul(A, B, tildeZ0, m, n, k, t_a, t_b);
      };

      auto f1 = [&]() {
        vector<mpc_t> A(size_a, 0), B(size_b, 0);
        for (int i = 0; i < size_a; i++) {
          A[i] = X[i].s1.A0;
        }
        for (int i = 0; i < size_b; i++) {
          B[i] = Y[i].s0.delta;
        }
        EigenMatMul(A, B, tildeZ1, m, n, k, t_a, t_b);
      };

      vector<thread> threads(2);
      threads[0] = thread(f0);
      threads[1] = thread(f1);
      for (int i = 0; i < 2; i++) {
        threads[i].join();
      }

      // tildeZ0 = tildeZ0 + tildeZ1 + C0;
      Add(tildeZ0, tildeZ1);
      Add(tildeZ0, C0);
      AUDIT("id:{}, MatMul2({},{},{}), P{} locally computes tildeZ0(=deltaX*B0+A0*deltaY+C0)(mpc_t){}", msgid.get_hex(), m, n, k, player, Vector<mpc_t>(tildeZ0));

      // step 6,7
      Trunc(tildeZ0, size_c, GetMpcContext()->FLOAT_PRECISION);
      AUDIT("id:{}, MatMul2({},{},{}), P{} locally tuncates tildeZ0(mpc_t){}", msgid.get_hex(), m, n, k, player, Vector<mpc_t>(tildeZ0));

      //hatZ0 = tildeZ0 - Z0;
      Sub(tildeZ0, Z0, hatZ0);
      AUDIT("id:{}, MatMul2({},{},{}), P{} locally computes hatZ0(=tildeZ0-Z0)(mpc_t){}", msgid.get_hex(), m, n, k, player, Vector<mpc_t>(hatZ0));

      send(PARTY_1, hatZ0, size_c);
      AUDIT("id:{}, MatMul2({},{},{}), P{} SEND to P{} hatZ0(mpc_t){}", msgid.get_hex(), m, n, k, player, PARTY_1, Vector<mpc_t>(hatZ0));

      recv(PARTY_1, hatZ1, size_c);
      AUDIT("id:{}, MatMul2({},{},{}), P{} RECV from P{} hatZ1(mpc_t){}", msgid.get_hex(), m, n, k, player, PARTY_1, Vector<mpc_t>(hatZ1));
    } else {
      // step 5 deltaX*deltaY + deltaX*B1 + A1*deltaY + C1
      vector<mpc_t> AB(size_c, 0);
      auto f0 = [&]() {
        //SimpleTimer timer;
        vector<mpc_t> A(size_a, 0), B(size_b, 0);
        for (int i = 0; i < size_a; i++) {
          A[i] = X[i].s0.delta;
        }
        for (int i = 0; i < size_b; i++) {
          B[i] = Y[i].s0.delta;
        }
        EigenMatMul(A, B, AB, m, n, k, t_a, t_b);
      };

      auto f1 = [&]() {
        //SimpleTimer timer;
        vector<mpc_t> A(size_a, 0), B(size_b, 0);
        for (int i = 0; i < size_a; i++) {
          A[i] = X[i].s0.delta;
        }
        for (int i = 0; i < size_b; i++) {
          B[i] = Y[i].s1.B1;
        }
        EigenMatMul(A, B, tildeZ0, m, n, k, t_a, t_b);
      };

      auto f2 = [&]() {
        vector<mpc_t> A(size_a, 0), B(size_b, 0);
        for (int i = 0; i < size_a; i++) {
          A[i] = X[i].s1.A1;
        }
        for (int i = 0; i < size_b; i++) {
          B[i] = Y[i].s0.delta;
        }
        EigenMatMul(A, B, tildeZ1, m, n, k, t_a, t_b);
      };

      vector<thread> threads(3);
      threads[0] = thread(f0);
      threads[1] = thread(f1);
      threads[2] = thread(f2);
      for (int i = 0; i < 3; i++) {
        threads[i].join();
      }

      // tildeZ1 = AB + tildeZ0 + tildeZ1 + C1;
      Add(tildeZ1, AB);
      Add(tildeZ1, tildeZ1);
      Add(tildeZ1, C1);
      AUDIT("id:{}, MatMul2({},{},{}), P{} locally compute tildeZ1(=deltaX*deltaY+deltaX*B1+A1*deltaY+C1)(mpc_t){}", msgid.get_hex(), m, n, k, player, Vector<mpc_t>(tildeZ1));

      // step 6,7
      Trunc(tildeZ1, size_c, GetMpcContext()->FLOAT_PRECISION);
      AUDIT("id:{}, MatMul2({},{},{}), P{} locally tuncates tildeZ1(mpc_t){}", msgid.get_hex(), m, n, k, player, Vector<mpc_t>(tildeZ1));

      // hatZ1 = tildeZ1 - Z1;
      Sub(tildeZ1, Z1, hatZ1);
      AUDIT("id:{}, MatMul2({},{},{}), P{} locally compute hatZ1(hatZ1=tildeZ1 - Z1){}", msgid.get_hex(), m, n, k, player, Vector<mpc_t>(hatZ1));

      recv(PARTY_0, hatZ0, size_c);
      AUDIT("id:{}, MatMul2({},{},{}), P{} RECV from P{} hatZ0(mpc_t){}", msgid.get_hex(), m, n, k, player, PARTY_0, Vector<mpc_t>(hatZ0));

      send(PARTY_0, hatZ1, size_c);
      AUDIT("id:{}, MatMul2({},{},{}), P{} SEND to P{} hatZ1(mpc_t){}", msgid.get_hex(), m, n, k, player, PARTY_0, Vector<mpc_t>(hatZ1));
    }

    // step 8 reveal hatZ
    //hatZ = hatZ0 + hatZ1;
    Add(hatZ0, hatZ1, hatZ);
    AUDIT("id:{}, MatMul2({},{},{}), P{} locally compute hatZ(=hatZ0+hatZ1)(mpc_t){}", msgid.get_hex(), m, n, k, player, Vector<mpc_t>(hatZ));
  }

  // each party sets the share
  for (int i = 0; i < size_c; i++) {
    if (is_helper()) {
      Z[i].s0.A0 = Z0[i];
      Z[i].s1.A1 = Z1[i];
    } else {
      Z[i].s0.delta = hatZ[i];
      if (player == PARTY_0)
        Z[i].s1.A0 = Z0[i];
      else
        Z[i].s1.A1 = Z1[i];
    }
  }

  AUDIT("id:{}, MatMul2({},{},{}), P{} output(Share){}", msgid.get_hex(), m, n, k, player, Vector<Share>(Z));
}

/**
 * 1 bit adder
 *
 * \param[in/out] C = C ^ ((X ^ C) & (Y ^ C))
 *
 */
void HelixInternal::BitAdder(
  const vector<BitShare>& X,
  const vector<BitShare>& Y,
  vector<BitShare>& C) {
  AUDIT("id:{}, P{} AdderCircuitL, input X(BitShare){}", msgid.get_hex(), player, Vector<BitShare>(X));
  AUDIT("id:{}, P{} AdderCircuitL, input Y(BitShare){}", msgid.get_hex(), player, Vector<BitShare>(Y));
  size_t size = X.size();
  assert(X.size() == Y.size());
  assert(C.size() >= size);

  // xc = x ^ c
  vector<BitShare> xc(size);
  for (int i = 0; i < size; i++) {
    xc[i].s0.delta = X[i].s0.delta ^ C[i].s0.delta;
    xc[i].s1.A0 = X[i].s1.A0 ^ C[i].s1.A0;
  }

  // yc = y ^ c
  vector<BitShare> yc(size);
  for (int i = 0; i < size; i++) {
    yc[i].s0.delta = Y[i].s0.delta ^ C[i].s0.delta;
    yc[i].s1.A0 = Y[i].s1.A0 ^ C[i].s1.A0;
  }

  // xy = xc & yc
  vector<BitShare> xy(size);
  Mul(xc, yc, xy);

  // c = c ^ xy
  for (int i = 0; i < size; i++) {
    C[i].s0.delta = xy[i].s0.delta ^ C[i].s0.delta;
    C[i].s1.A0 = xy[i].s1.A0 ^ C[i].s1.A0;
  }
  AUDIT("id:{}, P{} AdderCircuitL, output(BitShare){}", msgid.get_hex(), player, Vector<BitShare>(C));
}

/**
 *
 * Adder-L
 * X[i]: x_1,...,x_\ell
 * Y[i]: y_1,...,y_\ell
 *
 * X.shape = (k, \ell)
 */
void HelixInternal::AdderCircuitL(
  const vector<vector<BitShare>>& X,
  const vector<vector<BitShare>>& Y,
  vector<BitShare>& C) {
  for (int i=0; i<X.size(); ++i) {
    AUDIT("id:{}, P{} AdderCircuitL, input X[{}](BitShare){}", msgid.get_hex(), player, i, Vector<BitShare>(X[i]));
  }

  for (int i=0; i<Y.size(); ++i) {
    AUDIT("id:{}, P{} AdderCircuitL, input Y[{}](BitShare){}", msgid.get_hex(), player, i, Vector<BitShare>(Y[i]));
  }
  // X.size() == how many X == C.size()
  // X[i].size() == bitlen
  size_t size = X.size(); // k, how many X
  size_t bitlen = sizeof(mpc_t) * 8; // \ell, in general == 64
  resize_vector(C, size);

  vector<BitShare> tmpX(size), tmpY(size), tmpC(size);
  for (int j = 0; j < bitlen; j++) { // from low to high, that is from x_1 to x_\ell
    for (int i = 0; i < size; i++) {
      tmpX[i] = X[i][j];
      tmpY[i] = Y[i][j];
    }
    if (j < bitlen - 1) {
      BitAdder(tmpX, tmpY, tmpC);
    } else {
      for (int i = 0; i < size; i++) {
        C[i].s0.delta = tmpX[i].s0.delta ^ tmpY[i].s0.delta ^ tmpC[i].s0.delta;
        C[i].s1.A0 = tmpX[i].s1.A0 ^ tmpY[i].s1.A0 ^ tmpC[i].s1.A0;
      }
    }
  }
  AUDIT("id:{}, P{} AdderCircuitL, output(BitShare){}", msgid.get_hex(), player, Vector<BitShare>(C));
}

#define _print_adder_level_info 0
#if _print_adder_level_info
static inline void print_adder_level_info() {
  static int flag = 0;
  if (flag > 0)
    return;
  flag++;

  size_t bitlen = 64;
  size_t d = 6;
  cout << "init: P[i] = X[i] ^ Y[i]; G[i] = X[i] & Y[i]; (0<=i<=62)" << endl;

  for (size_t level = 0; level < d; ++level) {
    auto start = 1 << level; // 1,2,4,8,16,32
    auto step = 1 << (level + 1); // 2,4,8,16,32,64
    bool first = true;
    cout << "-----------------------------------------------------" << endl;
    for (size_t i = start; i < bitlen; i += step) {
      auto low = i - 1;
      auto cur = std::min(low + start, bitlen - 2);

      if (cur != low) {
        cout << "level:" << level << " start:" << setw(2) << start << " step:" << setw(2) << step
             << " low:" << setw(2) << low << " cur:" << setw(2) << cur << " first:" << first;
        cout << "  G[" << setw(2) << cur << "] = G[" << setw(2) << cur << "] ^ P[" << setw(2) << cur
             << "] & G[" << setw(2) << low << "]";
        if (!first) {
          cout << "; P[" << setw(2) << cur << "] = P[" << setw(2) << cur << "] & P[" << setw(2)
               << low << "]";
        }
        cout << endl;
      }
      first = false;
    }
  }
  cout << "---------------" << endl;
  cout << "end: P[63] = X[63] ^ Y[63]; MSB = P[63] ^ G[62]" << endl;
}
#endif

#define ADDER_SPEEDUP 1
/**
 * \param X  bitwise expansion. from item_1 to item_k, which item_i(1,2,3,...,\ell)
 * \param Y
 * \param C the MSB
 * note : round complexity : 7
 */
void HelixInternal::AdderCircuitW(
  const vector<BitShare>& X,
  const vector<BitShare>& Y,
  vector<BitShare>& C) {
#if _print_adder_level_info
  // you can set `_print_adder_level_info` to 1, See how does AdderCircuitW work
  print_adder_level_info();
#endif
  AUDIT("id:{}, P{} AdderCircuitW, input X(BitShare){}", msgid.get_hex(), player, Vector<BitShare>(X));
  AUDIT("id:{}, P{} AdderCircuitW, input Y(BitShare){}", msgid.get_hex(), player, Vector<BitShare>(Y));
  size_t bitlen = sizeof(mpc_t) * 8; // \ell, in general == 64
  size_t size = X.size() / bitlen; // k, how many X
  resize_vector(C, size);
  // beg_statistics();
  // step 1 init
  size_t all_size = size * bitlen;
  vector<BitShare> GG(all_size), PP(all_size);
  
  // SpeedUp!
  #if ADDER_SPEEDUP
  // get 1, 3, 5, ..., 63-th bit of X, if X is 64-bit long
  int half_size = int(all_size/2);
  vector<BitShare> odd_bits_in_X(half_size);
  vector<BitShare> odd_bits_in_Y(half_size);
  vector<BitShare> odd_GG(half_size);
  for(int i = 0; i < all_size; i = i + 2) {
    int j = int(i / 2);
    odd_bits_in_X[j] = X[i];
    odd_bits_in_Y[j] = Y[i];
  }
  Mul(odd_bits_in_X, odd_bits_in_Y, odd_GG);
  for(int i = 0; i < odd_GG.size(); i++) {
    int j = i * 2;
    GG[j] = odd_GG[i];
  }
  #else
  Mul(X, Y, GG); // g = x & y
  #endif
  PP = X ^ Y; // p = x ^ y
  // RevealAndPrint(GG, "Debug init G");
  // RevealAndPrint(PP, "DEBUG init P");

  // step 2 body
  {
    size_t d = log2ceil(bitlen);

    for (size_t level = 0; level < d; ++level) {

      auto start = 1 << level; // 1,2,4,8,16,32
      auto step = 1 << (level + 1); // 2,4,8,16,32,64
      bool first = true;

      
      // G1 = g1 ^ p1 & G0
      // P1 = p1 & P0
      vector<BitShare> P0, P1, G0, G1;
      
      #if ADDER_SPEEDUP
      //////////////// New: special handle of the first level.
      // the Update rule for first level is:
      // If g1= X AND Y, p1=X XOR Y
      //    (g1,p1)|(g0, p0) =(g0 XOR ((X XOR g0) AND (Y XOR g0)), p1 AND p0)
      if (level == 0) {
        vector<BitShare> tmp_X_xor_G0, tmp_Y_xor_G0;
        for (int j = 0; j < size; j++) {
          // one item
          for (size_t k = 1; k < bitlen; k += 2) {
            auto low = k - 1;
            auto cur = std::min(k, bitlen - 2);
            if (cur != low) {
              P0.push_back(PP[low + j * bitlen]);
              P1.push_back(PP[cur + j * bitlen]);
              G0.push_back(GG[low + j * bitlen]);
              tmp_X_xor_G0.push_back(X[cur + j * bitlen] ^ GG[low + j * bitlen]);
              tmp_Y_xor_G0.push_back(Y[cur + j * bitlen] ^ GG[low + j * bitlen]);
              // the first propegate no carry in
              if (!first) {
              }
            }
            first = false;
          }
        }
        // we pack them so that only one call of Mul is used.
        // ((X XOR g0) | P1)  AND ((Y XOR g0) | P0) = (X XOR g0) AND (Y XOR g0), P1 AND P0
        vector<BitShare> tmp_first, tmp_second, tmp_new_P_and_G;
        tmp_first.insert(tmp_first.end(), tmp_X_xor_G0.begin(), tmp_X_xor_G0.end());
        tmp_first.insert(tmp_first.end(), P1.begin(), P1.end());
        tmp_second.insert(tmp_second.end(), tmp_Y_xor_G0.begin(), tmp_Y_xor_G0.end());
        tmp_second.insert(tmp_second.end(), P0.begin(), P0.end());
        Mul(tmp_first, tmp_second, tmp_new_P_and_G);
        // unpack the result.
        auto G1_size = tmp_X_xor_G0.size();
        vector<BitShare> new_G1, new_P1;
        // new_P1 = P1 & P0
        new_P1.insert(new_P1.end(), tmp_new_P_and_G.begin() + G1_size, tmp_new_P_and_G.end());
        tmp_new_P_and_G.resize(G1_size);
        tmp_new_P_and_G.shrink_to_fit();
        
        new_G1 = G0 ^ tmp_new_P_and_G;
        // RevealAndPrint(new_G1, "Debug new G1");
        // update original PP and GG.
        // Todo: clean code.
        int index = 0;
        for (int j = 0; j < size; j++) { // all items
          for (size_t i = start; i < bitlen; i += step) { // one item
            auto low = i - 1;
            auto cur = std::min(low + start, bitlen - 2);

            if (cur != low) {
              GG[cur + j * bitlen] = new_G1[index];
              PP[cur + j * bitlen] = new_P1[index];
              index++;
              // the first propegate no carry in
              if (!first) {
              }
            }
            first = false;
          }
        }

        // RevealAndPrint(PP, "DEBUG curr P");
        // RevealAndPrint(GG, "DEBUG curr G");
        continue;
      }
      #endif
      ///////////////////
      for (int j = 0; j < size; j++) { // all items
        for (size_t i = start; i < bitlen; i += step) { // one item
          auto low = i - 1;
          auto cur = std::min(low + start, bitlen - 2);

          if (cur != low) {
            P0.push_back(PP[low + j * bitlen]);
            P1.push_back(PP[cur + j * bitlen]);
            G0.push_back(GG[low + j * bitlen]);
            G1.push_back(GG[cur + j * bitlen]);

            // the first propegate no carry in
            if (!first) {
            }
          }
          first = false;
        }
      }

      vector<BitShare> tmpP, tmpG, tmpPandG;
      tmpP.insert(tmpP.end(), P1.begin(), P1.end());
      tmpP.insert(tmpP.end(), P1.begin(), P1.end());
      tmpG.insert(tmpG.end(), G0.begin(), G0.end());
      tmpG.insert(tmpG.end(), P0.begin(), P0.end());
      Mul(tmpP, tmpG, tmpPandG); // P1,P1 & G0,P0 === (P1&G0, P1&P0)

      auto P0_size = P0.size();
      vector<BitShare> tmpG1, tmpP1;
      tmpP1.insert(tmpP1.end(), tmpPandG.begin() + P0_size, tmpPandG.end()); // P1 = P1 & P0
      tmpPandG.resize(P0_size);
      tmpPandG.shrink_to_fit();
      tmpG1 = G1 ^ tmpPandG; // G1 = G1 ^ P1 & G0

      // reset back
      int index = 0;
      for (int j = 0; j < size; j++) { // all items
        for (size_t i = start; i < bitlen; i += step) { // one item
          auto low = i - 1;
          auto cur = std::min(low + start, bitlen - 2);

          if (cur != low) {
            GG[cur + j * bitlen] = tmpG1[index];
            PP[cur + j * bitlen] = tmpP1[index];
            index++;

            // the first propegate no carry in
            if (!first) {
            }
          }
          first = false;
        }
      }
      // RevealAndPrint(PP, "DEBUG curr P");
      // RevealAndPrint(GG, "DEBUG curr G");
    }
  }

  // step 3 end
  //!! P[63] = X[63] ^ Y[63]; MSB = P[63] ^ G[62];
  vector<BitShare> P63, G62;
  for (int j = 0; j < size; j++) {
    P63.push_back(PP[(j + 1) * bitlen - 1]);
    G62.push_back(GG[(j + 1) * bitlen - 2]);
  }

  C = P63 ^ G62;
  AUDIT("id:{}, P{} AdderCircuitW, output(BitShare){}", msgid.get_hex(), player, Vector<BitShare>(C));
}

/**
 * C = MSB(X) in binary share
 *
 * Rounds: Input(1) + Circuit(7)
 *
 */
void HelixInternal::MSB(const vector<Share>& X, vector<BitShare>& C) {
  AUDIT("id:{}, P{} MSB, input X(Share){}", msgid.get_hex(), player, Vector<Share>(X));
  size_t size = X.size();
  size_t bitlen = sizeof(mpc_t) * 8;

  /**
   * bitX = x - a + a0 = delta + a0
   * bitY = a1
   * X = bitX + bitY
   */

  /** 
    @note: Optimized by Junjie Shi at 2020.10.30.
    Here, because a1 is known both to P1 and P2, so we view its 'share' as:
    P0: (0,  0)
    P1: (0, a1)
    P2: (0, a1)
    In this way, bitsYs can be obtained locally, rather than calling costly 'Input'.
  */
  vector<mpc_t> bitX(size, 0), bitY(size, 0);
  for (int i = 0; i < size; i++) {
    if (player == PARTY_0) {
      bitX[i] = X[i].s0.delta + X[i].s1.A0;
    } else if (player == PARTY_1 || player == PARTY_2) {
      bitY[i] = X[i].s1.A1;
    }
  }
  AUDIT("id:{}, P{} MSB, compute bitX=X.s0.delta + X.s1.A0, bitX(mpc_t){}", msgid.get_hex(), player, Vector<mpc_t>(bitX));
  AUDIT("id:{}, P{} MSB, compute bitY=X.s1.A1, bitY(mpc_t){}", msgid.get_hex(), player, Vector<mpc_t>(bitY));

  // 1. Flatten (expand) all bits
  vector<bit_t> bitsXs(bitlen * size, 0);
  vector<BitShare> bitsShareXs(bitlen * size), bitsShareYs(bitlen * size);
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < bitlen; j++) {
      int target_idx = i * bitlen + j;
      bitsXs[target_idx] = (bitX[i] >> j) & 1; // x_1,...,x_\ell <--> bitsX[0],...bitsX[\ell-1]
      // note[Junjie Shi]: the following is a little verbose, but good for readability.
      if (player == PARTY_2) {
        bitsShareYs[target_idx].s0.A0 = 0;
        bitsShareYs[target_idx].s1.A1 = (bitY[i] >> j) & 1;
      } else if (player == PARTY_1) {
        bitsShareYs[target_idx].s0.delta = 0;
        bitsShareYs[target_idx].s1.A1 = (bitY[i] >> j) & 1;
      } else if (player == PARTY_0) {
        bitsShareYs[target_idx].s0.delta = 0;
        bitsShareYs[target_idx].s1.A0 = 0;
      }
    }
  }
  AUDIT("id:{}, P{} MSB, bitsShareYs(BitShare){}", msgid.get_hex(), player, Vector<BitShare>(bitsShareYs));

  // get inputs on each bit
  Input(io->GetNodeId(0), bitsXs, bitsShareXs); // k*\ell
  // Input(1, bitsYs, bitsShareYs); // k*\ell
  AUDIT("id:{}, P{} MSB, bitsShareXs(BitShare){}", msgid.get_hex(), player, Vector<BitShare>(bitsShareXs));

#define use_adder_circuit_L 0
#if use_adder_circuit_L
  // set back to
  vector<vector<BitShare>> bitsShareX(size, vector<BitShare>(bitlen));
  vector<vector<BitShare>> bitsShareY(size, vector<BitShare>(bitlen));
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < bitlen; j++) {
      bitsShareX[i][j] = bitsShareXs[i * bitlen + j];
      bitsShareY[i][j] = bitsShareYs[i * bitlen + j];
    }
  }

  // AdderCircuit
  AdderCircuitL(bitsShareX, bitsShareY, C);
#else
  // AdderCircuit
  AdderCircuitW(bitsShareXs, bitsShareYs, C);
#endif
  AUDIT("id:{}, P{} MSB, output(BitShare){}", msgid.get_hex(), player, Vector<BitShare>(C));

  return;
}

/**
 * get shares aX, bX for uniformly random b \in {0,1}
 *
 * \param[out] aX arithmetic share
 * \param[out] bX binary share
 */
void HelixInternal::PreTuple(vector<Share>& aX, vector<BitShare>& bX, size_t size) {
  resize_vector(aX, size);
  resize_vector(bX, size);

  vector<bit_t> S0(size, 0), S1(size, 0);
  vector<mpc_t> R0(size, 0), R1(size, 0);

  // step 1,2
  PRF02(S0, size);
  if (player == PARTY_0 || player == PARTY_2) {
    AUDIT("id:{}, P{} PreTuple PRF02, S0(bit_t){}", msgid.get_hex(), player, Vector<bit_t>(S0));
  }
  PRF02(R0, size);
  if (player == PARTY_0 || player == PARTY_2) {
    AUDIT("id:{}, P{} PreTuple PRF02, R0(mpc_t){}", msgid.get_hex(), player, Vector<mpc_t>(R0));
  }
  PRF12(S1, size);
  if (player == PARTY_1 || player == PARTY_2) {
    AUDIT("id:{}, P{} PreTuple PRF12, S1(bit_t){}", msgid.get_hex(), player, Vector<bit_t>(S1));
  }

  // step 4
  if ((player == PARTY_1) || (player == PARTY_2)) {
    if (is_helper()) {
      vector<bit_t> B0(size, 0);
      B0 = S0 + S1; // ===> b = s0 ^ s1
      R1 = B0 - R0; // r1 = b - r0

      send(PARTY_1, R1, size);
      AUDIT("id:{}, P{} PreTuple SENT to P{}, R1(=B0-R0=S0+S1-R0)(bit_t){}", msgid.get_hex(), player, PARTY_1, Vector<mpc_t>(R1));
    } else {
      recv(PARTY_2, R1, size);
      AUDIT("id:{}, P{} PreTuple RECV from P{}, R1(=B0-R0=S0+S1-R0)(bit_t){}", msgid.get_hex(), player, PARTY_2, Vector<mpc_t>(R1));
    }
  }

  // step 3,5
  for (int i = 0; i < size; i++) {
    if (is_helper()) {
      aX[i].s0.A0 = R0[i];
      aX[i].s1.A1 = R1[i];
    } else {
      aX[i].s0.delta = 0;
      if (player == PARTY_0)
        aX[i].s1.A0 = R0[i];
      else
        aX[i].s1.A1 = R1[i];
    }
  }

  for (int i = 0; i < size; i++) {
    if (is_helper()) {
      bX[i].s0.A0 = S0[i];
      bX[i].s1.A1 = S1[i];
    } else {
      bX[i].s0.delta = 0;
      if (player == PARTY_0)
        bX[i].s1.A0 = S0[i];
      else
        bX[i].s1.A1 = S1[i];
    }
  }

  AUDIT("id:{}, P{} PreTuple compute: get shares Ax, Bx for uniformly random b in [0,1], output aX(Share){}", msgid.get_hex(), player, Vector<Share>(aX));
  AUDIT("id:{}, P{} PreTuple compute: get shares Ax, Bx for uniformly random b in [0,1], output bX(bit){}", msgid.get_hex(), player, Vector<BitShare>(bX));
}

/**
 * get shares aX, bX for uniformly random b \in {0,1}
 *
 * \param[in] Y arithmetic share
 * \param[out] aX arithmetic share, b \in {0,1}
 * \param[out] bX binary share, bY
 */
void HelixInternal::PreTupleA(const vector<Share>& Y, vector<Share>& aX, vector<BitShare>& bX) {
  AUDIT("id:{}, P{} PreTupleA compute: get shares Ax, Bx for uniformly random b in [0,1], input Y(Share){}", msgid.get_hex(), player, Vector<Share>(Y));
  size_t size = Y.size();
  resize_vector(aX, size);
  resize_vector(bX, size);

  vector<bit_t> S0(size, 0), S1(size, 0);
  vector<mpc_t> R0(size, 0), R1(size, 0);
  vector<mpc_t> T0(size, 0), T1(size, 0); // R'
  vector<mpc_t> Z0(size, 0), Z1(size, 0);

  // step 1,2
  if (player == PARTY_0 || player == PARTY_2) {
    PRF02(S0, size);
    AUDIT("id:{}, P{} PreTupleA PRF02, P0 and P2 generater S0(bit_t){}", msgid.get_hex(), player, Vector<bit_t>(S0));

    PRF02(R0, size);
    AUDIT("id:{}, P{} PreTupleA PRF02, P0 and P2 generater R0(mpc_t){}", msgid.get_hex(), player, Vector<mpc_t>(R0));

    PRF02(T0, size);
    AUDIT("id:{}, P{} PreTupleA PRF02, P0 and P2 generater T0(mpc_t){}", msgid.get_hex(), player, Vector<mpc_t>(T0));

    PRF02(Z0, size);
    AUDIT("id:{}, P{} PreTupleA PRF02, P0 and P2 generater Z0(mpc_t){}", msgid.get_hex(), player, Vector<mpc_t>(Z0));
  }

  if (player == PARTY_1 || player == PARTY_2) {
    PRF12(S1, size);
    AUDIT("id:{}, P{} PreTupleA PRF12, P1 and P2 generater S1(bit_t){}", msgid.get_hex(), player, Vector<bit_t>(S1));

    PRF12(Z1, size);
    AUDIT("id:{}, P{} PreTupleA PRF12, P1 and P2 generater Z1(mpc_t){}", msgid.get_hex(), player, Vector<mpc_t>(Z1));
  }

  // step 4
  if ((player == PARTY_1) || (player == PARTY_2)) {
    if (is_helper()) {
      vector<bit_t> B0(size, 0);
      // operator overloading, see helix_util.h
      B0 = S0 + S1; // ===> b = s0 ^ s1
      R1 = B0 - R0; // r1 = b - r0
      for (int i = 0; i < size; i++) { // r'1 = ab - r'0
        T1[i] = (Y[i].s0.A0 + Y[i].s1.A1) * B0[i] - T0[i];
      }

      // send(PARTY_1, R1, size);
      // send(PARTY_1, T1, size);
      sendTwoVec(PARTY_1, R1, T1, size, size);
      AUDIT("id:{}, P{} PreTupleA SEND to P{}, R1(mpc_t){}", msgid.get_hex(), player, PARTY_1, Vector<mpc_t>(R1));
      AUDIT("id:{}, P{} PreTupleA SEND to P{}, T1(mpc_t){}", msgid.get_hex(), player, PARTY_1, Vector<mpc_t>(T1));
    } else {
      // recv(PARTY_2, R1, size);
      // recv(PARTY_2, T1, size);
      recvTwoVec(PARTY_2, R1, T1, size, size);
      AUDIT("id:{}, P{} PreTupleA RECV from P{}, R1(mpc_t){}", msgid.get_hex(), player, PARTY_2, Vector<mpc_t>(R1));
      AUDIT("id:{}, P{} PreTupleA RECV from P{}, T1(mpc_t){}", msgid.get_hex(), player, PARTY_2, Vector<mpc_t>(T1));
    }
  }

  // step 3,5,6 exchange delta_y*r + r' + z
  vector<mpc_t> dY0(size, 0), dY1(size, 0);
  if (is_primary()) {
    if (player == PARTY_0) {
      for (int i = 0; i < size; i++) {
        dY0[i] = Y[i].s0.delta * R0[i] + T0[i] - Z0[i];
      }
      send(PARTY_1, dY0, size);
      AUDIT("id:{}, P{} PreTupleA SEND to P{}, dY0(mpc_t){}", msgid.get_hex(), player, PARTY_1, Vector<mpc_t>(dY0));

      recv(PARTY_1, dY1, size);
      AUDIT("id:{}, P{} PreTupleA RECV from P{}, dY1(mpc_t){}", msgid.get_hex(), player, PARTY_1, Vector<mpc_t>(dY1));
    } else {
      for (int i = 0; i < size; i++) {
        dY1[i] = Y[i].s0.delta * R1[i] + T1[i] - Z1[i];
      }
      send(PARTY_0, dY1, size);
      AUDIT("id:{}, P{} PreTupleA SEND to P{}, dY1(mpc_t){}", msgid.get_hex(), player, PARTY_0, Vector<mpc_t>(dY1));

      recv(PARTY_0, dY0, size);
      AUDIT("id:{}, P{} PreTupleA RECV from P{}, dY0(mpc_t){}", msgid.get_hex(), player, PARTY_0, Vector<mpc_t>(dY0));
    }
    dY0 = dY0 + dY1;
    AUDIT("id:{}, P{} PreTupleA compute dY0=dY0+dY1, dY0(mpc_t){}", msgid.get_hex(), player, Vector<mpc_t>(dY0));
  }
  vector<mpc_t>& dbY = dY0;

  // step 7
  for (int i = 0; i < size; i++) {
    if (is_helper()) {
      aX[i].s0.A0 = Z0[i];
      aX[i].s1.A1 = Z1[i];
    } else {
      aX[i].s0.delta = dbY[i];
      if (player == PARTY_0)
        aX[i].s1.A0 = Z0[i];
      else
        aX[i].s1.A1 = Z1[i];
    }
  }

  for (int i = 0; i < size; i++) {
    if (is_helper()) {
      bX[i].s0.A0 = S0[i];
      bX[i].s1.A1 = S1[i];
    } else {
      bX[i].s0.delta = 0;
      if (player == PARTY_0)
        bX[i].s1.A0 = S0[i];
      else
        bX[i].s1.A1 = S1[i];
    }
  }
  
  AUDIT("id:{}, P{} PreTupleA compute: get shares aX, bX for uniformly random b in [0,1], output aX(Share){}", msgid.get_hex(), player, Vector<Share>(aX));
  AUDIT("id:{}, P{} PreTupleA compute: get shares Ax, Bx for uniformly random b in [0,1], output bX(BitShare){}", msgid.get_hex(), player, Vector<BitShare>(bX));
}

/**
 *
 * Y = C0 + SUM (C*X)
 *
 * \param C constants
 * \param c_has_scaled indicates whether C is scaled. If true, will truncate Y by call Trunc.
 */
void HelixInternal::Linear(
  const vector<mpc_t>& C,
  const vector<Share>& X,
  Share& Y,
  bool c_has_scaled) {
  Linear(X, C, Y, c_has_scaled);
}
/**
 * Y = C0 + SUM (X*C)
 */
void HelixInternal::Linear(
  const vector<Share>& X,
  const vector<mpc_t>& C,
  Share& Y,
  bool c_has_scaled) {
  AUDIT("id:{}, P{} linear compute Y = C0 + SUM(C*X), input ShareX(Share){}", msgid.get_hex(), player, Vector<Share>(X));
  AUDIT("id:{}, P{} linear compute Y = C0 + SUM(C*X), input C(mpc_t){}", msgid.get_hex(), player, Vector<mpc_t>(C));
  assert(X.size() + 1 == C.size());
  size_t size = X.size();
  int float_precision = GetMpcContext()->FLOAT_PRECISION;

  memset(&Y, 0, sizeof(Y));
  for (int i = 0; i < size; i++) {
    if (is_helper()) {
      Y.s0.A0 += X[i].s0.A0 * C[i + 1];
      Y.s1.A1 += X[i].s1.A1 * C[i + 1];
    } else {
      Y.s0.delta += X[i].s0.delta * C[i + 1];
      Y.s1.A0 += X[i].s1.A0 * C[i + 1];
    }
  }

  if (is_primary()) {
    if (c_has_scaled) {
      Y.s0.delta += (C[0] << float_precision);
    } else {
      Y.s0.delta += C[0];
    }
  }

  if (c_has_scaled) {
    vector<Share> vY = {Y};
    Trunc(vY, 1, float_precision);
    Y = vY[0];
  }

  AUDIT("id:{}, P{} linear compute Y = C0 + SUM(C*X), output Y(Share){}", msgid.get_hex(), player, Y);
}

void HelixInternal::Linear(const vector<double>& C, const vector<Share>& X, Share& Y) {
  Linear(X, C, Y);
}
void HelixInternal::Linear(const vector<Share>& X, const vector<double>& C, Share& Y) {
  vector<mpc_t> fpC;
  convert_plain_to_fixpoint(C, fpC, GetMpcContext()->FLOAT_PRECISION);
  Linear(X, fpC, Y);
}

/**
 * Binary version
 *
 * C is constant
 *
 * X.size() + 1 == C.size()
 *
 */
void HelixInternal::Linear(const vector<bit_t>& C, const vector<BitShare>& X, BitShare& Y) {
  Linear(X, C, Y);
}
void HelixInternal::Linear(const vector<BitShare>& X, const vector<bit_t>& C, BitShare& Y) {
  AUDIT("id:{}, P{} linear compute Y = C0 + SUM(C*X), input bitX{}", msgid.get_hex(), player, Vector<bit_t>(C));
  AUDIT("id:{}, P{} linear compute Y = C0 + SUM(C*X), input X{}", msgid.get_hex(), player, Vector<BitShare>(X));
  assert(X.size() + 1 == C.size());
  size_t size = X.size();

  memset(&Y, 0, sizeof(Y));
  for (int i = 0; i < size; i++) {
    if (is_helper()) {
      Y.s0.A0 ^= X[i].s0.A0 & C[i + 1];
      Y.s1.A1 ^= X[i].s1.A1 & C[i + 1];
    } else {
      Y.s0.delta ^= X[i].s0.delta & C[i + 1];
      Y.s1.A0 ^= X[i].s1.A0 & C[i + 1];
    }
  }
  if (is_primary()) {
    Y.s0.delta ^= C[0];
  }

  AUDIT("id:{}, P{} linear compute Y = C0 + SUM(C*X), output Y(BitShare): {}", msgid.get_hex(), player, Y);
}

/**
 * Rounds: PreTuple(1) + Linear(0) + Reveal(1) + Linear(0)
 *
 */
void HelixInternal::B2A(const vector<BitShare>& bitX, vector<Share>& X) {
  AUDIT("id:{}, P{} B2A compute the arithmetic, input bitX(BitShare){}", msgid.get_hex(), player, Vector<BitShare>(bitX));
  size_t size = bitX.size();

  // 1
  vector<Share> aX;
  vector<BitShare> bX;
  PreTuple(aX, bX, size);

  // 2 Linear, locally
  vector<BitShare> bC(size);
  for (int i = 0; i < size; i++) {
    Linear({bitX[i], bX[i]}, {0, 1, 1}, bC[i]);
  }

  // 3 Reveal for all parties
  vector<bit_t> pC(size);
  Reveal(bC, pC, encode_reveal_mask(7));

  // 4 Linear, locally
  for (int i = 0; i < size; i++) {
    vector<mpc_t> vc = {pC[i], (mpc_t)(1 - 2 * pC[i])};
    Linear({aX[i]}, vc, X[i], false);
  }

  AUDIT("id:{}, P{} B2A compute the arithmetic, output shareX(Share){}", msgid.get_hex(), player, Vector<Share>(X));
}

/**
 *
 * X = bitX * Y
 *
 * Rounds: PreTupleA(2) + Linear(0) + Reveal(1) + Linear(0)
 *
 */
void HelixInternal::BMA(const vector<BitShare>& bitX, const vector<Share>& Y, vector<Share>& X) {
  AUDIT("id:{}, P{} BMA compute X = bitX * Y, input bitX(BitShare){}", msgid.get_hex(), player, Vector<BitShare>(bitX));
  AUDIT("id:{}, P{} BMA compute X = bitX * Y, input Y(Share){}", msgid.get_hex(), player, Vector<Share>(Y));
  size_t size = bitX.size();
  assert(size == Y.size());

  // 1
  vector<Share> aX;
  vector<BitShare> bX;
  PreTupleA(Y, aX, bX);

  // 2 Linear, locally
  vector<BitShare> bC(size);
  for (int i = 0; i < size; i++) {
    Linear({bitX[i], bX[i]}, {0, 1, 1}, bC[i]);
  }

  // 3
  vector<bit_t> pC(size);
  Reveal(bC, pC, encode_reveal_mask(7));

  // 4 Linear, locally
  for (int i = 0; i < size; i++) {
    vector<mpc_t> vc = {0, pC[i], (mpc_t)(1 - 2 * pC[i])};
    Linear({Y[i], aX[i]}, vc, X[i], false);
  }

  AUDIT("id:{}, P{} BMA compute X = bitX * Y, output shareX(Share){}", msgid.get_hex(), player, Vector<Share>(X));
}

/**
 * DReLU \n
 * base on MSB + B2A
 *
 * \param[out] Y = DReLU(X), 0 if X < 0; else 1
 *
 */
void HelixInternal::DReLU(const vector<Share>& X, vector<Share>& Y) {
  AUDIT("id:{}, P{} ReLU compute Y = ReLu(X), if(X < 0, 0, 1), input X(Share){}", msgid.get_hex(), player, Vector<Share>(X));
  size_t size = X.size();
  resize_vector(Y, size);

  // step 1
  vector<BitShare> bitX(size);
  MSB(X, bitX);

  // 1 ^ bitX
  for (int i = 0; i < size; i++) {
    bitX[i].s0.delta = 1 ^ bitX[i].s0.delta;
    bitX[i].s1.A0 = 1 ^ bitX[i].s1.A0;
  }

  // step 2
  B2A(bitX, Y);

  AUDIT("id:{}, P{} ReLU compute Y = ReLu(X), if(X < 0, 0, 1), output Y(Share){}", msgid.get_hex(), player, Vector<Share>(Y));
}

/**
 * ReLU \n
 * base on MSB + BMA
 *
 * \param[out] Y = ReLU(X), 0 if X < 0; else X
 *
 */
void HelixInternal::ReLU(const vector<Share>& X, vector<Share>& Y) {
  AUDIT("id:{}, P{} Helix ReLU compute Y = ReLu(X), if(X < 0, 0, X), input X(Share){}", msgid.get_hex(), player, Vector<Share>(X));
  
  size_t size = X.size();
  resize_vector(Y, size);

  // step 1
  vector<BitShare> bitX(size);
  MSB(X, bitX);

  // 1 ^ bitX
  for (int i = 0; i < size; i++) {
    bitX[i].s0.delta = 1 ^ bitX[i].s0.delta;
    bitX[i].s1.A0 = 1 ^ bitX[i].s1.A0;
  }

  // step 2
  BMA(bitX, X, Y); // BMA equals to B2A, Mul

  AUDIT("id:{}, P{} Helix ReLU compute Y = ReLu(X), if(X < 0, 0, X), output Y(Share){}", msgid.get_hex(), player, Vector<Share>(Y));
}

/**
 *
 * Inner Product (vectorization version)
 * 
 * \param[in] X X[i] --> X[i][0],X[i][1],...,X[i][J-1] (J = X[i].size())
 * \param[in] Y Y[i] --> Y[i][0],Y[i][1],...,Y[i][J-1]
 * \param[out] Z Z[i]  = X[i][0] * Y[i][0] + X[i][1] * Y[i][1] + ... + X[i][J-1] * Y[i][J-1]
 * \param trunc if one of X and Y is not scaled, then set trunc to false, you can @see SigmoidPiceWise6
 */
void HelixInternal::InnerProducts(
  const vector<vector<Share>>& X,
  const vector<vector<Share>>& Y,
  vector<Share>& Z,
  bool trunc) {
  size_t size = X.size();
  resize_vector(Z, size);
  assert(size > 0);
  int J = X[0].size();

  for (int i=0; i<X.size(); ++i) {
    AUDIT("id:{}, P{} InnerProductsV input X[{}](Share){}", msgid.get_hex(), player, i, Vector<Share>(X[i]));
  }

  for (int i=0; i<Y.size(); ++i) {
    AUDIT("id:{}, P{} InnerProductsV input Y[{}](Share){}", msgid.get_hex(), player, i, Vector<Share>(Y[i]));
  }

  // step 2 Ci
  vector<mpc_t> C0(size, 0), C1(size, 0);
  PRF02(C0, size);
  if (player == PARTY_0 || player == PARTY_2) {
    AUDIT("id:{}, P{} InnerProductsV PRF02, P0 and P2 generater C0(mpc_t){}", msgid.get_hex(), player, Vector<mpc_t>(C0));
  }
  if ((player == PARTY_1) || (player == PARTY_2)) {
    if (player == PARTY_2) {
      // step 3 SUM( (A0+A1)*(B0+B1) ) - C0
      for (int i = 0; i < size; i++) {
        for (int j = 0; j < J; j++) {
          C1[i] += (X[i][j].s0.A0 + X[i][j].s1.A1) * (Y[i][j].s0.B0 + Y[i][j].s1.B1);
        }
        C1[i] = C1[i] - C0[i];
      }

      send(PARTY_1, C1, size);
      AUDIT("id:{}, P{} InnerProductsV SEND to P{}, C1(mpc_t){}", msgid.get_hex(), player, PARTY_1, Vector<mpc_t>(C1));
    } else {
      recv(PARTY_2, C1, size);
      AUDIT("id:{}, P{} InnerProductsV RECV from P{}, C1(mpc_t){}", msgid.get_hex(), player, PARTY_2, Vector<mpc_t>(C1));
    }
  }

  // step 4 Zi
  vector<mpc_t> Z0(size, 0), Z1(size, 0);
  PRF02(Z0, size);
  if (player == PARTY_0 || player == PARTY_2) {
    AUDIT("id:{}, P{} InnerProductsV PRF02, P0 and P2 generater Z0(mpc_t){}", msgid.get_hex(), player, Vector<mpc_t>(Z0));
  }
  PRF12(Z1, size);
  if (player == PARTY_1 || player == PARTY_2) {
    AUDIT("id:{}, P{} InnerProductsV PRF12, P1 and P2 generater Z1(mpc_t){}", msgid.get_hex(), player, Vector<mpc_t>(Z1));
  }

  vector<mpc_t> tildeZ(size, 0), tildeZ0(size, 0), tildeZ1(size, 0);
  vector<mpc_t> hatZ(size, 0), hatZ0(size, 0), hatZ1(size, 0);
  if (is_primary()) {
    if (player == PARTY_0) {
      // step 5 SUM(deltaX*B0 + A0*deltaY) + C0
      for (int i = 0; i < size; i++) {
        for (int j = 0; j < J; j++) {
          tildeZ0[i] += X[i][j].s0.delta * Y[i][j].s1.B0 + X[i][j].s1.A0 * Y[i][j].s0.delta;
        }
        tildeZ0[i] = tildeZ0[i] + C0[i];
      }
      AUDIT("id:{}, P{} InnerProductsV, compute tildeZ0=tildeZ0+C0, tildeZ0(mpc_t){}", msgid.get_hex(), player, Vector<mpc_t>(tildeZ0));

      // step 6,7
      if (trunc)
        Trunc(tildeZ0, size, GetMpcContext()->FLOAT_PRECISION);

      hatZ0 = tildeZ0 - Z0;
      AUDIT("id:{}, P{} InnerProductsV, compute hatZ0=tildeZ0-Z0, hatZ0(mpc_t){}", msgid.get_hex(), player, Vector<mpc_t>(hatZ0));

      send(PARTY_1, hatZ0, size);
      AUDIT("id:{}, P{} InnerProductsV SEND to P{}, hatZ0(mpc_t){}", msgid.get_hex(), player, PARTY_1, Vector<mpc_t>(hatZ0));

      recv(PARTY_1, hatZ1, size);
      AUDIT("id:{}, P{} InnerProductsV RECV from P{}, hatZ1(mpc_t){}", msgid.get_hex(), player, PARTY_1, Vector<mpc_t>(hatZ1));
    } else {
      // step 5 SUM(deltaX*deltaY + deltaX*B1 + A1*deltaY) + C1
      for (int i = 0; i < size; i++) {
        for (int j = 0; j < J; j++) {
          tildeZ1[i] += X[i][j].s0.delta * Y[i][j].s0.delta + X[i][j].s0.delta * Y[i][j].s1.B1 +
            X[i][j].s1.A1 * Y[i][j].s0.delta;
        }
        tildeZ1[i] = tildeZ1[i] + C1[i];
      }
      AUDIT("id:{}, P{} InnerProductsV, compute tildeZ1=tildeZ1+C1, tildeZ1(mpc_t){}", msgid.get_hex(), player, Vector<mpc_t>(tildeZ1));

      // step 6,7
      if (trunc)
        Trunc(tildeZ1, size, GetMpcContext()->FLOAT_PRECISION);

      hatZ1 = tildeZ1 - Z1;
      AUDIT("id:{}, P{} InnerProductsV, compute hatZ1=tildeZ1-Z1, hatZ1(mpc_t){}", msgid.get_hex(), player, Vector<mpc_t>(hatZ1));

      recv(PARTY_0, hatZ0, size);
      AUDIT("id:{}, P{} InnerProductsV RECV from P{}, hatZ0(mpc_t){}", msgid.get_hex(), player, PARTY_1, Vector<mpc_t>(hatZ0));
      send(PARTY_0, hatZ1, size);
      AUDIT("id:{}, P{} InnerProductsV SEND to P{}, hatZ1(mpc_t){}", msgid.get_hex(), player, PARTY_0, Vector<mpc_t>(hatZ1));
    }

    // step 8 reveal hatZ
    hatZ = hatZ0 + hatZ1;
    AUDIT("id:{}, P{} InnerProductsV, compute hatZ=hatZ0+hatZ1, hatZ(mpc_t){}", msgid.get_hex(), player, Vector<mpc_t>(hatZ));
  }

  // 4. each party sets the share
  for (int i = 0; i < size; i++) {
    if (is_helper()) {
      Z[i].s0.A0 = Z0[i];
      Z[i].s1.A1 = Z1[i];
    } else {
      Z[i].s0.delta = hatZ[i];
      if (player == PARTY_0)
        Z[i].s1.A0 = Z0[i];
      else
        Z[i].s1.A1 = Z1[i];
    }
  }

  AUDIT("id:{}, P{} InnerProductsV, output Z(Share){}", msgid.get_hex(), player, Vector<Share>(Z));
}

/**
 * InnerProducts (flat version, this will call the vectorization one internally)
 * satisfy:
 * J>0
 * X.size() == Y.size() >= J
 * X.size() % J == 0
 * X,Y row first
 *
 * first convert X to XX, Y to YY :
 * XX[i] --> XX[i][0],XX[i][1],...,XX[i][J-1] (from X[i*J+j])
 * YY[i] --> YY[i][0],YY[i][1],...,YY[i][J-1] (from Y[i*J+j])
 *
 */
void HelixInternal::InnerProducts(
  const vector<Share>& X,
  const vector<Share>& Y,
  vector<Share>& Z,
  size_t J,
  bool trunc) {
  AUDIT("id:{}, P{} InnerProducts, input X(Share){}", msgid.get_hex(), player, Vector<Share>(X));
  AUDIT("id:{}, P{} InnerProducts, input Y(Share){}", msgid.get_hex(), player, Vector<Share>(Y));
  assert(J > 0);
  int I = X.size() / J;
  assert(I > 0);
  assert(X.size() == Y.size());
  assert(X.size() % J == 0);

  vector<vector<Share>> XX(I, vector<Share>(J));
  vector<vector<Share>> YY(I, vector<Share>(J));
  for (int i = 0; i < I; i++) {
    for (int j = 0; j < J; j++) {
      XX[i][j] = X[i * J + j];
      YY[i][j] = Y[i * J + j];
    }
  }
  InnerProducts(XX, YY, Z, trunc);
  AUDIT("id:{}, P{} InnerProducts, output(Share){}", msgid.get_hex(), player, Vector<Share>(Z));
}

void HelixInternal::InnerProducts(
  const vector<Share>& X,
  const vector<Share>& Y,
  Share& Z,
  bool trunc) {
  vector<Share> vZ = {Z};
  InnerProducts(X, Y, vZ, X.size(), trunc);
  Z = vZ[0];
}

/**
 * Y[j][i] = X_i^(j+1), which j in [0, N-1]
 *
 * Comm. Rounds: 2*ceil(logN)
 * Comm. Counts: P0/P1/P2: ceil(logN)
 *
 * @code
 * vector<Share> X;
 * size_t N = 6;
 * vector<vector<Share>> Y;
 * Pow(X,N,Y);
 * // for each X[i], we will get
 * // Y[0][i] = {X[0]^1,X[1]^1,...,X[i]^1}
 * // Y[1][i] = {X[0]^2,X[1]^2,...,X[i]^2}
 * // Y[2][i] = {X[0]^3,X[1]^3,...,X[i]^3}
 * // ...
 * // Y[5][i] = {X[0]^6,X[1]^6,...,X[i]^6}
 * @endcode
 *
 *
 * x^a * x^b = x^(a+b) = x^c
 * a  b  c
 * ---------
 * 1  1  2
 * ---------
 * 1  2  3
 * 2  2  4
 * ---------
 * 1  4  5
 * 2  4  6
 * 3  4  7
 * 4  4  8
 * ---------
 * 1  8  9
 */
void HelixInternal::Pow(const vector<Share>& X, size_t N, vector<vector<Share>>& Y) {
  AUDIT("id:{}, P{} Pow, input X{}", msgid.get_hex(), player, Vector<Share>(X));
  size_t size = X.size();
  assert(size > 0);

  Y.clear();
  Y.resize(N, vector<Share>(size));
  Y[0].assign(X.begin(), X.end());
  if (1 == N)
    return;

  bool ok = false;
  for (int k = 0; !ok; k++) {
    vector<Share> tmpY1;
    vector<Share> tmpY2;
    int b = pow(2, k);
    for (int a = 1; a <= b; a++) {
      int c = a + b;
      if (c > N) {
        ok = true;
        break;
      }
      tmpY1.insert(tmpY1.end(), Y[a - 1].begin(), Y[a - 1].end()); // x^a, index: 1,2,...,b
      tmpY2.insert(tmpY2.end(), Y[b - 1].begin(), Y[b - 1].end()); // x^b, index: b,b,...,b
    }
    vector<Share> tmpY3(tmpY1.size());
    Mul(tmpY1, tmpY2, tmpY3); // x^a * x^b = x^(a+b) = x^c
    for (int a = 1; a <= b; a++) {
      int c = a + b;
      if (c <= N) {
        Y[c - 1].assign(tmpY3.begin() + (a - 1) * size, tmpY3.begin() + a * size); // x^c
      }
    }
  }
  for (int i=0; i<Y.size(); ++i) {
    AUDIT("id:{}, P{} Pow, output Y[{}](Share){}", msgid.get_hex(), player, i, Vector<Share>(Y[i]));
  }
}
/**
 * Rounds: 2*(N-1)
 */
void HelixInternal::Pow_original(const vector<Share>& X, size_t N, vector<vector<Share>>& Y) {
  AUDIT("id:{}, P{} Pow_original, input X{}", msgid.get_hex(), player, Vector<Share>(X));
  size_t size = X.size();
  assert(size > 0);
  assert(N > 1);
  Y.clear();
  Y.resize(N, vector<Share>(size));

  Y[0].assign(X.begin(), X.end());
  bool ok = false;
  for (int k = 0; !ok; k++) {
    int b = pow(2, k);
    for (int a = 1; a <= b; a++) {
      int c = a + b;
      if (c > N) {
        ok = true;
        break;
      }
      Mul(Y[a - 1], Y[b - 1], Y[c - 1]); // x^a * x^b = x^(a+b) = x^c
    }
  }
  for (int i=0; i<Y.size(); ++i) {
    AUDIT("id:{}, P{} Pow_original, output Y[{}](Share){}", msgid.get_hex(), player, i, Vector<Share>(Y[i]));
  }
}

/**
 *
 * Y[j][i] = X_i^(N[j])
 *
 * @code
 * vector<Share> X;
 * vector<size_t> N = {2,3,6,8};
 * vector<vector<Share>> Y;
 * Pow(X,N,Y);
 * // for each X[i], we will get
 * // Y[0][i] = {X[0]^2,X[1]^2,...,X[i]^2}
 * // Y[1][i] = {X[0]^3,X[1]^3,...,X[i]^3}
 * // Y[2][i] = {X[0]^6,X[1]^6,...,X[i]^6}
 * // Y[3][i] = {X[0]^8,X[1]^8,...,X[i]^8}
 * @endcode
 */
void HelixInternal::Pow(const vector<Share>& X, const vector<size_t>& N, vector<vector<Share>>& Y) {
  size_t size = X.size();
  size_t n = N.size();
  assert(n > 0);
  Y.clear();
  Y.resize(n, vector<Share>(size));

  assert(false && "todo");
}

/**
 * Sigmoid, 6 picewise
 *
 * 0 (-inf, -4) 0
 * 1 [-4, -2)   0.0484792 * x + 0.1998976
 * 2 [-2, 0)    0.1928931 * x + 0.4761351
 * 3 [0, 2)     0.1928931 * x + 0.5238649
 * 4 [2, 4)     0.0484792 * x + 0.8001024
 * 5 [4, +inf)  1
 *
 * f(x) = a*x + b
 * F(x) = f0(x) + (x-c1)(f1(x)-f0(x)) + ...
 *
 * f1-f0 =  0.0484792 * x + 0.1998976
 * f2-f1 =  0.1444139 * x + 0.2762375
 * f3-f2 =  0.0000000 * x + 0.0477298
 * f4-f3 = -0.1444139 * x + 0.2762375
 * f5-f4 = -0.0484792 * x + 0.1998976
 *
 * @see SigmoidPiceWise6
 */
void HelixInternal::SigmoidPiceWise6(const vector<Share>& X, vector<Share>& Y) {
  AUDIT("id:{}, P{} SigmoidPiceWise6, input X(Share){}", msgid.get_hex(), player, Vector<Share>(X));
  size_t size = X.size();
  resize_vector(Y, size);

  // 1. Compare
  vector<vector<Share>> CMP(size, vector<Share>(5));
  // vectorization-style to call the costly GreaterEqual only once.
  {
    vector<double> batch_cmp_C;
    batch_cmp_C.insert(batch_cmp_C.end(), size, -4);
    batch_cmp_C.insert(batch_cmp_C.end(), size, -2);
    batch_cmp_C.insert(batch_cmp_C.end(), size, 0);
    batch_cmp_C.insert(batch_cmp_C.end(), size, 2);
    batch_cmp_C.insert(batch_cmp_C.end(), size, 4);

    vector<Share> batch_cmp_X;
    batch_cmp_X.insert(batch_cmp_X.end(), X.begin(), X.end());
    batch_cmp_X.insert(batch_cmp_X.end(), X.begin(), X.end());
    batch_cmp_X.insert(batch_cmp_X.end(), X.begin(), X.end());
    batch_cmp_X.insert(batch_cmp_X.end(), X.begin(), X.end());
    batch_cmp_X.insert(batch_cmp_X.end(), X.begin(), X.end());

    vector<Share> batch_cmp_res(batch_cmp_X.size());
    GreaterEqual(batch_cmp_X, batch_cmp_C, batch_cmp_res);
    batch_cmp_X.clear();
    batch_cmp_C.clear();
    for (int i = 0; i < size; i++) {
      CMP[i][0] = batch_cmp_res[i];
      CMP[i][1] = batch_cmp_res[1 * size + i];
      CMP[i][2] = batch_cmp_res[2 * size + i];
      CMP[i][3] = batch_cmp_res[3 * size + i];
      CMP[i][4] = batch_cmp_res[4 * size + i];
    }
    batch_cmp_res.clear();
  }

  // 2. Mul-const, 1 times, size: 2 * k
  vector<vector<Share>> YY(size, vector<Share>(5));
  {
    vector<Share> TmpX(2 * size); // X[0],X[1],...,X[0],X[1],...
    vector<double> TmpA(2 * size); // 0.0484792,...,0.1444139
    vector<Share> TmpY(2 * size);

    //! @todo optimized assignment
    for (int i = 0; i < size; i++) {
      TmpX[i] = X[i];
      TmpA[i] = 0.0484792;
    }
    for (int i = 0; i < size; i++) {
      TmpX[size + i] = X[i];
      TmpA[size + i] = 0.1444139;
    }
    Mul(TmpX, TmpA, TmpY); // locally multiply constants (scaled), and one trunc

    // -TmpY
    vector<Share> negTmpY(2 * size);
    Negative(TmpY, negTmpY);

	  int float_precision = GetMpcContext()->FLOAT_PRECISION;
    // YY = [[TmpY, 0, -TmpY],[TmpY, 0, -TmpY],...]
    for (int i = 0; i < size; i++) {
      YY[i][0] = TmpY[i + 0 * size];
      YY[i][1] = TmpY[i + 1 * size];
      YY[i][3] = negTmpY[i + 1 * size];
      YY[i][4] = negTmpY[i + 0 * size];

      if (is_primary()) {
        YY[i][0].s0.delta += FloatToMpcType(0.1998976, float_precision);
        YY[i][1].s0.delta += FloatToMpcType(0.2762375, float_precision);
        YY[i][2].s0.delta += FloatToMpcType(0.0477298, float_precision);
        YY[i][3].s0.delta += FloatToMpcType(0.2762375, float_precision);
        YY[i][4].s0.delta += FloatToMpcType(0.1998976, float_precision);
      }
    }
  }

  // 3. InnerProducts, 1 times, size: k
  InnerProducts(CMP, YY, Y, false);
  AUDIT("id:{}, P{} SigmoidPiceWise6, output(Share){}", msgid.get_hex(), player, Vector<Share>(Y));
}

/**
 * Simgoid, this is no optimized version
 *
 * @see SigmoidPiceWise6
 */
void HelixInternal::SigmoidPiceWise6_original_not_optimized(
  const vector<Share>& X,
  vector<Share>& Y) {
  AUDIT("id:{}, P{} SigmoidPiceWise6_original_not_optimized, input X(Share){}", msgid.get_hex(), player, Vector<Share>(X));
  size_t size = X.size();
  resize_vector(Y, size);

  vector<double> C1(size, -4), C2(size, -2), C3(size, 0), C4(size, 2), C5(size, 4);
  vector<Share> cmp1(size), cmp2(size), cmp3(size), cmp4(size), cmp5(size);
  vector<Share> Y1(size), Y2(size), Y3(size), Y4(size), Y5(size);

  // if X >= Ci
  GreaterEqual(X, C1, cmp1);
  GreaterEqual(X, C2, cmp2);
  GreaterEqual(X, C3, cmp3);
  GreaterEqual(X, C4, cmp4);
  GreaterEqual(X, C5, cmp5);

  // Linear
  // clang-format off
  vector<double> A1(size, 0.0484792), A2(size, 0.1444139), A3(size, 0),         A4(size, -0.1444139), A5(size, -0.0484792);
  vector<double> B1(size, 0.1998976), B2(size, 0.2762375), B3(size, 0.0477298), B4(size,  0.2762375), B5(size,  0.1998976);
  // clang-format on

  //! @todo optimize
  for (int i = 0; i < size; i++) {
    vector<Share> x = {X[i]};
    vector<double> c = {B1[i], A1[i]};
    Linear(x, c, Y1[i]);
    //
    c = {B2[i], A2[i]};
    Linear(x, c, Y2[i]);
    //
    c = {B3[i], A3[i]};
    Linear(x, c, Y3[i]);
    //
    c = {B4[i], A4[i]};
    Linear(x, c, Y4[i]);
    //
    c = {B5[i], A5[i]};
    Linear(x, c, Y5[i]);
  }

  // InnerProducts
  for (int i = 0; i < size; i++) {
    vector<Share> x = {cmp1[i], cmp2[i], cmp3[i], cmp4[i], cmp5[i]};
    vector<Share> y = {Y1[i], Y2[i], Y3[i], Y4[i], Y5[i]};
    InnerProducts(x, y, Y[i], false);
  }
  AUDIT("id:{}, P{} SigmoidPiceWise6_original_not_optimized, output(Share){}", msgid.get_hex(), player, Vector<Share>(Y));
}

/**
 * y = 0.5 + 0.2159198015 * x -0.0082176259 * x^3 + 0.0001825597 * x^5 - 0.0000018848 * x^7 + 0.0000000072 * x^9
 */
void HelixInternal::SigmoidChebyshev(const vector<Share>& X, vector<Share>& Y) {
  AUDIT("id:{}, P{} SigmoidChebyshev, input X(Share){}", msgid.get_hex(), player, Vector<Share>(X));
  size_t size = X.size();
  resize_vector(Y, size);

  vector<double> A0(size, 0.5 / 2), A1(size, 0.2159198015), A3(size, -0.0082176259),
    A5(size, 0.0001825597), A7(size, -0.0000018848), A9(size, 0.0000000072);

  vector<Share> X1 = X;
  vector<Share> X2, X3, X5, X7, X9;
  Mul(X1, X1, X2);
  Mul(X1, X2, X3);
  Mul(X2, X3, X5);
  Mul(X2, X5, X7);
  Mul(X2, X7, X9);

  vector<Share> _X1, _X3, _X5, _X7, _X9;
  Mul(X1, A1, _X1);
  Mul(X3, A3, _X3);
  Mul(X5, A5, _X5);
  Mul(X7, A7, _X7);
  Mul(X9, A9, _X9);

  Y = _X1;
  Add(Y, _X3);
  Add(Y, _X5);
  Add(Y, _X7);
  Add(Y, _X9);
  Add(Y, A0);
  AUDIT("id:{}, P{} SigmoidChebyshev, output(Share){}", msgid.get_hex(), player, Vector<Share>(Y));
}

void HelixInternal::Sigmoid(const vector<Share>& X, vector<Share>& Y) {
  AUDIT("id:{}, P{} Sigmoid compute Y=sigmode(X), input X(Share){}", msgid.get_hex(), player, Vector<Share>(X));
#if 0
  SigmoidChebyshev(X, Y);
#else
  SigmoidPiceWise6(X, Y);
#endif
  AUDIT("id:{}, P{} Sigmoid compute Y=sigmode(X), output Y(Share){}", msgid.get_hex(), player, Vector<Share>(Y));
}

void HelixInternal::Exp(const vector<Share>& X, vector<Share>& Y) {
  //// version1: exp(x) = (1 + x / n) ^ n, n can be 8, or 16
  //// actually we expect: exp(x) = (1 + x*0.002) ^ 500
  log_debug << "Exp ...";
  AUDIT("id:{}, P{} Exp(S), compute: Y=Exp(X), input X(Share){}", msgid.get_hex(), player, Vector<Share>(X));

  size_t size = X.size();
  size_t n = 500;
  const vector<double_t> n_reciprocal(size, 0.002);
  vector<Share> m(size);
  vector<Share> result(size);

  Mul(n_reciprocal, X, m);
  
  vector<Share> ones(size);
  const vector<double> float_ones(size, 1.0);
  ConstCommonInput(float_ones, ones);

  vector<Share> temp(size);
  Add(m, ones, temp);
  PowV2(temp, n, Y);

  AUDIT("id:{}, P{} Exp(S), compute: Y=Sqrt(X), output Y(Share){}", msgid.get_hex(), player, Vector<Share>(Y));
  log_debug << "Exp ok."; 
}

void HelixInternal::Sqrt(const vector<Share>& X, vector<Share>& Y) {
  // Sqrt(x) = Rsqrt(x) * x
  log_debug << "Sqrt ...";
  AUDIT("id:{}, P{} Sqrt(S), compute: Y=Sqrt(X), input X(Share){}", msgid.get_hex(), player, Vector<Share>(X));

  vector<Share> rsqrt(X.size());
  Rsqrt(X, rsqrt);
  Mul(rsqrt, X, Y);

  AUDIT("id:{}, P{} Sqrt(S), compute: Y=Sqrt(X), output Y(Share){}", msgid.get_hex(), player, Vector<Share>(Y));
  log_debug << "Sqrt ok.";
}

void HelixInternal::Rsqrt(const vector<Share>& X, vector<Share>& Y) {
  log_debug << "Helix Rsqrt ...";
  AUDIT("id:{}, P{} Rsqrt(S), compute: Y=1/sqrt(X), input X(Share){}", msgid.get_hex(), player, Vector<Share>(X));
  
  /*
    y = exp(self.div(2).add(0.2).neg()).mul(2.2).add(0.2)
    y -= self.div(1024)
    for _ in range(config.sqrt_nr_iters):
        y = y.mul_(3 - self * y.square()).div_(2)
    return y
  */
  const vector<Share>& a = X;
  const int sqrt_nr_iters = 3;
  vector<Share> rsqrt_nr_initial = a;
  size_t size = a.size();

  const vector<double> float_half(size, 0.5);
  const vector<double> float_two_ten(size, 0.2);
  const vector<double> float_two_dot_two(size, 2.2);
  const vector<double> float_milli(size, 0.0009765625);
  const vector<double> float_three(size, 3);

  vector<Share> inter_Number(size);

  Mul(a, float_half, rsqrt_nr_initial);

  Add(rsqrt_nr_initial, float_two_ten, inter_Number); //x/2+0.2

  Negative(inter_Number, rsqrt_nr_initial); // negative

  Exp(rsqrt_nr_initial, inter_Number); //exp(-(x/2+0.2))

  Mul(inter_Number, float_two_dot_two, rsqrt_nr_initial); // mul 2.2

  Add(rsqrt_nr_initial, float_two_ten, inter_Number);//add 0.2

  vector<Share> temp(a.size());
  Mul(a, float_milli, temp); //compute a/1024
  Sub(inter_Number, temp, rsqrt_nr_initial); //rsqrt_nr_initial -=a/1024

  vector<Share> temp_number0(size), temp_number1(size), temp_number2(size);
  for (int i = 0; i < sqrt_nr_iters; i++)
  {
    // y.mul_(3 - self * y.square()).div_(2)
    Mul(rsqrt_nr_initial, rsqrt_nr_initial, temp_number1);
    Mul(temp_number1, a, temp_number0); //temp_number0 = a * rsqrt_nr_initial^2
    Negative(temp_number0, temp_number1);

    Add(temp_number1, float_three, temp_number0);
    
    Mul(temp_number0, rsqrt_nr_initial, temp_number2); 
    Mul(temp_number2, float_half, temp_number0);
    rsqrt_nr_initial = temp_number0;
  }     

  // Y = rsqrt_nr_initial;
  // ABS(Y)
  auto& a_sign = temp_number0;
  auto& sign_multiplier = temp_number1;
  DReLU(rsqrt_nr_initial, a_sign);
  Select1Of2(vector<double>(size, 1.0), vector<double>(size, -1.0), a_sign, sign_multiplier);
  Mul(sign_multiplier, rsqrt_nr_initial, Y);

  AUDIT("id:{}, P{} Rsqrt(S), compute: Y=1/sqrt(X), output Y(Share){}", msgid.get_hex(), player, Vector<Share>(Y));
  log_debug << "helix Rsqrt ok.";
}

} // namespace helix
} // namespace rosetta
