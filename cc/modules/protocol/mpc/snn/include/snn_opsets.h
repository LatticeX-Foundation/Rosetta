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

#include "cc/modules/protocol/mpc/comm/include/mpc_helper.h"
#include "cc/modules/protocol/mpc/snn/include/opsets_local.h"
#include "cc/modules/protocol/utility/include/util.h"
#include "cc/modules/common/include/utils/str_type_convert.h"

#include <cmath>
#include <vector>
#include <string>
#include <typeinfo>
#include <iomanip>
#include <map>
#include <unordered_map>
#include <mutex>
#include <sstream>
#include <atomic>
using namespace std;

#if defined(__GNUC__)
#include <cxxabi.h> // abi::__cxa_demangle
#define TYPENAME(typeid_name) abi::__cxa_demangle(typeid_name, nullptr, nullptr, nullptr)
#else
#define TYPENAME(typeid_name) typeid_name
#endif

// only for internal caller
#define GetSnnOpInner(opname) std::make_shared<rosetta::snn::opname>(msg_id(), io)
// the folowing macros, for outer caller
// todo: optimize
#define GetSnnOpDefault(opname) std::make_shared<rosetta::snn::opname>(#opname)
#define GetSnnOpWithKey(opname, key) std::make_shared<rosetta::snn::opname>(key, io)

// only for internal caller
#define GetMpcOpInner(opname) GetSnnOpInner(opname)
// the folowing macros, for outer caller
// todo: optimize
#define GetMpcOpDefault(opname) GetSnnOpDefault(opname)
#define GetMpcOpWithKey(opname, key) GetSnnOpWithKey(opname, key)

namespace {
string generate_callstack(vector<string>& vs) {
  int idepth = 0;
  stringstream sss;
  sss << endl;
  for (auto& s : vs) {
    if (s[0] == '+') {
      idepth++;
      for (int i = 0; i < idepth; i++) {
        sss << "  ";
      }
      string st("+rosetta::snn::");
      s = s.substr(st.length(), s.length() - st.length());
      sss << s << endl;
    } else {
      idepth--;
    }
  }
  return sss.str();
}
} // namespace

#if OPEN_MPCOP_DEBUG_AND_CHECK
// the definitions in opsets.cpp
struct op_debug_stat {
  atomic<int64_t> bytes_sent{0};
  atomic<int64_t> bytes_received{0};
  atomic<int64_t> message_sent{0};
  atomic<int64_t> message_received{0};

  int64_t counter = 0; // incoming add one, outcoming sub one
  vector<string> call_stack; // operations call stack.
};
extern mutex g_key_stat_mtx;
extern map<string, op_debug_stat*> g_key_stat;
#define MPCOP_DEBUG_AND_CHECK_BEG(key, name)           \
  do {                                                 \
    unique_lock<mutex> lck(g_key_stat_mtx);            \
    auto iter = g_key_stat.find(key);                  \
    if (iter == g_key_stat.end()) {                    \
      g_key_stat[key] = new op_debug_stat();           \
    }                                                  \
    g_key_stat[key]->counter++;                        \
    g_key_stat[key]->call_stack.push_back("+" + name); \
  } while (0)
#define MPCOP_DEBUG_AND_CHECK_END(key, name)                                                     \
  do {                                                                                           \
    unique_lock<mutex> lck(g_key_stat_mtx);                                                      \
    auto iter = g_key_stat.find(key);                                                            \
    g_key_stat[key]->counter--;                                                                  \
    g_key_stat[key]->call_stack.push_back("-" + name);                                           \
    if (g_key_stat[key]->counter == 0) {                                                         \
      string ss(generate_callstack(g_key_stat[key]->call_stack));                                \
      log_info << "RUNMPCOP communication:         SB,RB,SN,RN " << g_key_stat[key]->bytes_sent  \
               << "," << g_key_stat[key]->bytes_received << "," << g_key_stat[key]->message_sent \
               << "," << g_key_stat[key]->message_received << " IN " << name << " with " << key  \
               << endl;                                                                          \
      log_info << "call stack:" << ss << endl;                                                   \
      g_key_stat.erase(iter);                                                                    \
    }                                                                                            \
  } while (0)
#else
#define MPCOP_DEBUG_AND_CHECK_BEG(key, name) (void)0
#define MPCOP_DEBUG_AND_CHECK_END(key, name) (void)0
#endif

#define MPCOP_COMMUNICATION_BEG()                \
  int64_t bytes_sent_tmp = bytes_sent();         \
  int64_t bytes_received_tmp = bytes_received(); \
  int64_t message_sent_tmp = message_sent();     \
  int64_t message_received_tmp = message_received()
#define MPCOP_COMMUNICATION_END()                             \
  bytes_sent_tmp = bytes_sent() - bytes_sent_tmp;             \
  bytes_received_tmp = bytes_received() - bytes_received_tmp; \
  message_sent_tmp = message_sent() - message_sent_tmp;       \
  message_received_tmp = message_received() - message_received_tmp

// this macro for statisitic comminucations and time
#if OPEN_MPCOP_DEBUG_AND_CHECK
#define MPCOP_RETURN(fn)                                                                           \
  int ret = 1;                                                                                     \
  do {                                                                                             \
    string key(msg_id().str());                                                                    \
    string name(TYPENAME(typeid(*this).name()));                                                   \
    MPCOP_DEBUG_AND_CHECK_BEG(key, name);                                                          \
    MPCOP_COMMUNICATION_BEG();                                                                     \
    SimpleTimer timer;                                                                             \
    ret = fn;                                                                                      \
    auto elapse = timer.us_elapse();                                                               \
    MPCOP_COMMUNICATION_END();                                                                     \
    MPCOP_DEBUG_AND_CHECK_END(key, name);                                                          \
    log_info << "RUNMPCOP elapse(us): " << setw(10) << elapse << " SB,RB,SN,RN " << bytes_sent_tmp \
             << "," << bytes_received_tmp << "," << message_sent_tmp << ","                        \
             << message_received_tmp << " IN " << name << " " << __FUNCTION__ << " with " << key   \
             << endl;                                                                              \
  } while (0);                                                                                     \
  return ret
#else
#define MPCOP_RETURN(fn) return fn
#endif

// using namespace rosetta::snn;
namespace rosetta {
namespace snn {

class OpBase : public OpBase_ {
 public:
  explicit OpBase(const msg_id_t& key) : msg_id_(key) { init(); }
  explicit OpBase(const std::shared_ptr<OpBase>& op) : msg_id_(op->msg_id()) { init(); }

  explicit OpBase(const msg_id_t& key, shared_ptr<NET_IO> io__) : msg_id_(key) {
    io = io__;
    init();
  }
  explicit OpBase(const std::shared_ptr<OpBase>& op, shared_ptr<NET_IO> io__)
      : msg_id_(op->msg_id()) {
    io = io__;
    init();
  }

  virtual ~OpBase() = default;

 public:
  virtual const msg_id_t& msg_id() const { return msg_id_; }

 protected:
  const msg_id_t& msg_id_;
};

/*
todo:
1. generate aes keys
2. init message key
*/
class Empty : public OpBase {
  using OpBase::OpBase;
};

class Synchronize : public OpBase {
  using OpBase::OpBase;

 public:
  int Run() { MPCOP_RETURN(funcSynchronize()); }

 private:
  int funcSynchronize();
};

class SyncAesKey : public OpBase {
  using OpBase::OpBase;

 public:
  int Run(int partyA, int partyB, std::string& key_send, std::string& key_recv) {
    MPCOP_RETURN(funcSyncAesKey(partyA, partyB, key_send, key_recv));
  }

 private:
  // A's send to B
  int funcSyncAesKey(int partyA, int partyB, std::string& key_send, std::string& key_recv);
};

// all(3) parties generates a random seed with the comm aeskey
class RandomSeed : public OpBase {
  using OpBase::OpBase;

 public:
  int Run(mpc_t& seed) { MPCOP_RETURN(funcRandomSeed(seed)); }

 private:
  int funcRandomSeed(mpc_t& seed) {
    seed = random_seed();
    return 0;
  }
};

/**
 * Pseudo Random Zero Share
 */
class PRZS : public OpBase {
  using OpBase::OpBase;

