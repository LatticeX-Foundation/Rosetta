#include "snn.h"

namespace rosetta {
namespace mpc {
namespace debug {

void testPrivateInput() {
  auto oppi = GetMpcOpDefault(PrivateInput);
  auto oprec = GetMpcOpWithKey(Reconstruct2PC, oppi->msg_id());
  auto opadd = GetMpcOpWithKey(Add, oppi->msg_id());

  vector<double> v1 = {
    -3.2, -1.0, 1.2, 3.4, 5.6,
  };
  vector<double> v2 = {
    -1.2, -1.0, 1.2, 1.4, 1.6,
  };
  int size = v1.size();
  {
    for (int i = 0; i < size; i++) {
      mpc_t t1, t2;
      oppi->Run(PARTY_A, v1[i], t1);
      oppi->Run(PARTY_B, v2[i], t2);
      cout << "oppi mpc_t i: " << i << " " << t1 << " " << t2 << endl;
    }
  }
  {
    for (int i = 0; i < size; i++) {
      double t1, t2;
      oppi->Run(PARTY_A, v1[i], t1);
      oppi->Run(PARTY_B, v2[i], t2);
      cout << "oppi double i: " << i << " " << t1 << " " << t2 << endl;
    }
  }
  {
    vector<mpc_t> t1(size), t2(size);
    oppi->Run(PARTY_A, v1, t1);
    oppi->Run(PARTY_B, v2, t2);
    print_vec(t1, 8, "oppi mpc_t vector t1");
    print_vec(t2, 8, "oppi mpc_t vector t2");

    vector<mpc_t> tt1(size), tt2(size);
    oprec->Run(t1, size, tt1, PARTY_A);
    oprec->Run(t2, size, tt2, PARTY_A);
    print_vec(tt1, 8, "oppi mpc_t vector tt1");
    print_vec(tt2, 8, "oppi mpc_t vector tt2");

    vector<mpc_t> t(size);
    opadd->Run(t1, t2, t, size);
    vector<mpc_t> rt(size);
    oprec->Run(t, size, rt, PARTY_A);
    print_vec(rt, 8, "oppi mpc_t vector rt(reveal)");

    vector<double> rv(size);
    convert_mpctype_to_double(rt, rv);
    print_vec(rv, 8, "oppi mpc_t vector rv(reveal)");
  }
  {
    vector<double> t1(size), t2(size);
    oppi->Run(PARTY_A, v1, t1);
    oppi->Run(PARTY_B, v2, t2);
    print_vec(t1, 8, "oppi double vector t1");
    print_vec(t2, 8, "oppi double vector t2");
  }
}

} // namespace debug
} // namespace mpc
} // namespace rosetta
