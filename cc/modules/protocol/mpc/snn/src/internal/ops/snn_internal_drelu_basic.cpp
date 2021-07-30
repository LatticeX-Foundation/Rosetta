
#include "cc/modules/protocol/mpc/snn/include/snn_internal.h"
#include "cc/modules/common/include/utils/str_type_convert.h"
#include <thread>

using std::thread;

namespace rosetta {
namespace snn {

// compute `(x>r) xor beta`, P2 get the result bit
int SnnInternal::PrivateCompare(
  const vector<small_mpc_t>& share_m,
  const vector<mpc_t>& r,
  const vector<small_mpc_t>& beta,
  vector<small_mpc_t>& betaPrime,
  size_t size,
  size_t dim) {
  assert(THREE_PC && "PrivateCompare called in non-3PC mode");
  assert(dim == BIT_SIZE && "Private Compare assert issue");
  tlog_debug << "PrivateCompare...";
  AUDIT("id:{}, P{} PrivateCompare, input share_m(small_mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<small_mpc_t>(share_m));
  AUDIT("id:{}, P{} PrivateCompare, input r(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(r));
  AUDIT("id:{}, P{} PrivateCompare, input beta(small_mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<small_mpc_t>(beta));
  size_t sizeLong = size * dim;
  size_t index3, index2;
  int PARTY = PARTY_C;

  if (PRIMARY) {
    small_mpc_t bit_r, a, tempM;
    vector<small_mpc_t> c(sizeLong);
    mpc_t valueX;

    if (PARALLEL) {
      // I have removed the parallel for private compare, by yyl, 2020.03.12
    } else {
      // Check the security of the first if condition
      for (size_t index2 = 0; index2 < size; ++index2) {
        if (beta[index2] == 1 and r[index2] != MINUS_ONE)
          valueX = r[index2] + 1;
        else
          valueX = r[index2];

        if (beta[index2] == 1 and r[index2] == MINUS_ONE) {
          // One share of zero and other shares of 1
          // Then multiply and shuffle
          for (size_t k = 0; k < dim; ++k) {
            index3 = index2 * dim + k;
            c[index3] = aes_common->randModPrime();
            if (partyNum == PARTY_A)
              c[index3] = subtractModPrime((k != 0), c[index3]);

            c[index3] = multiplyModPrime(c[index3], aes_common->randNonZeroModPrime());
          }
        } else {
          // Single for loop
          a = 0;
          for (size_t k = 0; k < dim; ++k) {
            index3 = index2 * dim + k;
            c[index3] = a;
            tempM = share_m[index3];

            bit_r = (small_mpc_t)((valueX >> (BIT_SIZE - 1 - k)) & 1);

            if (bit_r == 0)
              a = addModPrime(a, tempM);
            else
              a = addModPrime(a, subtractModPrime((partyNum == PARTY_A), tempM));

            if (!beta[index2]) {
              if (partyNum == PARTY_A)
                c[index3] = addModPrime(c[index3], 1 + bit_r);
              c[index3] = subtractModPrime(c[index3], tempM);
            } else {
              if (partyNum == PARTY_A)
                c[index3] = addModPrime(c[index3], 1 - bit_r);
              c[index3] = addModPrime(c[index3], tempM);
            }

            c[index3] = multiplyModPrime(c[index3], aes_common->randNonZeroModPrime());
          }
        }
        aes_common->AES_random_shuffle(c, index2 * dim, (index2 + 1) * dim);
      }
    }
    sendVector<small_mpc_t>(c, PARTY, sizeLong);
    AUDIT("id:{}, P{} PrivateCompare SEND to P{}(small_mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), PARTY, Vector<small_mpc_t>(c));
  }

  if (partyNum == PARTY) {
    vector<small_mpc_t> c1(sizeLong);
    vector<small_mpc_t> c2(sizeLong);

    receiveVector<small_mpc_t>(c1, PARTY_A, sizeLong);
    AUDIT("id:{}, P{} PrivateCompare RECV from P{}(small_mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), PARTY_A, Vector<small_mpc_t>(c1));

    receiveVector<small_mpc_t>(c2, PARTY_B, sizeLong);
    AUDIT("id:{}, P{} PrivateCompare RECV from P{}(small_mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), PARTY_B, Vector<small_mpc_t>(c2));

    for (size_t index2 = 0; index2 < size; ++index2) {
      betaPrime[index2] = 0;
      for (int k = 0; k < dim; ++k) {
        index3 = index2 * dim + k;
        if (addModPrime(c1[index3], c2[index3]) == 0) {
          betaPrime[index2] = 1;
          break;
        }
      }
    }
  }

  AUDIT("id:{}, P{} PrivateCompare output(small_mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<small_mpc_t>(betaPrime));
  tlog_debug << "PrivateCompare ok.";
  return 0;
}

// Convert shares of a in \Z_L to shares in \Z_{L-1} (in place)
// a \neq L-1
int SnnInternal::ShareConvert(vector<mpc_t>& a) {
  assert(THREE_PC && "ShareConvertMPC called in non-3PC mode");
  tlog_debug << "ShareConvert...";
  AUDIT("id:{}, P{} ShareConvert, input a(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(a));
  size_t size = a.size();
  vector<mpc_t> r(size);
  vector<small_mpc_t> etaDP(size);
  vector<small_mpc_t> alpha(size);
  vector<small_mpc_t> betai(size);
  vector<small_mpc_t> bit_shares(size * BIT_SIZE);
  vector<mpc_t> delta_shares(size);
  vector<small_mpc_t> etaP(size);
  vector<mpc_t> eta_shares(size);
  vector<mpc_t> theta_shares(size);
  size_t PARTY = PARTY_C;

  if (PRIMARY) {
    vector<mpc_t> r1(size);
    vector<mpc_t> r2(size);
    vector<mpc_t> a_tilde(size);

    populateRandomVector<mpc_t>(r1, size, "COMMON", "POSITIVE");
    AUDIT("id:{}, P{} ShareConvert, populateRandomVector r1(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(r1));

    populateRandomVector<mpc_t>(r2, size, "COMMON", "POSITIVE");
    AUDIT("id:{}, P{} ShareConvert, populateRandomVector r2(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(r2));

    addVectors<mpc_t>(r1, r2, r, size);
    AUDIT("id:{}, P{} ShareConvert, compute r=r1+r2, r(=r1+r2)(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(r));

    if (partyNum == PARTY_A)
      wrapAround(r1, r2, alpha, size);

    if (partyNum == PARTY_A) {
      addVectors<mpc_t>(a, r1, a_tilde, size);
      AUDIT("id:{}, P{} ShareConvert, compute a_tilde=a+r1, a_tilde(=a+r1)(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(a_tilde));

      wrapAround(a, r1, betai, size);
    }
    if (partyNum == PARTY_B) {
      addVectors<mpc_t>(a, r2, a_tilde, size);
      AUDIT("id:{}, P{} ShareConvert, compute a_tilde=a+r2, a_tilde(=a+r2)(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(a_tilde));

      wrapAround(a, r2, betai, size);
    }

    populateBitsVector(etaDP, "COMMON", size);
    AUDIT("id:{}, P{} ShareConvert, populateBitsVector etaDP(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<small_mpc_t>(etaDP));

    sendVector<mpc_t>(a_tilde, PARTY_C, size);
    AUDIT("id:{}, P{} ShareConvert SEND to P{},  a_tilde(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), PARTY_C, Vector<mpc_t>(a_tilde));
  }

  if (partyNum == PARTY_C) {
    vector<mpc_t> x(size);
    vector<small_mpc_t> delta(size);
    vector<mpc_t> a_tilde_1(size);
    vector<mpc_t> a_tilde_2(size);
    vector<small_mpc_t> bit_shares_x_1(size * BIT_SIZE);
    vector<small_mpc_t> bit_shares_x_2(size * BIT_SIZE);
    vector<mpc_t> delta_shares_1(size);
    vector<mpc_t> delta_shares_2(size);

    receiveVector<mpc_t>(a_tilde_1, PARTY_A, size);
    AUDIT("id:{}, P{} ShareConvert RECV from P{},  a_tilde_1(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), PARTY_A, Vector<mpc_t>(a_tilde_1));

    receiveVector<mpc_t>(a_tilde_2, PARTY_B, size);
    AUDIT("id:{}, P{} ShareConvert RECV from P{},  a_tilde_2(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), PARTY_B, Vector<mpc_t>(a_tilde_2));

    addVectors<mpc_t>(a_tilde_1, a_tilde_2, x, size);
    AUDIT("id:{}, P{} ShareConvert, compute x=a_tilde_1+a_tilde_2, x(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(x));

    wrapAround(a_tilde_1, a_tilde_2, delta, size);
    AUDIT("id:{}, P{} ShareConvert, delta(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<small_mpc_t>(delta));

#if 0
		sharesOfBits(bit_shares_x_1, bit_shares_x_2, x, size, "INDEP");
		sendVector<small_mpc_t>(bit_shares_x_1, PARTY_A, size*BIT_SIZE);
		sendVector<small_mpc_t>(bit_shares_x_2, PARTY_B, size*BIT_SIZE);
		sharesModuloOdd<small_mpc_t>(delta_shares_1, delta_shares_2, delta, size, "INDEP");
		sendVector<mpc_t>(delta_shares_1, PARTY_A, size);
		sendVector<mpc_t>(delta_shares_2, PARTY_B, size);
#else
    sharesOfBits(bit_shares_x_1, bit_shares_x_2, x, size, "a_1");
    AUDIT("id:{}, P{} ShareConvert, sharesOfBits, x(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(x));

    sendVector<small_mpc_t>(bit_shares_x_2, PARTY_B, size * BIT_SIZE);
    AUDIT("id:{}, P{} ShareConvert SEND to P{}, bit_shares_x_2(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), PARTY_B, Vector<small_mpc_t>(bit_shares_x_2));

    sharesModuloOdd<small_mpc_t>(delta_shares_1, delta_shares_2, delta, size, "a_1");
    AUDIT("id:{}, P{} ShareConvert compute: delta=delta_shares_1+delta_shares_2, delta(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<small_mpc_t>(delta));

    sendVector<mpc_t>(delta_shares_2, PARTY_B, size);
    AUDIT("id:{}, P{} ShareConvert SEND to P{}, delta_shares_2(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), PARTY_B, Vector<mpc_t>(delta_shares_2));
#endif
  }

  if (PRIMARY) {
#if 0
		receiveVector<small_mpc_t>(bit_shares, PARTY_C, size*BIT_SIZE);
		receiveVector<mpc_t>(delta_shares, PARTY_C, size);
#else
    if (partyNum == PARTY_A) {
      gen_side_shareOfBits(bit_shares, size, "a_1");
      AUDIT("id:{}, P{} ShareConvert gen_side, bit_shares(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<small_mpc_t>(bit_shares));

      gen_side_sharesModuloOdd<small_mpc_t>(delta_shares, size, "a_1");
      AUDIT("id:{}, P{} ShareConvert gen_side_delta, delta_shares(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(delta_shares));
    } else {
      receiveVector<small_mpc_t>(bit_shares, PARTY_C, size * BIT_SIZE);
      AUDIT("id:{}, P{} ShareConvert RECV from P{}, bit_shares(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), PARTY_C, Vector<small_mpc_t>(bit_shares));

      receiveVector<mpc_t>(delta_shares, PARTY_C, size);
      AUDIT("id:{}, P{} ShareConvert RECV from P{}, delta_shares(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), PARTY_C, Vector<mpc_t>(delta_shares));
    }
#endif
  }

  // [HGF]:  to fellow the definition of SecureNN, i add the code below
  for (size_t i = 0; i < size; ++i)
    if (r[i] != 0)
      r[i] = r[i] - 1;

  PrivateCompare(bit_shares, r, etaDP, etaP, size, BIT_SIZE);

  if (partyNum == PARTY) {
    vector<mpc_t> eta_shares_1(size);
    vector<mpc_t> eta_shares_2(size);

#if 0
    sharesModuloOdd<small_mpc_t>(eta_shares_1, eta_shares_2, etaP, size, "INDEP");
    sendVector<mpc_t>(eta_shares_1, PARTY_A, size);
    sendVector<mpc_t>(eta_shares_2, PARTY_B, size);
#else
    sharesModuloOdd<small_mpc_t>(eta_shares_1, eta_shares_2, etaP, size, "a_1");
    AUDIT("id:{}, P{} ShareConvert sharedModuloOdd, etaP(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<small_mpc_t>(etaP));

    sendVector<mpc_t>(eta_shares_2, PARTY_B, size);
    AUDIT("id:{}, P{} ShareConvert SEND to P{}, eta_shares_2(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), PARTY_B, Vector<mpc_t>(eta_shares_2));
#endif
  }

  if (PRIMARY) {
    //gen share or receive from Party-C
#if 0
    receiveVector<mpc_t>(eta_shares, PARTY, size);
#else
    if (partyNum == PARTY_A) {
      gen_side_sharesModuloOdd<small_mpc_t>(eta_shares, size, "a_1");
      AUDIT("id:{}, P{} ShareConvert gen_side_eta, eta_shares(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(eta_shares));
    } else {
      receiveVector<mpc_t>(eta_shares, PARTY, size);
      AUDIT("id:{}, P{} ShareConvert RECV from P{}, eta_shares(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), PARTY, Vector<mpc_t>(eta_shares));
    }
#endif

    XORModuloOdd(etaDP, eta_shares, theta_shares, size, partyNum);
    addModuloOdd<mpc_t, small_mpc_t>(theta_shares, betai, theta_shares, size);
    // // [HGF]:  I fix the the code below, from subtractModuloOdd to addModuloOdd
    addModuloOdd<mpc_t, mpc_t>(theta_shares, delta_shares, theta_shares, size);

    if (partyNum == PARTY_A) {
      // [HGF]:  I add the code below
      // alpha + 1
      for (size_t i = 0; i < size; ++i)
        alpha[i] += 1;

      subtractModuloOdd<mpc_t, small_mpc_t>(theta_shares, alpha, theta_shares, size);
    }

    subtractModuloOdd<mpc_t, mpc_t>(a, theta_shares, a, size);
  }

  AUDIT("id:{}, P{} ShareConvert, output(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(a));
  tlog_debug << "ShareConvert ok.";
  return 0;
}

// Compute MSB of a and store it in b
// 3PC: output is shares of MSB in \Z_L
int SnnInternal::ComputeMSB(const vector<mpc_t>& a, vector<mpc_t>& b) {
  assert(THREE_PC && "ComputeMSB called in non-3PC mode");
  tlog_debug << "ComputeMSB..."; 
  AUDIT("id:{}, P{} ComputeMSB, input(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(a));
  size_t size = a.size();
  vector<mpc_t> ri(size);
  vector<small_mpc_t> bit_shares(size * BIT_SIZE);
  vector<mpc_t> LSB_shares(size);
  vector<small_mpc_t> beta(size);
  vector<mpc_t> c(size);
  vector<small_mpc_t> betaP(size);
  vector<small_mpc_t> gamma(size);
  vector<mpc_t> theta_shares(size);

  if (partyNum == PARTY_C) {
    vector<mpc_t> r1(size);
    vector<mpc_t> r2(size);
    vector<mpc_t> r(size);
    vector<small_mpc_t> bit_shares_r_1(size * BIT_SIZE);
    vector<small_mpc_t> bit_shares_r_2(size * BIT_SIZE);
    vector<mpc_t> LSB_shares_1(size);
    vector<mpc_t> LSB_shares_2(size);

#if 0
    for (size_t i = 0; i < size; ++i) {
      r1[i] = aes_indep->randModuloOdd();
      r2[i] = aes_indep->randModuloOdd();
    }

    addModuloOdd<mpc_t, mpc_t>(r1, r2, r, size);
    sharesOfBits(bit_shares_r_1, bit_shares_r_2, r, size, "INDEP");
    sharesOfLSB(LSB_shares_1, LSB_shares_2, r, size, "INDEP");

    sendVector<small_mpc_t>(bit_shares_r_1, PARTY_A, size * BIT_SIZE);
    sendVector<small_mpc_t>(bit_shares_r_2, PARTY_B, size * BIT_SIZE);
    sendTwoVectors<mpc_t>(r1, LSB_shares_1, PARTY_A, size, size);
    sendTwoVectors<mpc_t>(r2, LSB_shares_2, PARTY_B, size, size);
#elif PRSS_OPT_VALUE == 1
    populateRandomVector<mpc_t>(r1, size, "a_1", "POSITIVE");
    AUDIT("id:{}, P{} ShareConvert, populateRandomVector r1(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(r1));

    populateRandomVector<mpc_t>(r2, size, "a_2", "POSITIVE");
    AUDIT("id:{}, P{} ShareConvert, populateRandomVector r2(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(r2));

    for (size_t i = 0; i < size; ++i) {
      if (r1[i] == MINUS_ONE)
        r1[i] = 0; ////[HGF] fix for odd ring Z_{2^128 - 1}, origin: r1[i]--
      if (r2[i] == MINUS_ONE)
        r2[i] = 0; ////[HGF] fix for odd ring Z_{2^128 - 1}, origin: r2[i]--
    }

    addModuloOdd<mpc_t, mpc_t>(r1, r2, r, size);
    AUDIT("id:{}, P{} ShareConvert, r(=r1+r2)(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(r));

    sharesOfBits(bit_shares_r_1, bit_shares_r_2, r, size, "INDEP");
    sharesOfLSB(LSB_shares_1, LSB_shares_2, r, size, "INDEP");

    sendVector<small_mpc_t>(bit_shares_r_1, PARTY_A, size * BIT_SIZE);
    AUDIT("id:{}, P{} ShareConvert SEND to P{}, bit_shares_r_1{}", msg_id().get_hex(), context_->GetMyRole(), PARTY_A, Vector<small_mpc_t>(bit_shares_r_1));

    sendVector<small_mpc_t>(bit_shares_r_2, PARTY_B, size * BIT_SIZE);
    AUDIT("id:{}, P{} ShareConvert SEND to P{}, bit_shares_r_2{}", msg_id().get_hex(), context_->GetMyRole(), PARTY_B, Vector<small_mpc_t>(bit_shares_r_2));

    sendVector<mpc_t>(LSB_shares_1, PARTY_A, size);
    AUDIT("id:{}, P{} ShareConvert SEND to P{}, LSB_shares_1{}", msg_id().get_hex(), context_->GetMyRole(), PARTY_A, Vector<mpc_t>(LSB_shares_1));

    sendVector<mpc_t>(LSB_shares_2, PARTY_B, size);
    AUDIT("id:{}, P{} ShareConvert SEND to P{}, LSB_shares_2{}", msg_id().get_hex(), context_->GetMyRole(), PARTY_B, Vector<mpc_t>(LSB_shares_2));
#else
    populateRandomVector<mpc_t>(r1, size, "a_1", "POSITIVE");
    AUDIT("id:{}, P{} ShareConvert, populateRandomVector r1(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(r1));

    populateRandomVector<mpc_t>(r2, size, "a_2", "POSITIVE");
    AUDIT("id:{}, P{} ShareConvert, populateRandomVector r2(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(r2));

    for (size_t i = 0; i < size; ++i) {
      if (r1[i] == MINUS_ONE)
        r1[i] = 0; ////[HGF] fix for odd ring Z_{2^128 - 1}, origin: r1[i]--
      if (r2[i] == MINUS_ONE)
        r2[i] = 0; ////[HGF] fix for odd ring Z_{2^128 - 1}, origin: r2[i]--
    }

    addModuloOdd<mpc_t, mpc_t>(r1, r2, r, size);
    AUDIT("id:{}, P{} ShareConvert, r(=r1+r2)(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(r));

    sharesOfBits(bit_shares_r_1, bit_shares_r_2, r, size, "a_1"); //PRSS, user a
    sharesOfLSB2(LSB_shares_1, LSB_shares_2, r, size, "a_1");

    sendVector<small_mpc_t>(bit_shares_r_2, PARTY_B, size * BIT_SIZE);
    AUDIT("id:{}, P{} ShareConvert SEND to P{}, bit_shares_r_2{}", msg_id().get_hex(), context_->GetMyRole(), PARTY_B, Vector<small_mpc_t>(bit_shares_r_2));

    sendVector<mpc_t>(LSB_shares_2, PARTY_B, size); //
    AUDIT("id:{}, P{} ShareConvert SEND to P{}, LSB_shares_2{}", msg_id().get_hex(), context_->GetMyRole(), PARTY_B, Vector<mpc_t>(LSB_shares_2));
#endif
  }

  if (PRIMARY) {
#if 0
    vector<mpc_t> temp(size);
    receiveVector<small_mpc_t>(bit_shares, PARTY_C, size * BIT_SIZE);
    receiveTwoVectors<mpc_t>(ri, LSB_shares, PARTY_C, size, size);
#elif PRSS_OPT_VALUE == 1
    vector<mpc_t> temp(size);
    receiveVector<small_mpc_t>(bit_shares, PARTY_C, size * BIT_SIZE);
    AUDIT("id:{}, P{} ShareConvert RECV from P{}, bit_shares{}", msg_id().get_hex(), context_->GetMyRole(), PARTY_C, Vector<small_mpc_t>(bit_shares));

    receiveVector<mpc_t>(LSB_shares, PARTY_C, size);
    AUDIT("id:{}, P{} ShareConvert RECV from P{}, LSB_shares{}", msg_id().get_hex(), context_->GetMyRole(), PARTY_C, Vector<mpc_t>(LSB_shares));

    if (partyNum == PARTY_A) {
      populateRandomVector<mpc_t>(ri, size, "a_1", "POSITIVE");
      AUDIT("id:{}, P{} ShareConvert, populateRandomVector ri(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(ri));
    } else {
      populateRandomVector<mpc_t>(ri, size, "a_2", "POSITIVE");
      AUDIT("id:{}, P{} ShareConvert, populateRandomVector ri(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(ri));
    }

    for (size_t i = 0; i < size; ++i) {
      if (ri[i] == MINUS_ONE)
        ri[i] -= 1;
    }
#else
    vector<mpc_t> temp(size);
    if (partyNum == PARTY_A) {
      populateRandomVector<mpc_t>(ri, size, "a_1", "POSITIVE");
      AUDIT("id:{}, P{} ShareConvert, populateRandomVector ri(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(ri));

      gen_side_shareOfBits(bit_shares, size, "a_1");
      AUDIT("id:{}, P{} ShareConvert gen_sid, bit_shares(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<small_mpc_t>(bit_shares));

      populateRandomVector<mpc_t>(LSB_shares, size, "a_1", "POSITIVE");
      AUDIT("id:{}, P{} ShareConvert, populateRandomVector LSB_shares(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(LSB_shares));
    } else {
      populateRandomVector<mpc_t>(ri, size, "a_2", "POSITIVE");
      AUDIT("id:{}, P{} ShareConvert, populateRandomVector ri(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(ri));

      receiveVector<small_mpc_t>(bit_shares, PARTY_C, size * BIT_SIZE);
      AUDIT("id:{}, P{} ShareConvert RECV from P{} bit_shares(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), PARTY_C, Vector<small_mpc_t>(bit_shares));

      receiveVector<mpc_t>(LSB_shares, PARTY_C, size);
      AUDIT("id:{}, P{} ShareConvert RECV from P{} LSB_shares(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), PARTY_C, Vector<mpc_t>(LSB_shares));
    }

    for (size_t i = 0; i < size; ++i) {
      if (ri[i] == MINUS_ONE)
        ri[i] = 0; ////[HGF] fix for odd ring Z_{2^128 - 1}, origin: ri[i] -= 1;
    }
#endif

    addModuloOdd<mpc_t, mpc_t>(a, a, c, size);
    AUDIT("id:{}, P{} ShareConvert, c(=c+a)(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(c));
    addModuloOdd<mpc_t, mpc_t>(c, ri, c, size);
    AUDIT("id:{}, P{} ShareConvert, c(=c+ri)(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(c));

    thread* threads = new thread[2];

    AUDIT("id:{}, P{} ShareConvert will SEND to P{} c(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), adversary(partyNum), Vector<mpc_t>(c));
    threads[0] = thread(&SnnInternal::sendVector<mpc_t>, this, ref(c), adversary(partyNum), size);

    threads[1] = thread(&SnnInternal::receiveVector<mpc_t>, this, ref(temp), adversary(partyNum), size);
    for (int i = 0; i < 2; i++)
      threads[i].join();
    delete[] threads;
    // sendVector<mpc_t>(c, adversary(partyNum), size);
    // receiveVector<mpc_t>(temp, adversary(partyNum), size);
    AUDIT("id:{}, P{} ShareConvert RECV from P{} temp(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), adversary(partyNum), Vector<mpc_t>(temp));

    addModuloOdd<mpc_t, mpc_t>(c, temp, c, size);
    AUDIT("id:{}, P{} ShareConvert, c(=c+temp)(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(c));

    populateBitsVector(beta, "COMMON", size);
    AUDIT("id:{}, P{} ShareConvert, populateBitsVector beta(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<small_mpc_t>(beta));
  }

  PrivateCompare(bit_shares, c, beta, betaP, size, BIT_SIZE);

  if (partyNum == PARTY_C) {
    vector<mpc_t> theta_shares_1(size);
    vector<mpc_t> theta_shares_2(size);

#if 0
    sharesOfBitVector(theta_shares_1, theta_shares_2, betaP, size, "INDEP");
    sendVector<mpc_t>(theta_shares_1, PARTY_A, size);
    sendVector<mpc_t>(theta_shares_2, PARTY_B, size);
#else
    sharesOfBitVector(theta_shares_1, theta_shares_2, betaP, size, "a_2");
    AUDIT("id:{}, P{} ShareConvert sharesOfBitVector, betaP{}", msg_id().get_hex(), context_->GetMyRole(), Vector<small_mpc_t>(betaP));

    sendVector<mpc_t>(theta_shares_1, PARTY_A, size);
    AUDIT("id:{}, P{} ShareConvert SEND to P{}, theta_shares_1{}", msg_id().get_hex(), context_->GetMyRole(), PARTY_A, Vector<mpc_t>(theta_shares_1));
#endif
  }

  vector<mpc_t> prod(size), temp(size);
  if (PRIMARY) {
    // theta_shares is the same as gamma (in older versions);
    // LSB_shares is the same as delta (in older versions);
#if 0
	  receiveVector<mpc_t>(theta_shares, PARTY_C, size);
#else
    if (partyNum == PARTY_A) {
      receiveVector<mpc_t>(theta_shares, PARTY_C, size);
      AUDIT("id:{}, P{} ShareConvert RECV from P{} theta_shares(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), PARTY_C, Vector<mpc_t>(theta_shares));
    } else {
      gen_side_shareOfBitVector(theta_shares, size, "a_2");
      AUDIT("id:{}, P{} ShareConvert gen_side theta_shares(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(theta_shares));
    }
#endif

    mpc_t j = 0;
    if (partyNum == PARTY_A)
      j = FloatToMpcType(1, GetMpcContext()->FLOAT_PRECISION);

    for (size_t i = 0; i < size; ++i) {
      theta_shares[i] = (1 - 2 * beta[i]) * theta_shares[i] + j * beta[i];
      LSB_shares[i] = (1 - 2 * (c[i] & 1)) * LSB_shares[i] + j * (c[i] & 1);
    }
  }

  DotProduct(theta_shares, LSB_shares, prod);
  AUDIT("id:{}, P{} ShareConvert compute: prod=theta_shares*LSB_shares, prod(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(prod));

  if (PRIMARY) {
    populateRandomVector<mpc_t>(temp, size, "COMMON", "NEGATIVE");
    AUDIT("id:{}, P{} ShareConvert, populateRandomVector temp(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(temp));
    for (size_t i = 0; i < size; ++i)
      b[i] = theta_shares[i] + LSB_shares[i] - 2 * prod[i] + temp[i];
  }

  AUDIT("id:{}, P{} ShareConvert output(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(b));
  tlog_debug << "ComputeMSB ok.";
  return 0;
}

}//snn
}//rosetta
