#include "snn.h"

namespace rosetta {
namespace mpc {
namespace debug {

void testMyType() {
  vector<mpc_t> a = {0};
  if (partyNum == PARTY_A) {
    cout << "aaa:0: 1 << FLOAT_PRECISION" << 1 << FLOAT_PRECISION << endl;
    cout << "aaa:0: (-1.0) * (1 << FLOAT_PRECISION)" << (-1.0) * (1 << FLOAT_PRECISION) << endl;
    cout << "aaa:0: ((mpc_t)((-1.0) * (1 << FLOAT_PRECISION)))"
         << ((mpc_t)((-1.0) * (1 << FLOAT_PRECISION))) << endl;
    cout << "aaa:0: ((mpc_t)(signed_mpc_t)((-132.0) * (1 << FLOAT_PRECISION))) "
         << ((mpc_t)(signed_mpc_t)((-132.0) * (1 << FLOAT_PRECISION))) << endl;
    cout << "aaa:0: ((mpc_t)(signed_mpc_t)((1.0) * (1 << FLOAT_PRECISION))) "
         << ((mpc_t)(signed_mpc_t)((1.0) * (1 << FLOAT_PRECISION))) << endl;
  }

  if (partyNum == PARTY_A) {
    a[0] = FloatToMpcType(-1.0);
    cout << "aaa:0: " << a[0] << endl;
  }

  if (partyNum == PARTY_B) {
    a[0] = FloatToMpcType(-2.0);
    cout << "aaa:1:" << a[0] << endl;
  }
  auto oprec = GetMpcOpDefault(Reconstruct2PC);
  oprec->Run(a, 1, "a"); // -3
}

mpc_t FloatToMpcType3(double d) {
  int64_t i = d;
  double dd = d - i;

  i = i << FLOAT_PRECISION;
  i += (int64_t)(dd * (1 << FLOAT_PRECISION));
  mpc_t t = (mpc_t)i;
  return t;
}

template <typename T>
void print_hex(T d) {
  cout << "------:" << d << " hex:" << ios::hex << d << " mem:";
  char* p = (char*)&d;
  for (int i = 0; i < sizeof(d); i++) {
    printf("%02x ", p[i] & 0xFF);
  }
  cout << endl;
};
void testTypeConvert() {
  {
    cout.setf(ios::fixed);
    {
      union mpc_f {
        double d;
        mpc_t t;
      };

      mpc_f a;
      a.d = 0.0012;
      print_hex(a.d);
      print_hex(a.t);

      mpc_t t = FloatToMpcTypeBC(a.d);
      print_hex(t);

      double d = MpcTypeToFloatBC(t);
      print_hex(d);

      cout.unsetf(ios::fixed);
    }
  }

  cout << FLOAT_PRECISION << endl;
  cout << FLOAT_PRECISION << endl;
  output_function();
  cout.setf(ios::fixed);
  {
    // diff: 1 >> 13
    double d1 = 0.0001220703125; // 1 >> 13
    mpc_t dd1 = FloatToMpcType(d1);
    cout << "d1:" << d1 << endl;
    cout << "dd1:" << dd1 << endl;

    double d2 = 0.000244140625; // 2 >> 13
    mpc_t dd2 = FloatToMpcType(d2);
    cout << "d2:" << d2 << endl;
    cout << "dd2:" << dd2 << endl;

    double d = d2 - d1;
    mpc_t dd = dd2 - dd1;
    cout << "d:" << d << endl;
    cout << "dd:" << dd << endl;
    /*
    d1:0.00012207
    dd1:1
    d2:0.000244141
    dd2:2
    d:0.00012207
    dd:1
    */
  }
  cout << "--------------" << endl;
  {
    double d1 = 1377744569001469.0;
    mpc_t dd1 = FloatToMpcType(d1);
    cout << "d1:" << d1 << endl;
    cout << "dd1:" << dd1 << endl;

    double d2 = 1377744569001469.2;
    mpc_t dd2 = FloatToMpcType(d2);
    cout << "d2:" << d2 << endl;
    cout << "dd2:" << dd2 << endl;

    double d = d2 - d1;
    mpc_t dd = dd2 - dd1;
    cout << "d:" << d << endl;
    cout << "dd:" << dd << endl;
    /*
    d1:1377744569001469.000000
    dd1:11286483509260034048
    d2:1377744569001469.250000
    dd2:11286483509260036096
    d:0.250000
    dd:2048
    */
  }
  cout << "--------------" << endl;
  cout << std::numeric_limits<double>::max() << endl;
  mpc_t t = 1;
  t <<= 52;
  cout << t << endl;
  {
    double d1 = 1377744569001469.0;
    mpc_t dd1 = FloatToMpcType(d1);
    cout << "d1:" << d1 << endl;
    cout << "dd1:" << dd1 << endl;

    double d2 = 1377744569001469.13;
    mpc_t dd2 = FloatToMpcType(d2);
    cout << "d2:" << d2 << endl;
    cout << "dd2:" << dd2 << endl;

    double d = d2 - d1;
    mpc_t dd = dd2 - dd1;
    cout << "d:" << d << endl;
    cout << "dd:" << dd << endl;
    /*
    d1:1377744569001469.000000
    dd1:11286483509260034048
    d2:1377744569001469.250000
    dd2:11286483509260036096
    d:0.250000
    dd:2048
    */
  }
  cout.unsetf(ios::fixed);
}
} // namespace debug
} // namespace mpc
} // namespace rosetta
