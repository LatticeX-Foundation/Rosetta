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

// used by IsZero internally, to generate arithematic-shared random value, and bit-share of its each bits.
void HelixInternal::_RandomShareAB(vector<Share>& aX, vector<vector<BitShare>>& bX, size_t size) {
  resize_vector(aX, size);
  resize_vector(bX, size);
  // for mpc_t, this should be 8*8 = 64
  int LEN = sizeof(mpc_t) * 8;

  AUDIT("id:{}, P{} _RandomShareAB, compute: get shares aX, bX for uniformly random b in(0,1)", msgid.get_hex(), player);

  vector<mpc_t> r_0(size, 0), r_1(size, 0), r(size, 0);
  // XOR bit-share in compact format
  vector<mpc_t> r_0_b(size, 0), r_1_b(size, 0);

  PRF02(r_0, size);
  if ((player == PARTY_0) || (player == PARTY_2)) {
    AUDIT("id:{}, P{} _RandomShareAB, compute: get shares aX, bX for uniformly random b in(0,1), P0 and P2 generate r0(mpc_t){}", msgid.get_hex(), player, Vector<mpc_t>(r_0));
  }

  PRF12(r_1, size);
  if ((player == PARTY_1) || (player == PARTY_2)) {
    AUDIT("id:{}, P{} _RandomShareAB, compute: get shares aX, bX for uniformly random b in(0,1), P1 and P2 generate r1(mpc_t){}", msgid.get_hex(), player, Vector<mpc_t>(r_1));
  }

  PRF02(r_0_b, size);
  if ((player == PARTY_0) || (player == PARTY_2)) {
    AUDIT("id:{}, P{} _RandomShareAB, compute: get shares aX, bX for uniformly random b in(0,1), P0 and P2 generate r0b(mpc_t){}", msgid.get_hex(), player, Vector<mpc_t>(r_0_b));
  }
  
  // fixed for debuging
  // r_0 = vector<mpc_t>(size, 2 << FLOAT_PRECISION_M);
  // r_1 = vector<mpc_t>(size, 3 << FLOAT_PRECISION_M);
  // r = r_0 + r_1;
  // r_0_b = vector<mpc_t>(size, 2 << FLOAT_PRECISION_M);

  if ((player == PARTY_1) || (player == PARTY_2)) {
    if (is_helper()) {
      vector<mpc_t> r(size, 0);
      r = r_0 + r_1;
      for (int i = 0; i < size; ++i) {
        r_1_b[i] = r[i] ^ r_0_b[i];
      }

      send(PARTY_1, r_1_b, size);
      AUDIT("id:{}, P{} _RandomShareAB, compute: get shares aX, bX for uniformly random b in(0,1), P{} locally compute r1b(=(r0+r1)^r0b) and SEND to P1, r1b(mpc_t){}", msgid.get_hex(), player, player, Vector<mpc_t>(r_1_b));
    } else {
      recv(PARTY_2, r_1_b, size);
      AUDIT("id:{}, P{} _RandomShareAB, compute: get shares aX, bX for uniformly random b in(0,1), P{} RECV from P2 r1b(mpc_t){}", msgid.get_hex(), player, player, Vector<mpc_t>(r_1_b));
    }
  }


  // Local repackage values in Share format.
  for (int i = 0; i < size; i++) {
    if (is_helper()) {
      aX[i].s0.A0 = r_0[i];
      aX[i].s1.A1 = r_1[i];
    } else {
      aX[i].s0.delta = 0;
      if (player == PARTY_0)
        aX[i].s1.A0 = r_0[i];
      else
        aX[i].s1.A1 = r_1[i];
    }
  }
  AUDIT("id:{}, P{} _RandomShareAB, compute: get shares aX, bX for uniformly random b in(0,1), output aX(Share){}", msgid.get_hex(), player, Vector<Share>(aX));

  // Local repackage values in BitShare format.
  for (int i = 0; i < size; i++) {
    bX[i] = vector<BitShare>(LEN);
    for ( int j = 0; j < LEN; ++j) {
      if (is_helper()) {
        bX[i][j].s0.A0 = bit_t( (r_0_b[i] >> j) & 0x01);
        bX[i][j].s1.A1 = bit_t( (r_1_b[i] >> j) & 0x01);
      } else {
        bX[i][j].s0.delta = 0;
        if (player == PARTY_0)
          bX[i][j].s1.A0 = bit_t( (r_0_b[i] >> j) & 0x01);
        else
          bX[i][j].s1.A1 = bit_t( (r_1_b[i] >> j) & 0x01);
      }
    }
  }

  AUDIT("id:{}, P{} _RandomShareAB, compute: get shares aX, bX for uniformly random b in(0,1), output bX(BitShare){}", msgid.get_hex(), player, Vector<BitShare>(bX[0]));
}

