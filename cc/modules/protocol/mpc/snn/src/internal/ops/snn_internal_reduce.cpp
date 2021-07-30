#include "cc/modules/protocol/mpc/snn/include/snn_internal.h"
#include "cc/modules/common/include/utils/str_type_convert.h"
#include <thread>

using std::thread;

namespace rosetta {
namespace snn {

// Chunk wise maximum of a vector of size rows*columns and maximum is calculated of every
// column number of elements. max is a vector of size rows.
// PARTY_A, PARTY_B start with the shares in a and {A,B} and {C,D} have the results in `max`.
// ToDo(GeorgeShi): If need_index is set to true, maxIndex contains the index of the maximum value.
int SnnInternal::Max(
  const vector<mpc_t>& a,
  vector<mpc_t>& max,
  vector<mpc_t>& maxIndex,
  size_t rows,
  size_t columns) {
  tlog_debug << "Max ...";
  if (THREE_PC) {
    AUDIT("id:{}, P{} MaxIndex({},{}), input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, columns, Vector<mpc_t>(a));
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
      // if(PRIMARY) {
      //     	GetMpcOpInner(Reconstruct2PC)->Run(level_data, level_data.size(), "curr level input value");
      // }
      // pack for batch Relu.
      for (int j = 0; j < rows; ++j) {
        for (int k = 0; k < curr_L - 1; k = k + 2) {
          first_v.push_back(level_data[j * curr_L + k]);
          second_v.push_back(level_data[j * curr_L + k + 1]);
          diff_v.push_back(first_v.back() - second_v.back());
        }
      }

      int comp_size = diff_v.size();
      comp_res.resize(comp_size);
      new_data.resize(comp_size);
      ReluPrime(diff_v, comp_res);
      Select1Of2(first_v, second_v, comp_res, new_data);
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
      for (int j = 0; j < rows; ++j) {
        for (int k = 0; k <= next_L - 1; ++k) {
          if (k == next_L - 1) {
            // reserve the last element if needed.
            if (need_pad && (next_L != 1)) {
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
    // if (need_index) {
    //   tlog_error << "NOT supported yet!" << endl;
    // }
    AUDIT("id:{}, P{} MaxIndex({},{}), output max(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, columns, Vector<mpc_t>(max));
    AUDIT("id:{}, P{} MaxIndex({},{}), output maxIndex(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, columns, Vector<mpc_t>(maxIndex));
  }

  tlog_debug << "Max ok.";
  return 0;
}

// Chunk wise maximum of a vector of size rows*columns and maximum is calculated of every
// column number of elements. max is a vector of size rows.
// PARTY_A, PARTY_B start with the shares in a and {A,B} and {C,D} have the results in `max`.
// ToDo(GeorgeShi): If need_index is set to true, maxIndex contains the index of the maximum value.
int SnnInternal::Max(
  const vector<mpc_t>& a,
  vector<mpc_t>& max,
  size_t rows,
  size_t columns) {
  tlog_debug << "Max without index ...";
  if (THREE_PC) {
    AUDIT("id:{}, P{} Max({},{}), input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, columns, Vector<mpc_t>(a));
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
      // if(PRIMARY) {
      //     	GetMpcOpInner(Reconstruct2PC)->Run(level_data, level_data.size(), "curr level input value");
      // }
      // pack for batch Relu.
      for (int j = 0; j < rows; ++j) {
        for (int k = 0; k < curr_L - 1; k = k + 2) {
          first_v.push_back(level_data[j * curr_L + k]);
          second_v.push_back(level_data[j * curr_L + k + 1]);
          diff_v.push_back(first_v.back() - second_v.back());
        }
      }

      int comp_size = diff_v.size();
      comp_res.resize(comp_size);
      new_data.resize(comp_size);
      ReluPrime(diff_v, comp_res);
      Select1Of2(first_v, second_v, comp_res, new_data);
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
      for (int j = 0; j < rows; ++j) {
        for (int k = 0; k <= next_L - 1; ++k) {
          if (k == next_L - 1) {
            // reserve the last element if needed.
            if (need_pad && (next_L != 1)) {
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
    AUDIT("id:{}, P{} Max({},{}), output(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, columns, Vector<mpc_t>(max));
  }

  tlog_debug << "Max without index ok.";
  return 0;
}

// MaxIndex is of size rows. a is of size rows*columns.
// a will be set to 0's except at maxIndex (in every set of column)
int SnnInternal::MaxIndex(
  vector<mpc_t>& a,
  const vector<mpc_t>& maxIndex,
  size_t rows,
  size_t columns) {
  INFO("funcMaxIndexMPC");
  assert(((columns & (columns - 1)) == 0) && "funcMaxIndexMPC works only for power of 2 columns");
  assert((columns < 257) && "This implementation does not support larger than 257 columns");

  tlog_debug << "MaxIndex ...";
  AUDIT("id:{}, P{} MaxIndex({},{}), input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, columns, Vector<mpc_t>(a));

  vector<small_mpc_t> random(rows);

  if (PRIMARY) {
    vector<small_mpc_t> toSend(rows);
    for (size_t i = 0; i < rows; ++i) {
      toSend[i] = (small_mpc_t)maxIndex[i] % columns;
    }

    populateRandomVector<small_mpc_t>(random, rows, "COMMON", "POSITIVE");
    AUDIT("id:{}, P{} MaxIndex({},{}) populateRandomVector random(small_mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, columns, Vector<small_mpc_t>(random));
    if (partyNum == PARTY_A) {
      addVectors<small_mpc_t>(toSend, random, toSend, rows);
    }

    sendVector<small_mpc_t>(toSend, PARTY_C, rows);
    AUDIT("id:{}, P{} MaxIndex({},{}) SEND to P{}, toSend(small_mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, columns, PARTY_C, Vector<small_mpc_t>(toSend));
  }

  if (partyNum == PARTY_C) {
    vector<small_mpc_t> index(rows), temp(rows);
    vector<mpc_t> vector(rows * columns, 0), share_1(rows * columns), share_2(rows * columns);
    receiveVector<small_mpc_t>(index, PARTY_A, rows);
    AUDIT("id:{}, P{} MaxIndex({},{}) RECV from P{}, index(small_mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, columns, PARTY_A, Vector<small_mpc_t>(index));
    receiveVector<small_mpc_t>(temp, PARTY_B, rows);
    AUDIT("id:{}, P{} MaxIndex({},{}) RECV from P{}, temp(small_mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, columns, PARTY_A, Vector<small_mpc_t>(temp));
    addVectors<small_mpc_t>(index, temp, index, rows);
    AUDIT("id:{}, P{} MaxIndex({},{}) compute: index=index+temp, index(=index+temp)(small_mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, columns, Vector<small_mpc_t>(index));

    for (size_t i = 0; i < rows; ++i)
      index[i] = index[i] % columns;

    for (size_t i = 0; i < rows; ++i)
      vector[i * columns + index[i]] = 1;

    splitIntoShares(vector, share_1, share_2, rows * columns);
    sendVector<mpc_t>(share_1, PARTY_A, rows * columns);
    AUDIT("id:{}, P{} MaxIndex({},{}) SEND to P{}, share_1{}", msg_id().get_hex(), context_->GetMyRole(), rows, columns, PARTY_A, Vector<mpc_t>(share_1));

    sendVector<mpc_t>(share_2, PARTY_B, rows * columns);
    AUDIT("id:{}, P{} MaxIndex({},{}) SEND to P{}, share_2{}", msg_id().get_hex(), context_->GetMyRole(), rows, columns, PARTY_B, Vector<mpc_t>(share_2));
  }

  if (PRIMARY) {
    receiveVector<mpc_t>(a, PARTY_C, rows * columns);
    AUDIT("id:{}, P{} MaxIndex({},{}) RECV from P{}, a{}", msg_id().get_hex(), context_->GetMyRole(), rows, columns, PARTY_C, Vector<mpc_t>(a));

    size_t offset = 0;
    for (size_t i = 0; i < rows; ++i) {
      rotate(
        a.begin() + offset, a.begin() + offset + (random[i] % columns),
        a.begin() + offset + columns);
      offset += columns;
    }
  }

  AUDIT("id:{}, P{} MaxIndex({},{}), output(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, columns, Vector<mpc_t>(maxIndex));
  tlog_debug << "MaxIndex ok.";
  return 0;
}

// Chunk wise minimum of a vector of size rows*columns and minimum is calculated of every
// column number of elements. max is a vector of size rows.
// PARTY_A, PARTY_B start with the shares in a and {A,B} and {C,D} have the results in `min`.
// ToDo(GeorgeShi): If need_index is set to true, minIndex contains the index of the minimum value.
int SnnInternal::Min(
  const vector<mpc_t>& a,
  vector<mpc_t>& min,
  vector<mpc_t>& minIndex,
  size_t rows,
  size_t columns) {
  tlog_debug << "Min ...";

  if (THREE_PC) {
    AUDIT("id:{}, P{} MinIndex({},{}), input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, columns, Vector<mpc_t>(a));
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
          first_v.push_back(level_data[j * curr_L + k]);
          second_v.push_back(level_data[j * curr_L + k + 1]);
          diff_v.push_back(first_v.back() - second_v.back());
        }
      }

      int comp_size = diff_v.size();
      comp_res.resize(comp_size);
      new_data.resize(comp_size);
      ReluPrime(diff_v, comp_res);
      // this is the ONLY difference with Max::funcMaxMPC, so we may clean the code in future.
      Select1Of2(second_v, first_v, comp_res, new_data);

      // unpack for next layer.
      int comp_res_col = comp_size / rows;
      vector<mpc_t> next_level_data;
      for (int j = 0; j < rows; ++j) {
        for (int k = 0; k <= next_L - 1; ++k) {
          if (k == next_L - 1) {
            // reserve the last element if needed.
            if (need_pad && (next_L != 1)) {
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
    // if (need_index) {
    //   tlog_error << "NOT supported yet!" << endl;
    // }
    AUDIT("id:{}, P{} MinIndex({},{}), output min(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, columns, Vector<mpc_t>(min));
    AUDIT("id:{}, P{} MinIndex({},{}), output minIndex(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, columns, Vector<mpc_t>(minIndex));
  }

  tlog_debug << "Min ok.";
  return 0;
}

// Chunk wise minimum of a vector of size rows*columns and minimum is calculated of every
// column number of elements. max is a vector of size rows.
// PARTY_A, PARTY_B start with the shares in a and {A,B} and {C,D} have the results in `min`.
// ToDo(GeorgeShi): If need_index is set to true, minIndex contains the index of the minimum value.
int SnnInternal::Min(
  const vector<mpc_t>& a,
  vector<mpc_t>& min,
  size_t rows,
  size_t columns) {
  tlog_debug << "Min without index...";
  if (THREE_PC) {
    AUDIT("id:{}, P{} Min({},{}), input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, columns, Vector<mpc_t>(a));
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
          first_v.push_back(level_data[j * curr_L + k]);
          second_v.push_back(level_data[j * curr_L + k + 1]);
          diff_v.push_back(first_v.back() - second_v.back());
        }
      }

      int comp_size = diff_v.size();
      comp_res.resize(comp_size);
      new_data.resize(comp_size);
      ReluPrime(diff_v, comp_res);
      // this is the ONLY difference with Max::funcMaxMPC, so we may clean the code in future.
      Select1Of2(second_v, first_v, comp_res, new_data);

      // unpack for next layer.
      int comp_res_col = comp_size / rows;
      vector<mpc_t> next_level_data;
      for (int j = 0; j < rows; ++j) {
        for (int k = 0; k <= next_L - 1; ++k) {
          if (k == next_L - 1) {
            // reserve the last element if needed.
            if (need_pad && (next_L != 1)) {
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
    AUDIT("id:{}, P{} Min({},{}), output(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, columns, Vector<mpc_t>(min));
  }

  tlog_debug << "Min ok...";
  return 0;
}

// MinIndex is of size rows. a is of size rows*columns.
// a will be set to 0's except at MinIndex (in every set of column)
int SnnInternal::MinIndex(
  vector<mpc_t>& a,
  const vector<mpc_t>& minIndex,
  size_t rows,
  size_t columns) {
  INFO("funcMinIndexMPC not implements, [WARNING] !!!!!\n");
  throw std::runtime_error("funcMinIndexMPC not implements, [WARNING] !!!!!");
  return 0;
}

int SnnInternal::Mean(const vector<mpc_t>& a, vector<mpc_t>& b, size_t rows, size_t cols) {
  tlog_debug << "Mean ...";
  if (PRIMARY) {
    const int float_precision = GetMpcContext()->FLOAT_PRECISION;
    AUDIT("id:{}, P{} Mean({},{}), input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, cols, Vector<mpc_t>(a));
    for (int i = 0; i < rows; i++) {
      mpc_t v = 0;
      for (int j = 0; j < cols; j++) {
        int idx = i * cols + j;
        v += a[idx];
      }
      b[i] = v * FloatToMpcType(1.0 / cols, float_precision);
    }
    Truncate(b, float_precision, rows, PARTY_A, PARTY_B, partyNum);
    AUDIT("id:{}, P{} Mean({},{}), output(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, cols, Vector<mpc_t>(b));
  }

  tlog_debug << "Mean ok.";
  return 0;
}

int SnnInternal::Sum(const vector<mpc_t>& a, vector<mpc_t>& b, size_t rows, size_t cols) {
  tlog_debug << "Sum ...";
  b.resize(rows);
  if (PRIMARY) {
    // each row accumulate
    AUDIT("id:{}, P{} Sum({},{}), input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, cols, Vector<mpc_t>(a));
    for (int i = 0; i < rows; i++) {
      mpc_t v = 0;
      for (int j = 0; j < cols; j++) {
        int idx = i * cols + j;
        v += a[idx];
      }
      b[i] = v;
    }
    AUDIT("id:{}, P{} Sum({},{}), output(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, cols, Vector<mpc_t>(b));
  }

  tlog_debug << "Sum ok.";
  return 0;
}

int SnnInternal::AddN(const vector<mpc_t>& a, vector<mpc_t>& b, size_t rows, size_t cols) {
  tlog_debug << "AddN ...";
  b.resize(cols, 0);
  if (PRIMARY) {
    // each column accumulate
    AUDIT("id:{}, P{} AddN({},{}), input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, cols, Vector<mpc_t>(a));
    for (int i = 0; i < cols; i++) {
      mpc_t v = 0;
      for (int j = 0; j < rows; j++) {
        v += a[j * cols + i];
      }
      b[i] = v;
    }
    AUDIT("id:{}, P{} AddN({},{}), output(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), rows, cols, Vector<mpc_t>(b));
  }

  tlog_debug << "AddN ok.";
  return 0;
}

}//snn
}//rosetta
