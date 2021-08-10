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

#include <cassert>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <chrono>
#include <random>
#include <unordered_map>
#include <algorithm>
using namespace std;

#include "cc/modules/common/include/utils/helper.h"
#include "cc/modules/common/include/utils/model_tool.h"
#include "cc/modules/common/include/utils/simple_timer.h"
#include "cc/modules/protocol/public/include/protocol_base.h"
#include "cc/modules/protocol/utility/include/prg.h"
#include "cc/modules/protocol/mpc/helix/include/helix_def.h"
#include "cc/modules/protocol/mpc/helix/include/helix_util.h"
#include "cc/modules/protocol/mpc/helix/include/helix_prgobjs.h"
#include "cc/modules/protocol/mpc/comm/include/mpc_prg_controller.h"

namespace rosetta {
class RttPRG;

namespace helix {

template <typename T>
inline void resize_vector(vector<T>& v, size_t size) {
  v.clear();
  v.resize(size);
  v.shrink_to_fit();
}

class HelixInternal {
  int player = -1;
  shared_ptr<NET_IO> io;
  SimpleTimer timer;
  string op_token = "";
  const msg_id_t& msgid;
  
 public:
  std::shared_ptr<RttPRG> gseed = nullptr;
  shared_ptr<ProtocolContext> context_ = nullptr;
  shared_ptr<MpcPRGObjsV2> prgobjs = nullptr;
  shared_ptr<MpcKeyPrgController> prg_controller_;

 public:
  HelixInternal(
    int _player,
    shared_ptr<NET_IO> _io,
    const msg_id_t& _msgid,
    shared_ptr<ProtocolContext> context, 
    shared_ptr<MpcKeyPrgController> controller)
      : player(_player),
        io(_io),
        op_token(_msgid.str()),
        msgid(_msgid),
        context_(context),
        prg_controller_(controller) {
    if (prg_controller_ != nullptr) {
      prgobjs = prg_controller_->Get(msgid);
    }
  }
  ~HelixInternal() {}

  void beg_statistics() {
    timer.start();
    io->clear_statistics();
  }
  void end_statistics(std::string msg) {
    cout << setw(15) << msg << " elapsed(us) P" << player << " " << timer.us_elapse() << endl;
    io->statistics(msg);
// some more context info if needed.
#if 0
  PerfStats perf_stats;
  io::NetStat net_stat = io->net_stat();
  perf_stats.s.bytes_sent = net_stat.bytes_sent();
  perf_stats.s.bytes_recv = net_stat.bytes_received();
  perf_stats.s.msg_sent = net_stat.message_sent();
  perf_stats.s.msg_recv = net_stat.message_received();
  cout << perf_stats.to_json() << endl;
#endif
  }

  inline bool is_primary() { return ((player == PARTY_0) || (player == PARTY_1)); }
  inline bool is_helper() { return (player == PARTY_2); }
  inline int party_id() { return player; }

  shared_ptr<ProtocolContext> GetMpcContext() { return context_; }

 private:
  /**
  * io
  */
  int send(int p, const char* data, size_t size) {
    int ret = io->send(p, data, size, msgid);

    AUDIT("id:{}, P{} IO SEND to P{}{}", msgid.get_hex(), player, p, CStr(data, size));
    return ret;
  }
  int recv(int p, char* data, size_t size) {
    int ret = io->recv(p, data, size, msgid);

    AUDIT("id:{}, P{} IO RECV from P{}{}", msgid.get_hex(), player, p, CStr(data, size));
    return ret;
  }
  int send(const string& node_id, const char* data, size_t size) {
    int ret = io->send(node_id, data, size, msgid);

    AUDIT("id:{}, P{} IO SEND to {}{}", msgid.get_hex(), player, node_id, CStr(data, size));
    return ret;
  }
  int recv(const string& node_id, char* data, size_t size) {
    int ret = io->recv(node_id, data, size, msgid);

    AUDIT("id:{}, P{} IO RECV from {}{}", msgid.get_hex(), player, node_id, CStr(data, size));
    return ret;
  }
  template <typename T>
  int send(const string& node_id, const vector<T>& data, size_t size) {
    if (sizeof(T) == sizeof(mpc_t)) {
      AUDIT("id:{}, P{} IO will SEND to {}{}", msgid.get_hex(), player, node_id, Vector<T>(data));
      return io->send(node_id, data, size, msgid);
    }

    int len = (size + 7) / 8;
    unsigned char* ptr = new unsigned char[len];
    memset(ptr, 0, len);
    int i = 0;
    for (int j = 0; j < len; j++) {
      for (int k = 0; k < 8; k++) {
        if (i < size) {
          ptr[j] = (((data[i++] & 0x1) << k) & 0xFF) | ptr[j];
        }
      }
    }

    int ret = io->send(node_id, (const char*)ptr, len, msgid);
    AUDIT("id:{}, P{} IO SEND to {}{}", msgid.get_hex(), player, node_id, CStr((const char*)ptr, len));
    delete[] ptr;
    return ret;
  }

  template <typename T>
  int recv(const string& node_id, vector<T>& data, size_t size) {
    if (sizeof(T) == sizeof(mpc_t)) {
      AUDIT("id:{}, P{} IO will RECV from {}{}", msgid.get_hex(), player, node_id, Vector<T>(data));
      return io->recv(node_id, data, size, msgid);
    }

    int len = (size + 7) / 8;
    unsigned char* ptr = new unsigned char[len];
    memset(ptr, 0, len);
    int ret = io->recv(node_id, (char*)ptr, len, msgid);
    AUDIT("id:{}, P{} IO RECV from {}{}", msgid.get_hex(), player, node_id, CStr((const char*)ptr, len));

    int i = 0;
    for (int j = 0; j < len; j++) {
      for (int k = 0; k < 8; k++) {
        if (i < size) {
          data[i++] = (ptr[j] >> k) & 0x01;
        }
      }
    }
    delete[] ptr;
    return ret;
  }

  template <typename T>
  int send(int p, const vector<T>& data, size_t size) {
    string node_id = io->GetNodeId(p);
    return send(node_id, data, size);
  }

  template <typename T>
  int recv(int p, vector<T>& data, size_t size) {
    string node_id = io->GetNodeId(p);
    return recv(node_id, data, size);
  }
  