void HelixInternal::FanInBitAdd(const vector<vector<BitShare>>& a,
                                vector<BitShare>& c, size_t vec_size) {
  // S0: check and init
  //    the inner vector must have the same size 
  // normally, the LEN should be sizeof(mpc_t) * 8
  // cout << "FanInBitAdd size:" << a.size() << " of inner:" << a[0].size() << endl;
  for (int i=0; i<a.size(); ++i) {
    AUDIT("id:{}, P{} FanInBitAdd input a(BitShare)[{}]{}", msgid.get_hex(), player, i, Vector<BitShare>(a[i]));
  }

  int LEN = a[0].size();
  for (int i = 0; i < vec_size; ++i) {
    assert(a[i].size() == LEN);
  }

  vector<vector<BitShare>> all_bits = a;
  int Depth = int(log2(LEN));
  assert((1 << Depth) == LEN && "Only 2^i len is supported!");

  vector<BitShare> flat_a_vec;
  vector<BitShare> flat_b_vec;
  int curr_len = LEN;

  // this is binary-tree strategy to perform AND in parallel.
  for(int i = 0; i < Depth; ++i) {
    // cout << "depth:" << i << endl;
    flat_a_vec.clear();
    flat_b_vec.clear();
    // flat matrix to vector
    for (int k = 0; k < vec_size; ++k) {
      for (int j = 0; j < curr_len -1; j = j + 2) {
        flat_a_vec.push_back(all_bits[k][j]);
        flat_b_vec.push_back(all_bits[k][j+1]);
      }
    }
    vector<BitShare> flat_c_vec(flat_a_vec.size());
    Mul(flat_a_vec, flat_b_vec, flat_c_vec);
    // for debuging
    // RevealAndPrint(flat_a_vec, "Debug Bit A");
    // RevealAndPrint(flat_b_vec, "Debug Bit B");
    // RevealAndPrint(flat_c_vec, "Debug Bit C");

    // reconstruct matrix from vector 
    curr_len = curr_len / 2;
    // cout << "new len:" << curr_len << endl;
    for(int k = 0; k < vec_size; ++k) {
      all_bits[k].clear();
      // Todo: reorder this so that we insert at the last.
      all_bits[k].insert(all_bits[k].begin(), flat_c_vec.begin() + k * curr_len, flat_c_vec.begin() + (k+1) * curr_len);
    }
  }

  for(int i = 0; i < vec_size; ++i) {
    // make sure result is legal: all size should be 1! 
    // cout << "check[" << i << "]:" << all_bits[i].size() << ":" << all_bits[i][0] << std::endl; 
    c[i] = all_bits[i][0];
  }

  AUDIT("id:{}, P{} FanInBitAdd output(BitShare){}", msgid.get_hex(), player, Vector<BitShare>(c));
}
/**
 * internal used by Equal
 */
