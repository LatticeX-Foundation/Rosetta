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
#include "cc/modules/protocol/mpc/snn/src/impl/op_impl.h"

namespace rosetta {
namespace snn {

// Chunk wise maximum of a vector of size rows*columns and maximum is caclulated of every
// column number of elements. max is a vector of size rows. maxIndex contains the index of
// the maximum value.
// PARTY_A, PARTY_B start with the shares in a and {A,B} and {C,D} have the results in
// max and maxIndex.
int Max::funcMaxMPC(
  const vector<mpc_t>& a,
  vector<mpc_t>& max,
  vector<mpc_t>& maxIndex,
  size_t rows,
  size_t columns) {
  if (THREE_PC) {
    vector<mpc_t> diff(rows), diffIndex(rows), rp(rows), indexShares(rows * columns, 0);

    for (size_t i = 0; i < rows; ++i) {
      max[i] = a[i * columns];
      maxIndex[i] = 0;
    }

    if (partyNum == PARTY_A) {
      for (size_t i = 0; i < rows; ++i)
        for (size_t j = 0; j < columns; ++j)
          indexShares[i * columns + j] = j;
    }

    for (size_t i = 1; i < columns; ++i) {
      for (size_t j = 0; j < rows; ++j)
        diff[j] = max[j] - a[j * columns + i];

      for (size_t j = 0; j < rows; ++j)
        diffIndex[j] = maxIndex[j] - indexShares[j * columns + i];

      ////funcRELUPrime3PC(diff, rp, rows);
      GetMpcOpInner(ReluPrime)->Run3PC(diff, rp, rows);

      ////funcSelectShares3PC(diff, rp, max, rows);
      GetMpcOpInner(SelectShares)->Run3PC(diff, rp, max, rows);
      ////funcSelectShares3PC(diffIndex, rp, maxIndex, rows);
      GetMpcOpInner(SelectShares)->Run3PC(diffIndex, rp, maxIndex, rows);

      for (size_t j = 0; j < rows; ++j)
        max[j] = max[j] + a[j * columns + i];

      for (size_t j = 0; j < rows; ++j)
        maxIndex[j] = maxIndex[j] + indexShares[j * columns + i];
    }
  }

  if (FOUR_PC) {
    // I have removed 4PC, by yyl, 202.03.12
  }
  return 0;
}

// MaxIndex is of size rows. a is of size rows*columns.
// a will be set to 0's except at maxIndex (in every set of column)
int MaxIndex::funcMaxIndexMPC(
  vector<mpc_t>& a,
  const vector<mpc_t>& maxIndex,
  size_t rows,
  size_t columns) {
  LOGI("funcMaxIndexMPC");
  assert(((columns & (columns - 1)) == 0) && "funcMaxIndexMPC works only for power of 2 columns");
  assert((columns < 257) && "This implementation does not support larger than 257 columns");

  vector<small_mpc_t> random(rows);

  if (PRIMARY) {
    vector<small_mpc_t> toSend(rows);
    for (size_t i = 0; i < rows; ++i)
      toSend[i] = (small_mpc_t)maxIndex[i] % columns;

    populateRandomVector<small_mpc_t>(random, rows, "COMMON", "POSITIVE");
    if (partyNum == PARTY_A)
      addVectors<small_mpc_t>(toSend, random, toSend, rows);

    sendVector<small_mpc_t>(toSend, PARTY_C, rows);
  }

  if (partyNum == PARTY_C) {
    vector<small_mpc_t> index(rows), temp(rows);
    vector<mpc_t> vector(rows * columns, 0), share_1(rows * columns), share_2(rows * columns);
    receiveVector<small_mpc_t>(index, PARTY_A, rows);
    receiveVector<small_mpc_t>(temp, PARTY_B, rows);
    addVectors<small_mpc_t>(index, temp, index, rows);

    for (size_t i = 0; i < rows; ++i)
      index[i] = index[i] % columns;

    for (size_t i = 0; i < rows; ++i)
      vector[i * columns + index[i]] = 1;

    splitIntoShares(vector, share_1, share_2, rows * columns);
    sendVector<mpc_t>(share_1, PARTY_A, rows * columns);
    sendVector<mpc_t>(share_2, PARTY_B, rows * columns);
  }

  if (PRIMARY) {
    receiveVector<mpc_t>(a, PARTY_C, rows * columns);
    size_t offset = 0;
    for (size_t i = 0; i < rows; ++i) {
      rotate(
        a.begin() + offset, a.begin() + offset + (random[i] % columns),
        a.begin() + offset + columns);
      offset += columns;
    }
  }
  return 0;
}

// Chunk wise minimum of a vector of size rows*columns and minimum is caclulated of every
// column number of elements. min is a vector of size rows. minIndex contains the index of
// the minimum value.
// PARTY_A, PARTY_B start with the shares in a and {A,B} and {C,D} have the results in
// min and minIndex.
int Min::funcMinMPC(
  const vector<mpc_t>& a,
  vector<mpc_t>& min,
  vector<mpc_t>& minIndex,
  size_t rows,
  size_t columns) {
  if (THREE_PC) {
    vector<mpc_t> diff(rows), diffIndex(rows), rp(rows), indexShares(rows * columns, 0);

    for (size_t i = 0; i < rows; ++i) {
      min[i] = a[i * columns];
      minIndex[i] = 0;
    }

    if (partyNum == PARTY_A) {
      for (size_t i = 0; i < rows; ++i)
        for (size_t j = 0; j < columns; ++j)
          indexShares[i * columns + j] = j;
    }

    for (size_t i = 1; i < columns; ++i) {
      for (size_t j = 0; j < rows; ++j)
        diff[j] = a[j * columns + i] - min[j];

      for (size_t j = 0; j < rows; ++j)
        diffIndex[j] = indexShares[j * columns + i] - minIndex[j];

      ////funcRELUPrime3PC(diff, rp, rows);
      GetMpcOpInner(ReluPrime)->Run3PC(diff, rp, rows);

      // // just local multiply minus one
      // for (size_t idx = 0; idx < rp.size(); ++idx) {
      //   rp[idx] = rp[idx] * FloatToMpcType(-1);
      // }
      // funcTruncate2PC(rp, FLOAT_PRECISION_M, rp.size(), PARTY_A, PARTY_B);

      ////funcSelectShares3PC(diff, rp, min, rows);
      GetMpcOpInner(SelectShares)->Run3PC(diff, rp, min, rows);
      ////funcSelectShares3PC(diffIndex, rp, minIndex, rows);
      GetMpcOpInner(SelectShares)->Run3PC(diffIndex, rp, minIndex, rows);

      for (size_t j = 0; j < rows; ++j)
        min[j] = a[j * columns + i] - min[j];

      for (size_t j = 0; j < rows; ++j)
        minIndex[j] = indexShares[j * columns + i] - minIndex[j];
    }
  } //THREE_PC

  if (FOUR_PC) {
    // I have removed 4PC, by yyl, 202.03.12
    //throw;
  }
  return 0;
}

// MinIndex is of size rows. a is of size rows*columns.
// a will be set to 0's except at MinIndex (in every set of column)
int MinIndex::funcMinIndexMPC(
  vector<mpc_t>& a,
  const vector<mpc_t>& minIndex,
  size_t rows,
  size_t columns) {
  LOGI("funcMinIndexMPC not implements, [WARNING] !!!!!\n");

  //   assert(((columns & (columns - 1)) == 0) && "funcMinIndexMPC works only for power of 2 columns");
  //   assert((columns < 257) && "This implementation does not support larger than 257 columns");

  //   vector<small_mpc_t> random(rows);

  //   if (PRIMARY) {
  //     vector<small_mpc_t> toSend(rows);
  //     for (size_t i = 0; i < rows; ++i)
  //       toSend[i] = (small_mpc_t)minIndex[i] % columns;

  //     populateRandomVector<small_mpc_t>(random, rows, "COMMON", "POSITIVE");
  //     if (partyNum == PARTY_A)
  //       addVectors<small_mpc_t>(toSend, random, toSend, rows);

  //     sendVector<small_mpc_t>(toSend, PARTY_C, rows);
  //   }

  //   if (partyNum == PARTY_C) {
  //     vector<small_mpc_t> index(rows), temp(rows);
  //     vector<mpc_t> vec(rows * columns, 0), share_1(rows * columns), share_2(rows * columns);
  //     receiveVector<small_mpc_t>(index, PARTY_A, rows);
  //     receiveVector<small_mpc_t>(temp, PARTY_B, rows);
  //     addVectors<small_mpc_t>(index, temp, index, rows);

  //     for (size_t i = 0; i < rows; ++i)
  //       index[i] = index[i] % columns;

  //     for (size_t i = 0; i < rows; ++i)
  //       vec[i * columns + index[i]] = 1;

  //     splitIntoShares(vec, share_1, share_2, rows * columns);
  //     sendVector<mpc_t>(share_1, PARTY_A, rows * columns);
  //     sendVector<mpc_t>(share_2, PARTY_B, rows * columns);
  //   }

  //   if (PRIMARY) {
  //     receiveVector<mpc_t>(a, PARTY_C, rows * columns);
  //     size_t offset = 0;
  //     for (size_t i = 0; i < rows; ++i) {
  //       rotate(
  //         a.begin() + offset, a.begin() + offset + (random[i] % columns),
  //         a.begin() + offset + columns);
  //       offset += columns;
  //     }
  //   }
  return 0;
}

int Mean::funcMean(const vector<mpc_t>& a, vector<mpc_t>& b, size_t rows, size_t cols) {
  if (PRIMARY) {
    for (int i = 0; i < rows; i++) {
      mpc_t v = 0;
      for (int j = 0; j < cols; j++) {
        int idx = i * cols + j;
        v += a[idx];
      }
      b[i] = v * FloatToMpcType(1.0 / cols);
    }
    funcTruncate2PC(b, FLOAT_PRECISION_M, rows, PARTY_A, PARTY_B);
  }
  return 0;
}

int Sum::funcSum(const vector<mpc_t>& a, vector<mpc_t>& b, size_t rows, size_t cols) {
  if (PRIMARY) {
    for (int i = 0; i < rows; i++) {
      mpc_t v = 0;
      for (int j = 0; j < cols; j++) {
        int idx = i * cols + j;
        v += a[idx];
      }
      b[i] = v;
    }
  }
  return 0;
}

int AddN::funcAddN(const vector<mpc_t>& a, vector<mpc_t>& b, size_t rows, size_t cols) {
  if (PRIMARY) {
    // each row accumulate
    for (int i = 0; i < cols; i++) {
      mpc_t v = 0;
      for (int j = 0; j < rows; j++) {
        v += a[j * cols + i];
      }
      b[i] = v;
    }
  }
  return 0;
}

} // namespace snn
} // namespace rosetta