  int sendTwoVec(int p, const vector<mpc_t>& vec_a, const vector<mpc_t>& vec_b, size_t size_a, size_t size_b) {
    assert(vec_a.size() == size_a && vec_b.size() == size_b);
    vector<mpc_t> batch_vec;
    batch_vec.insert(batch_vec.end(), vec_a.begin(), vec_a.end());
    batch_vec.insert(batch_vec.end(), vec_b.begin(), vec_b.end());
    return send(p, batch_vec, size_a + size_b);
  }

  int recvTwoVec(int p, vector<mpc_t>& vec_a, vector<mpc_t>& vec_b, size_t size_a, size_t size_b) {
    vector<mpc_t> batch_vec(size_a + size_b, 0);
    int ret = recv(p, batch_vec, size_a + size_b);
    vec_a.clear();
    vec_b.clear();
    vec_a.insert(vec_a.end(), batch_vec.begin(), batch_vec.begin() + size_a);
    vec_b.insert(vec_b.end(), batch_vec.begin() + size_a, batch_vec.end());
    return ret;
  }

 private:
  /**
  * template
  */

  /**
 * Reveal, arithmetic
 * 
 * \param plain the result (fixpoint for arithmetic), sets to p
 * \param p bit-wised, which party will get the plaintext
 * 
 * p --> 0x 0 1 1 1
 * P -->      2 1 0
 * eg.
 * p ==> 0x ... 0000 0001 --> P0
 * p ==> 0x ... 0000 0101 --> P2 & P0
 * p ==> 0x ... 0000 0111 --> P2 & P1 & P0
 * and so on.
 * 
 * for balancing traffic:
 * reveal P0: P1 sends A1 to P0
 * reveal P1: P2 sends A0 to P1
 * reveal P2: P0 sends delta to P2
 * 
 * @note
 * <T1,T2> ==>  <Share, mpc_t> or <BitShare, bit_t>
 */

  template <typename T1, typename T2>
  void Reveal_(const vector<T1>& X, vector<T2>& plain, const string& nodes) {
    AUDIT("id:{}, P{} Reveal, input X{}", msgid.get_hex(), player, Vector<T1>(X));
    vector<string> result_nodes = io->GetResultNodes();
    vector<string> parties = decode_reveal_nodes(nodes, io->GetParty2Node(), result_nodes);
    Reveal_(X, plain, parties);
  }

  template <typename T1, typename T2>
  void Reveal_(const vector<T1>& X, vector<T2>& plain, const vector<string>& parties) {
    AUDIT("id:{}, P{} Reveal_, input X{}", msgid.get_hex(), player, Vector<T1>(X));
    size_t size = X.size();
    resize_vector(plain, size);

    // for convenience
    vector<T2> DeltaX(size), A0(size), A1(size);
    for (int i = 0; i < size; i++) {
      if (is_helper()) {
        A0[i] = X[i].s0.A0;
        AUDIT("id:{}, P{} Reveal_, locally holds A0{}", msgid.get_hex(), player, Vector<T2>(A0));
        A1[i] = X[i].s1.A1;
        AUDIT("id:{}, P{} Reveal_, locally holds A1{}", msgid.get_hex(), player, Vector<T2>(A1));
      } else if (is_primary()) {
        DeltaX[i] = X[i].s0.delta;
        AUDIT("id:{}, P{} Reveal_, locally holds DeltaX{}", msgid.get_hex(), player, Vector<T2>(DeltaX));
        if (player == PARTY_0) {
          A0[i] = X[i].s1.A0;
          AUDIT("id:{}, P{} Reveal_, locally holds A0{}", msgid.get_hex(), player, Vector<T2>(A0));
        } else {
          A1[i] = X[i].s1.A1;
          AUDIT("id:{}, P{} Reveal_, locally holds A1{}", msgid.get_hex(), player, Vector<T2>(A1));
        }
      }
    }

    string node_0 = io->GetNodeId(PARTY_0);
    string node_1 = io->GetNodeId(PARTY_1);
    string node_2 = io->GetNodeId(PARTY_2);
    string current_node = io->GetCurrentNodeId();
    for (int i = 0; i < parties.size(); i++) {
      if (parties[i] == node_0) {
        if (player == PARTY_0) {
          recv(node_1, A1, size);
          AUDIT("id:{}, P{} Reveal_ P{} RECV from {}, A1{}", msgid.get_hex(), player, player, node_1, Vector<T2>(A1));
          plain = DeltaX + A0 + A1;

        } else if (player == PARTY_1) {
          send(node_0, A1, size);
          AUDIT("id:{}, P{} Reveal_, P{} SEND to {}, A1{}", msgid.get_hex(), player, player, node_0, Vector<T2>(A1));
        }
      }
      else if (parties[i] == node_1) {
        if (player == PARTY_1) {
          recv(node_2, A0, size);
          AUDIT("id:{}, P{} Reveal_, P{} RECV from {}, A0{}", msgid.get_hex(), player, player, node_2, Vector<T2>(A0));
          plain = DeltaX + A0 + A1;
        } else if (player == PARTY_2) {
          send(node_1, A0, size);
          AUDIT("id:{}, P{} Reveal_, P{} SEND to {}, A0{}", msgid.get_hex(), player, player, node_1, Vector<T2>(A0));
        }
      }
      else if (parties[i] == node_2) {
        if (player == PARTY_2) {
          recv(node_0, DeltaX, size);
          AUDIT("id:{}, P{} Reveal_, P{} RECV from {} DeltaX{}", msgid.get_hex(), player, player, node_0, Vector<T2>(DeltaX));
          plain = DeltaX + A0 + A1;
        } else if (player == PARTY_0) {
          send(node_2, DeltaX, size);
          AUDIT("id:{}, P{} Reveal_, P{} SEND to {} DeltaX{}", msgid.get_hex(), player, player, node_2, Vector<T2>(A0));
        }
      }
      else
      {
        if (parties[i] == current_node) {
          recv(node_0, A0, size);
          AUDIT("id:{}, P{} Reveal_, P{} RECV from {} A0{}", msgid.get_hex(), player, player, node_0, Vector<T2>(A0));
          recv(node_1, DeltaX, size);
          AUDIT("id:{}, P{} Reveal_, P{} RECV from {} DeltaX{}", msgid.get_hex(), player, player, node_1, Vector<T2>(DeltaX));
          recv(node_2, A1, size);
          AUDIT("id:{}, P{} Reveal_, P{} RECV from {} A1{}", msgid.get_hex(), player, player, node_2, Vector<T2>(A1));
          plain = DeltaX + A0 + A1;
        }
        else if (player == PARTY_0) {
          send(parties[i], A0, size);
          AUDIT("id:{}, P{} Reveal_, P{} SEND to {} A0{}", msgid.get_hex(), player, player, parties[i], Vector<T2>(A0));
        }
        else if (player == PARTY_1) {
          send(parties[i], DeltaX, size);
          AUDIT("id:{}, P{} Reveal_, P{} SEND to {} DeltaX{}", msgid.get_hex(), player, player, parties[i], Vector<T2>(DeltaX));
        }
        else if (player == PARTY_2) {
          send(parties[i], A1, size);
          AUDIT("id:{}, P{} Reveal_, P{} SEND to {} A1{}", msgid.get_hex(), player, player, parties[i], Vector<T2>(A1));
        }
      }
    }

    AUDIT("id:{}, P{} Reveal_, output{}", msgid.get_hex(), player, Vector<T2>(plain));
  }