void HelixInternal::IsZero(
  const vector<Share>& shareZ,
  vector<Share>& B,
  int eq) {
  size_t size = shareZ.size();

  AUDIT("id:{}, P{} _IsZero compute: B=iszero(Z) by checking share Z is zero or not, input(Share){}", msgid.get_hex(), player, Vector<Share>(shareZ));

  // step 1
  vector<Share> r;
  vector<vector<BitShare>> b;
  _RandomShareAB(r, b, size);

  vector<Share> masked_z = shareZ;
  Add(shareZ, r, masked_z);
  AUDIT("id:{}, P{} _IsZero compute: masked_z=shareZ+r, masked_z(Share){}", msgid.get_hex(), player, Vector<Share>(masked_z));

  vector<mpc_t> plain_masked_z(size, 0);
  // P0 and P1 reveal plaintext (z + r)
  Reveal_(masked_z, plain_masked_z, encode_reveal_mask(3));

  // tmp debuging:
  // RevealAndPrint(shareZ, " input z:");
  // RevealAndPrint(r, " masked r:");
  // RevealAndPrint(masked_z, "debug z+r:");
  // for(int i = 0; i < size; ++i) {
  //   RevealAndPrint(b[i], to_string(i) + "-th bit");
  // }
  // print_vec(plain_masked_z, 20, "revealed z+r");

  int BIT_L = sizeof(mpc_t)*8;
  vector<vector<BitShare>> masked_z_bit = b;
  // Note that here we can not use the 'Linear',
  // such as Linear({bitX[i], b[i]}, {1, 1, 1}, masked_z_bit[i])
  // because the masked_z_bit is not in shared status but plain status.
  // the plain_masked_z BB in Bitshare is [because the BB should not be exposed to P2] 
  // P0: [BB, 0], P1: [BB, 0], P2: [0, 0] that BB^0^0 = BB
  // const 1 in bitshare is:
  // P0: [1, 0], P1: [1, 0], P2: [0, 0], that 1^0^0 = 1
  for (int i = 0; i < size; ++i) {
    for (int j = 0; j < BIT_L; ++j) {
      bit_t BB = bit_t(((plain_masked_z[i] >> j) & 0x01));
      if (player == PARTY_0 || player == PARTY_1) {
        b[i][j].s0.delta = b[i][j].s0.delta ^ bit_t(0x01) ^ BB;
      }
    }
  }

  vector<BitShare> res(size);
  FanInBitAdd(b, res, size);

  // for NotEqual: 1^res
  if(eq == 0) {
    for (int i = 0; i < size; i++) {
      if (player == PARTY_0 || player == PARTY_1) {
        res[i].s0.delta = res[i].s0.delta ^ bit_t(0x01);
      }
    }
  }
  B2A(res, B);

  AUDIT("id:{}, P{} _IsZero compute: B=iszero(Z) by checking share Z is zero or not, output(Share){}", msgid.get_hex(), player, Vector<Share>(B));
}

/**
 * This version is retired. Do NOT use this any more.
 */
void HelixInternal::Equal_(
  const vector<Share>& shareX_Y,
  const vector<Share>& shareY_X,
  vector<Share>& Z,
  int eq) {
  AUDIT("id:{}, P{} Equal if(X==Y, bitZ=1, bitZ= 1^ bitX_Y ^ bitY_X) input X_Y(Share){}", msgid.get_hex(), player, Vector<Share>(shareX_Y));
  AUDIT("id:{}, P{} Equal if(X==Y, bitZ=1, bitZ= 1^ bitX_Y ^ bitY_X) input Y_X(Share){}", msgid.get_hex(), player, Vector<Share>(shareY_X));

  size_t size = shareX_Y.size();

  // step 1
  vector<BitShare> bitX_Y(size), bitY_X(size);
  MSB(shareX_Y, bitX_Y); // if X-Y >= 0, get 0
  MSB(shareY_X, bitY_X); // if Y-X >= 0, get 0

  // for Equal,    bitZ = 1 ^ bitX_Y ^ bitY_X. if X == Y, got 1
  // for NotEqual, bitZ = 0 ^ bitX_Y ^ bitY_X. if X != Y, got 1
  vector<BitShare> bitZ(size);
  for (int i = 0; i < size; i++) {
    bitZ[i].s0.delta = eq ^ bitX_Y[i].s0.delta ^ bitY_X[i].s0.delta;
    bitZ[i].s1.A0 = eq ^ bitX_Y[i].s1.A0 ^ bitY_X[i].s1.A0;
  }

  // step 2
  B2A(bitZ, Z);

  AUDIT("id:{}, P{} Equal if(X==Y, bitZ=1, bitZ= 1^ bitX_Y ^ bitY_X) outout Z(Share){}", msgid.get_hex(), player, Vector<Share>(Z));
}

