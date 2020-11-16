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
#include <cmath>
namespace rosetta {
namespace snn {

// Chunk wise maximum of a vector of size rows*columns and maximum is calculated of every
// column number of elements. max is a vector of size rows.
// PARTY_A, PARTY_B start with the shares in a and {A,B} and {C,D} have the results in `max`.
// ToDo(GeorgeShi): If need_index is set to true, maxIndex contains the index of the maximum value.
int Max::funcMaxMPC(
  const vector<mpc_t>& a,
  vector<mpc_t>& max,
  vector<mpc_t>& maxIndex,
  size_t rows,
  size_t columns,
  bool need_index) {
  if (THREE_PC) {
    // vector<mpc_t> diffIndex, indexShares;
    // if (need_index) {
    //   diffIndex.resize(rows);
    //   indexShares.resize(rows * columns);
      
    //   for (size_t i = 0; i < rows; ++i) {
    //     maxIndex[i] = 0;
    //     for (size_t j = 0; j < columns; ++j)
    //       indexShares[i * columns + j] = j;
    //   }
    // }
    // if(PRIMARY) {
    //   GetMpcOpInner(Reconstruct2PC)->Run(a, a.size(), "input value");
    // }
    // tree-structure to reduce iteration to call inner OPs.
    int depth = ceil(log2(columns));
    // cout << "debug: depth " << depth << endl;
    int curr_L = columns;
    vector<mpc_t> level_data = a;
    vector<mpc_t> first_v;
    vector<mpc_t> second_v;
    vector<mpc_t> diff_v;
    vector<mpc_t> comp_res;
    bool need_pad = false;
    for (size_t i = 0; i < depth; ++i) {
      // cout << "depth " << i << endl;
      // cout << "curr L:" << curr_L << endl;

      int next_L = ceil( curr_L / 2.0);
      vector<mpc_t> new_data;
      first_v.clear();
      second_v.clear();
      diff_v.clear();
      comp_res.clear();
      // If the numbers in a row is odd, there is no need to comapre the last element. 
      if (curr_L % 2 == 1) {
        need_pad = true;
      }
      // if(PRIMARY) {
      //     	GetMpcOpInner(Reconstruct2PC)->Run(level_data, level_data.size(), "curr level input value");
    	// }
      // pack for batch Relu.
      for (int j = 0; j < rows; ++j) {
        for (int k = 0; k < curr_L - 1; k = k + 2) {
          first_v.push_back(level_data[j*curr_L + k]);
          second_v.push_back(level_data[j*curr_L + k + 1]);
          diff_v.push_back(first_v.back() - second_v.back());
        }
      }

      int comp_size = diff_v.size();
      comp_res.resize(comp_size);
      new_data.resize(comp_size);
      GetMpcOpInner(ReluPrime)->Run3PC(diff_v, comp_res, comp_size);
			GetMpcOpInner(Select1Of2)->Run(first_v, second_v, comp_res, new_data, comp_size);
      // if(PRIMARY) {
      //     	GetMpcOpInner(Reconstruct2PC)->Run(first_v, first_v.size(), "first_v");
    	// }
      // if(PRIMARY) {
      //     	GetMpcOpInner(Reconstruct2PC)->Run(second_v, second_v.size(), "second_v");
    	// }
      // if(PRIMARY) {
      //     	GetMpcOpInner(Reconstruct2PC)->Run(new_data, new_data.size(), "COMP result");
    	// }
      int comp_res_col = comp_size / rows;
      // unpack for next layer.
      vector<mpc_t> next_level_data;
      for ( int j = 0; j < rows; ++j) {
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
      // debug 
		  // if(PRIMARY) {
      //     	GetMpcOpInner(Reconstruct2PC)->Run(level_data, level_data.size(), "curr level value");
    	// }
    }
    max = level_data;
      if (need_index) {
        log_error << "NOT supported yet!" << endl;
    }
  }

  if (FOUR_PC) {
    // I have removed 4PC, by yyl, 2020.03.12
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

// Chunk wise minimum of a vector of size rows*columns and minimum is calculated of every
// column number of elements. max is a vector of size rows.
// PARTY_A, PARTY_B start with the shares in a and {A,B} and {C,D} have the results in `min`.
// ToDo(GeorgeShi): If need_index is set to true, minIndex contains the index of the minimum value.
int Min::funcMinMPC(
  const vector<mpc_t>& a,
  vector<mpc_t>& min,
  vector<mpc_t>& minIndex,
  size_t rows,
  size_t columns,
  bool need_index) {
  if (THREE_PC) {
    // if(PRIMARY) {
    //   GetMpcOpInner(Reconstruct2PC)->Run(a, a.size(), "input value");
    // }
    // tree-structure to reduce iteration to call inner OPs.
    int depth = ceil(log2(columns));
    int curr_L = columns;
    vector<mpc_t> level_data = a;
    vector<mpc_t> first_v;
    vector<mpc_t> second_v;
    vector<mpc_t> diff_v;
    vector<mpc_t> comp_res;
    bool need_pad = false;
    for (size_t i = 0; i < depth; ++i) {
      int next_L = ceil(curr_L / 2.0);
      vector<mpc_t> new_data;
      first_v.clear();
      second_v.clear();
      diff_v.clear();
      comp_res.clear();
      // If the numbers in a row is odd, there is no need to comapre the last element. 
      if (curr_L % 2 == 1) {
        need_pad = true;
      }

      // pack for batch Relu.
      for (int j = 0; j < rows; ++j) {
        for (int k = 0; k < curr_L - 1; k = k + 2) {
          first_v.push_back(level_data[j*curr_L + k]);
          second_v.push_back(level_data[j*curr_L + k + 1]);
          diff_v.push_back(first_v.back() - second_v.back());
        }
      }

      int comp_size = diff_v.size();
      comp_res.resize(comp_size);
      new_data.resize(comp_size);
      GetMpcOpInner(ReluPrime)->Run3PC(diff_v, comp_res, comp_size);
      // this is the ONLY difference with Max::funcMaxMPC, so we may clean the code in future.
			GetMpcOpInner(Select1Of2)->Run(second_v, first_v, comp_res, new_data, comp_size);
      
      // unpack for next layer.
      int comp_res_col = comp_size / rows;
      vector<mpc_t> next_level_data;
      for ( int j = 0; j < rows; ++j) {
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
      if (need_index) {
        log_error << "NOT supported yet!" << endl;
    }
  }

  if (FOUR_PC) {
    // I have removed 4PC, by yyl, 2020.03.12
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