 public:
  int Run(int party0, int party1, vector<mpc_t>& shares) {
    MPCOP_RETURN(funPRZS(party0, party1, shares));
  }
  int Run(int party0, int party1, vector<double>& shares) {
    MPCOP_RETURN(funPRZS(party0, party1, shares));
  }
  int Run(int party0, int party1, mpc_t& shares) { MPCOP_RETURN(funPRZS(party0, party1, shares)); }
  int Run(int party0, int party1, double& shares) { MPCOP_RETURN(funPRZS(party0, party1, shares)); }

 private:
  int funPRZS(int party0, int party1, vector<mpc_t>& shares);
  int funPRZS(int party0, int party1, vector<double>& shares) {
    vector<mpc_t> ss(shares.size());
    funPRZS(party0, party1, ss);
    convert_mpctype_to_double(ss, shares);
    return 0;
  }
  int funPRZS(int party0, int party1, mpc_t& shares) {
    vector<mpc_t> ss(1);
    funPRZS(party0, party1, ss);
    shares = ss[0];
    return 0;
  }
  int funPRZS(int party0, int party1, double& shares) {
    mpc_t ss;
    funPRZS(party0, party1, ss);
    shares = MpcTypeToFloat(ss);
    return 0;
  }
};

/**
 * private input
 * now, only supports P0 or P1
 * now, only supports double as input(s)
 */
class PrivateInput : public OpBase {
  using OpBase::OpBase;

 public:
  int Run(int party, const vector<double>& v, vector<mpc_t>& shares) {
    MPCOP_RETURN(funPrivateInput(party, v, shares));
  }
  int Run(int party, const vector<double>& v, vector<double>& shares) {
    MPCOP_RETURN(funPrivateInput(party, v, shares));
  }
  int Run(int party, double v, mpc_t& shares) { MPCOP_RETURN(funPrivateInput(party, v, shares)); }
  int Run(int party, double v, double& shares) { MPCOP_RETURN(funPrivateInput(party, v, shares)); }

 private:
  int funPrivateInput(int party, const vector<double>& v, vector<mpc_t>& shares);
  int funPrivateInput(int party, const vector<double>& v, vector<double>& shares) {
    vector<mpc_t> ss(shares.size());
    funPrivateInput(party, v, ss);
    convert_mpctype_to_double(ss, shares);
    return 0;
  }
  int funPrivateInput(int party, double v, mpc_t& shares) {
    vector<mpc_t> ss(1);
    vector<double> vv = {v};
    funPrivateInput(party, vv, ss);
    shares = ss[0];
    return 0;
  }
  int funPrivateInput(int party, double v, double& shares) {
    mpc_t ss;
    funPrivateInput(party, v, ss);
    shares = MpcTypeToFloat(ss);
    return 0;
  }
};

/**
 * broadcast public values to peers
 * now, only supports P0 or P1
 * now, only supports double as input(s)
 */
class Broadcast : public OpBase {
  using OpBase::OpBase;

 public:
  int Run(int party, const string& msg, string& result) {
    MPCOP_RETURN(funcBroadcast(party, msg, result));
  }
  int Run(int party, const char* msg, char* result, size_t size) {
    MPCOP_RETURN(funcBroadcast(party, msg, result, size));
  }

 private:
  int funcBroadcast(int party, const string& msg, string& result);
  int funcBroadcast(int party, const char* msg, char* result, size_t size);
};

/*
c = a (matmul) b
a.shape = (rows, common_dim)
b.shape = (common_dim, columns)
row first
eg:
|1, 2|     |5, 6|     |19, 22|
|    |  X  |    |  =  |      |
|3, 4|     |7, 8|     |43, 50|
a = [1,2,3,4]
b = [5,6,7,8]
c = [19,22,43,50]
*/
class MatMul : public OpBase {
  using OpBase::OpBase;

 public:
  int Run(
    const vector<mpc_t>& a,
    const vector<mpc_t>& b,
    vector<mpc_t>& c,
    size_t rows,
    size_t common_dim,
    size_t columns,
    size_t transpose_a,
    size_t transpose_b) {
    MPCOP_RETURN(funcMatMulMPC(a, b, c, rows, common_dim, columns, transpose_a, transpose_b));
  }

  int Run(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& c,
    size_t rows,
    size_t common_dim,
    size_t columns,
    size_t transpose_a,
    size_t transpose_b) {
    MPCOP_RETURN(funcMatMulMPC(a, b, c, rows, common_dim, columns, transpose_a, transpose_b));
  }

 private:
  int funcMatMulMPC(
    const vector<mpc_t>& a,
    const vector<mpc_t>& b,
    vector<mpc_t>& c,
    size_t rows,
    size_t common_dim,
    size_t columns,
    size_t transpose_a,
    size_t transpose_b);
  int funcMatMulMPC(
    const vector<string>& as,
    const vector<string>& bs,
    vector<string>& cs,
    size_t rows,
    size_t common_dim,
    size_t columns,
    size_t transpose_a,
    size_t transpose_b) {
    vector<mpc_t> a, b, c;
    rosetta::convert::from_binary_str(as, a);
    rosetta::convert::from_binary_str(bs, b);

    MPCOP_RETURN(funcMatMulMPC(a, b, c, rows, common_dim, columns, transpose_a, transpose_b));

    rosetta::convert::to_binary_str<mpc_t>(c, cs);
    return 0;
  }
};
class Rsqrt : public OpBase{
  using OpBase::OpBase;

 public :
  int Run(const vector<mpc_t>& a,  vector<mpc_t>& b,size_t size){
    MPCOP_RETURN(funcRsqrt(a,b,size));
  }
  int Run(const vector<string>& a, vector<string>& b,size_t size){
    MPCOP_RETURN(funcRsqrt(a,b,size));
  }
  private:
    int funcRsqrt(const vector<mpc_t>& a,  vector<mpc_t>& b, size_t size);
    int funcRsqrt(const vector<string>& as,  vector<string>& bs,size_t size){
      vector<mpc_t>  a , b ;
      rosetta::convert::from_binary_str(as,a);
      MPCOP_RETURN(funcRsqrt(a,b,size));
      rosetta::convert::to_binary_str(b,bs);
      return 0;
    };
};
class Exp : public OpBase{
  using OpBase::OpBase;

 public :
  int Run(const vector<mpc_t>& a,  vector<mpc_t>& b,size_t size){
    MPCOP_RETURN(funcExp(a,b,size));
  }
  int Run(const vector<string>& a, vector<string>& b,size_t size){
    MPCOP_RETURN(funcExp(a,b,size));
  }
  private:
    int funcExp(const vector<mpc_t>& a,  vector<mpc_t>& b, size_t size);
    int funcExp(const vector<string>& as,  vector<string>& bs,size_t size){
      vector<mpc_t>  a , b ;
      rosetta::convert::from_binary_str(as,a);
      MPCOP_RETURN(funcExp(a,b,size));
      rosetta::convert::to_binary_str(b,bs);
      return 0;
    };
};

class Sqrt : public OpBase{
  using OpBase::OpBase;

 public :
  int Run(const vector<mpc_t>& a,  vector<mpc_t>& b,size_t size){
    MPCOP_RETURN(funcSqrt(a,b,size));
  }
  int Run(const vector<string>& a, vector<string>& b,size_t size){
    MPCOP_RETURN(funcSqrt(a,b,size));
  }
  private:
    int funcSqrt(const vector<mpc_t>& a,  vector<mpc_t>& b, size_t size);
    int funcSqrt(const vector<string>& as,  vector<string>& bs,size_t size){
      vector<mpc_t>  a , b ;
      rosetta::convert::from_binary_str(as,a);
      MPCOP_RETURN(funcSqrt(a,b,size));
      rosetta::convert::to_binary_str(b,bs);
      return 0;
    };
};

class Negative : public OpBase {
  using OpBase::OpBase;

 public:
  int Run(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size) {
    b.resize(a.size());
    MPCOP_RETURN(funcNegativeMPC(a, b, size));
  }

