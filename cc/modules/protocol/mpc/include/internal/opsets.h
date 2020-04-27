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

#include "mpc_helper.h"
#include "opsets_local.h"
#include "opsets_base.h"

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
#define GetMpcOpInner(opname) std::make_shared<rosetta::mpc::opname>(msg_id())
// the folowing macros, for outer caller
// todo: optimize
#define GetMpcOpDefault(opname) std::make_shared<rosetta::mpc::opname>(#opname)
#define GetMpcOpWithKey(opname, key) std::make_shared<rosetta::mpc::opname>(key)

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
      string st("+rosetta::mpc::");
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

namespace rosetta {
namespace mpc {

class OpBase : public OpBase_ {
 public:
  explicit OpBase(const msg_id_t& key) : msg_id_(key) {
    init();
  }
  explicit OpBase(const std::string& key) : msg_id_(key) {
    init();
  }
  explicit OpBase(const std::shared_ptr<OpBase>& op) : msg_id_(op->msg_id()) {
    init();
  }
  virtual ~OpBase() = default;

 public:
  virtual const msg_id_t& msg_id() const {
    return msg_id_;
  }

 protected:
  msg_id_t msg_id_;
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
  int Run() {
    MPCOP_RETURN(funcSynchronize());
  }

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
  int Run(mpc_t& seed) {
    MPCOP_RETURN(funcRandomSeed(seed));
  }

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
  int Run(int party0, int party1, mpc_t& shares) {
    MPCOP_RETURN(funPRZS(party0, party1, shares));
  }
  int Run(int party0, int party1, double& shares) {
    MPCOP_RETURN(funPRZS(party0, party1, shares));
  }

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
  int Run(int party, double v, mpc_t& shares) {
    MPCOP_RETURN(funPrivateInput(party, v, shares));
  }
  int Run(int party, double v, double& shares) {
    MPCOP_RETURN(funPrivateInput(party, v, shares));
  }

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
    const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t rows,
    size_t common_dim, size_t columns, size_t transpose_a, size_t transpose_b) {
    MPCOP_RETURN(funcMatMulMPC(a, b, c, rows, common_dim, columns, transpose_a, transpose_b));
  }

 private:
  int funcMatMulMPC(
    const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t rows,
    size_t common_dim, size_t columns, size_t transpose_a, size_t transpose_b);
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

 private:
  int funcRELUMPC(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size);
};

class ReluPrime : public OpBase {
  using OpBase::OpBase;

 public:
  int Run3PC(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size) {
    MPCOP_RETURN(funcRELUPrime3PC(a, b, size));
  }
  int Run(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size) {
    return Run3PC(a, b, size);
  }

 private:
  int funcRELUPrime3PC(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size);
};

class Max : public OpBase {
  using OpBase::OpBase;

 public:
  int Run(
    const vector<mpc_t>& a, vector<mpc_t>& maxv, vector<mpc_t>& maxIndex, size_t rows,
    size_t columns) {
    MPCOP_RETURN(funcMaxMPC(a, maxv, maxIndex, rows, columns));
  }

 private:
  int funcMaxMPC(
    const vector<mpc_t>& a, vector<mpc_t>& maxv, vector<mpc_t>& maxIndex, size_t rows,
    size_t columns);
};
class MaxIndex : public OpBase {
  using OpBase::OpBase;

 public:
  int Run(vector<mpc_t>& a, const vector<mpc_t>& maxIndex, size_t rows, size_t columns) {
    MPCOP_RETURN(funcMaxIndexMPC(a, maxIndex, rows, columns));
  }

 private:
  int funcMaxIndexMPC(vector<mpc_t>& a, const vector<mpc_t>& maxIndex, size_t rows, size_t columns);
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
    MPCOP_RETURN(funcMean(a, b, rows, cols));
  }

 private:
  int funcMean(const vector<mpc_t>& a, vector<mpc_t>& b, size_t rows, size_t cols);
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

  int Run(const mpc_t& a, mpc_t& out, int recv_party = PARTY_A) {
    MPCOP_RETURN(funcReconstruct2PC(a, out, recv_party));
  }

