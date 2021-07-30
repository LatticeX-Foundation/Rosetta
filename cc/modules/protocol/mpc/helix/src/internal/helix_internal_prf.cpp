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
#include "cc/modules/common/include/utils/generate_key.h"
#include "cc/modules/protocol/mpc/helix/include/helix_internal.h"
#include "cc/modules/protocol/utility/include/prg.h"
#include "cc/modules/protocol/mpc/comm/include/mpc_prg_controller.h"

#include <iostream>
#include <vector>
#include <cassert>
using namespace std;

namespace rosetta {
namespace helix {

namespace {
/**
 * utils. PRF
 */
void _PRF(std::shared_ptr<RttPRG> prg, char* data, int len) {
  int nblocks = (len + 15) / 16;
  block* blk = new block[nblocks];
  char* pblk = (char*)blk;
  prg->randomDatas(pblk, nblocks * sizeof(block));
  memcpy(data, pblk, len);
  delete[] blk;
}

template <typename T>
void _PRF(vector<T>& A, size_t size, std::shared_ptr<RttPRG> prg) {
  resize_vector(A, size);
#if 0 
  char* data = (char*)A.data();
  int len = sizeof(T) * size; // A.size()*sizeof(T);
  _PRF(prg, data, len);

  //! @todo optimize
  if (sizeof(T) == 1) {
    for (int i = 0; i < size; i++) {
      A[i] = (A[i] & 0x01) & 0xFF;
    }
  }
#else
  if (sizeof(T) == sizeof(bit_t)) {
    for (int i = 0; i < size; i++) {
      A[i] = prg->getBit();
    }
  } else if (sizeof(T) == sizeof(mpc_t)) {
    for (int i = 0; i < size; i++) {
      A[i] = prg->get64Bits();
    }
  }
#endif
}

template <typename T>
void _PRF(int player, int p0, int p1, vector<T>& A, size_t size, std::shared_ptr<RttPRG> prg) {
  resize_vector(A, size);
  if ((player != p0) && (player != p1))
    return;

  _PRF(A, size, prg);
}

template <typename T>
void _PRF(int player, int p, vector<T>& A, size_t size, std::shared_ptr<RttPRG> prg) {
  resize_vector(A, size);
  if (player != p)
    return;
  _PRF(A, size, prg);
}
} // namespace

/**
 * PRF01 // P0 and P1
 * PRF02 // P0 and P2
 * PRF12 // P1 and P2
 * PRF0  // P0
 * PRF1  // P1
 * PRF2  // P2
 * PRF   // P0 and P1 and P2
 */
// clang-format off
void HelixInternal::PRF01(vector<mpc_t>& A, size_t size) { _PRF(player, PARTY_0, PARTY_1, A, size, prgobjs->prg01); }
void HelixInternal::PRF01(vector<bit_t>& A, size_t size) { _PRF(player, PARTY_0, PARTY_1, A, size, prgobjs->prg01); }
void HelixInternal::PRF02(vector<mpc_t>& A, size_t size) { _PRF(player, PARTY_0, PARTY_2, A, size, prgobjs->prg02); }
void HelixInternal::PRF02(vector<bit_t>& A, size_t size) { _PRF(player, PARTY_0, PARTY_2, A, size, prgobjs->prg02); }
void HelixInternal::PRF12(vector<mpc_t>& A, size_t size) { _PRF(player, PARTY_1, PARTY_2, A, size, prgobjs->prg12); }
void HelixInternal::PRF12(vector<bit_t>& A, size_t size) { _PRF(player, PARTY_1, PARTY_2, A, size, prgobjs->prg12); }
void HelixInternal::PRF0(vector<mpc_t>& A, size_t size) { _PRF(player, PARTY_0, A, size, prgobjs->prg0); }
void HelixInternal::PRF0(vector<bit_t>& A, size_t size) { _PRF(player, PARTY_0, A, size, prgobjs->prg0); }
void HelixInternal::PRF1(vector<mpc_t>& A, size_t size) { _PRF(player, PARTY_1, A, size, prgobjs->prg1); }
void HelixInternal::PRF1(vector<bit_t>& A, size_t size) { _PRF(player, PARTY_1, A, size, prgobjs->prg1); }
void HelixInternal::PRF2(vector<mpc_t>& A, size_t size) { _PRF(player, PARTY_2, A, size, prgobjs->prg2); }
void HelixInternal::PRF2(vector<bit_t>& A, size_t size) { _PRF(player, PARTY_2, A, size, prgobjs->prg2); }
void HelixInternal::PRF(vector<mpc_t>& A, size_t size) { _PRF(A, size, prgobjs->prg); }
void HelixInternal::PRF(vector<bit_t>& A, size_t size) { _PRF(A, size, prgobjs->prg); }
void HelixInternal::PRF_PRIVATE(vector<mpc_t>& A, size_t size) { _PRF(A, size, prgobjs->prg_private); }
void HelixInternal::PRF_PRIVATE(vector<bit_t>& A, size_t size) { _PRF(A, size, prgobjs->prg_private); }
// clang-format on

void HelixInternal::SyncPRGKeyV1() {
#if MPC_DEBUG_USE_FIXED_AESKEY
  //return;
#endif
  //MpcPRGKeys::keys.fmt_print();
  //return;

  MpcPRGKeys::keys.reset();
  if (player == PARTY_0) {
    MpcPRGKeys::keys.key_prg0 = gen_key_str();
    AUDIT("id:{}, P{} locally generate P{}'s PRG key:{}", msgid.get_hex(), player, player, MpcPRGKeys::keys.key_prg0);
  } else if (player == PARTY_1) {
    MpcPRGKeys::keys.key_prg1 = gen_key_str();
    AUDIT("id:{}, P{} locally generate P{}'s PRG key:{}", msgid.get_hex(), player, player, MpcPRGKeys::keys.key_prg1);
  } else if (player == PARTY_2) {
    MpcPRGKeys::keys.key_prg2 = gen_key_str();
    AUDIT("id:{}, P{} locally generate P{}'s PRG key:{}", msgid.get_hex(), player, player, MpcPRGKeys::keys.key_prg2);
  }

  // common or public key
  string k01, k02, k12, k0;
  k0 = gen_key_str();
  k01 = gen_key_str();
  k02 = gen_key_str();
  k12 = gen_key_str();

  SyncPRGKey(PARTY_0, PARTY_1, k01, k01);
  SyncPRGKey(PARTY_0, PARTY_2, k02, k02);
  SyncPRGKey(PARTY_1, PARTY_2, k12, k12);
  SyncPRGKey(PARTY_2, PARTY_0, k0, k0);
  SyncPRGKey(PARTY_2, PARTY_1, k0, k0);
  MpcPRGKeys::keys.key_prg = k0;
  MpcPRGKeys::keys.key_prg01 = k01;
  MpcPRGKeys::keys.key_prg02 = k02;
  MpcPRGKeys::keys.key_prg12 = k12;
  //MpcPRGKeys::keys.fmt_print();

  //! set gseed, different from k0, simply
  string kgseed(k0);
  kgseed[0] = '\x44';
  gseed->reseed(kgseed);
  //cout << "KGS " << kgseed << endl;
}

/**
 * A's send to B
 * key_send.length() ==== key_recv.length() > 0
 */
void HelixInternal::SyncPRGKeyV1(
  int partyA,
  int partyB,
  std::string& key_send,
  std::string& key_recv) {
  AUDIT("id:{}, P{} locally generates and sysnc k{}{}:{}", msgid.get_hex(), player, partyA, partyB, key_send);
  if (player == partyA) {
    send(partyB, key_send.data(), key_send.length());
    AUDIT("id:{}, P{} SyncPRGKeyV1 SEND to P{}, key_send{}", msgid.get_hex(), player, partyA, key_send);
  }
  if (player == partyB) {
    recv(partyA, (char*)key_recv.data(), key_recv.length());
    AUDIT("id:{}, P{} SyncPRGKeyV1 RECV from P{}, key_send{}", msgid.get_hex(), player, partyB, key_send);
  }

}

shared_ptr<MpcKeyPrgController> HelixInternal::SyncPRGKey() {
  MpcPRGKeysV2 keys;
  if (player == PARTY_0) {
    keys.key_prg0 = gen_key_str();
  } else if (player == PARTY_1) {
    keys.key_prg1 = gen_key_str();
  } else if (player == PARTY_2) {
    keys.key_prg2 = gen_key_str();
  }
  keys.key_private = gen_key_str();

  // common or public key
  string k01, k02, k12, k0;
  k0 = gen_key_str();
  k01 = gen_key_str();
  k02 = gen_key_str();
  k12 = gen_key_str();

  SyncPRGKey(PARTY_0, PARTY_1, k01, k01);
  SyncPRGKey(PARTY_0, PARTY_2, k02, k02);
  SyncPRGKey(PARTY_1, PARTY_2, k12, k12);
  SyncPRGKey(PARTY_2, PARTY_0, k0, k0);
  SyncPRGKey(PARTY_2, PARTY_1, k0, k0);
  keys.key_prg = k0;
  keys.key_prg01 = k01;
  keys.key_prg02 = k02;
  keys.key_prg12 = k12;
  //keys.fmt_print();

  //! set gseed, different from k0, simply
  string kgseed(k0);
  kgseed[0] = '\x44';
  gseed->reseed(kgseed);
  //cout << "KGS " << kgseed << endl;

  auto prg_controller = std::make_shared<MpcKeyPrgController>();
  prg_controller->Init(keys);
  tlog_info << "helix SyncPRGKey ok.";
  return prg_controller;
}

/**
 * A's send to B
 * key_send.length() ==== key_recv.length() > 0
 */
void HelixInternal::SyncPRGKey(
  int partyA,
  int partyB,
  const std::string& key_send,
  std::string& key_recv) {
  if (player == partyA) {
    send(partyB, key_send.data(), key_send.length());
    AUDIT("id:{}, P{} send SyncPRGKey to P{}: {}", msgid.get_hex(), player, partyB, key_send);
  }
  if (player == partyB) {
    recv(partyA, (char*)key_recv.data(), key_recv.length());
    AUDIT("id:{}, P{} recv SyncPRGKey from P{}: {}", msgid.get_hex(), player, partyA, key_recv);
  }
}

/**
 * global random seed
 */
void HelixInternal::RandSeed(vector<uint64_t>& seeds, size_t size) {
  resize_vector(seeds, size);
  _PRF(gseed, (char*)seeds.data(), size * sizeof(uint64_t));
  AUDIT("id:{}, P0, P1 and P2 generate common random number{}", msgid.get_hex(), Vector<uint64_t>(seeds));
}

} // namespace helix
} // namespace rosetta