 private:
  int funcNegativeMPC(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size) {
    if (PRIMARY) {
      // just local multiply minus one
      for (auto i = 0; i < a.size(); ++i)
        b[i] = a[i] * FloatToMpcType(-1);

      funcTruncate2PC(b, FLOAT_PRECISION_M, size, PARTY_A, PARTY_B);
    }

    return 0;
  }
};

/*
PARTY_A, PARTY_B hold shares in a, want shares of RELU in b
only supports 3PC
*/
class Relu : public OpBase {
  using OpBase::OpBase;

 public:
  int Run(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size) {
    MPCOP_RETURN(funcRELUMPC(a, b, size));
  }

  int Run(const vector<string>& a, vector<string>& b, size_t size) {
    MPCOP_RETURN(funcRELUMPC(a, b, size));
  }

 private:
  int funcRELUMPC(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size);

  int funcRELUMPC(const vector<string>& as, vector<string>& bs, size_t size) {
    vector<mpc_t> a, b;
    rosetta::convert::from_binary_str(as, a);

    MPCOP_RETURN(funcRELUMPC(a, b, size));

    rosetta::convert::to_binary_str<mpc_t>(b, bs);
    return 0;
  }
};

class ReluPrime : public OpBase {
  using OpBase::OpBase;

 public:
  int Run3PC(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size) {
    MPCOP_RETURN(funcRELUPrime3PC(a, b, size));
  }
  int Run(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size) { return Run3PC(a, b, size); }

 private:
  int funcRELUPrime3PC(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size);
};

class Min : public OpBase {
  using OpBase::OpBase;

 public:
  int Run(const vector<mpc_t>& a, vector<mpc_t>& minv, size_t rows, size_t columns) {
    vector<mpc_t> minIndex(rows);
    minv.resize(rows);
    MPCOP_RETURN(funcMinMPC(a, minv, minIndex, rows, columns, false));
  }

 private:
  int funcMinMPC(
    const vector<mpc_t>& a,
    vector<mpc_t>& minv,
    vector<mpc_t>& minIndex,
    size_t rows,
    size_t columns,
    bool need_index = true);
};

class MinIndex : public OpBase {
  using OpBase::OpBase;

 public:
  int Run(vector<mpc_t>& a, const vector<mpc_t>& minIndex, size_t rows, size_t cols) {
    MPCOP_RETURN(funcMinIndexMPC(a, minIndex, rows, cols));
  }

 private:
  int funcMinIndexMPC(vector<mpc_t>& a, const vector<mpc_t>& minIndex, size_t rows, size_t cols);
};

class Max : public OpBase {
  using OpBase::OpBase;

 public:
  int Run(const vector<mpc_t>& a, vector<mpc_t>& maxv, size_t rows, size_t cols) {
    vector<mpc_t> maxIndex;
    maxv.resize(rows);
    // no need to get maxIndex, 'maxIndex' is just a adapted placeholder.
    MPCOP_RETURN(funcMaxMPC(a, maxv, maxIndex, rows, cols, false));
  }


  int Run(
    const vector<mpc_t>& a,
    vector<mpc_t>& maxv,
    vector<mpc_t>& maxIndex,
    size_t rows,
    size_t cols) {
    maxIndex.resize(rows);
    maxv.resize(rows);
    MPCOP_RETURN(funcMaxMPC(a, maxv, maxIndex, rows, cols));
  }

 private:
  int funcMaxMPC(
    const vector<mpc_t>& a,
    vector<mpc_t>& maxv,
    vector<mpc_t>& maxIndex,
    size_t rows,
    size_t cols,
    bool need_index = true);
};
class MaxIndex : public OpBase {
  using OpBase::OpBase;

 public:
  int Run(vector<mpc_t>& a, const vector<mpc_t>& maxIndex, size_t rows, size_t cols) {
    MPCOP_RETURN(funcMaxIndexMPC(a, maxIndex, rows, cols));
  }

 private:
  int funcMaxIndexMPC(vector<mpc_t>& a, const vector<mpc_t>& maxIndex, size_t rows, size_t cols);
};

/*
only support 1-d 2-d
rows: for 1-d, rows == 1
cols: 
row first
*/
class Mean : public OpBase {
  using OpBase::OpBase;

 public:
  int Run(const vector<mpc_t>& a, vector<mpc_t>& b, size_t rows, size_t cols) {
    b.resize(rows);
    MPCOP_RETURN(funcMean(a, b, rows, cols));
  }

 private:
  int funcMean(const vector<mpc_t>& a, vector<mpc_t>& b, size_t rows, size_t cols);
};

/*
only support 1-d 2-d
rows: for 1-d, rows == 1
cols: 
row first
*/
class Sum : public OpBase {
  using OpBase::OpBase;

 public:
  int Run(const vector<mpc_t>& a, vector<mpc_t>& b, size_t rows, size_t cols) {
    b.resize(rows);
    MPCOP_RETURN(funcSum(a, b, rows, cols));
  }

 private:
  int funcSum(const vector<mpc_t>& a, vector<mpc_t>& b, size_t rows, size_t cols);
};

/*
only support 1-d 2-d
rows: for 1-d, rows == 1
cols: 
row first
*/
class AddN : public OpBase {
  using OpBase::OpBase;

 public:
  int Run(const vector<mpc_t>& a, vector<mpc_t>& b, size_t rows, size_t cols) {
    b.resize(cols);
    MPCOP_RETURN(funcAddN(a, b, rows, cols));
  }

 private:
  int funcAddN(const vector<mpc_t>& a, vector<mpc_t>& b, size_t rows, size_t cols);
};

class Reconstruct2PC : public OpBase {
  using OpBase::OpBase;

 public:
  int Run(const vector<mpc_t>& a, size_t size, string str) {
    MPCOP_RETURN(funcReconstruct2PC(a, size, str));
  }
  int Run(const vector<mpc_t>& a, size_t size, vector<mpc_t>& out, int recv_party = PARTY_A) {
    MPCOP_RETURN(funcReconstruct2PC(a, size, out, recv_party));
  }

  int Run(const vector<mpc_t>& a, vector<mpc_t>& out, int recv_party = PARTY_A) {
    MPCOP_RETURN(funcReconstruct2PC(a, a.size(), out, recv_party));
  }

  int RunEx(const vector<mpc_t>& a, vector<mpc_t>& out, int recv_party = PARTY_A) {
    MPCOP_RETURN(funcReconstruct2PC_ex(a, a.size(), out, recv_party));
  }

  int Run(const mpc_t& a, mpc_t& out, int recv_party = PARTY_A) {
    MPCOP_RETURN(funcReconstruct2PC(a, out, recv_party));
  }

  int RunV2(const vector<mpc_t>& a, size_t size, vector<mpc_t>& out, int recv_party = PARTY_A) {
    MPCOP_RETURN(reconstruct_general(a, size, out, recv_party));
  }

 private:
  int funcReconstruct2PC(const mpc_t& a, mpc_t& out, int recv_party) {
    vector<mpc_t> va = {a}, vo(1);
    funcReconstruct2PC(va, 1, vo, recv_party);
    out = vo[0];
    return 0;
  }
  int funcReconstruct2PC(const vector<mpc_t>& a, size_t size, string str);
  int funcReconstruct2PC(const vector<mpc_t>& a, size_t size, vector<mpc_t>& out, int recv_party);
  int funcReconstruct2PC_ex(
    const vector<mpc_t>& a,
    size_t size,
    vector<mpc_t>& out,
    int recv_party);

  /**
   * @brief: the shared_v will be 'revealed' to plaintext_v held by rev_party 
   **/
  int reconstruct_general(
    const vector<mpc_t>& shared_v,
    size_t size,
    vector<mpc_t>& plaintext_v,
    int recv_party);
};

class ReconstructBit2PC : public OpBase {
  using OpBase::OpBase;

 public:
  int Run(const vector<small_mpc_t>& a, size_t size, string str) {
    MPCOP_RETURN(funcReconstructBit2PC(a, size, str));
  }

 private:
  int funcReconstructBit2PC(const vector<small_mpc_t>& a, size_t size, string str);
};

/*
  @note: this Polynomial is mostly for internal usage, 
    especially for complex funtionalities, such as Log and Log1p, that 
    are implemented with polynomial interpolation.
  
  @attention: for now DO NOT use this directly if you are not sure.
*/
class Polynomial : public OpBase {
  using OpBase::OpBase;