  int RunV2(const vector<mpc_t>& a, size_t size, vector<mpc_t>& out, int recv_party = PARTY_A) {
    MPCOP_RETURN(reconstrct_general(a, size, out, recv_party));
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

  /**
   * @brief: the shared_v will be 'revealed' to plaintext_v held by rev_party 
   **/
  int reconstrct_general(
    const vector<mpc_t>& shared_v, size_t size, vector<mpc_t>& plaintext_v, int recv_party);
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

// TODO: remove these internal functionalities in a standalone file.
///////**************************some internal functionalities ********************************
/*
	****Polynomials******************
*/

/*
	for example:
		y = 1 + 2 * X + 5 * X^ 3 , \in (0, 4)
		get_power_list --> [0, 1, 3]
		get_coff_list --> [1, 2, 5]
		get_start --> 0
		get_end --> 4
*/
class ConstPolynomial {
 public:
  //ConstPolynomial(): __start(0), __end(0) {}
  explicit ConstPolynomial(
    mpc_t init_start, mpc_t init_end, std::vector<std::vector<mpc_t>> init_poly);

  bool get_power_list(vector<mpc_t>& out_vec);
  bool get_coff_list(vector<mpc_t>& out_vec);

  mpc_t get_start() {
    return __start;
  };
  mpc_t get_end() {
    return __end;
  };

 private:
  // internal presentation for initialization
  std::vector<std::vector<mpc_t>> __inner_poly;
  // Note: if __end == __start, this function is for all X, [-\inf, +\inf].
  mpc_t __start = FloatToMpcType(0.0); // >=
  mpc_t __end = FloatToMpcType(0.0); // <
};

/*
	@brief: function approximation registering entry for mapping from func_name to its
	segemental polynomials.
	// TODO: make this as singleton
*/
struct PolyConfFactory {
  // eg : "log_v1" --> A1
  // 		"log_v2" --> [B1, B2, B3]
  // TODO: add mutex_lock or RW lock
 public:
  static void func_register(const std::string& func_name, vector<ConstPolynomial>* approx_polys);

  static bool get_func_polys(const std::string& func_name, vector<ConstPolynomial>** approx_polys);
  //private:
  //	static unordered_map<std::string, vector<ConstPolynomial>> FUNC_POLY_MAP;
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
			 containing some computed k --> Y, to acclerate.
  */
  void mpc_pow_const(
    const mpc_t& shared_X, mpc_t common_k, mpc_t& shared_Y,
    unordered_map<mpc_t, mpc_t>* curr_cache = NULL);

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
    const mpc_t& shared_X, const vector<mpc_t>& common_power_list,
    const vector<mpc_t>& common_coff_list, mpc_t& shared_Y);
  /*
  	@brief: secret-shared version of Y = X * X
  */
  void mpc_squre(const mpc_t& shared_X, mpc_t& shared_Y);
};

class PrivateCompare : public OpBase {
  using OpBase::OpBase;

 public:
  int Run(
    const vector<small_mpc_t>& share_m, const vector<mpc_t>& r, const vector<small_mpc_t>& beta,
    vector<small_mpc_t>& betaPrime, size_t size, size_t dim) {
    MPCOP_RETURN(funcPrivateCompareMPC(share_m, r, beta, betaPrime, size, dim));
  }

 private:
  int funcPrivateCompareMPC(
    const vector<small_mpc_t>& share_m, const vector<mpc_t>& r, const vector<small_mpc_t>& beta,
    vector<small_mpc_t>& betaPrime, size_t size, size_t dim);
};

class ShareConvert : public OpBase {
  using OpBase::OpBase;

 public:
  int Run(vector<mpc_t>& a, size_t size) {
    MPCOP_RETURN(funcShareConvertMPC(a, size));
  }

 private:
  int funcShareConvertMPC(vector<mpc_t>& a, size_t size);
};

class ComputeMSB : public OpBase {
  using OpBase::OpBase;

 public:
  int Run3PC(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size) {
    MPCOP_RETURN(funcComputeMSB3PC(a, b, size));
  }
  int Run(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size) {
    return Run3PC(a, b, size);
  }

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
    const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size);
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
  int RunAliPieceWise(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size) {
    MPCOP_RETURN(funcSigmoidAliPieceWiseMPC(a, b, size));
  }
  int Run(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size) {
#if USE_PIECE_WISE_SIGMOID
    return RunPieceWise(a, b, size);
#else
    return RunG3(a, b, size);
#endif
  }