  /**
 * Input, get the secert sharing of X.
 * 
 * \param[in] p which party owns the private data
 * \param[in] X the private data (real-data)
 * \param[out] shareX the sharing of X
 * \return nothing
 * 
 * @note
 * <T1,T2> ==>  <mpc_t, Share> or <bit_t, BitShare>
 * 
 * 1. each party must know the shape of X. \n
 * 2. X, the fix-point of real-data(double), if T1 is mpc_t. \n
 * 3. X.shape = (m,n), row first. \n
 */
  template <typename T1, typename T2>
  void Input_(const string& node_id, const vector<T1>& X, vector<T2>& shareX) {
    assert(!node_id.empty());
    int p = io->GetPartyId(node_id);
    const string& current_node_id = io->GetCurrentNodeId();
    AUDIT("id:{}, P{} input from P{}{}", msgid.get_hex(), player, p, Vector<T1>(X));

    size_t size = X.size(); // m x n
    resize_vector(shareX, size);

    // 1. Pi and P2 generates A0 and A1 (A = A0 + A1)
    vector<T1> A0(size, 0);
    vector<T1> A1(size, 0);
    vector<T1> deltaX(size, 0);
    if (p == PARTY_0) {
      PRF02(A0, size);
      AUDIT("id:{}, P{} input from P{}, generate A0{}", msgid.get_hex(), player, p, Vector<T1>(A0));
    } else if (p == PARTY_1) {
      PRF12(A1, size);
      AUDIT("id:{}, P{} input from P{}, generate A1{}", msgid.get_hex(), player, p, Vector<T1>(A1));
    } else if (p == PARTY_2) {
      PRF02(A0, size);
      AUDIT("id:{}, P{} input from P{}, generate A0{}", msgid.get_hex(), player, p, Vector<T1>(A0));
    } else {
      PRF_DATA_A0(node_id, A0, size);
      AUDIT("id:{}, P{} input from P{}, generate A0{}", msgid.get_hex(), node_id, p, Vector<T1>(A0));

      PRF_DATA_A1(node_id, A1, size);
      AUDIT("id:{}, P{} input from P{}, generate A1{}", msgid.get_hex(), node_id, p, Vector<T1>(A1));
    }

    // 2. Pi sets deltaX
    if (p != PARTY_2 && current_node_id == node_id) {
      deltaX = X - A0 - A1;
    } else if (p == PARTY_2 && player == PARTY_2) {
      A1 = X - A0;
    }
    AUDIT("id:{}, P{} input from P{}, locally compute deltaX=X-A0-A1, deltaX{}", msgid.get_hex(), player, p, Vector<T1>(deltaX));
    //vector<T1> deltaX = X;
    //Sub(deltaX, A0);
    //Sub(deltaX, A1);

    // 3.
    if ((p == PARTY_0) || (p == PARTY_1)) {
      // Pi sends deltaX to P(1-i)
      if (is_primary()) {
        if (player == p) {
          send(adversary(player), deltaX, size);
          AUDIT("id:{}, P{} input from P{}, P{} SEND to P{}, deltaX{}", msgid.get_hex(), player, p, player, adversary(player), Vector<T1>(deltaX));
        } else {
          recv(adversary(player), deltaX, size);
          AUDIT("id:{}, P{} input from P{}, P{} RECV from P{}, deltaX{}", msgid.get_hex(), player, p, player, adversary(player), Vector<T1>(deltaX));
        }
      }
    } else if (p == PARTY_2) {
      // P2 sends A1 to P1
      if (is_helper()) {
        send(PARTY_1, A1, size);
        AUDIT("id:{}, P{} input from P{}, P{} SEND to P{}, A1{}", msgid.get_hex(), player, p, player, PARTY_1, Vector<T1>(A1));
      } else if (player == PARTY_1) {
        recv(PARTY_2, A1, size);
        AUDIT("id:{}, P{} input from P{}, P{} RECV from P{}, A1{}", msgid.get_hex(), player, p, player, PARTY_2, Vector<T1>(A1));
      }
    } else {
      if (node_id == current_node_id) {
        send(PARTY_0, deltaX, size);
        AUDIT("id:{}, P{} input from P{}, P{} SEND to P{}, deltaX{}", msgid.get_hex(), player, p, player, PARTY_0, Vector<T1>(deltaX));

        send(PARTY_1, deltaX, size);
        AUDIT("id:{}, P{} input from P{}, P{} SEND to P{}, deltaX{}", msgid.get_hex(), player, p, player, PARTY_1, Vector<T1>(deltaX));
      } else if (is_primary()) {
        recv(node_id, deltaX, size);
        AUDIT("id:{}, P{} input from P{}, P{} RECV from {}, deltaX{}", msgid.get_hex(), player, p, player, node_id, Vector<T1>(deltaX));
      }
    }

    // 4. each party sets the share
    for (int i = 0; i < size; i++) {
      if (is_helper()) {
        shareX[i].s0.A0 = A0[i];
        shareX[i].s1.A1 = A1[i];
      } else if (is_primary()) {
        shareX[i].s0.delta = deltaX[i];
        if (player == PARTY_0)
          shareX[i].s1.A0 = A0[i];
        else
          shareX[i].s1.A1 = A1[i];
      }
    }

    AUDIT("id:{}, P{} input from P{}, output shareX{}", msgid.get_hex(), player, p, Vector<T2>(shareX));
  }