 public:
  /*
	@brief: Secret-shared version for calculating X^k, of which
			k is common known
	@param:
		[in] shared_X, the variable;
		[in] common_k, power value.
		[out] shared_Y, the resulting value.
		[in] curr_cache, the auxiliary cache,
			 containing some computed k --> Y, to accelerate.
  */
  void mpc_pow_const(const mpc_t& shared_X, mpc_t common_k, mpc_t& shared_Y);

  void mpc_pow_const(const vector<mpc_t>& shared_X, mpc_t common_k, vector<mpc_t>& shared_Y);

  void local_const_mul(const vector<mpc_t>& shared_X, mpc_t common_V, vector<mpc_t>& shared_Y);

  /*
	  @brief: secret-shared version for computing a univariate polynomial:
	  Y = C0 * X^P0 + C1 * X^P1 + ... + Cn * X^Pn
	  @param:
	  	[in] shared_X, the variable.
	  	[in] common_power_list, the {Pi} in the polynomial in order.
	  	[in] common_coff_list, the {Ci} in the polynomial, which must
	  			1-to-1 associate with {Pi}.
	  	[out] shared_Y, the result.
      @Note:
	  	for efficiency, it will be better to sort the list in ascending order!
  */
  void mpc_uni_polynomial(
    const mpc_t& shared_X,
    const vector<mpc_t>& common_power_list,
    const vector<mpc_t>& common_coff_list,
    mpc_t& shared_Y);

  void mpc_uni_polynomial(
    const vector<mpc_t>& shared_X,
    const vector<mpc_t>& common_power_list,
    const vector<mpc_t>& common_coff_list,
    vector<mpc_t>& shared_Y);
};

class PrivateCompare : public OpBase {
  using OpBase::OpBase;

 public:
  int Run(
    const vector<small_mpc_t>& share_m,
    const vector<mpc_t>& r,
    const vector<small_mpc_t>& beta,
    vector<small_mpc_t>& betaPrime,
    size_t size,
    size_t dim) {
    MPCOP_RETURN(funcPrivateCompareMPC(share_m, r, beta, betaPrime, size, dim));
  }

 private:
  int funcPrivateCompareMPC(
    const vector<small_mpc_t>& share_m,
    const vector<mpc_t>& r,
    const vector<small_mpc_t>& beta,
    vector<small_mpc_t>& betaPrime,
    size_t size,
    size_t dim);
};

class ShareConvert : public OpBase {
  using OpBase::OpBase;

 public:
  int Run(vector<mpc_t>& a, size_t size) { MPCOP_RETURN(funcShareConvertMPC(a, size)); }

 private:
  int funcShareConvertMPC(vector<mpc_t>& a, size_t size);
};

class ComputeMSB : public OpBase {
  using OpBase::OpBase;

 public:
  int Run3PC(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size) {
    MPCOP_RETURN(funcComputeMSB3PC(a, b, size));
  }
  int Run(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size) { return Run3PC(a, b, size); }

 private:
  // only 3PC, not cope 4PC
  int funcComputeMSB3PC(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size);
};

class SelectShares : public OpBase {
  using OpBase::OpBase;

 public:
  int Run3PC(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size) {
    MPCOP_RETURN(funcSelectShares3PC(a, b, c, size));
  }
  int Run(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size) {
    return Run3PC(a, b, c, size);
  }

 private:
  // only 3PC, not cope 4PC
  int funcSelectShares3PC(
    const vector<mpc_t>& a,
    const vector<mpc_t>& b,
    vector<mpc_t>& c,
    size_t size);
};

class Sigmoid : public OpBase {
  using OpBase::OpBase;

 public:
  int RunG3(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size, bool use_fastPow3 = false) {
    MPCOP_RETURN(funcSigmoidG3MPC(a, b, size, use_fastPow3));
  }
  int RunG3Prime(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size) {
    MPCOP_RETURN(funcSigmoidG3PrimeMPC(a, b, size));
  }
  int RunPieceWise(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size) {
    MPCOP_RETURN(funcSigmoidPieceWiseMPC(a, b, size));
  }
  int Run3PieceWise(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size) {
    MPCOP_RETURN(funcSigmoid3PieceWiseMPC(a, b, size));
  }
  int RunAliPieceWise(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size) {
    MPCOP_RETURN(funcSigmoidAliPieceWiseMPC(a, b, size));
  }
  int RunChebyshevPoly(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size) {
    MPCOP_RETURN(funcSigmoidChebyshevPolyMPC(a, b, size));
  }

  int Run(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size) {
#define USE_SIGMOID_VERSION 2
#if USE_SIGMOID_VERSION == 0
    return RunChebyshevPoly(a, b, size);
#elif USE_SIGMOID_VERSION == 1
    return Run3PieceWise(a, b, size); // 3-segments
#elif USE_SIGMOID_VERSION == 2 // 6-segments
    return RunPieceWise(a, b, size);
#else
    return RunG3(a, b, size);
#endif
  }

 private:
  int funcSigmoidG3MPC(
    const vector<mpc_t>& a,
    vector<mpc_t>& b,
    size_t size,
    bool use_fastPow3 = true);
  int funcSigmoidG3PrimeMPC(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size);
  int funcSigmoidPieceWiseMPC(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size);
  int funcSigmoid3PieceWiseMPC(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size);
  int funcSigmoidAliPieceWiseMPC(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size);
  int funcSigmoidChebyshevPolyMPC(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size);

 private:
  int funcPrivateCompareMPCEx(
    const vector<mpc_t>& a,
    const vector<mpc_t>& r,
    vector<mpc_t>& b,
    size_t size);
  int funcPrivateCompareMPCEx2(
    const vector<mpc_t>& r,
    const vector<mpc_t>& a,
    vector<mpc_t>& b,
    size_t size);
  int funcCubeMPC(const vector<mpc_t>& a, vector<mpc_t>& cube, size_t size);
  int funcFastPow3MPC(const vector<mpc_t>& x, vector<mpc_t>& out, size_t size);
  int funcLinearMPC(const vector<mpc_t>& x, mpc_t a, mpc_t b, vector<mpc_t>& out, size_t size);
};

/*
element-wise
a: input0
b: input1
c: equal to a (op) b
size: a.size == b.size == c.size
(op): Add/Sub/DotProduct/Mul/Div/Truediv
*/
class BinaryOp : public OpBase {
  using OpBase::OpBase;

 public:
  virtual ~BinaryOp() = default;
  int Run(const mpc_t& a, const mpc_t& b, mpc_t& c) { MPCOP_RETURN(funcBinaryOp(a, b, c)); }
  int Run(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size) {
    c.resize(size);
    MPCOP_RETURN(funcBinaryOp(a, b, c, size));
  }
  // const OP, in P0, c = a (op) b; in P1, c = 0 (op) b
  int Run(const vector<double>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size) {
    c.resize(size);
    MPCOP_RETURN(funcBinaryOp(a, b, c, size));
  }
  // const OP, in P0, c = a (op) b; in P1, c = a (op) 0
  int Run(const vector<mpc_t>& a, const vector<double>& b, vector<mpc_t>& c, size_t size) {
    c.resize(size);
    MPCOP_RETURN(funcBinaryOp(a, b, c, size));
  }
  //[kelvin] NOTE:  string represents const, in P0, c = a (op) b; in P1, c = 0 (op) b
  int Run(const vector<string>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size) {
    c.resize(size);
    MPCOP_RETURN(funcBinaryOp(a, b, c, size));
  }
  //[kelvin] NOTE: string represents const, in P0, c = a (op) b; in P1, c = a (op) 0
  int Run(const vector<mpc_t>& a, const vector<string>& b, vector<mpc_t>& c, size_t size) {
    c.resize(size);
    MPCOP_RETURN(funcBinaryOp(a, b, c, size));
  }