 private:
  int funcSigmoidG3MPC(
    const vector<mpc_t>& a, vector<mpc_t>& b, size_t size, bool use_fastPow3 = true);
  int funcSigmoidG3PrimeMPC(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size);
  int funcSigmoidPieceWiseMPC(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size);
  int funcSigmoidAliPieceWiseMPC(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size);

 private:
  int funcPrivateCompareMPCEx(
    const vector<mpc_t>& a, const vector<mpc_t>& r, vector<mpc_t>& b, size_t size);
  int funcPrivateCompareMPCEx2(
    const vector<mpc_t>& r, const vector<mpc_t>& a, vector<mpc_t>& b, size_t size);
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
  int Run(const mpc_t& a, const mpc_t& b, mpc_t& c) {
    MPCOP_RETURN(funcBinaryOp(a, b, c));
  }
  int Run(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size) {
    MPCOP_RETURN(funcBinaryOp(a, b, c, size));
  }
  // const OP, in P0, c = a (op) b; in P1, c = 0 (op) b
  int Run(const vector<double>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size) {
    MPCOP_RETURN(funcBinaryOp(a, b, c, size));
  }
  // const OP, in P0, c = a (op) b; in P1, c = a (op) 0
  int Run(const vector<mpc_t>& a, const vector<double>& b, vector<mpc_t>& c, size_t size) {
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
    const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size) = 0;
  virtual int funcBinaryOp(
    const vector<double>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size) {
    vector<mpc_t> va(a.size(), 0);
    if (partyNum == PARTY_A) {
      convert_double_to_mpctype(a, va);
    }
    return funcBinaryOp(va, b, c, size);
  }
  virtual int funcBinaryOp(
    const vector<mpc_t>& a, const vector<double>& b, vector<mpc_t>& c, size_t size) {
    vector<mpc_t> vb(b.size(), 0);
    if (partyNum == PARTY_A) {
      convert_double_to_mpctype(b, vb);
    }
    return funcBinaryOp(a, vb, c, size);
  }
};

class Add : public BinaryOp {
  using BinaryOp::BinaryOp;

  virtual int funcBinaryOp(
    const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size) {
    addVectors(a, b, c, size);
    return 0;
  }
};

class Sub : public BinaryOp {
  using BinaryOp::BinaryOp;

  virtual int funcBinaryOp(
    const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size) {
    subtractVectors(a, b, c, size);
    return 0;
  }
};

class DotProduct : public BinaryOp {
  using BinaryOp::BinaryOp;

  virtual int funcBinaryOp(
    const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size) {
    return funcDotProduct(a, b, c, size);
  }

  virtual int funcBinaryOp(
    const vector<double>& fa, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size) {
    vector<mpc_t> a(fa.size(), 0);
    convert_double_to_mpctype(fa, a);
    c.resize(a.size());
    for (size_t i = 0; i < size; i++) {
      c[i] = a[i] * b[i];
    }
    if (PRIMARY)
      funcTruncate2PC(c, FLOAT_PRECISION, size, PARTY_A, PARTY_B);
    return 0;
  }
  virtual int funcBinaryOp(
    const vector<mpc_t>& a, const vector<double>& b, vector<mpc_t>& c, size_t size) {
    return funcBinaryOp(b, a, c, size);
  }

 private:
  int funcDotProduct(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size);
};

class Mul : public DotProduct {
  using DotProduct::DotProduct;
};

class DivBase : public BinaryOp {
  using BinaryOp::BinaryOp;

 protected:
  virtual int funcBinaryOp(
    const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size) = 0;

