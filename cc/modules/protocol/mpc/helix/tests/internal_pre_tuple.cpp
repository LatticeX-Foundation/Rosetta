#include "helix__test.h"

void run(int partyid) {
  HELIX_PROTOCOL_INTERNAL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////

  {
    // PreTuple
    size_t size = 5;
    vector<Share> aX;
    vector<BitShare> bX;

    hi->beg_statistics();
    hi->PreTuple(aX, bX, size);
    hi->end_statistics("RTT PreTuple(k=" + to_string(size) + "):");

    hi->RevealAndPrint2(aX, "aX:");
    hi->RevealAndPrint(bX, "bX:");

    // check
    if (hi->is_helper()) {
      for (int i = 0; i < size; i++) {
        //cout << (aX[i].s0.A0 + aX[i].s1.A1 == bX[i].s0.A0 ^ bX[i].s1.A1) << endl;
        cout << to_readable_dec(aX[i].s0.A0) << " " << to_readable_dec(aX[i].s1.A1) << " " << to_string(bX[i].s0.A0) << " "
             << to_string(bX[i].s1.A1) << " ---> " << to_readable_dec(aX[i].s0.A0 + aX[i].s1.A1) << " "
             << to_string(bX[i].s0.A0 ^ bX[i].s1.A1) << endl;
        assert((aX[i].s0.A0 + aX[i].s1.A1 == bX[i].s0.A0 ^ bX[i].s1.A1));
      }
    }
  }
  cout << "------------------------------------" << endl;
  {
    // PreTupleA
    vector<mpc_t> y = {2, 3, 4, 5, 6, 1, 0, 2};
    size_t size = y.size();
    vector<Share> Y;
    hi->Input(node_id_2, y, Y);

    vector<Share> aX;
    vector<BitShare> bX;

    hi->beg_statistics();
    hi->PreTupleA(Y, aX, bX);
    hi->end_statistics("RTT PreTupleA(k=" + to_string(size) + "):");

    hi->RevealAndPrint2(aX, "aX:");
    hi->RevealAndPrint(bX, "bX:");

    // check
    if (hi->is_helper()) {
      for (int i = 0; i < size; i++) {
        //cout << (aX[i].s0.A0 + aX[i].s1.A1 == bX[i].s0.A0 ^ bX[i].s1.A1) << endl;
        cout << to_readable_dec(aX[i].s0.A0) << " " << to_readable_dec(aX[i].s1.A1) 
            << " " << to_string(bX[i].s0.A0) << " "
            << to_string(bX[i].s1.A1) << " ---> " << to_readable_dec(aX[i].s0.A0 + aX[i].s1.A1) << " "
            << to_string(bX[i].s0.A0 ^ bX[i].s1.A1) << endl;
        //assert((aX[i].s0.A0 + aX[i].s1.A1 == bX[i].s0.A0 ^ bX[i].s1.A1));
      }
    }
  }

#if PERFORMANCE_TEST
  {
    // PreTuple
    size_t size = 13579;

    vector<Share> aX;
    vector<BitShare> bX;

    hi->beg_statistics();
    hi->PreTuple(aX, bX, size);
    hi->end_statistics("PERF-RTT PreTuple(k=" + to_string(size) + "):");
  }

  {
    // PreTupleA
    size_t size = 1357;

    vector<double> y;
    random_vector(y, size);
    vector<Share> Y;
    hi->Input(node_id_2, y, Y);

    vector<Share> aX;
    vector<BitShare> bX;

    hi->beg_statistics();
    hi->PreTupleA(Y, aX, bX);
    hi->end_statistics("PERF-RTT PreTupleA(k=" + to_string(size) + "):");
  }
#endif

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  HELIX_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);