 protected:
  virtual int funcBinaryOp(const mpc_t& a, const mpc_t& b, mpc_t& c) {
    vector<mpc_t> va = {a};
    vector<mpc_t> vb = {b};
    vector<mpc_t> vc = {c};
    funcBinaryOp(va, vb, vc, 1);
    c = vc[0];
    return 0;
  }
  virtual int funcBinaryOp(
    const vector<mpc_t>& a,
    const vector<mpc_t>& b,
    vector<mpc_t>& c,
    size_t size) = 0;
  virtual int funcBinaryOp(
    const vector<double>& a,
    const vector<mpc_t>& b,
    vector<mpc_t>& c,
    size_t size) {
    vector<mpc_t> va(a.size(), 0);
    if (partyNum == PARTY_A) {
      convert_double_to_mpctype(a, va);
    }
    return funcBinaryOp(va, b, c, size);
  }
  virtual int funcBinaryOp(
    const vector<mpc_t>& a,
    const vector<double>& b,
    vector<mpc_t>& c,
    size_t size) {
    vector<mpc_t> vb(b.size(), 0);
    if (partyNum == PARTY_A) {
      convert_double_to_mpctype(b, vb);
    }
    return funcBinaryOp(a, vb, c, size);
  }
  //[kelvin] NOTE: string is const
  virtual int funcBinaryOp(
    const vector<string>& a,
    const vector<mpc_t>& b,
    vector<mpc_t>& c,
    size_t size) {
    vector<mpc_t> va(a.size(), 0);
    if (partyNum == PARTY_A) {
      vector<double> da(a.size(), 0);
      rosetta::convert::from_double_str(a, da);
      convert_double_to_mpctype(da, va);
    }
    return funcBinaryOp(va, b, c, size);
  }
  //[kelvin] NOTE: string represents const
  virtual int funcBinaryOp(
    const vector<mpc_t>& a,
    const vector<string>& b,
    vector<mpc_t>& c,
    size_t size) {
    vector<mpc_t> vb(b.size(), 0);
    if (partyNum == PARTY_A) {
      vector<double> db(b.size());
      rosetta::convert::from_double_str(b, db);
      convert_double_to_mpctype(db, vb);
    }
    return funcBinaryOp(a, vb, c, size);
  }
};

class Add : public BinaryOp {
  using BinaryOp::BinaryOp;

  virtual int funcBinaryOp(
    const vector<mpc_t>& a,
    const vector<mpc_t>& b,
    vector<mpc_t>& c,
    size_t size) {
    c.resize(size);
    addVectors(a, b, c, size);
    return 0;
  }
};

class Sub : public BinaryOp {
  using BinaryOp::BinaryOp;

  virtual int funcBinaryOp(
    const vector<mpc_t>& a,
    const vector<mpc_t>& b,
    vector<mpc_t>& c,
    size_t size) {
    c.resize(size);
    subtractVectors(a, b, c, size);
    return 0;
  }
};

class DotProduct : public BinaryOp {
  using BinaryOp::BinaryOp;

  virtual int funcBinaryOp(
    const vector<mpc_t>& a,
    const vector<mpc_t>& b,
    vector<mpc_t>& c,
    size_t size) {
    c.resize(size);
    return funcDotProduct(a, b, c, size);
  }

  virtual int funcBinaryOp(
    const vector<double>& fa,
    const vector<mpc_t>& b,
    vector<mpc_t>& c,
    size_t size) {
    vector<mpc_t> a(fa.size(), 0);
    convert_double_to_mpctype(fa, a);
    c.resize(a.size());
    for (size_t i = 0; i < size; i++) {
      c[i] = a[i] * b[i];
    }
    if (PRIMARY)
      funcTruncate2PC(c, FLOAT_PRECISION_M, size, PARTY_A, PARTY_B);
    return 0;
  }
  //[kelvin] NOTE: string represents const
  virtual int funcBinaryOp(
    const vector<string>& sa,
    const vector<mpc_t>& b,
    vector<mpc_t>& c,
    size_t size) {
    vector<mpc_t> a(sa.size(), 0);
    c.resize(a.size());

    vector<double> da(sa.size());
    rosetta::convert::from_double_str(sa, da);
    convert_double_to_mpctype(da, a);

    for (size_t i = 0; i < size; i++) {
      c[i] = a[i] * b[i];
    }
    if (PRIMARY)
      funcTruncate2PC(c, FLOAT_PRECISION_M, size, PARTY_A, PARTY_B);
    return 0;
  }
  virtual int funcBinaryOp(
    const vector<mpc_t>& a,
    const vector<string>& b,
    vector<mpc_t>& c,
    size_t size) {
    return funcBinaryOp(b, a, c, size);
  }

 public:
  // this is actually the AND functionality for bit-share.
  int BitMul(
    const vector<small_mpc_t>& a,
    const vector<small_mpc_t>& b,
    vector<small_mpc_t>& c,
    size_t size);

 private:
  int funcDotProduct(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size);
};

class Mul : public DotProduct {
  using DotProduct::DotProduct;
};

class Square : public OpBase {
  using OpBase::OpBase;

 public:
  int Run(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size) {
    MPCOP_RETURN(funcSquareMPC(a, b, size));
  }

 private:
  int funcSquareMPC(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size);
};

class DivBase : public BinaryOp {
  using BinaryOp::BinaryOp;

 protected:
  virtual int funcBinaryOp(
    const vector<mpc_t>& a,
    const vector<mpc_t>& b,
    vector<mpc_t>& c,
    size_t size) = 0;

  virtual int funcBinaryOp(
    const vector<double>& fa,
    const vector<mpc_t>& b,
    vector<mpc_t>& c,
    size_t size) {
    c.resize(size);
    vector<mpc_t> a(fa.size(), 0);
    if (partyNum == PARTY_A) {
      convert_double_to_mpctype(fa, a);
    }
    funcBinaryOp(a, b, c, size);
    return 0;
  }
  //[kelvin] NOTE: string represents const
  virtual int funcBinaryOp(
    const vector<string>& sa,
    const vector<mpc_t>& b,
    vector<mpc_t>& c,
    size_t size) {
    c.resize(size);
    vector<double> da(sa.size());
    vector<mpc_t> a(sa.size(), 0);
    rosetta::convert::from_double_str(sa, da);
    if (partyNum == PARTY_A) {
      convert_double_to_mpctype(da, a);
    }
    funcBinaryOp(a, b, c, size);
    return 0;
  }

  virtual int funcBinaryOp(
    const vector<mpc_t>& a,
    const vector<double>& fb,
    vector<mpc_t>& c,
    size_t size) {
    // std::cout << "debug stub DIV SC" << endl;
    // locally compute c = a * (1/fb)
    vector<double> tb(fb.size(), 0);
    vector<size_t> power_list(size, FLOAT_PRECISION_M);
    for (size_t i = 0; i < size; i++) {
      tb[i] = 1.0 / fb[i];
      // This is for dealing with big constant denominator, part I:
      //  scale it up by left-shifting
      double abs_v = abs(fb[i]);
      if (abs_v > 1) {
        power_list[i] = ceil(log2(abs_v));
        tb[i] = tb[i] * (1 << power_list[i]);
        power_list[i] = power_list[i] + FLOAT_PRECISION_M;
      }
    }
    vector<mpc_t> b(tb.size(), 0);
    convert_double_to_mpctype(tb, b);
    c.resize(size);
    for (size_t i = 0; i < size; ++i) {
      c[i] = a[i] * b[i];
    }
    if (PRIMARY) {
      funcTruncate2PC_many(c, power_list, size, PARTY_A, PARTY_B);
    }
    return 0;
  }
  //[kelvin] NOTE: string represents const
  virtual int funcBinaryOp(
    const vector<mpc_t>& a,
    const vector<string>& sb,
    vector<mpc_t>& c,
    size_t size) {
    //std::cout << "debug stub DIV SC" << endl;
    // locally compute c = a * (1/b)
    // b to 1/b, div replace with mul
    vector<double> db(sb.size());
    rosetta::convert::from_double_str(sb, db);
    return funcBinaryOp(a, db, c, size);
  }
  //   for (size_t i = 0; i < size; i++) {
  //     db[i] = 1.0 / db[i];
  //   }
  //   vector<mpc_t> b(sb.size(), 0);
  //   convert_double_to_mpctype(db, b);

  //   c.resize(a.size());
  //   for (size_t i = 0; i < size; i++) {
  //     c[i] = a[i] * b[i];
  //   }
  //   if (PRIMARY)
  //     funcTruncate2PC(c, FLOAT_PRECISION_M, size, PARTY_A, PARTY_B);
  //   return 0;
  // }
};

class Div : public DivBase {
  using DivBase::DivBase;