  virtual int funcBinaryOp(
    const vector<double>& fa, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size) {
    // locally compute c = fa * (1/b)
    {
      vector<mpc_t> a(b.size(), FloatToMpcType(0.5));
      funcBinaryOp(a, b, c, size);
    }
    vector<mpc_t> a(fa.size(), 0);
    convert_double_to_mpctype(fa, a);
    for (size_t i = 0; i < size; i++) {
      c[i] = a[i] * c[i];
    }
    if (PRIMARY)
      funcTruncate2PC(c, FLOAT_PRECISION, size, PARTY_A, PARTY_B);
    return 0;
  }
  virtual int funcBinaryOp(
    const vector<mpc_t>& a, const vector<double>& fb, vector<mpc_t>& c, size_t size) {
    // locally compute c = a * (1/fb)
    vector<double> tb(fb.size(), 0);
    for (size_t i = 0; i < size; i++) {
      tb[i] = 1.0 / fb[i];
    }
    vector<mpc_t> b(tb.size(), 0);
    convert_double_to_mpctype(tb, b);
    c.resize(a.size());
    for (size_t i = 0; i < size; i++) {
      c[i] = a[i] * b[i];
    }
    if (PRIMARY)
      funcTruncate2PC(c, FLOAT_PRECISION, size, PARTY_A, PARTY_B);
    return 0;
  }
};

class Div : public DivBase {
  using DivBase::DivBase;

  virtual int funcBinaryOp(
    const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size) {
    return funcDiv(a, b, c, size);
  }

 private:
  int funcDiv(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size);
};

class Truediv : public DivBase {
  using DivBase::DivBase;

  virtual int funcBinaryOp(
    const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size) {
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
    MPCOP_RETURN(funcPow(x, n, y, size));
  }
  int Run(const vector<mpc_t>& x, vector<int64_t> n, vector<mpc_t>& y, size_t size) {
    MPCOP_RETURN(funcPow2(x, n, y, size));
  }

 private:
  int funcPow(const vector<mpc_t>& x, size_t n, vector<mpc_t>& y, size_t size) {
    vector<int64_t> vn(size, n);
    return funcPow2(x, vn, y, size);
  }
  int funcPow2(const vector<mpc_t>& x, vector<int64_t> n, vector<mpc_t>& y, size_t size) {
    assert(x.size() == n.size());
    assert(x.size() == size);
    y.resize(size);
    auto poly = GetMpcOpInner(Polynomial);
    for (int i = 0; i < (int)size; i++) {
      unordered_map<mpc_t, mpc_t> curr_cache;
      poly->mpc_pow_const(x[i], n[i], y[i], &curr_cache);
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
    MPCOP_RETURN(funcLog(a, b, size));
  }

  int Run1p(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size) {
    MPCOP_RETURN(funcLog1p(a, b, size));
  }

  int RunHd(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size) {
    MPCOP_RETURN(mpc_log_hd(a, b, size));
  }

  /*
  	@brief: General high precision approximation(gurantee four decimal part)
  			for LOG in the domain
  */
  int mpc_log_hd(const vector<mpc_t>& shared_X, vector<mpc_t>& shared_Y, size_t vec_size);

  //private:
  int funcLog(const mpc_t& a, mpc_t& b) {
    mpc_log_v2(a, b);
    return 0;
  }
  int funcLog(const vector<mpc_t>& a, vector<mpc_t>& b, size_t size) {
    b.resize(a.size());
    //TODO: vectorization
    for (size_t i = 0; i < size; ++i) {
      //mpc_t res = a[i];
      mpc_log_v2(a[i], b[i]);
    }
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
	  @brief: secret-sahred version of logarithm with one-segment polynomial
	  with domain x \in [0.3, 1.8)
	  TODO: to support vectorized input
  */
  void mpc_log_v1(const mpc_t& shared_X, mpc_t& shared_Y);

  /*
  	@brief: secret-sahred version of logarithm with three-segment polynomial
  */
  void mpc_log_v2(const mpc_t& shared_X, mpc_t& shared_Y);
};

class CompareOp : public OpBase {
  using OpBase::OpBase;

 public:
  int Run(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size) {
    MPCOP_RETURN(RunCompareOp(a, b, c, size));
  }
  int Run(const vector<mpc_t>& a, const vector<double>& b, vector<mpc_t>& c, size_t size) {
    /// TODO
    return 0;
  }
  int Run(const vector<double>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size) {
    /// TODO
    return 0;
  }
  int Run(const mpc_t& a, const mpc_t& b, mpc_t& c) {
    vector<mpc_t> in_a{a};
    vector<mpc_t> in_b{b};
    vector<mpc_t> ret(1, 0);
    Run(in_a, in_b, ret, in_a.size());
    c = ret[0];

    return 0;
  }

 protected:
  virtual int RunCompareOp(
    const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size) = 0;
};

class Equal : public CompareOp {
  using CompareOp::CompareOp;

  virtual int RunCompareOp(
    const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size) {
    return funcEqual(a, b, c, size);
  }

 private:
  int funcEqual(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size);
};

class Less : public CompareOp {
  using CompareOp::CompareOp;

  virtual int RunCompareOp(
    const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size) {
    return funcLess(a, b, c, size);
  }

 private:
  int funcLess(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size);
};

class Greater : public CompareOp {
  using CompareOp::CompareOp;

  virtual int RunCompareOp(
    const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size) {
    return funcGreater(a, b, c, size);
  }

 private:
  int funcGreater(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size);
};

class GreaterEqual : public CompareOp {
  using CompareOp::CompareOp;

  virtual int RunCompareOp(
    const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size) {
    return funcGreaterEqual(a, b, c, size);
  }

 private:
  int funcGreaterEqual(
    const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size);
};

class LessEqual : public CompareOp {
  using CompareOp::CompareOp;

  virtual int RunCompareOp(
    const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size) {
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
    MPCOP_RETURN(funcDivisionMPC(a, b, c, size));
  }

 private:
  int funcDivisionMPC(
    const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& quotient, size_t size);
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
    const vector<mpc_t> shared_x, const vector<mpc_t> shared_y, const vector<mpc_t> shared_bool,
    vector<mpc_t>& shared_result, size_t size) {
    MPCOP_RETURN(mpc_select_1_of_2(shared_x, shared_y, shared_bool, shared_result, size));
  }

 private:
  int mpc_select_1_of_2(
    const vector<mpc_t> shared_x, const vector<mpc_t> shared_y, const vector<mpc_t> shared_bool,
    vector<mpc_t>& shared_result, size_t size);
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
    const vector<mpc_t> shared_x, const vector<mpc_t> shared_y, vector<mpc_t>& shared_result,
    size_t size) {
    MPCOP_RETURN(mpc_xor_bit(shared_x, shared_y, shared_result, size));
  }

