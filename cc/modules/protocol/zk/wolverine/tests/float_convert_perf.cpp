#include "wvr_test.h"

#include <iostream>
using namespace std;

#include "cc/modules/protocol/zk/wolverine/include/wolverine_internal.h"
using namespace rosetta::zk;

void run(int partyid) {
  WVR_PROTOCOL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  int SCALE = 16;

  string separation_line = "----------------------------------------------------------------------";
  {
    Float f1(1.23456789, ALICE);
    cout << "f1:1.23456789 reveal(ALICE):" << f1.reveal<double>(ALICE) << endl;
    cout << "f1:1.23456789 reveal(BOB):" << f1.reveal<double>(BOB) << endl;
    cout << "f1:1.23456789 reveal(PUBLIC):" << f1.reveal<double>(PUBLIC) << endl;
    cout << separation_line << endl;
  }
  {
    Float f1(1.23456789, BOB);
    cout << "f1:1.23456789 reveal(ALICE):" << f1.reveal<double>(ALICE) << endl;
    cout << "f1:1.23456789 reveal(BOB):" << f1.reveal<double>(BOB) << endl;
    cout << "f1:1.23456789 reveal(PUBLIC):" << f1.reveal<double>(PUBLIC) << endl;
    cout << separation_line << endl;
  }
  {
    Float f1(1.23456789, ALICE);
    cout << FloatToInt62(f1, SCALE).reveal<uint64_t>(PUBLIC) << endl;
    cout << separation_line << endl;
  }
  {
    int64_t size = 100000;
    SimpleTimer timer;
    for (int64_t i = 0; i < size; i++) {
      Integer a(62, 12, ALICE);
      Int62ToFloat(a, SCALE);
    }
    double elpased = timer.elapse();
    cout << "Int62ToFloat --> size:" << size << ", elapse(s):" << elpased
         << ", avg(s):" << elpased / size << endl;
    cout << separation_line << endl;
  }
  {
    int64_t size = 100000;
    SimpleTimer timer;
    for (int64_t i = 0; i < size; i++) {
      Float f1(1.23456789, ALICE);
      FloatToInt62(f1, SCALE);
    }
    double elpased = timer.elapse();
    cout << "FloatToInt62 --> size:" << size << ", elapse(s):" << elpased
         << ", avg(s):" << elpased / size << endl;
    cout << separation_line << endl;
  }
  {
    int64_t size = 100000;
    SimpleTimer timer;
    Integer a(62, 12, ALICE);
    for (int64_t i = 0; i < size; i++) {
      Int62ToFloat(a, SCALE);
    }
    double elpased = timer.elapse();
    cout << "Int62ToFloat --> size:" << size << ", elapse(s):" << elpased
         << ", avg(s):" << elpased / size << endl;
    cout << separation_line << endl;
  }
  {
    int64_t size = 100000;
    SimpleTimer timer;
    Float f1(1.23456789, ALICE);
    for (int64_t i = 0; i < size; i++) {
      FloatToInt62(f1, SCALE);
    }
    double elpased = timer.elapse();
    cout << "FloatToInt62 --> size:" << size << ", elapse(s):" << elpased
         << ", avg(s):" << elpased / size << endl;
    cout << separation_line << endl;
  }

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  WVR_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_ZK_TEST(run);