  virtual int funcBinaryOp(
    const vector<mpc_t>& a,
    const vector<mpc_t>& b,
    vector<mpc_t>& c,
    size_t size) {
    return funcDiv(a, b, c, size);
  }

 private:
  int funcDiv(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size);
};

class Truediv : public DivBase {
  using DivBase::DivBase;

  virtual int funcBinaryOp(
    const vector<mpc_t>& a,
    const vector<mpc_t>& b,
    vector<mpc_t>& c,
    size_t size) {
    c.resize(size);
    return funcTruediv(a, b, c, size);
  }

 private:
  int funcTruediv(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size);
};

/*
y = Pow(x, n)
x is a variable, n is a const value
if n == 0, set all y are 1
if n == 1, set y as x
*/
class Pow : public OpBase {
  using OpBase::OpBase;

 public:
  int Run(const vector<mpc_t>& x, size_t n, vector<mpc_t>& y, size_t size) {
    y.resize(size);
    MPCOP_RETURN(funcPow(x, n, y, size));
  }
  int Run(const vector<mpc_t>& x, vector<int64_t> n, vector<mpc_t>& y, size_t size) {
    y.resize(size);
    MPCOP_RETURN(funcPow2(x, n, y, size));
  }
  //[kelvin] NOTE: string represents const
  int Run(const vector<mpc_t>& x, const string& n, vector<mpc_t>& y, size_t size) {
    y.resize(size);
    int64_t ni = std::stoll(n);
    MPCOP_RETURN(funcPow(x, ni, y, size));
  }
  //[kelvin] NOTE: string represents const
  int Run(const vector<mpc_t>& x, const vector<string>& n, vector<mpc_t>& y, size_t size) {
    y.resize(size);
    vector<int64_t> ni(size, 0);
    rosetta::convert::from_int_str(n, ni);
    MPCOP_RETURN(funcPow2(x, ni, y, size));
  }

 private:
  int funcPow(const vector<mpc_t>& x, size_t n, vector<mpc_t>& y, size_t size) {
    vector<int64_t> vn(size, (int64_t)n);
    return funcPow2(x, vn, y, size);
  }
  int funcPow2(const vector<mpc_t>& x, vector<int64_t> n, vector<mpc_t>& y, size_t size) {
    assert(x.size() == n.size());
    assert(x.size() == size);
    auto poly = GetSnnOpInner(Polynomial);

    // Added in V0.2.1 to support vectorization
    bool is_common_k = true;
    for (int i = 1; i < size; ++i) {
      if (n[i] != n[i - 1]) {
        is_common_k = false;
        break;
      }
    }

    if (is_common_k) {
      // cout << "DEBUG; common k" << endl;
      poly->mpc_pow_const(x, n[0], y);
      return 0;
    }

    for (auto i = 0; i < size; i++) {
      poly->mpc_pow_const(x[i], n[i], y[i]);
    }
    return 0;
  }
};

/*
  Log: y = log_e (x)
Log1p: y = log_e (1 + x)
*/
/**
	*******LOGARITHM [approximation with polynomials]***************************
 */
class Log : public OpBase {
  using OpBase::OpBase;

 public:
  int Run(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size) {
    b.resize(a.size());
    MPCOP_RETURN(funcLog(a, b, size));
  }

  int Run1p(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size) {
    b.resize(a.size());
    MPCOP_RETURN(funcLog1p(a, b, size));
  }

  int RunHd(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size) {
    b.resize(a.size());
    MPCOP_RETURN(mpc_log_hd(a, b, size));
  }

  /*
  	@brief: General high precision approximation(guarantee four decimal part)
  			for LOG in the domain
  */
  int mpc_log_hd(const vector<mpc_t>& shared_X, vector<mpc_t>& shared_Y, size_t vec_size);

  //private:
  int funcLog(const mpc_t& a, mpc_t& b) {
    mpc_log_v2(a, b);
    return 0;
  }
  int funcLog(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size) {
    //TODO: vectorization
    //cout << "vector-version Log!" << endl;
    mpc_log_v2(a, b);
    return 0;
  }
  int funcLog1p(const mpc_t& a, mpc_t& b) {
    mpc_t real_a = a;

    if (PRIMARY) {
      mpc_t local_one = FloatToMpcType(1.0);
      if (partyNum == PARTY_A) {
        real_a = a + local_one;
      }
    }
    mpc_log_v2(real_a, b);
    return 0;
  }
  int funcLog1p(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size) {
    vector<mpc_t> va(a.size(), 0);
    vector<mpc_t> a_plus_one(a.size(), 0);
    if (PRIMARY) {
      mpc_t local_one = FloatToMpcType(1.0);
      if (partyNum == PARTY_A) {
        vector<mpc_t> v_one(a.size(), local_one);
        va.swap(v_one);
      }
      addVectors(a, va, a_plus_one, size);
    }
    funcLog(a_plus_one, b, size);
    return 0;
  }
  /*
    obsolete!
	  @brief: secret-shared version of logarithm with one-segment polynomial
	  with domain x \in [0.3, 1.8)
  */
  void mpc_log_v1(const mpc_t& shared_X, mpc_t& shared_Y);

  /*
  	@brief: secret-shared version of logarithm with three-segment polynomial
  */
  void mpc_log_v2(const mpc_t& shared_X, mpc_t& shared_Y);

  void mpc_log_v2(const vector<mpc_t>& shared_X, vector<mpc_t>& shared_Y);
};

class CompareOp : public OpBase {
  using OpBase::OpBase;

 public:
  int Run(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size) {
    c.resize(a.size());
    MPCOP_RETURN(RunCompareOp(a, b, c, size));
  }
  int Run(const mpc_t& a, const mpc_t& b, mpc_t& c) { MPCOP_RETURN(RunCompareOp(a, b, c)); }

  // const OP, in P0, c = a (op) b; in P1, c = 0 (op) b
  int Run(const vector<double>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size) {
    c.resize(a.size());
    MPCOP_RETURN(RunCompareOp(a, b, c, size));
  }
  // const OP, in P0, c = a (op) b; in P1, c = a (op) 0
  int Run(const vector<mpc_t>& a, const vector<double>& b, vector<mpc_t>& c, size_t size) {
    c.resize(a.size());
    MPCOP_RETURN(RunCompareOp(a, b, c, size));
  }
  //[kelvin] NOTE:  string represents const, in P0, c = a (op) b; in P1, c = 0 (op) b
  int Run(const vector<string>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size) {
    c.resize(a.size());
    MPCOP_RETURN(RunCompareOp(a, b, c, size));
  }
  //[kelvin] NOTE: string represents const, in P0, c = a (op) b; in P1, c = a (op) 0
  int Run(const vector<mpc_t>& a, const vector<string>& b, vector<mpc_t>& c, size_t size) {
    c.resize(a.size());
    MPCOP_RETURN(RunCompareOp(a, b, c, size));
  }

 protected:
  virtual int RunCompareOp(const mpc_t& a, const mpc_t& b, mpc_t& c) {
    vector<mpc_t> va = {a};
    vector<mpc_t> vb = {b};
    vector<mpc_t> vc = {c};
    RunCompareOp(va, vb, vc, 1);
    c = vc[0];
    return 0;
  }
  virtual int RunCompareOp(
    const vector<mpc_t>& a,
    const vector<mpc_t>& b,
    vector<mpc_t>& c,
    size_t size) = 0;