 private:
  int mpc_xor_bit(
    const vector<mpc_t> shared_x, const vector<mpc_t> shared_y, vector<mpc_t>& shared_result,
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
				its corresponding denomimators.
				This is for efficency in this special case.
		[out] shared_quotient_vec: the resulting vector of quotients
	@note:
		This function supports both x>=y and x<y; 
		and both positiveness negetiveness.
		And if x is not divisable by y,
		the precision of the fractional part is FLOAT_PRECISION.
  @author: SJJ
*/
class DivisionV2 : public OpBase {
  using OpBase::OpBase;

 public:
  int Run(
    const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size,
    bool all_less = false) {
    MPCOP_RETURN(funcDivisionMPCV2(a, b, c, size, all_less));
  }

 private:
  int funcDivisionMPCV2(
    const vector<mpc_t>& shared_numerator_vec, const vector<mpc_t>& shared_denominator_vec,
    vector<mpc_t>& shared_quotient_vec, size_t common_vec_size, bool common_all_less = false);
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
    const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size,
    bool all_positive = false) {
    MPCOP_RETURN(mpc_floor_division(a, b, c, size, all_positive));
  }

 private:
  int mpc_floor_division(
    const vector<mpc_t>& shared_numerator_vec, const vector<mpc_t>& shared_denominator_vec,
    vector<mpc_t>& shared_quotient_vec, size_t common_vec_size, bool all_positive = false);
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
    const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size,
    bool all_positive = false) {
    MPCOP_RETURN(mpc_frac_division(a, b, c, size, all_positive));
  }

 private:
  int mpc_frac_division(
    const vector<mpc_t>& shared_numerator_vec, const vector<mpc_t>& shared_denominator_vec,
    vector<mpc_t>& shared_quotient_vec, size_t common_vec_size, bool all_positive = false);
};

} // namespace mpc
} // namespace rosetta
