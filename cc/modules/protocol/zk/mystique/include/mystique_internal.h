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

#include "cc/modules/protocol/utility/include/prg.h"
#include "cc/modules/protocol/zk/mystique/include/zk_int_fp_eigen.h"
#include "emp-tool/emp-tool.h"
#include "emp-tool/circuits/integer.h"
#include "emp-zk/emp-zk.h"

#include "cc/modules/protocol/zk/mystique/include/wvr_util.h"

using emp::Integer;
using rosetta::zk::ZkIntFp;
using std::vector;
#include <atomic>

// float2int/int2float
extern std::atomic<int64_t> fxf_counter;
extern std::atomic<int64_t> fxf_elapsed;
extern std::atomic<int64_t> float2int_counter;
extern std::atomic<int64_t> float2int_elapsed;
extern std::atomic<int64_t> int2float_counter;
extern std::atomic<int64_t> int2float_elapsed;

extern int zk_party_id;
#define ZK_IS_ALICE (zk_party_id == 1)
#define ZK_IS_BOB (zk_party_id == 2)

#define USE_INNER_PRODUCT_OPT 1

namespace rosetta {
namespace zk {

static inline void mystique_truncate(ZkIntFp* in, size_t size) {
  // out.resize(in.size());
  SimpleTimer timer;
  vector<Integer> bc(size);
  sync_zk_bool<BoolIO<ZKNetIO>>();
  arith2bool<BoolIO<ZKNetIO>>(bc.data(), (ZkIntFp*)in, size);
  log_debug << "truncation use bool zk_fp -> bool_int -> bool_float -> bool_int -> zk_fp ...";
  float2int_counter += size;
  int2float_counter += size;
  for (size_t i = 0; i < bc.size(); i++) {
    SimpleTimer timer_b2f;
    Float fl = std::move(Int62ToFloat(bc[i], 2*ZK_FLOAT_SIZE));
    int2float_elapsed += timer_b2f.ns_elapse();
    SimpleTimer timer_f2b;
    bc[i] = FloatToInt62(fl, ZK_FLOAT_SIZE);
    float2int_elapsed += timer_f2b.ns_elapse();
  }

  // sync_zk_bool<BoolIO<ZKNetIO>>();
  bool2arith<BoolIO<ZKNetIO>>(in, bc.data(), bc.size());
  sync_zk_bool<BoolIO<ZKNetIO>>();
  log_debug << "truncation OK. cost: " << timer.ns_elapse()/1000 << " us";
}

static inline void mystique_relu(const vector<ZkIntFp>& in, vector<ZkIntFp>& out, int scale_count=1) {
  size_t size = in.size();
  vector<Integer> bin(size);
  sync_zk_bool<BoolIO<ZKNetIO>>();
  arith2bool<BoolIO<ZKNetIO>>(bin.data(), (IntFp*)in.data(), size);

  if (scale_count == 1) {
    // do nothing
  } else if (scale_count == 2) {
    for (size_t i = 0; i < bin.size(); i++)
    {
      Float fl = std::move(Int62ToFloat(bin[i], 2*ZK_FLOAT_SIZE));
      bin[i] = FloatToInt62(fl, ZK_FLOAT_SIZE);
    }
  } else {
    log_error << "bad scale_count:  " << scale_count;
    throw std::runtime_error("bad scale_count");
  }
  // sync_zk_bool<BoolIO<ZKNetIO>>();
  // std::cout << "arith2bool<BoolIO<ZKNetIO>> ok." << endl;

  Integer zero(ZK_INT_LEN, 0, PUBLIC);
  Integer one(ZK_INT_LEN, 1, PUBLIC);

  vector<Integer> relu(size);
  Integer smallest_neg(ZK_INT_LEN, (uint64_t)((PR-1)/2)+1, PUBLIC);
  for (size_t i = 0; i < size; ++i)
    relu[i] = bin[i].select(bin[i].geq(smallest_neg), zero);
  
  // std::cout << "bool relu ok." << endl;
  // sync_zk_bool<BoolIO<ZKNetIO>>();
  out.resize(size);
  bool2arith<BoolIO<ZKNetIO>>((IntFp*)out.data(), relu.data(), size);
  sync_zk_bool<BoolIO<ZKNetIO>>();
}

static inline void mystique_relu_prime(const ZkIntFp* in, size_t size, vector<ZkIntFp>& output) {
  vector<Integer> bin(size, Integer(ZK_INT_LEN, 0, PUBLIC));
  output.resize(size);
  sync_zk_bool<BoolIO<ZKNetIO>>();
  arith2bool<BoolIO<ZKNetIO>>(bin.data(), (IntFp*)in, size);
  // std::cout << "arith2bool<BoolIO<ZKNetIO>> ok." << endl;
  Integer zero(ZK_INT_LEN, 0, PUBLIC);
  Integer one(ZK_INT_LEN, 1, PUBLIC);

  vector<Integer> relu_prime(size, Integer(ZK_INT_LEN, 0, PUBLIC));
  // Bit* cmp = new Bit[size];
  Integer smallest_neg(ZK_INT_LEN, (uint64_t)((PR-1)/2)+1, PUBLIC);
  for (size_t i = 0; i < size; ++i)
    relu_prime[i] = one.select(bin[i].geq(smallest_neg), zero);
  
  // std::cout << "bool relu_prime ok." << endl;
  bool2arith<BoolIO<ZKNetIO>>((IntFp*)output.data(), relu_prime.data(), size);
  sync_zk_bool<BoolIO<ZKNetIO>>();
}

static inline void mystique_relu_prime(const vector<ZkIntFp>& in, vector<ZkIntFp>& out) {
  mystique_relu_prime(in.data(), in.size(), out);
}

static inline void mystique_max(const vector<ZkIntFp>& in, ZkIntFp& out) {
  if (in.empty())
    throw std::runtime_error("null input, mystique_max");
  // sync_zk_bool<BoolIO<ZKNetIO>>();
  // simple bubble 
  size_t size = in.size();
  log_debug << "calling new Max debug stub, size: " << size;
  // todo: vectorization 
  vector<ZkIntFp> curr_max(1, in[0]);
  vector<ZkIntFp> curr_pos(1);
  vector<ZkIntFp> temp_vec(1);
  // SJJ: tmp disable this since we call this in SecureOp for now! 
  // sync_zk_bool<BoolIO<ZKNetIO>>();
  for(int i = 1; i < size; ++i) {
    curr_pos[0] = in[i];
    zk_max(curr_max, curr_pos, temp_vec);
    curr_max.swap(temp_vec);
  }
  
  // sync_zk_bool<BoolIO<ZKNetIO>>();
  out = curr_max[0];
}

// get max elements of matrix, axis=1, get max of each row
static inline void mystique_max_matrix(const vector<ZkIntFp>& in, size_t rows, size_t cols, vector<ZkIntFp>& out) {
  if (in.size() != rows*cols)
    throw std::runtime_error("input size ! = rows*cols, mystique_max_matrix");
  
  log_debug << "mystique_max_matrix, rows: " << rows << ", cols: " << cols;
  // simple bubble to get max element of each row
  out.resize(rows);
  
  size_t size = rows*cols;
  vector<Integer> input_bin(size); //, Integer(ZK_INT_LEN, 0, PUBLIC));
  vector<Integer> bin_c(rows);
  sync_zk_bool<BoolIO<ZKNetIO>>();
  arith2bool<BoolIO<ZKNetIO>>(input_bin.data(), (IntFp*)in.data(), size);
  // sync_zk_bool<BoolIO<ZKNetIO>>();

  Integer NEG_SIGN(ZK_INT_LEN, HALF_PR + 1, PUBLIC);// the smallest negative value
  // a as the formal one, b as the latter one
  Integer same_sign_as_bigger_one;
  Integer a_positive_as_bigger_one;
  Bit sign_a;
  Bit sign_b;
  Bit compare_sign;
  Bit a_and_b_not_same_sign;
  
  // log_info << "mystique_max_matrix, start run...";
  // get a bigger one
  for(size_t i = 0; i < rows; ++i) {
    // log_info << "mystique_max_matrix, start run-" << i << " row...";
    bin_c[i] = input_bin[i*cols];
    for (size_t j = 1; j < cols; ++j) {
      sign_a = bin_c[i].geq(NEG_SIGN);// is negative
      sign_b = input_bin[i*cols + j].geq(NEG_SIGN); // is negative

      a_and_b_not_same_sign = (sign_a ^ sign_b);// xor
      // case 1, if a and b have same sign, we get the larger encoded one.
      compare_sign = input_bin[i*cols + j].geq(bin_c[i]);
      same_sign_as_bigger_one = bin_c[i].select(compare_sign, input_bin[i*cols + j]);
      // case 2. if a and b have different sign, and a has positive sign, we choose a.
      a_positive_as_bigger_one = bin_c[i].select(sign_a, input_bin[i*cols + j]);
      // choose case based on sign-XOR
      bin_c[i] = same_sign_as_bigger_one.select(a_and_b_not_same_sign, a_positive_as_bigger_one);
    }
  }
  // sync_zk_bool<BoolIO<ZKNetIO>>();
  bool2arith<BoolIO<ZKNetIO>>((IntFp*)out.data(), bin_c.data(), rows);
  sync_zk_bool<BoolIO<ZKNetIO>>();
  log_debug << "mystique_max_matrix ok.";
}

// get max elements of matrix, axis=1, get max of each row
static inline void mystique_max_matrix_bin(const vector<Integer>& input_bin, size_t rows, size_t cols, vector<Integer>& out) {
  if (input_bin.size() != rows*cols)
    throw std::runtime_error("input size ! = rows*cols, mystique_max_matrix");
  
  log_debug << "mystique_max_matrix_bool, rows: " << rows << ", cols: " << cols;
  // simple bubble to get max element of each row
  out.resize(rows);
  
  size_t size = rows*cols;
  vector<Integer> &bin_c = out;
  Integer NEG_SIGN(ZK_INT_LEN, HALF_PR + 1, PUBLIC);// the smallest negative value
  // a as the formal one, b as the latter one
  Integer same_sign_as_bigger_one;
  Integer a_positive_as_bigger_one;
  Bit sign_a;
  Bit sign_b;
  Bit compare_sign;
  Bit a_and_b_not_same_sign;
  
  // log_info << "mystique_max_matrix, start run...";
  // get a bigger one
  for(size_t i = 0; i < rows; ++i) {
    // log_info << "mystique_max_matrix, start run-" << i << " row...";
    bin_c[i] = input_bin[i*cols];
    for (size_t j = 1; j < cols; ++j) {
      sign_a = bin_c[i].geq(NEG_SIGN);// is negative
      sign_b = input_bin[i*cols + j].geq(NEG_SIGN); // is negative

      a_and_b_not_same_sign = (sign_a ^ sign_b);// xor
      // case 1, if a and b have same sign, we get the larger encoded one.
      compare_sign = input_bin[i*cols + j].geq(bin_c[i]);
      same_sign_as_bigger_one = bin_c[i].select(compare_sign, input_bin[i*cols + j]);
      // case 2. if a and b have different sign, and a has positive sign, we choose a.
      a_positive_as_bigger_one = bin_c[i].select(sign_a, input_bin[i*cols + j]);
      // choose case based on sign-XOR
      bin_c[i] = same_sign_as_bigger_one.select(a_and_b_not_same_sign, a_positive_as_bigger_one);
    }
  }
  
  log_debug << "mystique_max_matrix_bool ok.";
}

static inline void mystique_min(const vector<ZkIntFp>& in, ZkIntFp& out) {
  throw std::runtime_error("Mystique_min NOT IMPL!");
  if (in.empty())
    throw std::runtime_error("null input, mystique_min");
  
  //[kelvin] TODO: implement min the same way as mystique_max does
}

// matmul proof 
// markdown format below:
// $input:\; [A]_{m,k},\; [B]_{k,n},\;\; which\;are\;IT\text{-}MAC\;format$
// $output: \; [C\prime_{m,n}] \;as\; [A_{m,k} * B_{k,n}]$
// $implementation:$
// 1. Prover decodes and gets float plaintext $A\prime_{m,k},\; B\prime_{k,n}$,  and fixpoint plaintext $\tilde{A}_{m,k},\; \tilde{B}_{k,n} \;(in\;Fp)$;
// 2. Prover computes $C\prime_{m,n} = A\prime_{m,k} * B\prime_{k,n}$, $\tilde{C}_{m,n} = \tilde{A}_{m,k} * \tilde{B}_{k,n}$;
// 3. Prover/Verifier $[\tilde{C}_{m,n} ] = private\_input(\tilde{C}_{m,n} )$, $[C\prime_{m,n}] = private\_input(C\prime_{m,n})$, $[C\prime\prime_{m,n}] = [C\prime_{m,n}] * 2^{16}$;
// 4. Verifier generates seed and send to Prover;
// 5. Verifier/Prover chooses random vectors $u_{1,m}, v_{n,1}$;
// 6. Prover/Verifier computes $[y]_{1,k} = u_{}\cdot [A] $ and $[x]_{k,1} =[B] \cdot v$,  then compute $[t]_{1,1} = [x]_{1,k}\cdot [y]_{k,1}$; 
// 7. Prover/Verifier reveal $[t] - u\cdot [\tilde{C}_{m,n} ]  \cdot v$, and check it is $0$ or not
// 8. Prover/Verifier compute $[\Delta]_{m,n} = [C\prime\prime_{m,n}] - [\tilde{C}_{m,n}]$;
// 9. Prover/Verifier convert $[\Delta]_{m,n}$ to  $\left|\Delta\right|_{m*n}$ vector;
// 10. $\text{for}\; i\; in\; [0,1,\;\cdots \;(m*n)):$
// 	- prover/verifier check if $abs(\left|\Delta\right|_{m*n}[i]) \leq 2^{16}$ is satisfied, true go next, false then abort;
static inline int mystique_matmul_fp_with_proof(const ZkMatrix& a, const ZkMatrix& b, ZkMatrix& c) {
  log_debug << "party: " << zk_party_id << " : mystique_matmul_fp_with_proof, (" << a.rows() << "," << a.cols() << ") * (" << b.rows() << "," << b.cols() << "), start..." << ENDL;
  assert(a.size() != 0 && b.size() != 0);
  if (c.size() == 0)
    c.resize(a.rows(), b.cols());
  
  ZkMatrix& C_prime = c;
  ZkMatrix C_tilde(c.rows(), c.cols());
  
  static std::atomic<bool> is_first_call{true};
  static emp::block seed_block{0};
  static RttPRG prg;
  
  if (ZK_IS_ALICE) {
    log_debug << "step1. Prover decodes and gets float plaintext, and Fp plaintext...";
    DoubleMatrix a_prime(a.rows(), a.cols()), b_prime(b.rows(), b.cols());
    ZkU64Matrix a_tilde(a.rows(), a.cols()), b_tilde(b.rows(), b.cols());

    // Todo[GeorgeShi]: these 4 naive calling of zk_decode has many duplicated computation, we should remove them.
    zk_decode(a.data(), a.size(), a_prime.data());
    zk_decode(b.data(), b.size(), b_prime.data());
    
    zk_decode(a.data(), a.rows() * a.cols(), (uint64_t*)a_tilde.data());
    zk_decode(b.data(), b.rows() * b.cols(), (uint64_t*)b_tilde.data());
    log_debug << "step2. Prover computes plaintext c_prime, c_tilde... ";
    DoubleMatrix c_prime = a_prime * b_prime;
    // SJJ: perf this 
    ZkU64Matrix c_tilde = a_tilde * b_tilde;    
    log_debug << "step3. Prover/Verifier private_input c_prime, c_tilde to C_prime and C_tilde...";
    U64Matrix c_prime_fix(a.rows(), b.cols());
    ZkIntFp::zk_fp_encode(c_prime.data(), c_prime_fix.data(), c_prime_fix.size());
    batch_feed((IntFp*)C_prime.data(), c_prime_fix.data(), c_prime_fix.size());
    batch_feed((IntFp*)C_tilde.data(), (uint64_t*)c_tilde.data(), c_tilde.size());
    if (is_first_call) {
      // step4. Verifier generates seed and send to Prover;
      log_debug << "step4. Prover recv seed from Verifier...";
      sync_zk_bool<BoolIO<ZKNetIO>>();
      ((ZKFpExecPrv<BoolIO<ZKNetIO>>*)(ZKFpExec::zk_exec))->io->recv_data(&seed_block, sizeof(seed_block));
    }
  }//Prover
  else {//Verifier
    log_debug << "step3. Prover/Verifier private_input(c_tilde), [c_prime] = private_input(c_prime), [c_prime_prime] = [c_prime] * 2^16...";
    batch_feed((IntFp*)C_prime.data(), nullptr, C_prime.size());
    batch_feed((IntFp*)C_tilde.data(), nullptr, C_tilde.size());
    if (is_first_call) {
      // step4. Verifier generates seed and send to Prover;
      log_debug << "step4. Verifier generates seed and send to Prover...";
      prg.randomDatas(&seed_block, sizeof(seed_block));
      ((ZKFpExecPrv<BoolIO<ZKNetIO>>*)(ZKFpExec::zk_exec))->io->send_data(&seed_block, sizeof(seed_block));
      sync_zk_bool<BoolIO<ZKNetIO>>();
    }
  }
  // step5. Verifier/Prover chooses random vectors u, v
  log_debug << "step5. Verifier/Prover chooses random vectors u, v...";
  if (is_first_call) {
    prg.reseed(&seed_block);
    is_first_call = false;
  }
  // // eg. A_tilde(m,k), B_tilde(k,n), u(1, m) and v(n,1), then x*y=t(1,1)
  U64Matrix u(1, a.rows()), v(b.cols(), 1);

  // random F_p element u, v derived from random 128-bit blocks source
  size_t u_size = std::ceil(a.rows()/(2.0));
  size_t v_size = std::ceil(b.cols()/(2.0));
  vector<emp::block> u_rds(u_size), v_rds(v_size);
  // public input u, v
  for (size_t i = 0; i < a.rows(); ++i) {
    u(0, i) = mod(prg.getFpBits(PR, false)); // prg.getFpBits(PR, true);
  }
  for (size_t i = 0; i < b.cols(); ++i) {
    v(i, 0) = mod(prg.getFpBits(PR, false)); // prg.getFpBits(PR, true);
  }

  log_debug << "step6. Prover and Verifier computes [x] = u*[A] and [y] = [B]*v in F_p , then compute [t]=[x] *[y]";
  ZkMatrix x(1, a.cols());
  zk_eigen_const_matmul(u, a, x);// local compute: x = u * a

  ZkMatrix y(b.rows(), 1);// local compute: y = b * v
  zk_eigen_const_matmul(b, v, y);

  ZkMatrix t = x * y;// zk multiplication, output (1,1)

#if ZK_INNER_CHECK
  log_debug << "step7.1. Prover/Verifier reveal [t] - u* [tilde_C] * v, and check it is 0 or not ...";
  ZkMatrix temp1(u.rows(), C_tilde.cols());
  ZkMatrix temp2(temp1.rows(), v.cols());
  zk_eigen_const_matmul(u, C_tilde, temp1);
  zk_eigen_const_matmul(temp1, v, temp2);
  ZkMatrix result = t - temp2;
  log_debug << "mystique_matmul_fp_with_proof, after [t] - u* [tilde_C] * v, " << zk_party_id << ": " << result(0,0);

  bool status = batch_reveal_check_zero((IntFp*)result.data(), result.size());
  log_debug << "step7.2 Prover/Verifier reveal [t] - u* [tilde_C] * v ok, and check:  " << status;

  log_debug << "step8.1. (step8,9,10 together) each element do batch multiplication check...";
  ZkMatrix C_prime_prime(C_prime.rows(), C_prime.cols());
  zk_eigen_const_mul(C_prime, (uint64_t)(1ULL << ZK_FLOAT_SIZE), C_prime_prime);// [C_prime_prime] = [C_prime] * 2^16
  int ret = _inner_proof_and_check(C_prime_prime.data(), C_tilde.data(), C_tilde.size());
  log_debug << "step8.2. (step8,9,10 together) each element do batch multiplication ok, check: " << (ret == 0);
#endif

  log_debug << " mystique_matmul_fp_with_proof ok." << "party: " << zk_party_id;
  return 0;
}

// matmul proof with right operhand constant
// markdown format below:
// $input:\; [A]_{m,k} \; is \;\textbf{IT-MAC}\;format,\; B_{k,n}\;\; is \;\textbf{float}\;format$
// $output: \; [\tilde{C}_{m,n}] \;as\; [A_{m,k} * B_{k,n}]$
// $implementation:$
// 1. Prover decodes and gets float plaintext $\tilde{A}_{m,k}$,  and fixpoint plaintext $\bar{A}_{m,k},\; \bar{B}_{k,n} = F2N(B_{k,n})\;(in\;Fp)$;
// 2. Prover computes $\tilde{C}_{m,n} = \tilde{A}_{m,k} * B_{k,n}$, $\bar{C}_{m,n} = \bar{A}_{m,k} * \bar{B}_{k,n}$;
// 3. Prover/Verifier $[\tilde{C}_{m,n}] = private\_input(\tilde{C}_{m,n} )$, $[\tilde{C}^\prime_{m,n}] = [\tilde{C}_{m,n}] * 2^{16}$;
// 4. multiplication error check:
//   4.1. Prover/Verifier compute $[\Delta]_{m,n} = [\tilde{C}^\prime_{m,n}] - [\bar{A}_{m,k}] * \bar{B}_{k,n}$;
//   4.2. Prover/Verifier convert $[\Delta]_{m,n}$ to  $\left|\Delta\right|_{m*n}$ vector;
//   4.3. $\text{for}\; i\; in\; [0,1,\;\cdots \;(m*n)):$
// 	     - prover/verifier check if $abs(\left|\Delta\right|_{m*n}[i]) \leq 2^{16}$ is satisfied, true go next, false then abort;
static inline int mystique_matmul_fp_with_proof_r_const(const ZkMatrix& a, const DoubleMatrix& b, ZkMatrix& c) {
  log_debug << "party: " << zk_party_id << " : mystique_matmul_fp_with_proof, (" 
          << a.rows() << "," << a.cols() << ") * (" 
          << b.rows() << "," << b.cols() << "), B is const start..." << ENDL;
  assert(a.size() != 0 && b.size() != 0);
  if (c.size() == 0)
    c.resize(a.rows(), b.cols());
  
  SimpleTimer matmul_proof_timer;
  ZkMatrix& C_tilde = c;
  ZkMatrix C_prime(c.rows(), c.cols());

  ZkU64Matrix b_fix(b.rows(), b.cols());
  zk_encode_fp(b.data(), b.rows() * b.cols(), (uint64_t*)b_fix.data());

  zk_eigen_const_matmul(a, b_fix, C_prime);//[C_prime] = [a] * b
  if (ZK_IS_ALICE) {
    log_debug<< "step1. Prover decodes and gets float plaintext, and Fp plaintext...";
    DoubleMatrix a_tilde(a.rows(), a.cols());
    DoubleMatrix b_tilde(b.rows(), b.cols());//(b.rows(), b.cols());

    // zk to float
    zk_decode(a.data(), a.rows() * a.cols(), a_tilde.data());
    // Note[GeorgeShi]: We must use the re-decoded float values (with truncation error) rather than the original ones. 
    zk_decode((uint64_t *)b_fix.data(), (double*)b_tilde.data(), b.rows() * b.cols());
    log_debug << "step2. Prover computes plaintext c_prime, c_tilde... ";
    DoubleMatrix c_tilde_f = a_tilde * b_tilde;
    
    log_debug << "step3. Prover/Verifier private_input c_prime, c_tilde to C_prime and C_tilde...";
    U64Matrix c_fix(a.rows(), b.cols());
    ZkIntFp::zk_fp_encode(c_tilde_f.data(), c_fix.data(), c_tilde_f.size());
    batch_feed((IntFp*)C_tilde.data(), c_fix.data(), c_fix.size());
  }//Prover
  else {//Verifier
    log_debug << "step3. Prover/Verifier private_input(c_tilde), [c_prime] = private_input(c_prime), [c_prime_prime] = [c_prime] * 2^16...";
    batch_feed((IntFp*)C_tilde.data(), nullptr, c.size());
  }

#if ZK_INNER_CHECK
  log_debug << "step4. each element do batch multiplication check...";
  ZkMatrix C_tilde_prime(C_tilde.rows(), C_tilde.cols());
  zk_eigen_const_mul(C_tilde, (uint64_t)(1ULL << ZK_FLOAT_SIZE), C_tilde_prime);// [C_prime_prime] = [C_prime] * 2^16

  int ret = _inner_proof_and_check(C_tilde_prime.data(), C_prime.data(), C_prime.size()); // , (1ULL << 2*ZK_FLOAT_SIZE));
  log_debug << "step4. each element do batch multiplication ok, check: " << (ret == 0);
#endif

  log_debug << "party: " << zk_party_id << " : mystique_matmul_fp_with_proof (B is const) ok.";
  return 0;
}

// matmul proof with right operhand constant
static inline int mystique_matmul_proof_r_const(const ZkMatrix& a, const DoubleMatrix& b, ZkMatrix& c, bool need_truncate=true) {
  log_debug << "party: " << zk_party_id << " : mystique_matmul_fp_with_proof, (" 
          << a.rows() << "," << a.cols() << ") * (" 
          << b.rows() << "," << b.cols() << "), B is const start..." << ENDL;
  assert(a.size() != 0 && b.size() != 0);
  if (c.size() == 0)
    c.resize(a.rows(), b.cols());
  
  SimpleTimer matmul_proof_timer;
  ZkMatrix& C_prime = c;

  ZkU64Matrix b_fix(b.rows(), b.cols());
  zk_encode_fp(b.data(), b.rows() * b.cols(), (uint64_t*)b_fix.data());

  zk_eigen_const_matmul(a, b_fix, C_prime);//[C_prime] = [a] * b

  // TODO: truncation to Fp or just encode as emp::Float
  if (need_truncate) {
    mystique_truncate(C_prime.data(), C_prime.size());
  }

  log_debug << "party: " << zk_party_id << " : mystique_matmul_fp_with_proof (B is const) ok.";
  return 0;
}

// matmul proof (not private_input float encoded a*b )
static inline int mystique_matmul_proof(const ZkMatrix& a, const ZkMatrix& b, ZkMatrix& c, bool need_truncate=true) {
  log_debug << "party: " << zk_party_id << " : mystique_matmul_fp_with_proof, (" << a.rows() << "," << a.cols() << ") * (" << b.rows() << "," << b.cols() << "), start..." << ENDL;
  assert(a.size() != 0 && b.size() != 0);
  if (c.size() == 0)
    c.resize(a.rows(), b.cols());
  
  ZkMatrix& C_prime = c;
  static std::atomic<bool> is_first_call{true};
  static emp::block seed_block{0};
  static RttPRG prg;
  
  if (ZK_IS_ALICE) {//Prover
    log_debug << "Prover decodes and gets float plaintext, and Fp plaintext...";
    ZkU64Matrix a_prime(a.rows(), a.cols()), b_prime(b.rows(), b.cols());
    zk_decode(a.data(), a.rows() * a.cols(), (uint64_t*)a_prime.data());
    zk_decode(b.data(), b.rows() * b.cols(), (uint64_t*)b_prime.data());

    log_debug << "Prover computes plaintext c_prime, c_tilde... ";
    ZkU64Matrix c_prime = a_prime * b_prime;    
    log_debug << "Prover/Verifier private_input c_tilde then get C_tilde...";
    batch_feed((IntFp*)C_prime.data(), (uint64_t*)c_prime.data(), c_prime.size());
    if (is_first_call) {
      // Verifier generates seed and send to Prover;
      log_debug << "step4. Prover recv seed from Verifier...";
      sync_zk_bool<BoolIO<ZKNetIO>>();
      ((ZKFpExecPrv<BoolIO<ZKNetIO>>*)(ZKFpExec::zk_exec))->io->recv_data(&seed_block, sizeof(seed_block));
    }
  }//Prover
  else {//Verifier
    log_debug << "Prover/Verifier private_input [c_prime] = private_input(c_prime) ...";
    batch_feed((IntFp*)C_prime.data(), nullptr, C_prime.size());
    if (is_first_call) {
      // Verifier generates seed and send to Prover;
      log_debug << "Verifier generates seed and send to Prover...";
      prg.randomDatas(&seed_block, sizeof(seed_block));
      ((ZKFpExecPrv<BoolIO<ZKNetIO>>*)(ZKFpExec::zk_exec))->io->send_data(&seed_block, sizeof(seed_block));
      sync_zk_bool<BoolIO<ZKNetIO>>();
    }
  }

  // TODO: in the near future we may execute following step for \roh /log_p times, here we set it 1
  log_debug << "Verifier/Prover chooses random vectors u, v...";
  if (is_first_call) {
    prg.reseed(&seed_block);
    is_first_call = false;
  }
  // // eg. A_tilde(m,k), B_tilde(k,n), u(1, m) and v(n,1), then x*y=t(1,1)
  U64Matrix u(1, a.rows()), v(b.cols(), 1);

  // random F_p element u, v derived from random 128-bit blocks source
  size_t u_size = std::ceil(a.rows()/(2.0));
  size_t v_size = std::ceil(b.cols()/(2.0));
  vector<emp::block> u_rds(u_size), v_rds(v_size);
  // public input u, v
  for (size_t i = 0; i < a.rows(); ++i)
  {
    u(0, i) = prg.getFpBits(PR, true);
  }
  for (size_t i = 0; i < b.cols(); ++i)
  {
    v(i, 0) = prg.getFpBits(PR, true);
  }

  log_debug << "Prover and Verifier computes [x] = u*[A] and [y] = [B]*v in F_p , then compute [t]=[x] *[y]";
  ZkMatrix x(1, a.cols());
  // local compute: x = u * a
  zk_eigen_const_matmul(u, a, x);

  // local compute: y = b * v
  ZkMatrix y(b.rows(), 1);
  zk_eigen_const_matmul(b, v, y);

#if !USE_INNER_PRODUCT_OPT
  ZkMatrix t = x * y;// zk multiplication, output (1,1)
#else
  ZkMatrix t(1,1);
  zk_eigen_matmul_with_inner_prdt_opt(x, y, t);
#endif

  log_debug << "Prover/Verifier reveal [t] - u* [tilde_C] * v, and check it is 0 or not ...";
  ZkMatrix temp1(u.rows(), C_prime.cols());
  ZkMatrix temp2(temp1.rows(), v.cols());
  zk_eigen_const_matmul(u, C_prime, temp1);
  zk_eigen_const_matmul(temp1, v, temp2);
  ZkMatrix result = t - temp2; // actually t has shape (1,1)
  log_debug << "mystique_matmul_fp_with_proof, after [t] - u* [tilde_C] * v, " << zk_party_id << ": " << result(0,0);

  bool status = batch_reveal_check_zero((IntFp*)result.data(), result.size());
  log_debug << "Prover/Verifier reveal [t] - u* [tilde_C] * v ok, and check:  " << status;

  // TODO: truncation to Fp or just encode as emp::Float
  if (need_truncate) {
    mystique_truncate(C_prime.data(), C_prime.size());
  }
  
  log_info << "party: " << zk_party_id << "mystique_matmul_fp_with_proof ok.";
  return 0;
}

}//zk
}//rosetta