  /**
 * Mul, multiply share by share
 * 
 * Z = X * Y
 * 
 * @note
 * <T1,T2> ==>  <mpc_t, Share> or <bit_t, BitShare>
 * 
 *  If either X or Y is non-scaled Share, i.e., the result of comparison, we can set
 *  need_trunc as 'false' to reduce one calling of 'Trunc' Operation. 
 */
  template <typename T1, typename T2>
  void Mul_(const vector<T2>& X, const vector<T2>& Y, vector<T2>& Z, int precision, bool need_trunc = true) {
    size_t size = X.size();
    resize_vector(Z, size);
    //AUDIT("id:{}, Mul({},{}), P{} input X{}", msgid.get_hex(), size, size, player, Vector<ShareT<T2>>(X));
    AUDIT("id:{}, Mul({},{}), P{} input X{}", msgid.get_hex(), size, size, player, Vector<T2>(X));
    AUDIT("id:{}, Mul({},{}), P{} input Y{}", msgid.get_hex(), size, size, player, Vector<T2>(Y));

    // step 2 Ci
    vector<T1> C0(size, 0), C1(size, 0);
    PRF02(C0, size);
    if ((player == PARTY_1) || (player == PARTY_2)) {
      AUDIT("id:{}, Mul({},{}), P{} genertor C0{}", msgid.get_hex(), size, size, player, Vector<T1>(C0));
      if (player == PARTY_2) {
        // step 3 (A0+A1)*(B0+B1)-C0
        if (sizeof(T1) == sizeof(mpc_t)) {
          for (int i = 0; i < size; i++) {
            C1[i] = (X[i].s0.A0 + X[i].s1.A1) * (Y[i].s0.B0 + Y[i].s1.B1) - C0[i];
          }
        } else {
          for (int i = 0; i < size; i++) {
            C1[i] = (X[i].s0.A0 ^ X[i].s1.A1) & (Y[i].s0.B0 ^ Y[i].s1.B1) ^ C0[i];
          }
        }
        AUDIT("id:{}, Mul({},{}), P{} locally compute C1(=(A0+A1)*(B0+B1)-C0){}", msgid.get_hex(), size, size, player, Vector<T1>(C1));

        send(PARTY_1, C1, size);
        AUDIT("id:{}, Mul({},{}), P{} SEND to P{} C1{}", msgid.get_hex(), size, size, player, PARTY_1, Vector<T1>(C1));
      } else {
        recv(PARTY_2, C1, size);
        AUDIT("id:{}, Mul({},{}), P{} RECV from P{} C1{}", msgid.get_hex(), size, size, player, PARTY_2, Vector<T1>(C1));
      }
    }

    // step 4 Zi
    vector<T1> Z0(size, 0), Z1(size, 0);
    PRF02(Z0, size);
    if (player == PARTY_0 || player == PARTY_2) { 
      AUDIT("id:{}, Mul({},{}), P0 and P2 generat Z0{}", msgid.get_hex(), size, size, Vector<T1>(Z0));
    }

    PRF12(Z1, size);
    if (player == PARTY_1 || player == PARTY_2) { 
      AUDIT("id:{}, Mul({},{}), P1 and P2 generat Z1{}", msgid.get_hex(), size, size, Vector<T1>(Z1));
    }

    vector<T1> tildeZ(size, 0), tildeZ0(size, 0), tildeZ1(size, 0);
    vector<T1> hatZ(size, 0), hatZ0(size, 0), hatZ1(size, 0);
    if (is_primary()) {
      if (player == PARTY_0) {
        // step 5 deltaX*B0 + A0*deltaY + C0
        if (sizeof(T1) == sizeof(mpc_t)) {
          for (int i = 0; i < size; i++) {
            tildeZ0[i] = X[i].s0.delta * Y[i].s1.B0 + X[i].s1.A0 * Y[i].s0.delta + C0[i];
          }
        } else {
          for (int i = 0; i < size; i++) {
            tildeZ0[i] = X[i].s0.delta & Y[i].s1.B0 ^ X[i].s1.A0 & Y[i].s0.delta ^ C0[i];
          }
        }
        AUDIT("id:{}, Mul({},{}), P{} locally compute tildeZ0(=deltaX*B0+A0*deltaY+C0){}", msgid.get_hex(), size, size, player, Vector<T1>(tildeZ0));

        // step 6,7
        if (sizeof(T1) == sizeof(mpc_t)) {
          if (need_trunc) {
            //cout << "DEBUG: NEED TRUNC!" << endl;
            Trunc(tildeZ0, size, precision);
            AUDIT("id:{}, Mul({},{}), P{} locally tuncates tildeZ0{}", msgid.get_hex(), size, size, player, Vector<T1>(tildeZ0));
          }
          // else {
          //   cout << "DEBUG NO TRUNC!" << endl;
          // }
        }

        hatZ0 = tildeZ0 - Z0;
        AUDIT("id:{}, Mul({},{}), P{} locally compute hatZ0(=tildeZ0-Z0){}", msgid.get_hex(), size, size, player, Vector<T1>(hatZ0));

        send(PARTY_1, hatZ0, size);
        AUDIT("id:{}, Mul({},{}), P{} SEND to P{} hatZ0{}", msgid.get_hex(), size, size, player, PARTY_1, Vector<T1>(hatZ0));

        recv(PARTY_1, hatZ1, size);
        AUDIT("id:{}, Mul({},{}), P{} RECV from P{} hatZ1{}", msgid.get_hex(), size, size, player, PARTY_1, Vector<T1>(hatZ1));
      } else {
        // step 5 deltaX*deltaY + deltaX*B1 + A1*deltaY + C1
        if (sizeof(T1) == sizeof(mpc_t)) {
          for (int i = 0; i < size; i++) {
            tildeZ1[i] = X[i].s0.delta * Y[i].s0.delta + X[i].s0.delta * Y[i].s1.B1 +
              X[i].s1.A1 * Y[i].s0.delta + C1[i];
          }
        } else {
          for (int i = 0; i < size; i++) {
            tildeZ1[i] = X[i].s0.delta & Y[i].s0.delta ^ X[i].s0.delta & Y[i].s1.B1 ^
              X[i].s1.A1 & Y[i].s0.delta ^ C1[i];
          }
        }
        AUDIT("id:{}, Mul({},{}), P{} locally compute tildeZ1=deltaX*deltaY+deltaX*B1+A1*deltaY+C1, tildeZ1{}", msgid.get_hex(), size, size, player, Vector<T1>(tildeZ1));
        // step 6,7
        if (sizeof(T1) == sizeof(mpc_t)) {
          if (need_trunc) {
            Trunc(tildeZ1, size, precision);
            AUDIT("id:{}, Mul({},{}), P{} locally tuncates tildeZ1{}", msgid.get_hex(), size, size, player, Vector<T1>(tildeZ1));
          }
        }

        hatZ1 = tildeZ1 - Z1;
        AUDIT("id:{}, Mul({},{}), P{} locally compute hatZ1=tildeZ1-Z1, hatZ1{}", msgid.get_hex(), size, size, player, Vector<T1>(hatZ1));

        recv(PARTY_0, hatZ0, size);
        AUDIT("id:{}, Mul({},{}), P{} RECV from P{} hatZ0{}", msgid.get_hex(), size, size, player, PARTY_0, Vector<T1>(hatZ0));
        
        send(PARTY_0, hatZ1, size);
        AUDIT("id:{}, Mul({},{}), P{} SEND to P{} hatZ1{}", msgid.get_hex(), size, size, player, PARTY_0, Vector<T1>(hatZ1));
      }

      // step 8 reveal hatZ
      hatZ = hatZ0 + hatZ1;
      AUDIT("id:{}, Mul({},{}), P{} locally compute hatZ=hatZ0+hatZ1, hatZ{}", msgid.get_hex(), size, size, player, Vector<T1>(hatZ));
    }

    // each party sets the share
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
    AUDIT("id:{}, Mul({},{}), P{} output Z{}", msgid.get_hex(), size, size, player, Vector<T2>(Z));
  }