/**
 * Z = (X != Y) ? 1 : 0 ---> (X >= Y && Y >= X) ? 0 : 1
 */
void HelixInternal::NotEqual(const vector<Share>& X, const vector<Share>& Y, vector<Share>& Z) {
  AUDIT("id:{}, P{} NotEqual if(X!=Y, 1, 0) input X(Share){}", msgid.get_hex(), player, Vector<Share>(X));
  AUDIT("id:{}, P{} NotEqual if(X!=Y, 1, 0) input Y(Share){}", msgid.get_hex(), player, Vector<Share>(Y));

  size_t size = X.size();
  resize_vector(Z, size);

  vector<Share> shareX_Y(size);
  Sub(X, Y, shareX_Y);
  IsZero(shareX_Y, Z, 0);

  AUDIT("id:{}, P{} NotEqual if(X!=Y, 1, 0) outout Z(Share){}", msgid.get_hex(), player, Vector<Share>(Z));
}

/**
 * Z = (X == Y) ? 1 : 0 ---> (X >= Y && Y >= X) ? 1 : 0
 */
void HelixInternal::Equal(const vector<Share>& X, const vector<Share>& Y, vector<Share>& Z) {
  AUDIT("id:{}, P{} Equal if(X==Y, 1, 0) input X(Share){}", msgid.get_hex(), player, Vector<Share>(X));
  AUDIT("id:{}, P{} Equal if(X==Y, 1, 0) input Y(Share){}", msgid.get_hex(), player, Vector<Share>(Y));

  size_t size = X.size();
  resize_vector(Z, size);

  vector<Share> shareX_Y(size);
  Sub(X, Y, shareX_Y);

  IsZero(shareX_Y, Z, 1);

  AUDIT("id:{}, P{} Equal if(X==Y, 1, 0) outout Z(Share){}", msgid.get_hex(), player, Vector<Share>(Z));
}

/**
 * LessDReLU \n
 * compare with DReLU, no `1^bitX`
 * 
 * base on MSB + B2A
 * 
 * \param[out] Y = LessDReLU(X), 1 if X < 0; else 0
 * 
 */
void HelixInternal::LessDReLU_(const vector<Share>& X, vector<Share>& Y) {
  AUDIT("id:{}, P{} LessDReLU_ if(X < 0, 1, 0) input X(Share){}", msgid.get_hex(), player, Vector<Share>(X));

  size_t size = X.size();
  resize_vector(Y, size);

  // step 1
  vector<BitShare> bitX(size);
  MSB(X, bitX);

  // step 2
  B2A(bitX, Y);

  AUDIT("id:{}, P{} LessDReLU_ if(X < 0, 1, 0) outout Y(Share){}", msgid.get_hex(), player, Vector<Share>(Y));
}
/**
 * Z = (X < Y) ? 1 : 0
 */
void HelixInternal::Less(const vector<Share>& X, const vector<Share>& Y, vector<Share>& Z) {
  AUDIT("id:{}, P{} Less if(X<Y, 1, 0) input X(Share){}", msgid.get_hex(), player, Vector<Share>(X));
  AUDIT("id:{}, P{} Less if(X<Y, 1, 0) input Y(Share){}", msgid.get_hex(), player, Vector<Share>(Y));

  vector<Share> shareXY;
  Sub(X, Y, shareXY);
  LessDReLU_(shareXY, Z);

  AUDIT("id:{}, P{} Less if(X<Y, 1, 0) outout Z(Share){}", msgid.get_hex(), player, Vector<Share>(Z));
}
/**
 * Z = (X <= Y) ? 1 : 0
 */
