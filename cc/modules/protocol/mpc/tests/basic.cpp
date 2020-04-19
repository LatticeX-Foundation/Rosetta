#include "snn.h"

namespace rosetta {
namespace mpc {
namespace debug {

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void debugDotProd() {
  LOGI("debugDotProd");
  auto opdot = GetMpcOpDefault(DotProduct);
  auto oprec = GetMpcOpWithKey(Reconstruct2PC, opdot->msg_id());

  size_t size = 10;
  vector<mpc_t> a(size, 0), b(size, 0), c(size);
  vector<mpc_t> temp(size);

  opdot->populateRandomVector<mpc_t>(temp, size, "COMMON", "NEGATIVE");
  for (size_t i = 0; i < size; ++i) {
    if (partyNum == PARTY_A)
      a[i] = temp[i] + FloatToMpcType(i);
    else
      a[i] = temp[i];
  }

  opdot->populateRandomVector<mpc_t>(temp, size, "COMMON", "NEGATIVE");
  for (size_t i = 0; i < size; ++i) {
    if (partyNum == PARTY_A)
      b[i] = temp[i] + FloatToMpcType(i);
    else
      b[i] = temp[i];
  }

  if (PRIMARY)
    oprec->Run(a, size, "a");
  if (PRIMARY)
    oprec->Run(b, size, "b");

  opdot->Run(a, b, c, size);

  if (PRIMARY)
    oprec->Run(c, size, "c");
}

void debugComputeMSB() {
  LOGI("debugComputeMSB");
  auto opmsb = GetMpcOpDefault(ComputeMSB);
  auto oprec = GetMpcOpWithKey(Reconstruct2PC, opmsb->msg_id());

  size_t size = 10;
  vector<mpc_t> a(size, 0);

  if (partyNum == PARTY_A)
    for (size_t i = 0; i < size; ++i)
      a[i] = i - 5;

  if (THREE_PC) {
    vector<mpc_t> c(size);
    opmsb->Run3PC(a, c, size);

    if (PRIMARY)
      oprec->Run(c, size, "c");
  }
}

void debug_mpc_power() {
  mpc_t v1 = FloatToMpcType(3.7);
  unordered_map<mpc_t, mpc_t> tmp_cache;
  mpc_t v2;
  mpc_t p = 12;
  auto op_power = GetMpcOpDefault(Polynomial);
  auto op_rcs = GetMpcOpWithKey(Reconstruct2PC, op_power->msg_id());
  op_power->mpc_pow_const(v1, p, v2, &tmp_cache);
  if (PRIMARY) {
    cout << MpcTypeToFloat(v1 * 2) << "** " << p << ":" << endl;
    vector<mpc_t> lcoal_v = {v1, v2};
    op_rcs->Run(lcoal_v, 2, "power result: ");
  }
}

void debugPC() {
  LOGI("debugPC beg");

  auto oppc = GetMpcOpDefault(PrivateCompare);
  auto oprec = GetMpcOpWithKey(Reconstruct2PC, oppc->msg_id());

  size_t size = 10;
  vector<mpc_t> r(size);
  vector<small_mpc_t> bit_shares(size * BIT_SIZE, 0);

  for (size_t i = 0; i < size; ++i)
    r[i] = 5 + i;

  if (partyNum == PARTY_A)
    for (size_t i = 0; i < size; ++i)
      for (size_t j = 0; j < BIT_SIZE; ++j)
        if (j == BIT_SIZE - 1 - i)
          bit_shares[i * BIT_SIZE + j] = 1;

  vector<small_mpc_t> beta(size);
  vector<small_mpc_t> betaPrime(size);

  if (PRIMARY)
    oppc->populateBitsVector(beta, "COMMON", size);

  oppc->Run(bit_shares, r, beta, betaPrime, size, BIT_SIZE);

  if (PRIMARY) {
    cout << "xx:";
    for (size_t i = 0; i < size; ++i)
      cout << (int)beta[i] << " ";
    cout << endl;
  }

  if (partyNum == PARTY_D) {
    for (size_t i = 0; i < size; ++i)
      cout << (int)(1 << i) << " " << (int)r[i] << " " << (int)betaPrime[i] << endl;
  }
  LOGI("debugPC end");
}

void debugPC2() {
  LOGI("debugPC2 beg");

  auto oppc = GetMpcOpDefault(PrivateCompare);
  auto oprec = GetMpcOpWithKey(Reconstruct2PC, oppc->msg_id());

  size_t size = 10;
  vector<mpc_t> r(size);
  vector<small_mpc_t> bit_shares(size * BIT_SIZE, 0);
  vector<mpc_t> rbit(size);

  /*
  x: 2 4 6 8 10 12 14 16 18 20
  r: 6 7 8 9 10 11 12 13 14 15
  */
  for (size_t i = 0; i < size; ++i)
    r[i] = 6 + i;

  if (PRIMARY && (partyNum == PARTY_A)) {
    cout << "bit_shares:";
    vector<mpc_t> data(size);
    for (size_t i = 0; i < size; ++i)
      data[i] = 2 * (i + 1); // 2,4,6,8,...
    for (size_t i = 0; i < size; ++i) {
      cout << "\n";
      for (size_t j = 0; j < BIT_SIZE; ++j) {
        auto idx = i * BIT_SIZE + j;
        bit_shares[idx] = (data[i] >> (BIT_SIZE - j - 1)) & 1;
        cout << to_string(bit_shares[idx]);
        if (j % 8 == 7)
          cout << " ";
      }
    }
    cout << "\n";
  }
  if (false && (partyNum == PARTY_A)) {
    cout << "bit_shares:";
    for (size_t i = 0; i < size; ++i) {
      cout << "\n";
      for (size_t j = 0; j < BIT_SIZE; ++j) {
        if (j == BIT_SIZE - 1 - i) {
          bit_shares[i * BIT_SIZE + j] = 1;
          rbit[i] |= (1 << (BIT_SIZE - j));
        }
        cout << to_string(bit_shares[i * BIT_SIZE + j]);
        if (j % 8 == 7)
          cout << " ";
      }
    }
    cout << "\n";
  }

  if (PRIMARY) {
    cout << "   r:";
    for (size_t i = 0; i < size; ++i) {
      cout << r[i] << " ";
    }
    cout << "\n";
    cout << "rbit:";
    for (size_t i = 0; i < size; ++i) {
      cout << rbit[i] << " ";
    }
    cout << "\n";
  }

  vector<small_mpc_t> beta(size);
  vector<small_mpc_t> betaPrime(size);

  if (PRIMARY)
    oppc->populateBitsVector(beta, "COMMON", size);

  oppc->Run(bit_shares, r, beta, betaPrime, size, BIT_SIZE);

  if (PRIMARY) {
    cout << "xx:";
    for (size_t i = 0; i < size; ++i)
      cout << (int)beta[i] << " ";
    cout << endl;
  }

  cout << "bP:\n";
  if (FOUR_PC && (partyNum == PARTY_D)) {
    for (size_t i = 0; i < size; ++i)
      cout << (int)(1 << i) << " " << (int)r[i] << " " << (int)betaPrime[i] << endl;
  }
  if (THREE_PC && (partyNum == PARTY_C)) {
    for (size_t i = 0; i < size; ++i)
      cout << (int)(1 << i) << " " << (int)r[i] << " " << (int)betaPrime[i] << endl;
  }
  LOGI("debugPC end");
}

void debugDivision() {
  LOGI("debugDivisiond");
  auto opdiv = GetMpcOpDefault(Division);
  auto oprec = GetMpcOpWithKey(Reconstruct2PC, opdiv->msg_id());

  size_t size = 10;
  vector<mpc_t> numerator(size);
  vector<mpc_t> denominator(size);
  vector<mpc_t> quotient(size, 0);

  for (size_t i = 0; i < size; ++i)
    numerator[i] = 30 * i;

  for (size_t i = 0; i < size; ++i)
    denominator[i] = 50 * size;

  opdiv->Run(numerator, denominator, quotient, size);

  if (PRIMARY) {
    oprec->Run(numerator, size, "Numerator");
    oprec->Run(denominator, size, "Denominator");
    oprec->Run(quotient, size, "Quotient");
  }
}

void debugDivisionV2() {
  LOGI("debugDivisionV2");

  auto opdiv = GetMpcOpDefault(DivisionV2);
  auto oprec = GetMpcOpWithKey(Reconstruct2PC, opdiv->msg_id());

  size_t size = 10;
  vector<mpc_t> numerator(size, 1 << FLOAT_PRECISION);
  vector<mpc_t> denominator(size, 1 << FLOAT_PRECISION);
  vector<mpc_t> quotient(size, 0);
  vector<double> x = {-1, -1.3, -10, -10.1, 1.01, 1.3, 10, 10.1, 0, 3.3};
  vector<double> y = {-0.3, 3.5, 10, -23.1, -1.6, 0.6, 0.1, 23.1, 3.3, 0};

  for (size_t i = 0; i < size; ++i) {
    numerator[i] = FloatToMpcType(x[i]);
  }

  for (size_t i = 0; i < size; ++i) {
    denominator[i] = FloatToMpcType(y[i]);
  }
  opdiv->Run(numerator, denominator, quotient, size);

  if (PRIMARY) {
    oprec->Run(numerator, size, "Numerator");
    oprec->Run(denominator, size, "Denominator");
    oprec->Run(quotient, size, "Quotient");
  }
}

void debugSS() {
  LOGI("debugSS");
  auto opsss = GetMpcOpDefault(SelectShares);
  auto oprec = GetMpcOpWithKey(Reconstruct2PC, opsss->msg_id());

  size_t size = 10;
  vector<mpc_t> inputs(size, 0), outputs(size, 0);

  if (THREE_PC) {
    vector<mpc_t> selector(size, 0);

    if (partyNum == PARTY_A)
      for (size_t i = 0; i < size; ++i)
        selector[i] = (mpc_t)(aes_indep->getBit() << FLOAT_PRECISION);

    if (PRIMARY)
      oprec->Run(selector, size, "selector");

    if (partyNum == PARTY_A)
      for (size_t i = 0; i < size; ++i)
        inputs[i] = (mpc_t)aes_indep->get8Bits();

    opsss->Run3PC(inputs, selector, outputs, size);

    if (PRIMARY) {
      oprec->Run(inputs, size, "inputs");
      oprec->Run(outputs, size, "outputs");
    }
  }
}
} // namespace debug
} // namespace mpc
} // namespace rosetta