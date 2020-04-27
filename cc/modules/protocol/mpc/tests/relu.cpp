#include "snn.h"

namespace rosetta {
namespace mpc {
namespace debug {

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void debugReluPrime() {
  LOGI("debugReluPrime beg");
  auto op = GetMpcOpDefault(ReluPrime);
  auto oprec = GetMpcOpWithKey(Reconstruct2PC, op->msg_id());

  size_t size = 10;
  vector<mpc_t> inputs(size, 0);

  if (partyNum == PARTY_A) {
    op->populateRandomVector(inputs, size, "INDEP", "POSITIVE");
    //for (size_t i = 0; i < size; ++i)
    //  inputs[i] = aes_indep->get8Bits() - aes_indep->get8Bits();
  }

  if (THREE_PC) {
    vector<mpc_t> outputs(size, 0);
    op->Run3PC(inputs, outputs, size);
    if (PRIMARY) {
      oprec->Run(inputs, size, "inputs");
      oprec->Run(outputs, size, "outputs");
    }
  }

  LOGI("debugReluPrime end");
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void testRelu(size_t r, size_t c, size_t iter) {
  auto op = GetMpcOpDefault(SelectShares);
  auto oprec = GetMpcOpWithKey(Reconstruct2PC, op->msg_id());

  vector<mpc_t> a(r * c, 1);
  vector<small_mpc_t> reluPrimeSmall(r * c, 1);
  vector<mpc_t> reluPrimeLarge(r * c, 1);
  vector<mpc_t> b(r * c, 0);

  for (int runs = 0; runs < iter; ++runs) {
    if (STANDALONE)
      for (size_t i = 0; i < r * c; ++i)
        b[i] = a[i] * reluPrimeSmall[i];

    //if (FOUR_PC)
    //  funcSelectShares4PC(a, reluPrimeSmall, b, r * c);

    if (THREE_PC)
      op->Run3PC(a, reluPrimeLarge, b, r * c);
  }
}

void testReluPrime(size_t r, size_t c, size_t iter) {
  auto op = GetMpcOpDefault(ReluPrime);
  auto oprec = GetMpcOpWithKey(Reconstruct2PC, op->msg_id());

  vector<mpc_t> a(r * c, 1);
  vector<mpc_t> b(r * c, 0);
  vector<small_mpc_t> d(r * c, 0);

  for (int runs = 0; runs < iter; ++runs) {
    if (STANDALONE)
      for (size_t i = 0; i < r * c; ++i)
        b[i] = (a[i] < LARGEST_NEG ? 1 : 0);

    if (THREE_PC)
      op->Run3PC(a, b, r * c);

    //if (FOUR_PC)
    //  funcRELUPrime4PC(a, d, r * c);
  }
}

void testRelu() {
  size_t iterations = 10;
  testRelu(128, 128, iterations);
  testRelu(576, 20, iterations);
  testRelu(64, 16, iterations);
}

void testReluPrime() {
  size_t iterations = 10;
  testReluPrime(128, 128, iterations);
  testReluPrime(576, 20, iterations);
  testReluPrime(64, 16, iterations);
}
} // namespace debug
} // namespace mpc
} // namespace rosetta