void HelixInternal::LessEqual(const vector<Share>& X, const vector<Share>& Y, vector<Share>& Z) {
  AUDIT("id:{}, P{} LessEqual if(X<=Y, 1, 0) input X(Share){}", msgid.get_hex(), player, Vector<Share>(X));
  AUDIT("id:{}, P{} LessEqual if(X<=Y, 1, 0) input Y(Share){}", msgid.get_hex(), player, Vector<Share>(Y));

  GreaterEqual(Y, X, Z);

  AUDIT("id:{}, P{} LessEqual if(X<=Y, 1, 0) outout Z(Share){}", msgid.get_hex(), player, Vector<Share>(Z));
}
/**
 * Z = (X > Y) ? 1 : 0
 */
void HelixInternal::Greater(const vector<Share>& X, const vector<Share>& Y, vector<Share>& Z) {
  AUDIT("id:{}, P{} Greater if(X>Y, 1, 0) input X(Share){}", msgid.get_hex(), player, Vector<Share>(X));
  AUDIT("id:{}, P{} Greater if(X>Y, 1, 0) input Y(Share){}", msgid.get_hex(), player, Vector<Share>(Y));

  Less(Y, X, Z);

  AUDIT("id:{}, P{} Greater if(X>Y, 1, 0) outout Z(Share){}", msgid.get_hex(), player, Vector<Share>(Z));
}
/**
 * Z = (X >= Y) ? 1 : 0
 */
void HelixInternal::GreaterEqual(const vector<Share>& X, const vector<Share>& Y, vector<Share>& Z) {
  AUDIT("id:{}, P{} GreaterEqual if(X>=Y, 1, 0) input X(Share){}", msgid.get_hex(), player, Vector<Share>(X));
  AUDIT("id:{}, P{} GreaterEqual if(X>=Y, 1, 0) input Y(Share){}", msgid.get_hex(), player, Vector<Share>(Y));

  vector<Share> shareXY;
  Sub(X, Y, shareXY);
  DReLU(shareXY, Z);

  AUDIT("id:{}, P{} GreaterEqual if(X>=Y, 1, 0) outout Z(Share){}", msgid.get_hex(), player, Vector<Share>(Z));
}

/**
 * sharing and constant version
 */
void HelixInternal::NotEqual(const vector<Share>& X, const vector<double>& C, vector<Share>& Z) {
  AUDIT("id:{}, P{} NotEqual if(X!=Y, 1, 0) input X(Share){}", msgid.get_hex(), player, Vector<Share>(X));
  AUDIT("id:{}, P{} NotEqual if(X!=Y, 1, 0) input Y(double){}", msgid.get_hex(), player, Vector<double>(C));

  size_t size = X.size();
  resize_vector(Z, size);

  vector<Share> shareX_Y(size);
  Sub(X, C, shareX_Y);

  IsZero(shareX_Y, Z, 0);

  AUDIT("id:{}, P{} NotEqual if(X!=Y, 1, 0) outout Z(Share){}", msgid.get_hex(), player, Vector<Share>(Z));
}

void HelixInternal::NotEqual(const vector<double>& C, const vector<Share>& X, vector<Share>& Z) {
  AUDIT("id:{}, P{} NotEqual if(X!=Y, 1, 0) input X(Share){}", msgid.get_hex(), player, Vector<Share>(X));
  AUDIT("id:{}, P{} NotEqual if(X!=Y, 1, 0) input Y(double){}", msgid.get_hex(), player, Vector<double>(C));

  NotEqual(X, C, Z);

  AUDIT("id:{}, P{} NotEqual if(X!=Y, 1, 0) outout Z(Share){}", msgid.get_hex(), player, Vector<Share>(Z));
}

void HelixInternal::Equal(const vector<Share>& X, const vector<double>& C, vector<Share>& Z) {
  AUDIT("id:{}, P{} NotEqual if(X!=Y, 1, 0) input X(Share){}", msgid.get_hex(), player, Vector<Share>(X));
  AUDIT("id:{}, P{} NotEqual if(X!=Y, 1, 0) input Y(double){}", msgid.get_hex(), player, Vector<double>(C));

  size_t size = X.size();
  resize_vector(Z, size);

  vector<Share> shareX_Y(size);
  Sub(X, C, shareX_Y);

  IsZero(shareX_Y, Z, 1);

  AUDIT("id:{}, P{} NotEqual if(X!=Y, 1, 0) outout Z(Share){}", msgid.get_hex(), player, Vector<Share>(Z));
}