 public:
  /**
   * PRG Pseudorandom Number Generator
   * PRF Pseudo-Random Function
   */
  void PRF01(vector<mpc_t>& A, size_t size);
  void PRF01(vector<bit_t>& A, size_t size);
  void PRF02(vector<mpc_t>& A, size_t size);
  void PRF02(vector<bit_t>& A, size_t size);
  void PRF12(vector<mpc_t>& A, size_t size);
  void PRF12(vector<bit_t>& A, size_t size);
  void PRF0(vector<mpc_t>& A, size_t size);
  void PRF0(vector<bit_t>& A, size_t size);
  void PRF1(vector<mpc_t>& A, size_t size);
  void PRF1(vector<bit_t>& A, size_t size);
  void PRF2(vector<mpc_t>& A, size_t size);
  void PRF2(vector<bit_t>& A, size_t size);
  void PRF(vector<mpc_t>& A, size_t size);
  void PRF(vector<bit_t>& A, size_t size);
  void PRF_DATA_A0(const string &node_id, vector<mpc_t>& A, size_t size);
  void PRF_DATA_A1(const string &node_id, vector<mpc_t>& A, size_t size);
  void PRF_DATA_A0(const string &node_id, vector<bit_t>& A, size_t size);
  void PRF_DATA_A1(const string &node_id, vector<bit_t>& A, size_t size);

  void RandSeed(vector<uint64_t>& seeds, size_t size);
  void SyncPRGKeyV1();
  void SyncPRGKeyV1(int partyA, int partyB, std::string& key_send, std::string& key_recv);
  shared_ptr<MpcKeyPrgController> SyncPRGKey();
  void SyncPRGKey(int partyA, int partyB, const std::string& key_send, std::string& key_recv);
  void SyncPRGKey(const string& node_id, int party_id, const std::string& key_send, std::string& key_recv);

 public:
  // scale up by X << power locally and securely.
  void Scale(vector<Share>& X, size_t power);
  void Scale(const vector<Share>& X, vector<Share>& Y, size_t power);

  /**
   *
   * basic and locally
   *
   */
  //
  void Add(const vector<mpc_t>& X, const vector<mpc_t>& Y, vector<mpc_t>& Z);
  void Add(vector<mpc_t>& X, const vector<mpc_t>& Y);
  void Add(const vector<bit_t>& X, const vector<bit_t>& Y, vector<bit_t>& Z);
  void Add(vector<bit_t>& X, const vector<bit_t>& Y);

  void Sub(const vector<mpc_t>& X, const vector<mpc_t>& Y, vector<mpc_t>& Z);
  void Sub(vector<mpc_t>& X, const vector<mpc_t>& Y);
  void Sub(const vector<bit_t>& X, const vector<bit_t>& Y, vector<bit_t>& Z);
  void Sub(vector<bit_t>& X, const vector<bit_t>& Y);

  //
  void Add(const vector<Share>& X, const vector<mpc_t>& C, vector<Share>& Z);
  void Add(vector<Share>& X, const vector<mpc_t>& C);
  void Add(const vector<Share>& X, const vector<double>& C, vector<Share>& Z);
  void Add(vector<Share>& X, const vector<double>& C);

  void Sub(const vector<Share>& X, const vector<mpc_t>& C, vector<Share>& Z);
  void Sub(vector<Share>& X, const vector<mpc_t>& C);
  void Sub(const vector<Share>& X, const vector<double>& C, vector<Share>& Z);
  void Sub(vector<Share>& X, const vector<double>& C);

  void Add(const vector<mpc_t>& C, const vector<Share>& X, vector<Share>& Z);
  void Add(const vector<double>& C, const vector<Share>& X, vector<Share>& Z);
  void Sub(const vector<mpc_t>& C, const vector<Share>& X, vector<Share>& Z);
  void Sub(const vector<double>& C, const vector<Share>& X, vector<Share>& Z);

  //
  void Add(const vector<Share>& X, const vector<Share>& Y, vector<Share>& Z);
  void Add(vector<Share>& X, const vector<Share>& Y);
  void Add(const vector<Share>& X, const Share& Y, vector<Share>& Z);
  void Add(vector<Share>& X, const Share& Y);
  void Add(Share& X, const Share& Y);
  void Add(const Share& X, const Share& Y, Share& Z);

  void Sub(const vector<Share>& X, const vector<Share>& Y, vector<Share>& Z);
  void Sub(vector<Share>& X, const vector<Share>& Y);
  void Sub(const vector<Share>& X, const Share& Y, vector<Share>& Z);
  void Sub(vector<Share>& X, const Share& Y);
  void Sub(Share& X, const Share& Y);
  void Sub(const Share& X, const Share& Y, Share& Z);

