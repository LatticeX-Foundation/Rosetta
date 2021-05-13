#include "helix__test.h"

void run(int partyid) {
  HELIX_PROTOCOL_INTERNAL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////

#define _TEST_PRF(f) \
  f(A, size);        \
  print_vec(A, size, #f)

  // todo here
  {
    vector<mpc_t> A;
    size_t size = 5;
    _TEST_PRF(PRF01);
    _TEST_PRF(PRF02);
    _TEST_PRF(PRF12);
    _TEST_PRF(PRF0);
    _TEST_PRF(PRF1);
    _TEST_PRF(PRF2);
    _TEST_PRF(PRF);
  }
  {
    vector<bit_t> A;
    size_t size = 5;
    _TEST_PRF(PRF01);
    _TEST_PRF(PRF02);
    _TEST_PRF(PRF12);
    _TEST_PRF(PRF0);
    _TEST_PRF(PRF1);
    _TEST_PRF(PRF2);
    _TEST_PRF(PRF);
  }
  {
    test_prg();
    // test_prg();
  }

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  HELIX_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);