void HelixInternal::Equal(const vector<double>& C, const vector<Share>& X, vector<Share>& Z) {
  Equal(X, C, Z);
}

void HelixInternal::Less(const vector<Share>& X, const vector<double>& C, vector<Share>& Z) {
  AUDIT("id:{}, P{} Less if(X<Y, 1, 0) input X(Share){}", msgid.get_hex(), player, Vector<Share>(X));
  AUDIT("id:{}, P{} Less if(X<Y, 1, 0) input Y(double){}", msgid.get_hex(), player, Vector<double>(C));

  vector<Share> sC;
  Sub(X, C, sC);
  LessDReLU_(sC, Z);

  AUDIT("id:{}, P{} Less if(X<Y, 1, 0) outout Z(Share){}", msgid.get_hex(), player, Vector<Share>(Z));
}

void HelixInternal::Less(const vector<double>& C, const vector<Share>& X, vector<Share>& Z) {
  AUDIT("id:{}, P{} Less if(X<Y, 1, 0) input X(double){}", msgid.get_hex(), player, Vector<double>(C));
  AUDIT("id:{}, P{} Less if(X<Y, 1, 0) input Y(Share){}", msgid.get_hex(), player, Vector<Share>(X));

  vector<Share> sC;
  Sub(C, X, sC);
  LessDReLU_(sC, Z);

  AUDIT("id:{}, P{} Less if(X<Y, 1, 0) outout Z(Share){}", msgid.get_hex(), player, Vector<Share>(Z));
}
void HelixInternal::LessEqual(const vector<Share>& X, const vector<double>& C, vector<Share>& Z) {
  GreaterEqual(C, X, Z);
}
void HelixInternal::LessEqual(const vector<double>& C, const vector<Share>& X, vector<Share>& Z) {
  GreaterEqual(X, C, Z);
}
void HelixInternal::Greater(const vector<Share>& X, const vector<double>& C, vector<Share>& Z) {
  Less(C, X, Z);
}
void HelixInternal::Greater(const vector<double>& C, const vector<Share>& X, vector<Share>& Z) {
  Less(X, C, Z);
}

void HelixInternal::GreaterEqual(
  const vector<Share>& X,
  const vector<double>& C,
  vector<Share>& Z) {
  AUDIT("id:{}, P{} GreaterEqual if(X>=Y, 1, 0) input X(Share){}", msgid.get_hex(), player, Vector<Share>(X));
  AUDIT("id:{}, P{} GreaterEqual if(X>=Y, 1, 0) input Y(double){}", msgid.get_hex(), player, Vector<double>(C));

  vector<Share> sC;
  Sub(X, C, sC);
  DReLU(sC, Z);

  AUDIT("id:{}, P{} GreaterEqual if(X>=Y, 1, 0) outout Z(Share){}", msgid.get_hex(), player, Vector<Share>(Z));
}

void HelixInternal::GreaterEqual(
  const vector<double>& C,
  const vector<Share>& X,
  vector<Share>& Z) {
  AUDIT("id:{}, P{} GreaterEqual if(X>=Y, 1, 0) input X(Share){}", msgid.get_hex(), player, Vector<Share>(X));
  AUDIT("id:{}, P{} GreaterEqual if(X>=Y, 1, 0) input Y(double){}", msgid.get_hex(), player, Vector<double>(C));

  vector<Share> sC;
  Sub(C, X, sC);
  DReLU(sC, Z);

  AUDIT("id:{}, P{} GreaterEqual if(X>=Y, 1, 0) outout Z(Share){}", msgid.get_hex(), player, Vector<Share>(Z));
}

} // namespace helix
} // namespace rosetta
