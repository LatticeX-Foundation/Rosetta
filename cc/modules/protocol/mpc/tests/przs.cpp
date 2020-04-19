#include "snn.h"

namespace rosetta {
namespace mpc {
namespace debug {

void testPRZS() {
  auto przs = GetMpcOpDefault(PRZS);
  auto oprec = GetMpcOpWithKey(Reconstruct2PC, przs->msg_id());

  {
    for (int i = 0; i < 3; i++) {
      mpc_t t;
      przs->Run(PARTY_A, PARTY_B, t);
      cout << "przs mpc_t i: " << i << " " << t << endl;
    }
  }
  {
    for (int i = 0; i < 3; i++) {
      double t;
      przs->Run(PARTY_A, PARTY_B, t);
      cout << "przs double i: " << i << " " << t << endl;
    }
  }
  {
    vector<mpc_t> t(3);
    przs->Run(PARTY_A, PARTY_B, t);
    print_vec(t, 8, "przs mpc_t vector");
    vector<mpc_t> tt(3);
    oprec->Run(t, 3, tt, PARTY_A);
    print_vec(tt, 8, "przs mpc_t vector tt");
  }
  {
    vector<double> t(3);
    przs->Run(PARTY_A, PARTY_B, t);
    print_vec(t, 8, "przs double vector");
  }
}

} // namespace debug
} // namespace mpc
} // namespace rosetta
