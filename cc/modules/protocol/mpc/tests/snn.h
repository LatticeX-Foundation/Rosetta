#pragma once

#include "tools.h"
#include "logger.h"
#include "model_tool.h"

#include "mpc.h"

#include <algorithm>
#include <thread>
using namespace std;

#define output_function() cout << __FUNCTION__ << endl

namespace {
// debug helper
// r --> reconstruct
static inline void test_debug_print(
  std::shared_ptr<rosetta::mpc::Reconstruct2PC> oprec, vector<mpc_t>& vo, size_t size,
  string s = "", bool r = true) {
  // print mpc_t and double
  cout << "---------------- " << s << endl;
  print_vec(vo, size, "original");

  vector<mpc_t> vt(size);
  if (r) {
    if (PRIMARY) {
      oprec->Run(vo, size, vt);
    }
    print_vec(vt, size, "reconstruct");
  } else {
    vt.assign(vo.begin(), vo.end());
  }

  vector<double> vd;
  convert_mytype_to_double(vt, vd);
  print_vec(vd, size, "double");
  cout << "----------------" << endl;
}
} // namespace

///////////////
///////////////
///////////////
///////////////

// show usages

namespace rosetta {
namespace mpc {
namespace debug {

// show usages
void usage();

void testMyType();
void testTypeConvert();
void testEachOpComm();
void testPRZS();
void testPrivateInput();

// clang-format off
/*
** 2PC
*/


/*
** MPC (Basic)
*/
void debugDotProd();
void debugDivision();
void debugDivisionV2();
void debugComputeMSB();
void debugPC();
void debugSS();
void testBinaryOps(int thread_nums = 1);
void testMulThreads(int thread_nums = 1);

/*
** MatMul
*/
void debugMatMul();
void debugMatMul_tp();
void testMatMul();
void testConvolution();
void testMatMulThreads(int thread_nums = 1);

/*
** MaxPool
*/
void debugMax();
void debugMaxIndex();
void testMaxPool(size_t p_range, size_t q_range, size_t px, size_t py, size_t D, size_t iter);
void testMaxPoolDerivative(size_t p_range, size_t q_range, size_t px, size_t py, size_t D, size_t iter);

/*
** Log & Pow const
*/
void debug_LOG();
void debug_mpc_power();

void debug_Mix_OP();

/*
** Relu
*/
void debugReluPrime();
void testRelu();
void testReluPrime();

/*
** Sigmoid
*/
void debugPrivateCompEx(const vector<mpc_t> &a, const vector<mpc_t> &public_r, vector<mpc_t> &c);
void debugFastPow3();

void debugSigmoidG3(const vector<mpc_t> &a, vector<mpc_t> &reveal_sigmoid, bool use_fastPow3 = true);
void debugSigmoidG3Prime(const vector<mpc_t> &a, vector<mpc_t> &public_b);
void debugSigmoidPieceWise(const vector<mpc_t> &a, vector<mpc_t> &reveal_sigmoid);
void debugSigmoidAliPieceWise(const vector<mpc_t>& a, vector<mpc_t>& reveal_sigmoid);
void testSigmoidG3();
void testSigmoidG3Prime();
void testSigmoidPieceWise();
void testSigmoidAliPieceWise();
void testSigmoidPieceWise2();

/*
  Neural Network functionalities
*/
void debug_NN();


void testPow2();

void testMatmul2();
void testSigmoid2();
void testMax2();
void testMean2();
void testRelu2();

void testMSB2();
void testReluPrime2();
void testShareConvert2();

// clang-format on
} // namespace debug
} // namespace mpc
} // namespace rosetta