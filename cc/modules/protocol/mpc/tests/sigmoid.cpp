#include "snn.h"

namespace rosetta {
namespace mpc {
namespace debug {

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
#ifdef OKOKOOOOOOOOOOOOOOOOOOK
void debugPrivateCompEx(const vector<mpc_t>& a, const vector<mpc_t>& public_r, vector<mpc_t>& c) {
  size_t size = a.size();

  vector<mpc_t> relu_prime(size);
  funcPrivateCompareMPCEx(a, public_r, relu_prime, size);

  if (PRIMARY) {
    funcReconstruct2PC(relu_prime, size, c);
  }
}

void debugFastPow3() {
  vector<mpc_t> a{2};
  size_t size = a.size();
  vector<mpc_t> cube(size);
  funcFastPow3MPC(a, cube, size);

  if (PRIMARY) {
    funcReconstruct2PC(cube, size, "c");
  }
}
#endif

void debugSigmoidG3(const vector<mpc_t>& a, vector<mpc_t>& reveal_sigmoid, bool use_fastPow3) {
  auto op = GetMpcOpDefault(Sigmoid);
  auto oprec = GetMpcOpWithKey(Reconstruct2PC, op->msg_id());

  size_t size = a.size();

  vector<mpc_t> sigmoid = reveal_sigmoid;
  op->RunG3(a, sigmoid, size, use_fastPow3);

  if (PRIMARY) {
    oprec->Run(sigmoid, size, reveal_sigmoid);
  }
}

void debugSigmoidG3Prime(const vector<mpc_t>& a, vector<mpc_t>& public_b) {
  auto op = GetMpcOpDefault(Sigmoid);
  auto oprec = GetMpcOpWithKey(Reconstruct2PC, op->msg_id());

  size_t size = a.size();

  vector<mpc_t> b(size);
  op->RunG3Prime(a, b, size);

  if (PRIMARY) {
    oprec->Run(b, size, public_b);
  }
}

void debugSigmoidPieceWise(const vector<mpc_t>& a, vector<mpc_t>& reveal_sigmoid) {
  auto op = GetMpcOpDefault(Sigmoid);
  auto oprec = GetMpcOpWithKey(Reconstruct2PC, op->msg_id());

  size_t size = a.size();
  vector<mpc_t> b(size);
  op->RunPieceWise(a, b, size);

  if (PRIMARY) {
    oprec->Run(b, size, reveal_sigmoid);
  }
}

void debugSigmoidAliPieceWise(const vector<mpc_t>& a, vector<mpc_t>& reveal_sigmoid) {
  auto op = GetMpcOpDefault(Sigmoid);
  auto oprec = GetMpcOpWithKey(Reconstruct2PC, op->msg_id());

  size_t size = a.size();
  vector<mpc_t> b(size);
  op->RunAliPieceWise(a, b, size);

  if (PRIMARY) {
    oprec->Run(b, size, reveal_sigmoid);
  }
}

void testSigmoidG3() {
  vector<mpc_t> a = {1234, 5678, 9012};
  vector<mpc_t> b;
  debugSigmoidG3Prime(a, b);
}
void testSigmoidG3Prime() {
  vector<mpc_t> a = {1234, 5678, 9012};
  vector<mpc_t> b;
  debugSigmoidG3Prime(a, b);
}
void testSigmoidPieceWise() {
  vector<mpc_t> a = {1234, 5678, 9012};
  vector<mpc_t> b;
  debugSigmoidPieceWise(a, b);
}
void testSigmoidAliPieceWise() {
  vector<mpc_t> a = {1234, 5678, 9012};
  vector<mpc_t> b;
  debugSigmoidAliPieceWise(a, b);
}
} // namespace debug
} // namespace mpc
} // namespace rosetta