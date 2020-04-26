#include "snn.h"

namespace rosetta {
namespace mpc {
namespace debug {

template <typename T>
void print_hex(T d) {
  cout << "------:" << d << " hex:" << ios::hex << d << " mem:";
  char* p = (char*)&d;
  for (int i = 0; i < sizeof(d); i++) {
    printf("%02x ", p[i] & 0xFF);
  }
  cout << endl;
};

template <typename T>
string get_hex_str(T d) {
  char buf[100] = {0};
  char* p = (char*)&d;
  string s;
  for (int i = 0; i < sizeof(d); i++) {
    sprintf(buf, "%02x", p[i] & 0xFF);
    s.append(buf);
  }
  return s;
};

namespace {

union mpc_f {
  double d;
  mpc_t t;
};
static inline mpc_t FloatToMpcTypeBitC(double d) {
  mpc_f f;
  f.d = d;
  return f.t;
}
static inline double MpcTypeToFloatBitC(mpc_t t) {
  mpc_f f;
  f.t = t;
  return f.d;
}

#define MpcTypeToFloatCast(a) (double((signed_mpc_t)(a)))
#define FloatToMpcTypeCast(a) ((mpc_t)(((signed_mpc_t)(a))))
} // namespace

void testCast() {
  auto opadd = GetMpcOpWithKey(Add, "cccccccccccccccccccccccccast");
  auto prii = GetMpcOpWithKey(PrivateInput, opadd->msg_id());
  auto oprec = GetMpcOpWithKey(Reconstruct2PC, opadd->msg_id());

  cout << setprecision(20);
  auto simulate_mpc_op = [&](double d1, double d2) -> double {
    mpc_t t1 = FloatToMpcTypeCast(d1);
    mpc_t t2 = FloatToMpcTypeCast(d2);
    mpc_t xr;
    oprec->Run(t1, xr);
    cout << "cast t1 xr:" << xr << endl;
    oprec->Run(t2, xr);
    cout << "cast t2 xr:" << xr << endl;

    mpc_t t3;
    opadd->Run(t1, t2, t3);
    oprec->Run(t3, xr);
    cout << "cast t3 xr:" << xr << endl;

    double d3;
    d3 = MpcTypeToFloatCast(t3);
    return d3;
  };
  auto reveal_op = [&](double d) -> double {
    mpc_t t = FloatToMpcTypeCast(d);
    mpc_t r;
    oprec->Run(t, r);
    double dr = MpcTypeToFloat(r);
    return dr;
  };

  // values
  mpc_t xa, xb, xc, xr;
  prii->Run(PARTY_A, 1.2345, xa);
  prii->Run(PARTY_B, 5.4321, xb);
  prii->Run(PARTY_C, 2.2222, xc);
  oprec->Run(xa, xr);
  cout << "cast xa xr:" << xr << endl;
  oprec->Run(xb, xr);
  cout << "cast xb xr:" << xr << endl;
  oprec->Run(xc, xr);
  cout << "cast xc xr:" << xr << endl;

  cout << "cast bit info:" << endl;
  print_hex(xa);
  double da = MpcTypeToFloatCast(xa);
  print_hex(da);

  double db = MpcTypeToFloatCast(xb);
  double dc = MpcTypeToFloatCast(xc);

  double dab = simulate_mpc_op(da, db);
  double dd = simulate_mpc_op(dab, dc);

  double dr = reveal_op(dd);
  cout << "cast reveal_op dd:" << dr << endl;
}
void testBitCopy() {
  auto opadd = GetMpcOpWithKey(Add, "bbbbbbbbbbbbbbbbbbbbitcopy");
  auto prii = GetMpcOpWithKey(PrivateInput, opadd->msg_id());
  auto oprec = GetMpcOpWithKey(Reconstruct2PC, opadd->msg_id());

  cout << setprecision(20);
  auto simulate_mpc_op = [&](double d1, double d2) -> double {
    //cout << setprecision(20) << "cccccccccc bitcopy:" << i << endl;
    mpc_t t1 = FloatToMpcTypeBitC(d1);
    mpc_t t2 = FloatToMpcTypeBitC(d2);
    mpc_t xr;
    oprec->Run(t1, xr);
    cout << "bitcopy t1 xr:" << xr << endl;
    oprec->Run(t2, xr);
    cout << "bitcopy t2 xr:" << xr << endl;

    mpc_t t3;
    opadd->Run(t1, t2, t3);
    oprec->Run(t3, xr);
    cout << "bitcopy t3 xr:" << xr << endl;

    double d3;
    d3 = MpcTypeToFloatBitC(t3);
    return d3;
  };
  auto reveal_op = [&](double d) -> double {
    mpc_t t = FloatToMpcTypeCast(d);
    mpc_t r;
    oprec->Run(t, r);
    double dr = MpcTypeToFloat(r);
    return dr;
  };
  // values
  mpc_t xa, xb, xc, xr;
  prii->Run(PARTY_A, 1.2345, xa);
  prii->Run(PARTY_B, 5.4321, xb);
  prii->Run(PARTY_C, 2.2222, xc);
  oprec->Run(xa, xr);
  cout << "bitcopy xa xr:" << xr << endl;
  oprec->Run(xb, xr);
  cout << "bitcopy xb xr:" << xr << endl;
  oprec->Run(xc, xr);
  cout << "bitcopy xc xr:" << xr << endl;

  cout << "bitcopy bit info:" << endl;
  print_hex(xa);
  double da = MpcTypeToFloatBitC(xa);
  print_hex(da);

  double db = MpcTypeToFloatBitC(xb);
  double dc = MpcTypeToFloatBitC(xc);

  double dab = simulate_mpc_op(da, db);
  double dd = simulate_mpc_op(dab, dc);

  double dr = reveal_op(dd);
  cout << "bitcopy reveal_op dd:" << dr << endl;
}
void testComparision() {
  auto opadd = GetMpcOpWithKey(Add, "ccccccccccccccccccccccccccccomparision");
  auto prii = GetMpcOpWithKey(PrivateInput, opadd->msg_id());
  auto oprec = GetMpcOpWithKey(Reconstruct2PC, opadd->msg_id());

  vector<double> vd;
  vector<mpc_t> vt;
  //for (double d = -1; d < 1; d += 0.01) {
  for (int i = 0; i < 20; i++) {
    double d = (rand() % 100000) / 10000.0;
    if (rand() % 2 == 0)
      d = d * -1;
    vd.push_back(d);
    mpc_t t;
    prii->Run(rand() % 3, d, t);
    vt.push_back(t);
  }
  int size = vd.size();

  for (int i = 0; i < size; i++) {
    cout << "===================================:" << i << endl;
    mpc_t ss = vt[i];
    mpc_t xr;
    oprec->Run(ss, xr);
    string org_mem = get_hex_str(ss);

    // bitcopy schema
    double d1 = MpcTypeToFloatBitC(ss);
    string bitcopy_mem = get_hex_str(d1);
    mpc_t t1 = FloatToMpcTypeBitC(d1);
    mpc_t xr1;
    oprec->Run(t1, xr1);
    double d1r = MpcTypeToFloat(xr1);

    // cast schema
    double d2 = MpcTypeToFloatCast(ss);
    string cast_mem = get_hex_str(d2);
    mpc_t t2 = FloatToMpcTypeCast(d2);
    mpc_t xr2;
    oprec->Run(t2, xr2);
    double d2r = MpcTypeToFloat(xr2);

    // clang-format off
    cout << "  plain(double):" << vd[i] << endl;
    cout << "plain(fixpoint):" << xr << endl;
    cout << "  plain(memory):" << org_mem << endl;
    cout << " first mpc to float (bitcopy) memory:" << get_hex_str(d1) << "   double value:" << d1 << endl;
    cout << " first mpc to float    (cast) memory:" << get_hex_str(d2) << "   double value:" << d2 << endl;
    cout << " next. float to mpc (bitcopy) memory:" << get_hex_str(t1) << " fixpoint value:" << xr1 << endl;
    cout << " next. float to mpc    (cast) memory:" << get_hex_str(t2) << " fixpoint value:" << xr2 << endl;
    cout << " last. mpc to real  (bitcopy) reveal:" << d1r << endl;
    cout << " last. mpc to real     (cast) reveal:" << d2r << endl;
    // clang-format on
  }
}
void testTypeConvert2() {
  cout.setf(ios::fixed);

  cout << "CHECK CHECK CHECK CHECK CHECK CHECK CHECK CHECK BIT COPY" << endl;
  testBitCopy();

  cout << "CHECK CHECK CHECK CHECK CHECK CHECK CHECK CHECK CAST" << endl;
  testCast();

  cout << "CHECK CHECK CHECK CHECK CHECK CHECK CHECK CHECK COMPARISION" << endl;
  testComparision();

  cout.unsetf(ios::fixed);
}

} // namespace debug
} // namespace mpc
} // namespace rosetta