  virtual int RunCompareOp(
    const vector<double>& a,
    const vector<mpc_t>& b,
    vector<mpc_t>& c,
    size_t size) {
    vector<mpc_t> va(a.size(), 0);
    if (partyNum == PARTY_A) {
      convert_double_to_mpctype(a, va);
    }
    return RunCompareOp(va, b, c, size);
  }
  virtual int RunCompareOp(
    const vector<mpc_t>& a,
    const vector<double>& b,
    vector<mpc_t>& c,
    size_t size) {
    vector<mpc_t> vb(b.size(), 0);
    if (partyNum == PARTY_A) {
      convert_double_to_mpctype(b, vb);
    }
    return RunCompareOp(a, vb, c, size);
  }
  //[kelvin] NOTE: string is const
  virtual int RunCompareOp(
    const vector<string>& a,
    const vector<mpc_t>& b,
    vector<mpc_t>& c,
    size_t size) {
    vector<mpc_t> sa(a.size(), 0);
    if (partyNum == PARTY_A) {
      vector<double> va(a.size());
      rosetta::convert::from_double_str(a, va);
      convert_double_to_mpctype(va, sa);
    }
    return RunCompareOp(sa, b, c, size);
  }
  //[kelvin] NOTE: string represents const
  virtual int RunCompareOp(
    const vector<mpc_t>& a,
    const vector<string>& b,
    vector<mpc_t>& c,
    size_t size) {
    vector<mpc_t> sb(b.size(), 0);
    if (partyNum == PARTY_A) {
      vector<double> vb(a.size());
      rosetta::convert::from_double_str(b, vb);
      convert_double_to_mpctype(vb, sb);
    }
    return RunCompareOp(a, sb, c, size);
  }
};

class Equal : public CompareOp {
  using CompareOp::CompareOp;

  virtual int RunCompareOp(
    const vector<mpc_t>& a,
    const vector<mpc_t>& b,
    vector<mpc_t>& c,
    size_t size) {
    return funcFastEqual(a, b, c, size);
  }

  // temp make these as public funcs
 public:
  // retired! NOT use this any more.
  int funcEqual(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size);
  // Optimized version, default one.
  int funcFastEqual(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size);

  // for each a_i in a, get a_i[0] AND a_i[1] AND a_i[2] ... AND a_i[n] as c[i]
  int FanInBitAdd(const vector<vector<small_mpc_t>>& a, vector<small_mpc_t>& c, size_t size);
  // convert binary bit-share to arithematic share of the same plain value.
  int B2A(const vector<small_mpc_t>& bit_shares, vector<mpc_t>& arith_shares, size_t size);
};

class NotEqual : public CompareOp {
  using CompareOp::CompareOp;

  virtual int RunCompareOp(
    const vector<mpc_t>& a,
    const vector<mpc_t>& b,
    vector<mpc_t>& c,
    size_t size) {
    c.resize(size);
    return funcFastNotEqual(a, b, c, size);
  }

 private:
  // retired! NOT use this any more.
  int funcNotEqual(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size);
  // Optimized version, default one.
  int funcFastNotEqual(
    const vector<mpc_t>& a,
    const vector<mpc_t>& b,
    vector<mpc_t>& c,
    size_t size);
};

class Less : public CompareOp {
  using CompareOp::CompareOp;

  virtual int RunCompareOp(
    const vector<mpc_t>& a,
    const vector<mpc_t>& b,
    vector<mpc_t>& c,
    size_t size) {
    c.resize(size);
    return funcLess(a, b, c, size);
  }

 private:
  int funcLess(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size);
};

class Greater : public CompareOp {
  using CompareOp::CompareOp;

  virtual int RunCompareOp(
    const vector<mpc_t>& a,
    const vector<mpc_t>& b,
    vector<mpc_t>& c,
    size_t size) {
    c.resize(size);
    return funcGreater(a, b, c, size);
  }

 private:
  int funcGreater(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size);
};

class GreaterEqual : public CompareOp {
  using CompareOp::CompareOp;

  virtual int RunCompareOp(
    const vector<mpc_t>& a,
    const vector<mpc_t>& b,
    vector<mpc_t>& c,
    size_t size) {
    c.resize(size);
    return funcGreaterEqual(a, b, c, size);
  }

 private:
  int funcGreaterEqual(
    const vector<mpc_t>& a,
    const vector<mpc_t>& b,
    vector<mpc_t>& c,
    size_t size);
};

class LessEqual : public CompareOp {
  using CompareOp::CompareOp;

  virtual int RunCompareOp(
    const vector<mpc_t>& a,
    const vector<mpc_t>& b,
    vector<mpc_t>& c,
    size_t size) {
    c.resize(size);
    return funcLessEqual(a, b, c, size);
  }

 private:
  int funcLessEqual(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size);
};

/*
special for Division
*/
// only support a < b
// deprecated
class Division : public OpBase {
  using OpBase::OpBase;

 public:
  int Run(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size) {
    c.resize(size);
    MPCOP_RETURN(funcDivisionMPC(a, b, c, size));
  }

 private:
  int funcDivisionMPC(
    const vector<mpc_t>& a,
    const vector<mpc_t>& b,
    vector<mpc_t>& quotient,
    size_t size);
};

/**
	@brief: secret-shared version of 'If' clause: 
		result = x * BOOL + y (1 - BOOL)
  @author: SJJ
*/
class Select1Of2 : public OpBase {
  using OpBase::OpBase;

 public:
  int Run(
    const vector<mpc_t> shared_x,
    const vector<mpc_t> shared_y,
    const vector<mpc_t> shared_bool,
    vector<mpc_t>& shared_result,
    size_t size) {
    shared_result.resize(size);
    MPCOP_RETURN(mpc_select_1_of_2(shared_x, shared_y, shared_bool, shared_result, size));
  }

 private:
  int mpc_select_1_of_2(
    const vector<mpc_t> shared_x,
    const vector<mpc_t> shared_y,
    const vector<mpc_t> shared_bool,
    vector<mpc_t>& shared_result,
    size_t size);
};

/**
  @brief: XOR two secret-shared bits:
  		X XOR Y = x + y -2XY
  @author: SJJ
*/
class XorBit : public OpBase {
  using OpBase::OpBase;

 public:
  int Run(
    const vector<mpc_t> shared_x,
    const vector<mpc_t> shared_y,
    vector<mpc_t>& shared_result,
    size_t size) {
    shared_result.resize(size);
    MPCOP_RETURN(mpc_xor_bit(shared_x, shared_y, shared_result, size));
  }

 private:
  int mpc_xor_bit(
    const vector<mpc_t> shared_x,
    const vector<mpc_t> shared_y,
    vector<mpc_t>& shared_result,
    size_t size);
};

/**
	@brief: This interface is for general division of two secret-shared inputs.
	@params:
		[in] shared_numerator_vec: a vector of independent numerators
		[in] shared_denominator_vec: a vector of independent denominators
		[in] common_vec_size[plaintext]: the size of the input vectors.
		[in] common_all_less[plaintext]:  
				bool value to indicate whether all numerators are less than
				its corresponding denominators.
				This is for efficiency in this special case.
		[out] shared_quotient_vec: the resulting vector of quotients
	@note:
		This function supports both x>=y and x<y; 
		and both positiveness negativeness.
		And if x is not divisable by y,
		the precision of the fractional part is FLOAT_PRECISION_M.
  @author: SJJ
*/
class DivisionV2 : public DivBase {
  using DivBase::DivBase;

 public:
  int funcBinaryOp(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size) {
    c.resize(size);
    return funcDivisionMPCV2(a, b, c, size);
  }

 private:
  /**
  * @param: 
  *   @common_vec_size: vector size which all party known
  *   @common_all_less: indicate whether all element (numerator < denominator) to speed up.      
  * 
  */
  int funcDivisionMPCV2(
    const vector<mpc_t>& shared_numerator_vec,
    const vector<mpc_t>& shared_denominator_vec,
    vector<mpc_t>& shared_quotient_vec,
    size_t common_vec_size,
    bool common_all_less = false);
};

/**
  for two integers, compute math.floor(x, y).
  eg: 6 / 4 = 1, (-6) / 4 = -2

  @author: SJJ
*/
class FloorDivision : public OpBase {
  using OpBase::OpBase;

 public:
  int Run(
    const vector<mpc_t>& a,
    const vector<mpc_t>& b,
    vector<mpc_t>& c,
    size_t size,
    bool all_positive = false) {
    c.resize(size);
    MPCOP_RETURN(mpc_floor_division(a, b, c, size, all_positive));
  }
  //[kelvin] NOTE: string represents const
  int Run(
    const vector<string>& a,
    const vector<mpc_t>& b,
    vector<mpc_t>& c,
    size_t size,
    bool all_positive = false) {
    c.resize(size);
    MPCOP_RETURN(mpc_floor_division(a, b, c, size, all_positive));
  }
  //[kelvin] NOTE: string represents const
  int Run(
    const vector<mpc_t>& a,
    const vector<string>& b,
    vector<mpc_t>& c,
    size_t size,
    bool all_positive = false) {
    c.resize(size);
    MPCOP_RETURN(mpc_floor_division(a, b, c, size, all_positive));
  }