  //
  void Sum(const vector<Share>& X, Share& Y);
  void Negative(const vector<Share>& X, vector<Share>& Y);

  /**
   *
   * basic
   *
   */

  // truncate (X >> d) in a secure way
  void Trunc(vector<mpc_t>& X, size_t size, size_t d);
  void Trunc_many(vector<mpc_t>& X, size_t size, vector<size_t> power_list);
  void Trunc(vector<Share>& X, size_t size, size_t d);
  void Trunc_many(vector<Share>& X, size_t size, vector<size_t> power_list);
  void Trunc(vector<bit_t>& X, size_t size, size_t d);
  void Trunc(vector<BitShare>& X, size_t size, size_t d);

  void Reveal(const vector<Share>& X, vector<mpc_t>& plain, const string& nodes);
  void Reveal(const vector<Share>& X, vector<double>& plain, const string& nodes);
  void Reveal(const vector<BitShare>& X, vector<bit_t>& plain, const string& nodes);
  void Reveal(const vector<Share>& X, vector<mpc_t>& plain, const vector<string>& nodes);
  void Reveal(const vector<Share>& X, vector<double>& plain, const vector<string>& nodes);

  // RevealAndPrint*, only for test & debug
  void RevealAndPrint(const vector<Share>& X, std::string msg);
  void RevealAndPrint2(const vector<Share>& X, std::string msg);
  void RevealAndPrint(const vector<BitShare>& X, std::string msg);
  void RevealAndPrint(const Share& X, std::string msg);
  void RevealAndPrint(const BitShare& X, std::string msg);

  void Input(const string& node_id, const vector<mpc_t>& X, vector<Share>& shareX);
  void Input(const string& node_id, const vector<double>& X, vector<Share>& shareX);
  void Input(const string& node_id, const vector<vector<double>>& X, vector<vector<Share>>& shareX);
  void Input(const string& node_id, const vector<bit_t>& X, vector<BitShare>& shareX);

  /** 
   * @note: this input X should be a common SAME value for each party!
   * To convert a constant input X to 'Share' format, 
   * rather than call (private) Input, which costs one round on communication,
   * we present it in the following legal way, locally:
   * P0: (X, 0)
   * P1: (X, 0)
   * P2: (0, 0) 
   */
  void ConstCommonInput(const vector<mpc_t>& X, vector<Share>& shareX);
  void ConstCommonInput(const vector<double>& X, vector<Share>& shareX);

  
  void Broadcast(const string& node_id, const string& msg, string& result);
  void Broadcast(const string& node_id, const char* msg, char* result, size_t size);

  void CombineInputInVertical(
    const vector<vector<Share>>& shareX0,
    const vector<vector<Share>>& shareX1,
    const vector<vector<Share>>& shareX2,
    vector<vector<Share>>& shareX);
  void CombineInputInHorizontal(
    const vector<vector<Share>>& shareX0,
    const vector<vector<Share>>& shareX1,
    const vector<vector<Share>>& shareX2,
    vector<vector<Share>>& shareX);

  void Mul(
    const vector<Share>& X,
    const vector<mpc_t>& C,
    vector<Share>& Z,
    bool c_has_scaled = true);
  void Mul(const vector<Share>& X, const vector<double>& C, vector<Share>& Z);
  void Mul(const vector<double>& C, const vector<Share>& X, vector<Share>& Z);
  // This is the so-called FPMul if need_trunc is true.
  void Mul(
    const vector<Share>& X,
    const vector<Share>& Y,
    vector<Share>& Z,
    bool need_trunc = true);
  void Mul(const vector<BitShare>& X, const vector<BitShare>& Y, vector<BitShare>& Z);

  void Div(const vector<Share>& X, const vector<Share>& Y, vector<Share>& Z);
  void Div(const vector<Share>& X, const vector<double>& C, vector<Share>& Z);
  void Div(const vector<double>& C, const vector<Share>& X, vector<Share>& Z);

  // @TODO: remove these to TF layer due to they are the same as 'Div'
  void Truediv(const vector<Share>& X, const vector<Share>& Y, vector<Share>& Z);
  void Truediv(const vector<Share>& X, const vector<double>& C, vector<Share>& Z);
  void Truediv(const vector<double>& C, const vector<Share>& X, vector<Share>& Z);

  void Floordiv(const vector<Share>& X, const vector<Share>& Y, vector<Share>& Z);
  void Floordiv(const vector<Share>& X, const vector<double>& C, vector<Share>& Z);
  void Floordiv(const vector<double>& C, const vector<Share>& X, vector<Share>& Z);

  void Reciprocaldiv(const vector<Share>& X, const vector<Share>& Y, vector<Share>& Z);
  void Reciprocaldiv(const vector<Share>& X, const vector<double>& C, vector<Share>& Z);
  void Reciprocaldiv(const vector<double>& C, const vector<Share>& X, vector<Share>& Z);

  void Sum(const vector<Share>& X, vector<Share>& Y, int rows, int cols);
  void AddN(const vector<Share>& X, vector<Share>& Y, int rows, int cols);
  void Mean(const vector<Share>& X, vector<Share>& Y, int rows, int cols);
  void Max(const vector<Share>& X, vector<Share>& Y, int rows, int cols);
  void Min(const vector<Share>& X, vector<Share>& Y, int rows, int cols);

  void Square(const vector<Share>& X, vector<Share>& Y);
  void Rsqrt(const vector<Share>& X, vector<Share>& Y);
  void Sqrt(const vector<Share>& X, vector<Share>& Y);

  void MatMul(
    const vector<Share>& X,
    const vector<Share>& Y,
    vector<Share>& Z,
    size_t m,
    size_t n,
    size_t k,
    bool t_a,
    bool t_b);
  void MatMul2(
    const vector<Share>& X,
    const vector<Share>& Y,
    vector<Share>& Z,
    size_t m,
    size_t n,
    size_t k,
    bool t_a,
    bool t_b);

  void BitAdder(const vector<BitShare>& X, const vector<BitShare>& Y, vector<BitShare>& C);
  void AdderCircuitL(
    const vector<vector<BitShare>>& X,
    const vector<vector<BitShare>>& Y,
    vector<BitShare>& C);
  void AdderCircuitW(const vector<BitShare>& X, const vector<BitShare>& Y, vector<BitShare>& C);
  void MSB(const vector<Share>& X, vector<BitShare>& C);
  void PreTuple(vector<Share>& aX, vector<BitShare>& bX, size_t size);
  void PreTupleA(const vector<Share>& Y, vector<Share>& aX, vector<BitShare>& bX);