 private:
  int mpc_floor_division(
    const vector<mpc_t>& shared_numerator_vec,
    const vector<mpc_t>& shared_denominator_vec,
    vector<mpc_t>& shared_quotient_vec,
    size_t common_vec_size,
    bool all_positive = false);

  //[kelvin] NOTE: string represents const
  int mpc_floor_division(
    const vector<string>& sa,
    const vector<mpc_t>& b,
    vector<mpc_t>& c,
    size_t size,
    bool all_positive = false) {
    // not like the true division, the local speedup may cause 0. So we use ordinary convention here.
    vector<double> da(sa.size());
    rosetta::convert::from_double_str(sa, da);
    // split it even in two parties so the sum (real value) is still the sa value.
    for (size_t i = 0; i < size; ++i) {
      da[i] = da[i] / 2;
    }
    vector<mpc_t> a(sa.size(), 0);
    convert_double_to_mpctype(da, a);
    mpc_floor_division(a, b, c, size, all_positive);
    return 0;
  }

  //[kelvin] NOTE: string represents const
  int mpc_floor_division(
    const vector<mpc_t>& a,
    const vector<string>& sb,
    vector<mpc_t>& c,
    size_t size,
    bool all_positive = false) {
    vector<double> db(sb.size());
    rosetta::convert::from_double_str(sb, db);
    vector<size_t> power_list(size, FLOAT_PRECISION_M);
    for (size_t i = 0; i < size; ++i) {
      // This is for dealing with big constant denominator, part I:
      //  scale it up by left-shifting
      double abs_v = abs(db[i]);
      db[i] = 1.0 / db[i];
      
      if(abs_v > 1) {
          power_list[i] = ceil(log2(abs_v));
          db[i] = db[i] * (1 << power_list[i]);
          power_list[i] = power_list[i] + FLOAT_PRECISION_M;
          // cout << "power_list: " << sb[i] << "->:" << db[i] << ", " << power_list[i] << endl;
      }
    }
    vector<mpc_t> b(sb.size(), 0);
    convert_double_to_mpctype(db, b);

    c.resize(size);
    for (size_t i = 0; i < size; ++i) {
      c[i] = a[i] * b[i];
    }
    if (PRIMARY) {
      funcTruncate2PC_many(c, power_list, size, PARTY_A, PARTY_B);
    }
    // set the float part as 0.
    if (PRIMARY) {
      funcTruncate2PC(c, FLOAT_PRECISION_M, size, PARTY_A, PARTY_B);
      for (size_t i = 0; i < size; ++i) {
        c[i] = static_cast<mpc_t>(static_cast<int64_t>(c[i]) << FLOAT_PRECISION_M);
      }
    }
    return 0;
  }
};

/**
  for positive x < y only!
  eg: 5 / 7 = 0.7143
  @author: SJJ
*/
class FracDivision : public OpBase {
  using OpBase::OpBase;

 public:
  int Run(
    const vector<mpc_t>& a,
    const vector<mpc_t>& b,
    vector<mpc_t>& c,
    size_t size,
    bool all_positive = false) {
    c.resize(size);
    MPCOP_RETURN(mpc_frac_division(a, b, c, size, all_positive));
  }

 private:
  int mpc_frac_division(
    const vector<mpc_t>& shared_numerator_vec,
    const vector<mpc_t>& shared_denominator_vec,
    vector<mpc_t>& shared_quotient_vec,
    size_t common_vec_size,
    bool all_positive = false);
};

/**
 * Logical Operations, all the inputs/outputs have scaled, 
 * the real value of all the inputs must be 1/0, or the result is unexpected!
 * 
 * @note we can derive from BinaryOp, but maybe some specials for logical ops in the future.
 */
class LogicalOp : public OpBase {
  using OpBase::OpBase;

 public:
  virtual ~LogicalOp() = default;
  int Run(const vector<mpc_t> a, const vector<mpc_t> b, vector<mpc_t>& c, size_t size) {
    MPCOP_RETURN(funcLogicalOp(a, b, c, size));
  }
  int Run(const vector<mpc_t> a, const vector<string> b, vector<mpc_t>& c, size_t size) {
    MPCOP_RETURN(funcLogicalOp(a, b, c, size));
  }
  int Run(const vector<string> a, const vector<mpc_t> b, vector<mpc_t>& c, size_t size) {
    MPCOP_RETURN(funcLogicalOp(a, b, c, size));
  }

  int Run(const vector<vector<mpc_t>> shared_x, vector<mpc_t>& shared_result) {
    MPCOP_RETURN(funcLogicalOp(shared_x, shared_result));
  }

 protected:
  /**
   * AND (x and y) = x*y
   * OR  (x or y)  = x + y - x*y
   * XOR (x xor y) = x + y - 2*x*y
   * NOT (not x)   = 1 - x
   * Z[i] = X[i] (OP) Y[i]
   */
  virtual int funcLogicalOp(
    const vector<mpc_t> X,
    const vector<mpc_t> Y,
    vector<mpc_t>& Z,
    size_t size) = 0;
  virtual int funcLogicalOp(
    const vector<mpc_t> X,
    const vector<string> sY,
    vector<mpc_t>& Z,
    size_t size) {
    Z.resize(size);
    vector<double> dY(size, 0);
    vector<mpc_t> Y(size, 0);
    rosetta::convert::from_double_str(sY, dY);
    convert_double_to_mpctype(dY, Y);

    ///////////////////////////////////
    for (size_t i = 0; i < size; i++) {
      Z[i] = X[i] * Y[i];
    }
    if (PRIMARY)
      funcTruncate2PC(Z, FLOAT_PRECISION_M, size, PARTY_A, PARTY_B);
    ///////////////////////////////////
    return 0;
  }

  virtual int funcLogicalOp(
    const vector<string> X,
    const vector<mpc_t> Y,
    vector<mpc_t>& Z,
    size_t size) {
    return funcLogicalOp(Y, X, Z, size);
  }
  /**
   * K-*, recursion version
   * 
   * XX is a 2-d vector
   * if XX.size() == 0; return 0
   * if XX.size() == 1; return XX[0]
   * if XX.size() >= 1; return XX[0] (OP) XX[1] (OP)... XX[i]
   */
  int funcLogicalOp(const vector<vector<mpc_t>> XX, vector<mpc_t>& Z) {
    return funcLogicalOp_(XX, Z);
  };
  int funcLogicalOp_(const vector<vector<mpc_t>> XX, vector<mpc_t>& Z);
};

class LogicalAND : public LogicalOp {
  using LogicalOp::LogicalOp;

 private:
  int funcLogicalOp(const vector<mpc_t> X, const vector<mpc_t> Y, vector<mpc_t>& Z, size_t size);
  int funcLogicalOp(const vector<mpc_t> X, const vector<string> sY, vector<mpc_t>& Z, size_t size);
};

class LogicalOR : public LogicalOp {
  using LogicalOp::LogicalOp;

 private:
  int funcLogicalOp(const vector<mpc_t> X, const vector<mpc_t> Y, vector<mpc_t>& Z, size_t size);
  int funcLogicalOp(const vector<mpc_t> X, const vector<string> sY, vector<mpc_t>& Z, size_t size);
};

class LogicalXOR : public LogicalOp {
  using LogicalOp::LogicalOp;

 private:
  int funcLogicalOp(const vector<mpc_t> X, const vector<mpc_t> Y, vector<mpc_t>& Z, size_t size);
  int funcLogicalOp(const vector<mpc_t> X, const vector<string> sY, vector<mpc_t>& Z, size_t size);
};

class LogicalNOT : public OpBase {
  using OpBase::OpBase;

 public:
  int Run(const vector<mpc_t> X, vector<mpc_t>& Z, size_t size) {
    Z.resize(size);
    MPCOP_RETURN(funcLogicalOp(X, Z, size));
  }

 private:
  int funcLogicalOp(const vector<mpc_t> X, vector<mpc_t>& Z, size_t size);
};

} // namespace snn
} // namespace rosetta