  void Linear(const vector<mpc_t>& C, const vector<Share>& X, Share& Y, bool c_has_scaled = true);
  void Linear(const vector<Share>& X, const vector<mpc_t>& C, Share& Y, bool c_has_scaled = true);
  void Linear(const vector<double>& C, const vector<Share>& X, Share& Y);
  void Linear(const vector<Share>& X, const vector<double>& C, Share& Y);
  void Linear(const vector<bit_t>& C, const vector<BitShare>& X, BitShare& Y);
  void Linear(const vector<BitShare>& X, const vector<bit_t>& C, BitShare& Y);

  void B2A(const vector<BitShare>& bX, vector<Share>& X);
  void BMA(const vector<BitShare>& bitX, const vector<Share>& Y, vector<Share>& X);

  void DReLU(const vector<Share>& X, vector<Share>& Y);
  void ReLU(const vector<Share>& X, vector<Share>& Y);

  /**
   * @brief: wrapper for implementing 'If' clause 
   *    result = x * Cond + y * (1 - Cond)
   * @note: this is introduced for inner usage, but may be exposed later.
   *        Attention! the cond is 'non-scaled' 'Share' type, such as the reuslt of 'DReLU'.
   * @todo: make the 'cond' as BitShare type.
   */
  void Select1Of2(
    const vector<Share>& X,
    const vector<Share>& Y,
    const vector<Share>& cond,
    vector<Share>& result, bool is_scaled=false);

  void Select1Of2(
    const vector<Share>& X,
    const vector<Share>& Y,
    const vector<BitShare>& cond,
    vector<Share>& result);
  /**
   * @attention: the 'cond' should be internal result, which means it is not scaled!
   */
  void Select1Of2(
    const vector<double>& X,
    const vector<double>& Y,
    const vector<Share>& cond,
    vector<Share>& result);

  //! Wrapper for X \xor Y by X + Y - 2(X * Y)
  //    Note that the input X and Y should be normal scaled Share!
  void XORShare(const vector<Share>& X, const vector<Share>& Y, vector<Share>& Z);

  /**
   * Logical Operations, all the inputs/outputs have scaled [default]
   * @note the real value of all the inputs must be 1/0, or the result is unexpected!
   * 
   * AND (x and y) = x*y
   * OR  (x or y)  = x + y - x*y
   * XOR (x xor y) = x + y - 2*x*y
   * NOT (not x)   = 1 - x
   * Z[i] = X[i] (OP) Y[i]
   * 
   * \param scaled Indicates whether the Share-input is scaled or not.
   * \return scaled value
   */
  void AND(const vector<Share>& X, const vector<Share>& Y, vector<Share>& Z, bool scaled = true);
  void OR(const vector<Share>& X, const vector<Share>& Y, vector<Share>& Z, bool scaled = true);
  void XOR(const vector<Share>& X, const vector<Share>& Y, vector<Share>& Z, bool scaled = true);
  void NOT(const vector<Share>& X, vector<Share>& Z, bool scaled = true);

  /**
   * This is the constant version, use "1.0" represents true(1), "0.0" represents false(0)
   * @todo There is some special handling of the inputs
   * 
   * \param C constant
   */
  void AND(const vector<Share>& X, const vector<double>& C, vector<Share>& Z, bool scaled = true);
  void OR(const vector<Share>& X, const vector<double>& C, vector<Share>& Z, bool scaled = true);
  void XOR(const vector<Share>& X, const vector<double>& C, vector<Share>& Z, bool scaled = true);
  void AND(const vector<double>& C, const vector<Share>& Y, vector<Share>& Z, bool scaled = true);
  void OR(const vector<double>& C, const vector<Share>& Y, vector<Share>& Z, bool scaled = true);
  void XOR(const vector<double>& C, const vector<Share>& Y, vector<Share>& Z, bool scaled = true);

  /**
   * K-*, recursion version, all the inputs/outputs have scaled [default]
   * XX is a 2-d vector
   * if XX.size() == 0; return 0
   * if XX.size() == 1; return XX[0]
   * if XX.size() >= 1; return XX[0] (OP) XX[1] (OP)... XX[i]
   */
  typedef void (
    HelixInternal::*_Lfunc)(const vector<Share>&, const vector<Share>&, vector<Share>&, bool);
  void logical_op_r_(_Lfunc f, const vector<vector<Share>>& XX, vector<Share>& Z, bool scaled);
  void AND(const vector<vector<Share>>& XX, vector<Share>& Z, bool scaled = true);
  void OR(const vector<vector<Share>>& XX, vector<Share>& Z, bool scaled = true);
  void XOR(const vector<vector<Share>>& XX, vector<Share>& Z, bool scaled = true);

  //! sharing and sharing version
  void LessDReLU_(const vector<Share>& X, vector<Share>& Y);
  void NotEqual(const vector<Share>& X, const vector<Share>& Y, vector<Share>& Z);
  // retired version
  void Equal_(const vector<Share>& X, const vector<Share>& Y, vector<Share>& Z, int eq = 1);
  // improved version
  // if eq = 1, if X is 0 then return 1, otherwise 0;
  // if eq = 0, if X is 0 then return 0, otherwise 1;
  void IsZero(const vector<Share>& X, vector<Share>& Z, int eq = 1);
  // used by IsZero internally, to generate arithematic-shared random value, and bit-share of its each bits.
  void _RandomShareAB(vector<Share>& aX, vector<vector<BitShare>>& bX, size_t size);
  void FanInBitAdd(const vector<vector<BitShare>>& a, vector<BitShare>& c, size_t vec_size);
  void Equal(const vector<Share>& X, const vector<Share>& Y, vector<Share>& Z);

  void Less(const vector<Share>& X, const vector<Share>& Y, vector<Share>& Z);
  void LessEqual(const vector<Share>& X, const vector<Share>& Y, vector<Share>& Z);
  void Greater(const vector<Share>& X, const vector<Share>& Y, vector<Share>& Z);
  void GreaterEqual(const vector<Share>& X, const vector<Share>& Y, vector<Share>& Z);

  //! sharing and constant version
  void NotEqual(const vector<Share>& X, const vector<double>& C, vector<Share>& Z);
  void NotEqual(const vector<double>& C, const vector<Share>& X, vector<Share>& Z);
  void Equal(const vector<Share>& X, const vector<double>& C, vector<Share>& Z);
  void Equal(const vector<double>& C, const vector<Share>& X, vector<Share>& Z);
  void Less(const vector<Share>& X, const vector<double>& C, vector<Share>& Z);
  void Less(const vector<double>& C, const vector<Share>& X, vector<Share>& Z);
  void LessEqual(const vector<Share>& X, const vector<double>& C, vector<Share>& Z);
  void LessEqual(const vector<double>& C, const vector<Share>& X, vector<Share>& Z);
  void Greater(const vector<Share>& X, const vector<double>& C, vector<Share>& Z);
  void Greater(const vector<double>& C, const vector<Share>& X, vector<Share>& Z);
  void GreaterEqual(const vector<Share>& X, const vector<double>& C, vector<Share>& Z);
  void GreaterEqual(const vector<double>& C, const vector<Share>& X, vector<Share>& Z);

  void InnerProducts(
    const vector<vector<Share>>& X,
    const vector<vector<Share>>& Y,
    vector<Share>& Z,
    bool trunc = true);
  void InnerProducts(
    const vector<Share>& X,
    const vector<Share>& Y,
    vector<Share>& Z,
    size_t J,
    bool trunc = true);
  void InnerProducts(const vector<Share>& X, const vector<Share>& Y, Share& Z, bool trunc = true);

  void Pow(const vector<Share>& X, size_t N, vector<vector<Share>>& Y);
  void Pow(const vector<Share>& X, const vector<size_t>& N, vector<vector<Share>>& Y);
  void Pow_original(const vector<Share>& X, size_t N, vector<vector<Share>>& Y);
  /**
	 * @brief: Helix version for calculating X^k, of which k is common known
	 * @param:
	 * [in] X, the variable;
	 * [in] k, power value.
	 * [out] Y, the resulting value.
   */
  void PowV2(const Share& X, const int& k, Share& Y);
  void PowV2(const vector<Share>& X, const int& common_k, vector<Share>& Y);
  void PowV2(const vector<Share>& X, const vector<int>& k, vector<Share>& Y);

  void Exp(const vector<Share>& X, vector<Share>& Y);

  void SigmoidPiceWise6(const vector<Share>& X, vector<Share>& Y);
  void SigmoidPiceWise6_original_not_optimized(const vector<Share>& X, vector<Share>& Y);
  void SigmoidChebyshev(const vector<Share>& X, vector<Share>& Y);
  void Sigmoid(const vector<Share>& X, vector<Share>& Y);

  /**
	 * @brief: secret-shared version for computing a univariate polynomial:
	 * Y = C0 * X^P0 + C1 * X^P1 + ... + Cn * X^Pn
	 * @param:
	 * 	[in] shared_X, the variable.
	 * 	[in] common_power_list, the {Pi} in the polynomial in order.
	 * 	[in] common_coff_list, the {Ci} in the polynomial, which must
	 * 			1-to-1 associate with {Pi}.
	 * 	[out] shared_Y, the result.
   *   @Note:
	 * 	for efficiency, it will be better to sort the list in ascending order!
   */
  void UniPolynomial(
    const Share& shared_X,
    const vector<mpc_t>& common_power_list,
    const vector<mpc_t>& common_coff_list,
    Share& shared_Y);
  void UniPolynomial(
    const vector<Share>& shared_X,
    const vector<mpc_t>& common_power_list,
    const vector<mpc_t>& common_coff_list,
    vector<Share>& shared_Y);

  /**
   * @desc: use 3-segment piecewise polynomials to implement logarithm function
   */
  void LogV2(const vector<Share>& X, vector<Share>& Z);

  /**
   * @desc: high-dimension logarithm function
   */
  void HLog(const vector<Share>& X, vector<Share>& Z);

  /**
   * @desc: optimization for computing SigmoidCrossEntropy
   */
  void SigmoidCrossEntropy(const vector<Share>& X, const vector<Share>& Y, vector<Share>& Z);

  /**
   * @desc: batch-version to reduce communication cost for SigmoidCrossEntropy
   * 
   */
  void SigmoidCrossEntropy_batch(const vector<Share>& X, const vector<Share>& Y, vector<Share>& Z);
};
// send
extern template int HelixInternal::send<mpc_t>(int, const std::vector<mpc_t>&, size_t); 
extern template int HelixInternal::send<unsigned char>(int, const std::vector<unsigned char>&, size_t); 
extern template int HelixInternal::send<unsigned char>(const std::string&, const std::vector<unsigned char>&, size_t); 
// recv
extern template int HelixInternal::recv<mpc_t>(int, std::vector<mpc_t>&, size_t); 
extern template int HelixInternal::recv<unsigned char>(int, std::vector<unsigned char>&, size_t); 
extern template int HelixInternal::recv<unsigned char>(const std::string&, std::vector<unsigned char>&, size_t); 

// Reveal_
extern template void HelixInternal::Reveal_<Share, mpc_t>(const std::vector<Share>&, vector<mpc_t>& , const std::vector<std::string>&); 
extern template void HelixInternal::Reveal_<Share, mpc_t>(const std::vector<Share>&, vector<mpc_t>& , const std::string&); 
extern template void HelixInternal::Reveal_<BitShare, mpc_t>(const std::vector<BitShare>&, vector<mpc_t>& , const std::vector<std::string>&); 
extern template void HelixInternal::Reveal_<BitShare, mpc_t>(const std::vector<BitShare>&, vector<mpc_t>& , const std::string&); 
// Input 
extern template void HelixInternal::Input_<mpc_t, Share>(const std::string& , const std::vector<mpc_t>& X, std::vector<Share>&);
extern template void HelixInternal::Input_<bit_t, BitShare>(const std::string& , const std::vector<bit_t>& X, std::vector<BitShare>&);
// Mul_
extern template void HelixInternal::Mul_<bit_t, BitShare>(const std::vector<BitShare>& , const std::vector<BitShare>&, std::vector<BitShare>& , int , bool); 
extern template void HelixInternal::Mul_<mpc_t, Share>(const std::vector<Share>& , const std::vector<Share>&, std::vector<Share>& , int , bool); 
} // namespace helix
} // namespace rosetta
