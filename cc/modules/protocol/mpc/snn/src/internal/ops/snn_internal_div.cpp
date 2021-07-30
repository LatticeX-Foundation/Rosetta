#include "cc/modules/protocol/mpc/snn/include/snn_internal.h"
#include "cc/modules/common/include/utils/str_type_convert.h"
#include <thread>

using std::thread;

namespace rosetta {
namespace snn {


int SnnInternal::Truedivision(
  const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c) {
  tlog_debug << "Truedivision ... ";
  
  int ret = Division(a, b, c);
  tlog_debug << "Truedivision ok.";
  
  return ret;
}

int SnnInternal::Truedivision(const vector<string>& a, const vector<mpc_t>& b, vector<mpc_t>& c) {
  tlog_debug << "Truedivision lh_is_const ...";
  
  size_t size = a.size();
  vector<mpc_t> numerator(size, 0);
  if (partyNum == PARTY_A) {
    vector<double> da(size, 0);
    rosetta::convert::from_double_str(a, da);
    convert_double_to_mpctype(da, numerator, GetMpcContext()->FLOAT_PRECISION);
  }
  
  int ret = Truedivision(numerator, b, c);
  tlog_debug << "Truedivision lh_is_const ok. ";
  
  return ret;
}

int SnnInternal::Truedivision(const vector<mpc_t>& a, const vector<string>& sb, vector<mpc_t>& c) {
  // std::cout << "debug stub DIV SC" << endl;
  // locally compute c = a * (1/fb)
  tlog_debug << "Truedivision rh_is_const ..";
  
  size_t size = a.size();
  vector<double> fb(size, 0.0);
  vector<double> inv_b(size, 0);
  int float_precision = GetMpcContext()->FLOAT_PRECISION;
  rosetta::convert::from_double_str(sb, fb);

  vector<size_t> power_list(size, float_precision);
  for (size_t i = 0; i < size; i++) {
    inv_b[i] = 1.0 / fb[i];
    // This is for dealing with big constant denominator, part I:
    //  scale it up by left-shifting
    double abs_v = abs(fb[i]);
    if (abs_v > 1) {
      power_list[i] = ceil(log2(abs_v));
      inv_b[i] = inv_b[i] * (1 << power_list[i]);
      power_list[i] = power_list[i] + float_precision;
    }
  }

  vector<mpc_t> b(inv_b.size(), 0);
  convert_double_to_mpctype(inv_b, b, float_precision);
  c.resize(size);
  for (size_t i = 0; i < size; ++i) {
    c[i] = a[i] * b[i];
  }

  if (PRIMARY) {
    Truncate_many(c, power_list, size, PARTY_A, PARTY_B, partyNum);
  }

  tlog_debug << "Truedivision rh_is_const ok.";
  
  return 0;
}

int SnnInternal::DivisionV1(const vector<string>& a, const vector<mpc_t>& b, vector<mpc_t>& quotient) {
  size_t size = a.size();
  vector<mpc_t> numerator(size, 0);
  if (partyNum == PARTY_A) {
    vector<double> da(size, 0);
    rosetta::convert::from_double_str(a, da);
    convert_string_to_mpctype(a, numerator);
  }
  
  return DivisionV1(numerator, b, quotient);
}

int SnnInternal::DivisionV1(const vector<mpc_t>& a, const vector<string>& b, vector<mpc_t>& quotient) {
  return Truedivision(a, b, quotient);
}

// All parties start with shares of a number in a and b and the quotient is in quotient.
// only support a < b
int SnnInternal::DivisionV1(
  const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& quotient) {
  // LOGI("funcDivisionMPC");
  size_t size = a.size();
  if (THREE_PC) {
    vector<mpc_t> varQ(size, 0);
    vector<mpc_t> varP(size, 0);
    vector<mpc_t> varD(size, 0);
    vector<mpc_t> tempZeros(size, 0);
    vector<mpc_t> varB(size, 0);
    vector<mpc_t> input_1(size, 0), input_2(size, 0);

    for (size_t i = 0; i < size; ++i) {
      varP[i] = 0;
      quotient[i] = 0;
    }

    const int float_precision = GetMpcContext()->FLOAT_PRECISION;
    for (size_t looper = 1; looper < float_precision + 1; ++looper) {
      if (PRIMARY) {
        for (size_t i = 0; i < size; ++i)
          input_1[i] = -b[i];

        Truncate(input_1, looper, size, PARTY_A, PARTY_B, partyNum);
        addVectors<mpc_t>(input_1, a, input_1, size);
        subtractVectors<mpc_t>(input_1, varP, input_1, size);
      }

      ReluPrime(input_1, varB);

      // Get the required shares of y/2^i and 2^FLOAT_PRECISION_M/2^i in input_1 and input_2
      for (size_t i = 0; i < size; ++i)
        input_1[i] = b[i];

      if (PRIMARY)
        Truncate(input_1, looper, size, PARTY_A, PARTY_B, partyNum);

      if (partyNum == PARTY_A)
        for (size_t i = 0; i < size; ++i)
          input_2[i] = (1 << float_precision);

      if (partyNum == PARTY_B)
        for (size_t i = 0; i < size; ++i)
          input_2[i] = 0;

      if (PRIMARY)
        Truncate(input_2, looper, size, PARTY_A, PARTY_B, partyNum);

      // funcSelectShares3PC(input_1, varB, varD, size);
      // funcSelectShares3PC(input_2, varB, varQ, size);

      vector<mpc_t> A_one(size, 0), B_one(size, 0), C_one(size, 0);
      vector<mpc_t> A_two(size, 0), B_two(size, 0), C_two(size, 0);

      if (HELPER) {
        vector<mpc_t> A1_one(size, 0), A2_one(size, 0), B1_one(size, 0), B2_one(size, 0),
          C1_one(size, 0), C2_one(size, 0);

        vector<mpc_t> A1_two(size, 0), A2_two(size, 0), B1_two(size, 0), B2_two(size, 0),
          C1_two(size, 0), C2_two(size, 0);

        populateRandomVector<mpc_t>(A1_one, size, "INDEP", "POSITIVE");
        populateRandomVector<mpc_t>(A2_one, size, "INDEP", "POSITIVE");
        populateRandomVector<mpc_t>(B1_one, size, "INDEP", "POSITIVE");
        populateRandomVector<mpc_t>(B2_one, size, "INDEP", "POSITIVE");
        populateRandomVector<mpc_t>(A1_two, size, "INDEP", "POSITIVE");
        populateRandomVector<mpc_t>(A2_two, size, "INDEP", "POSITIVE");
        populateRandomVector<mpc_t>(B1_two, size, "INDEP", "POSITIVE");
        populateRandomVector<mpc_t>(B2_two, size, "INDEP", "POSITIVE");

        addVectors<mpc_t>(A1_one, A2_one, A_one, size);
        addVectors<mpc_t>(B1_one, B2_one, B_one, size);
        addVectors<mpc_t>(A1_two, A2_two, A_two, size);
        addVectors<mpc_t>(B1_two, B2_two, B_two, size);

        for (size_t i = 0; i < size; ++i)
          C_one[i] = A_one[i] * B_one[i];

        for (size_t i = 0; i < size; ++i)
          C_two[i] = A_two[i] * B_two[i];

        splitIntoShares(C_one, C1_one, C2_one, size);
        splitIntoShares(C_two, C1_two, C2_two, size);

        sendSixVectors<mpc_t>(
          A1_one, B1_one, C1_one, A1_two, B1_two, C1_two, PARTY_A, size, size, size, size, size,
          size);
        sendSixVectors<mpc_t>(
          A2_one, B2_one, C2_one, A2_two, B2_two, C2_two, PARTY_B, size, size, size, size, size,
          size);
        // sendThreeVectors<mpc_t>(A1_one, B1_one, C1_one, PARTY_A, size, size, size);
        // sendThreeVectors<mpc_t>(A2_one, B2_one, C2_one, PARTY_B, size, size, size);
        // sendThreeVectors<mpc_t>(A1_two, B1_two, C1_two, PARTY_A, size, size, size);
        // sendThreeVectors<mpc_t>(A2_two, B2_two, C2_two, PARTY_B, size, size, size);
      }

      if (PRIMARY) {
        receiveSixVectors<mpc_t>(
          A_one, B_one, C_one, A_two, B_two, C_two, PARTY_C, size, size, size, size, size, size);
        // receiveThreeVectors<mpc_t>(A_one, B_one, C_one, PARTY_C, size, size, size);
        // receiveThreeVectors<mpc_t>(A_two, B_two, C_two, PARTY_C, size, size, size);

        vector<mpc_t> E_one(size), F_one(size), temp_E_one(size), temp_F_one(size);
        vector<mpc_t> E_two(size), F_two(size), temp_E_two(size), temp_F_two(size);
        mpc_t temp_one, temp_two;

        subtractVectors<mpc_t>(input_1, A_one, E_one, size);
        subtractVectors<mpc_t>(varB, B_one, F_one, size);
        subtractVectors<mpc_t>(input_2, A_two, E_two, size);
        subtractVectors<mpc_t>(varB, B_two, F_two, size);

        thread* threads = new thread[2];

        threads[0] = thread(
          &SnnInternal::sendFourVectors<mpc_t>, this, ref(E_one), ref(F_one), ref(E_two), ref(F_two),
          adversary(partyNum), size, size, size, size);
        threads[1] = thread(
          &SnnInternal::receiveFourVectors<mpc_t>, this, ref(temp_E_one), ref(temp_F_one),
          ref(temp_E_two), ref(temp_F_two), adversary(partyNum), size, size, size, size);

        for (int i = 0; i < 2; i++)
          threads[i].join();

        delete[] threads;

        addVectors<mpc_t>(E_one, temp_E_one, E_one, size);
        addVectors<mpc_t>(F_one, temp_F_one, F_one, size);
        addVectors<mpc_t>(E_two, temp_E_two, E_two, size);
        addVectors<mpc_t>(F_two, temp_F_two, F_two, size);

        for (size_t i = 0; i < size; ++i) {
          varD[i] = input_1[i] * F_one[i];
          temp_one = E_one[i] * varB[i];
          varD[i] = varD[i] + temp_one;

          if (partyNum == PARTY_A) {
            temp_one = E_one[i] * F_one[i];
            varD[i] = varD[i] - temp_one;
          }
        }

        for (size_t i = 0; i < size; ++i) {
          varQ[i] = input_2[i] * F_two[i];
          temp_two = E_two[i] * varB[i];
          varQ[i] = varQ[i] + temp_two;

          if (partyNum == PARTY_A) {
            temp_two = E_two[i] * F_two[i];
            varQ[i] = varQ[i] - temp_two;
          }
        }

        addVectors<mpc_t>(varD, C_one, varD, size);
        Truncate(varD, float_precision, size, PARTY_A, PARTY_B, partyNum);

        addVectors<mpc_t>(varQ, C_two, varQ, size);
        Truncate(varQ, float_precision, size, PARTY_A, PARTY_B, partyNum);
      }

      addVectors<mpc_t>(varP, varD, varP, size);
      addVectors<mpc_t>(quotient, varQ, quotient, size);
    }
  }

  return 0;
}

int SnnInternal::Division(
  const vector<string>& sa,
  const vector<mpc_t>& b,
  vector<mpc_t>& c,
  bool common_all_less/* = false*/) {
  size_t size = sa.size();
  c.resize(size);
  
  vector<mpc_t> a(sa.size(), 0);
  if (partyNum == PARTY_A) {
    vector<double> da(sa.size());
    rosetta::convert::from_double_str(sa, da);
    convert_double_to_mpctype(da, a, GetMpcContext()->FLOAT_PRECISION);
  }
  Division(a, b, c, common_all_less);
  return 0;
}

int SnnInternal::Division(
  const vector<mpc_t>& a,
  const vector<string>& sb,
  vector<mpc_t>& c,
  bool common_all_less/* = false*/) {
  // locally compute c = a * (1/b)
  // b to 1/b, div replace with mul
  AUDIT("id:{}, P{} Division, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(a));
  size_t size = a.size();
  vector<double> fb(sb.size());
  rosetta::convert::from_double_str(sb, fb);
  AUDIT("id:{}, P{} Division, input Y(double){}", msg_id().get_hex(), context_->GetMyRole(), Vector<double>(fb));

  vector<double> tb(fb.size(), 0);
  int float_precision = GetMpcContext()->FLOAT_PRECISION;
  vector<size_t> power_list(size, float_precision);
  for (size_t i = 0; i < size; i++) {
    tb[i] = 1.0 / fb[i];
    // This is for dealing with big constant denominator, part I:
    //  scale it up by left-shifting
    double abs_v = abs(fb[i]);
    if (abs_v > 1) {
      power_list[i] = ceil(log2(abs_v));
      tb[i] = tb[i] * (1 << power_list[i]);
      power_list[i] = power_list[i] + float_precision;
    }
  }
  vector<mpc_t> b(tb.size(), 0);
  convert_double_to_mpctype(tb, b, float_precision);
  c.resize(size);
  for (size_t i = 0; i < size; ++i) {
    c[i] = a[i] * b[i];
  }
  if (PRIMARY) {
    Truncate_many(c, power_list, size, PARTY_A, PARTY_B, partyNum);
  }
  AUDIT("id:{}, P{} Division, output Z(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(c));

  return 0;
}

// clang-format off
int SnnInternal::Division(
  const vector<mpc_t>& shared_numerator_vec, const vector<mpc_t>& shared_denominator_vec,
  vector<mpc_t>& shared_quotient_vec, bool all_less) {
  tlog_debug << "Division v2...";
  AUDIT("id:{}, P{} Division V2, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(shared_numerator_vec));
  AUDIT("id:{}, P{} Division V2, input Y(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(shared_denominator_vec));
  
  size_t vec_size = shared_numerator_vec.size();
  if (all_less) {
    Division(shared_numerator_vec, shared_denominator_vec, shared_quotient_vec);
    return 0;
  }

  const int float_precision = GetMpcContext()->FLOAT_PRECISION;
  if (THREE_PC) {
    mpc_t LEN = 8 * sizeof(mpc_t);

    /// PART 0: determine whether the values are positive or negative.
    tlog_debug << "PART 0: determine whether the values are positive or negative ...";
    vector<mpc_t> shared_numer_sign(vec_size, 0);
    ComputeMSB(shared_numerator_vec, shared_numer_sign);

    vector<mpc_t> shared_denom_sign(vec_size, 0);
    ComputeMSB(shared_denominator_vec, shared_denom_sign);

    // if (PRIMARY) {
    // 	funcReconstruct2PC(shared_numer_sign, vec_size, "x sign");
    // 	funcReconstruct2PC(shared_denom_sign, vec_size, "y sign");
    // }

    vector<mpc_t> shared_sign_pos(vec_size, 0);
    if (partyNum == PARTY_A) {
      shared_sign_pos = vector<mpc_t>(vec_size, FloatToMpcType(1, float_precision));
    }
    vector<mpc_t> shared_sign_neg(vec_size, 0);
    if (partyNum == PARTY_A) {
      shared_sign_neg = vector<mpc_t>(vec_size, FloatToMpcType(-1, float_precision));
    }
    vector<mpc_t> shared_x_sign(vec_size, 0);

    // Note: actually we can do the three independent MPC calls together
    // in one call by using a triple-size vector.
    Select1Of2(shared_sign_neg, shared_sign_pos, shared_numer_sign, shared_x_sign);

    vector<mpc_t> shared_y_sign(vec_size, 0);
    Select1Of2(shared_sign_neg, shared_sign_pos, shared_denom_sign, shared_y_sign);

    vector<mpc_t> quotient_sign_bit(vec_size, 0);
    XorBit(shared_numer_sign, shared_denom_sign, quotient_sign_bit);

    // Note: actually we can do the three independent MPC calls together
    // in one call by using a triple-size vector.
    vector<mpc_t> numerator_vec(vec_size, 0);
    vector<mpc_t> denominator_vec(vec_size, 0);

    DotProduct(shared_numerator_vec, shared_x_sign, numerator_vec);
    DotProduct(shared_denominator_vec, shared_y_sign, denominator_vec);

    vector<mpc_t> quotient_sign(vec_size, 0);
    Select1Of2(shared_sign_neg, shared_sign_pos, quotient_sign_bit, quotient_sign);

    vector<mpc_t> quotient_vec = shared_quotient_vec;

    // PART I: compute the compute the integer part.
    tlog_debug << "PART I: compute the compute the integer part. ...";
    vector<mpc_t> curr_q(vec_size, 0);
    auto curr_x = numerator_vec;
    auto curr_y = denominator_vec;
    // this is just like the original algorithm 8 in PET's paper.
    for (int i = LEN - 1 - float_precision; i >= 0; --i) {
      auto shared_z = curr_x;
      vector<mpc_t> shared_beta(vec_size, 0);
      if (PRIMARY) {
        Truncate(shared_z, i, vec_size, PARTY_A, PARTY_B, partyNum);
        subtractVectors<mpc_t>(shared_z, curr_y, shared_z, vec_size);
      }

      ReluPrime(shared_z, shared_beta);

      // selection base on beta
      vector<mpc_t> candidate_x_update(vec_size, 0);
      vector<mpc_t> candidate_q_update(vec_size, 0);
      if (PRIMARY) {
        for (auto j = 0; j < vec_size; ++j) {
          // TODO: check whether this is OK: direct local left shift.
          candidate_x_update[j] = curr_y[j] << i;
          if (partyNum == PARTY_A) {
            // to set it in the ring value!
            candidate_q_update[j] = (1 << i) << float_precision;
          } else if (partyNum == PARTY_B) {
            candidate_q_update[j] = 0;
          }
        }
      }
      SelectShares(candidate_x_update, shared_beta, candidate_x_update);
      SelectShares(candidate_q_update, shared_beta, candidate_q_update);

      addVectors<mpc_t>(curr_q, candidate_q_update, curr_q, vec_size);
      subtractVectors<mpc_t>(curr_x, candidate_x_update, curr_x, vec_size);
    }
    // if(PRIMARY)
    //	  funcReconstruct2PC(curr_q, vec_size, "curr_q");

    // compute remainder: x = x -(q*y)
    tlog_debug << "compute remainder: x = x -(q*y) ...";
    vector<mpc_t> curr_prod(vec_size, 0);
    DotProduct(curr_y, curr_q, curr_prod);
    subtractVectors(numerator_vec, curr_prod, curr_x, vec_size);

    // PART II: compute the fractional part,
    // if(PRIMARY)
    tlog_debug << "compute the compute the integer part. ...";
    for (int i = 1; i < float_precision + 1; ++i) {
      vector<mpc_t> candidate_x_update = curr_y;
      vector<mpc_t> candidate_q_update(vec_size, 0);
      // z = (x << 1) - y
      vector<mpc_t> shared_z(vec_size, 0);
      vector<mpc_t> shared_beta(vec_size, 0);
      if (PRIMARY) {
        for (int j = 0; j < vec_size; ++j) {
          curr_x[j] = curr_x[j] << 1;
        }
      }
      subtractVectors(curr_x, curr_y, shared_z, vec_size);
      ReluPrime(shared_z, shared_beta);

      // selection base on beta
      if (PRIMARY) {
        for (auto j = 0; j < vec_size; ++j) {
          // share of 2^(f-i)
          if (partyNum == PARTY_A) {
            candidate_q_update[j] = 1 << (float_precision - i);
          } else if (partyNum == PARTY_B) {
            candidate_q_update[j] = 0;
          }
        }
      }
      SelectShares(candidate_x_update, shared_beta, candidate_x_update);
      SelectShares(candidate_q_update, shared_beta, candidate_q_update);

      addVectors<mpc_t>(curr_q, candidate_q_update, curr_q, vec_size);
      subtractVectors<mpc_t>(curr_x, candidate_x_update, curr_x, vec_size);
    }
    quotient_vec = curr_q;
    ////funcDotProductMPC(quotient_vec, quotient_sign, shared_quotient_vec, vec_size);
    DotProduct(quotient_vec, quotient_sign, shared_quotient_vec);
  }
  AUDIT("id:{}, P{} Division V2, output Z(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(shared_quotient_vec));
  tlog_debug << "Division v2 ok.";
  
  return 0;
}

int SnnInternal::Floordivision(
  const vector<mpc_t>& ori_numerator_vec, const vector<mpc_t>& ori_denominator_vec,
  vector<mpc_t>& ori_quotient_vec, bool all_positive) {
  tlog_debug << "Floordivision ...";
  AUDIT("id:{}, P{} Floordivision , input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(ori_numerator_vec));
  AUDIT("id:{}, P{} Floordivision , input Y(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(ori_denominator_vec));
  
  size_t common_vec_size = ori_denominator_vec.size();
  ori_quotient_vec.resize(common_vec_size);
  if (THREE_PC) {
    mpc_t LEN = 8 * sizeof(mpc_t);
    vector<mpc_t> shared_numerator_vec = ori_numerator_vec;
    vector<mpc_t> shared_denominator_vec = ori_denominator_vec;
    vector<mpc_t>& shared_quotient_vec = ori_quotient_vec;
    vector<mpc_t> quotient_sign(common_vec_size, 0);
    vector<mpc_t> shared_offset(common_vec_size, 0);
    int float_precision = GetMpcContext()->FLOAT_PRECISION;
    if (!all_positive) {
      /// PART 0: determine whether the values are positive or negative.
      vector<mpc_t> shared_numer_sign(common_vec_size, 0);
      ComputeMSB(shared_numerator_vec, shared_numer_sign);

      vector<mpc_t> shared_denom_sign(common_vec_size, 0);
      ComputeMSB(shared_denominator_vec, shared_denom_sign);

      vector<mpc_t> shared_sign_pos(common_vec_size, 0);
      if (partyNum == PARTY_A) {
        shared_sign_pos = vector<mpc_t>(common_vec_size, FloatToMpcType(1, float_precision));
      }
      vector<mpc_t> shared_sign_neg(common_vec_size, 0);
      if (partyNum == PARTY_A) {
        shared_sign_neg = vector<mpc_t>(common_vec_size, FloatToMpcType(-1, float_precision));
      }
      // Note: this is for negative case floor:
      //    eg : math.floor(-6/4) = -math.floor(6/4) + (-1)
      vector<mpc_t> shared_zero(common_vec_size, 0);
      vector<mpc_t> shared_neg_one(common_vec_size, 0);
      if(partyNum == PARTY_A) {
        shared_neg_one = vector<mpc_t>(common_vec_size, FloatToMpcType(-1, float_precision));
      }
      
      vector<mpc_t> shared_x_sign(common_vec_size, 0);
      // Note: actually we can do the three independent MPC calls together
      // in one call by using a double-size vector.
      Select1Of2(shared_sign_neg, shared_sign_pos, shared_numer_sign, shared_x_sign);
      
      vector<mpc_t> shared_y_sign(common_vec_size, 0);
      Select1Of2(shared_sign_neg, shared_sign_pos, shared_denom_sign, shared_y_sign);
     
      vector<mpc_t> quotient_sign_bit(common_vec_size, 0);
      XorBit(shared_numer_sign, shared_denom_sign, quotient_sign_bit);

      // Note: actually we can do the three independent MPC calls together
      // in one call by using a double-size vector.
      vector<mpc_t> numerator_vec(common_vec_size, 0);
      vector<mpc_t> denominator_vec(common_vec_size, 0);
      DotProduct(shared_numerator_vec, shared_x_sign, numerator_vec);
      DotProduct(shared_denominator_vec, shared_y_sign, denominator_vec);

      Select1Of2(shared_sign_neg, shared_sign_pos, quotient_sign_bit, quotient_sign);
      Select1Of2(shared_neg_one, shared_zero, quotient_sign_bit, shared_offset); 
      vector<mpc_t> quotient_vec = shared_quotient_vec;
      shared_numerator_vec = numerator_vec;
      shared_denominator_vec = denominator_vec;
    }

    vector<mpc_t> curr_q(common_vec_size, 0);
    auto curr_x = shared_numerator_vec;
    auto curr_y = shared_denominator_vec;
    // this is just like the original algorithm 8 in PET's paper.
    for (int i = LEN - 1 - float_precision; i >= 0; --i) {
      auto shared_z = curr_x;
      vector<mpc_t> shared_beta(common_vec_size, 0);
      if (PRIMARY) {
        Truncate(shared_z, i, common_vec_size, PARTY_A, PARTY_B, partyNum);
        subtractVectors<mpc_t>(shared_z, curr_y, shared_z, common_vec_size);
      }

      ReluPrime(shared_z, shared_beta);
      
      // selection base on beta
      vector<mpc_t> candidate_x_update(common_vec_size, 0);
      vector<mpc_t> candidate_q_update(common_vec_size, 0);
      if (PRIMARY) {
        for (auto j = 0; j < common_vec_size; ++j) {
          // TODO: check whether this is OK: direct local left shift.
          candidate_x_update[j] = curr_y[j] << i;
          if (partyNum == PARTY_A) {
            // to set it in the ring value!
            candidate_q_update[j] = (1 << i) << float_precision;
          } else if (partyNum == PARTY_B) {
            candidate_q_update[j] = 0;
          }
        }
      }
      SelectShares(candidate_x_update, shared_beta, candidate_x_update);
      SelectShares(candidate_q_update, shared_beta, candidate_q_update);
      
      addVectors<mpc_t>(curr_q, candidate_q_update, curr_q, common_vec_size);
      subtractVectors<mpc_t>(curr_x, candidate_x_update, curr_x, common_vec_size);
      if(i == 0 && !all_positive) {
        // 20210721: part I to fix bug such as (-2)/2, we got - (2/2) - 1 = - 2, not -1
        //  Note that if curr_x is still bigger than 0, it means actually it has decimal part 
        //  if we are using RealDivision.
        vector<mpc_t> CONST_ZERO(common_vec_size, 0);
        vector<mpc_t> indivisable(common_vec_size, 0);
        FastNotEqual(curr_x, CONST_ZERO, indivisable);
        // if neg, we minus 1 only when it is indivisiable;
        DotProduct(shared_offset, indivisable, shared_offset);
        // log_debug << "SJJ TEST STUB!";
        // if (PRIMARY) {
    	  //   Reconstruct2PC(indivisable, "indivisable val:");
        // }
      }
    }
    if (all_positive) {
      shared_quotient_vec = curr_q;
    } else {
      DotProduct(curr_q, quotient_sign, shared_quotient_vec);
      addVectors<mpc_t>(shared_quotient_vec, shared_offset, shared_quotient_vec, common_vec_size); 
    }
  }

  AUDIT("id:{}, P{} Floordivision , output Z(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(ori_quotient_vec));
  tlog_debug << "Floordivision ok.";
  
  return 0;
}

int SnnInternal::Floordivision(
    const vector<string>& ori_numerator_vec,
    const vector<mpc_t>& ori_denominator_vec,
    vector<mpc_t>& ori_quotient_vec,
    bool all_positive/* = false*/) {
  
  size_t size = ori_numerator_vec.size();
  vector<mpc_t> numerator(size, 0);
  if (partyNum == PARTY_A)
  {
    vector<double> dv(size, 0.0);
    rosetta::convert::from_double_str(ori_numerator_vec, dv);
    convert_double_to_mpctype(dv, numerator, GetMpcContext()->FLOAT_PRECISION);
  }

  return Floordivision(numerator, ori_denominator_vec, ori_quotient_vec, all_positive);
}

int SnnInternal::Floordivision(
  const vector<mpc_t>& ori_numerator_vec,
  const vector<string>& ori_denominator_vec,
  vector<mpc_t>& ori_quotient_vec,
  bool all_positive /* = false*/) {
  tlog_debug << "Floordivision rh_is_const ... ";
  AUDIT("id:{}, P{} Floordivision rh_is_const, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(ori_numerator_vec));

  size_t size = ori_denominator_vec.size();
  vector<mpc_t> denominator(size, 0);
  if (partyNum == PARTY_A)
  {
    vector<double> dv(size, 0.0);
    rosetta::convert::from_double_str(ori_denominator_vec, dv);
    convert_double_to_mpctype(dv, denominator, GetMpcContext()->FLOAT_PRECISION);
  }

  return Floordivision(ori_numerator_vec, denominator, ori_quotient_vec, all_positive);
  // Todo: due to special precision case, such as -2.01/ -2.01 and 19/10, 
  //      we can not use the following optimization for now. 
  // size_t size = ori_numerator_vec.size();
  // auto& sb = ori_denominator_vec;
  // auto& a = ori_numerator_vec;
  // vector<double> db(size);
  // rosetta::convert::from_double_str(sb, db);
  // AUDIT("id:{}, P{} Floordivision rh_is_const, input Y(double){}", msg_id().get_hex(), context_->GetMyRole(), Vector<double>(db));
  // int float_precision = GetMpcContext()->FLOAT_PRECISION;


  // vector<size_t> power_list(size, float_precision);
  // for (size_t i = 0; i < size; ++i) {
  //   // This is for dealing with big constant denominator, part I:
  //   //  scale it up by left-shifting
  //   double abs_v = abs(db[i]);
  //   db[i] = 1.0 / db[i];
    
  //   if(abs_v > 1) {
  //       power_list[i] = ceil(log2(abs_v));
  //       db[i] = db[i] * (1 << power_list[i]);
  //       power_list[i] = power_list[i] + float_precision;
  //       // log_debug << " SJJ DEBUG power_list: " << sb[i] << "->:" << db[i] << ", " << power_list[i];
  //   }
  // }

  // vector<mpc_t> b(sb.size(), 0);
  // convert_double_to_mpctype(db, b, float_precision);

  // ori_quotient_vec.resize(size);
  // for (size_t i = 0; i < size; ++i) {
  //   ori_quotient_vec[i] = a[i] * b[i];
  // }
  // if (PRIMARY) {
  //   Truncate_many(ori_quotient_vec, power_list, size, PARTY_A, PARTY_B, partyNum);
  // }

  // // set the float part as 0. 
  // if (PRIMARY) {
  //   Truncate(ori_quotient_vec, float_precision, size, PARTY_A, PARTY_B, partyNum);
  //   for (size_t i = 0; i < size; ++i) {
  //     ori_quotient_vec[i] = static_cast<mpc_t>(static_cast<int64_t>(ori_quotient_vec[i]) << float_precision);
  //   }
  // }

  // AUDIT("id:{}, P{} Floordivision rh_is_const, output Z(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(ori_quotient_vec));
  // tlog_debug << "Floordivision rh_is_const ok.  ";
  // return 0;
}

/**
  for positive x < y only!
  eg: 5 / 7 = 0.7143
  @author: SJJ
*/
int SnnInternal::Fracdivision(
  const vector<mpc_t>& ori_numerator_vec, 
  const vector<mpc_t>& ori_denominator_vec, 
  vector<mpc_t>& ori_quotient_vec, 
  bool all_positive) {
  tlog_debug << "Fracdivision ...";
  AUDIT("id:{}, P{} Fracdivision, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(ori_numerator_vec));
  AUDIT("id:{}, P{} Fracdivision, input Y(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(ori_denominator_vec));

  size_t common_vec_size = ori_numerator_vec.size();
  if (THREE_PC) {
    // mpc_t LEN = 8 * sizeof(mpc_t);
    vector<mpc_t> shared_numerator_vec = ori_numerator_vec;
    vector<mpc_t> shared_denominator_vec = ori_denominator_vec;
    vector<mpc_t>& shared_quotient_vec = ori_quotient_vec;
    vector<mpc_t> quotient_sign(common_vec_size, 0);
    int float_precision = GetMpcContext()->FLOAT_PRECISION;
    if (!all_positive) {
      /// PART 0: determine whether the values are positive or negative.
      vector<mpc_t> shared_numer_sign(common_vec_size, 0);
      ComputeMSB(shared_numerator_vec, shared_numer_sign);
      
      vector<mpc_t> shared_denom_sign(common_vec_size, 0);
      ComputeMSB(shared_denominator_vec, shared_denom_sign);

      vector<mpc_t> shared_sign_pos(common_vec_size, 0);
      if (partyNum == PARTY_A) {
        shared_sign_pos = vector<mpc_t>(common_vec_size, FloatToMpcType(1, float_precision));
      }
      vector<mpc_t> shared_sign_neg(common_vec_size, 0);
      if (partyNum == PARTY_A) {
        shared_sign_neg = vector<mpc_t>(common_vec_size, FloatToMpcType(-1, float_precision));
      }
      vector<mpc_t> shared_x_sign(common_vec_size, 0);

      // Note: actually we can do the three independent MPC calls together
      // in one call by using a double-size vector.
      Select1Of2(shared_sign_neg, shared_sign_pos, shared_numer_sign, shared_x_sign);
      
      vector<mpc_t> shared_y_sign(common_vec_size, 0);
      Select1Of2(shared_sign_neg, shared_sign_pos, shared_denom_sign, shared_y_sign);
     
      vector<mpc_t> quotient_sign_bit(common_vec_size, 0);
      XorBit(shared_numer_sign, shared_denom_sign, quotient_sign_bit);

      // Note: actually we can do the three independent MPC calls together
      // in one call by using a double-size vector.
      vector<mpc_t> numerator_vec(common_vec_size, 0);
      vector<mpc_t> denominator_vec(common_vec_size, 0);
      DotProduct(shared_numerator_vec, shared_x_sign, numerator_vec);
      DotProduct(shared_denominator_vec, shared_y_sign, denominator_vec);

      Select1Of2(shared_sign_neg, shared_sign_pos, quotient_sign_bit, quotient_sign);

      vector<mpc_t> quotient_vec = shared_quotient_vec;
      shared_numerator_vec = numerator_vec;
      shared_denominator_vec = denominator_vec;
    }
    vector<mpc_t> curr_q(common_vec_size, 0);
    auto curr_x = shared_numerator_vec;
    auto curr_y = shared_denominator_vec;
    for (int i = 1; i < float_precision + 1; ++i) {
      vector<mpc_t> candidate_x_update = curr_y;
      vector<mpc_t> candidate_q_update(common_vec_size, 0);
      // z = x << 1 - y
      vector<mpc_t> shared_z(common_vec_size, 0);
      vector<mpc_t> shared_beta(common_vec_size, 0);
      if (PRIMARY) {
        for (int j = 0; j < common_vec_size; ++j) {
          curr_x[j] = curr_x[j] << 1;
        }
      }
      subtractVectors(curr_x, curr_y, shared_z, common_vec_size);
      ReluPrime(shared_z, shared_beta);
      // selection base on beta

      if (PRIMARY) {
        for (auto j = 0; j < common_vec_size; ++j) {
          // share of 2^(f-i)
          if (partyNum == PARTY_A) {
            candidate_q_update[j] = 1 << (float_precision - i);
          } else if (partyNum == PARTY_B) {
            candidate_q_update[j] = 0;
          }
        }
      }
      SelectShares(candidate_x_update, shared_beta, candidate_x_update);
      SelectShares(candidate_q_update, shared_beta, candidate_q_update);

      addVectors<mpc_t>(curr_q, candidate_q_update, curr_q, common_vec_size);
      subtractVectors<mpc_t>(curr_x, candidate_x_update, curr_x, common_vec_size);
    }

    if (all_positive) {
      shared_quotient_vec = curr_q;
    } else {
      DotProduct(curr_q, quotient_sign, shared_quotient_vec);
    }
  }

  AUDIT("id:{}, P{} Fracdivision, output Z(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(ori_quotient_vec));
  tlog_debug << "Fracdivision ok.";
  return 0;
}

int SnnInternal::Fracdivision(
  const vector<string>& ori_numerator_vec,
  const vector<mpc_t>& ori_denominator_vec,
  vector<mpc_t>& ori_quotient_vec,
  bool all_positive/* = false*/) {

  size_t size = ori_numerator_vec.size();
  vector<mpc_t> numerator(size, 0);
  if (partyNum == PARTY_A)
  {
    vector<double> dv(size, 0.0);
    rosetta::convert::from_double_str(ori_numerator_vec, dv);
    convert_double_to_mpctype(dv, numerator, GetMpcContext()->FLOAT_PRECISION);
  }

  return Fracdivision(numerator, ori_denominator_vec, ori_quotient_vec, all_positive);
}

int SnnInternal::Fracdivision(
  const vector<mpc_t>& ori_numerator_vec,
  const vector<string>& ori_denominator_vec,
  vector<mpc_t>& ori_quotient_vec,
  bool all_positive /* = false*/) {
  throw std::runtime_error("FracDivision(mpc_t, const) not implement now!");
  return 0;
}

int SnnInternal::Reciprocaldivision(
  const vector<mpc_t>& a, 
  const vector<mpc_t>& b, 
  vector<mpc_t>& c) {
    log_debug << "Reciprocaldivision ...";

    ReciprocalDivfor2(a, b, c);

    log_debug << "Reciprocaldivision ok.";
    return 0;
}

int SnnInternal::Reciprocaldivision(
  const vector<string>& a, 
  const vector<mpc_t>& b, 
  vector<mpc_t>& c) {  
  size_t size = a.size();
  vector<mpc_t> numerator(size, 0);
  if (partyNum == PARTY_A) {
    vector<double> da(size, 0.0);
    rosetta::convert::from_double_str(a, da);
    convert_double_to_mpctype(da, numerator, GetMpcContext()->FLOAT_PRECISION);
  }

  return Reciprocaldivision(numerator, b, c);
}

int SnnInternal::Reciprocaldivision(
  const vector<mpc_t>& a, 
  const vector<string>& b, 
  vector<mpc_t>& c) {
  size_t size = b.size();
  vector<mpc_t> Denominator(size, 0);
  if (partyNum == PARTY_A) {
    vector<double> db(size, 0.0);
    rosetta::convert::from_double_str(b, db);
    convert_double_to_mpctype(db, Denominator, GetMpcContext()->FLOAT_PRECISION);
  }

  return Reciprocaldivision(a, Denominator, c);
}

int SnnInternal::ReciprocalDivfor2(
  const vector<mpc_t>& shared_numerator_vec, 
  const vector<mpc_t>& shared_denominator_vec, 
  vector<mpc_t>& shared_quotient_vec,
  bool all_less /*= false*/) {
  log_debug << "Reciprocaldiv ...";
  size_t vec_size = shared_numerator_vec.size();
  if (all_less) {
    Division(shared_numerator_vec, shared_denominator_vec, shared_quotient_vec, all_less);
    return 0;
  }

  if (THREE_PC) {
    /*In the whole aspect,we are going to compute the reciprocial of shared_denominator_vec,
    then we can make the reciprocal to matmul the shared_numerator_vec.
    First, because we dicide to use the iteration function,
    so we should make sure the postivate/negativate of the den,and then get the sign of the final result, SJJ have done this part.
    Second,we should decide the iteration initial A by 3*exp(1-2*den)+0.003,but in this method we will use our exprience to set two initial value.but we should notice the formula above is more precise,but more exhaustive.
    Third,15 times for iteration is enough ,and we make each circle to compute:A=2*A-A*A*den.
    Finally,the A is 1/den,we realize num/den by num*（1/den).
    */

    /// determine whether the values are positive or negative.
    vector<mpc_t> shared_numer_sign(vec_size, 0);
    ComputeMSB(shared_numerator_vec, shared_numer_sign);

    vector<mpc_t> shared_denom_sign(vec_size, 0);
    ComputeMSB(shared_denominator_vec, shared_denom_sign);

    vector<mpc_t> shared_sign_pos(vec_size, 0);
    if (partyNum == PARTY_A) {
      shared_sign_pos = vector<mpc_t>(vec_size, FloatToMpcType(1, GetMpcContext()->FLOAT_PRECISION));
	  }

    vector<mpc_t> shared_sign_neg(vec_size, 0);
    if (partyNum == PARTY_A) {
      shared_sign_neg = vector<mpc_t>(vec_size, FloatToMpcType(-1, GetMpcContext()->FLOAT_PRECISION));
    }

    vector<mpc_t> shared_x_sign(vec_size, 0);
    Select1Of2(shared_sign_neg, shared_sign_pos, shared_numer_sign, shared_x_sign);

    vector<mpc_t> shared_y_sign(vec_size, 0);
    Select1Of2(shared_sign_neg, shared_sign_pos, shared_denom_sign, shared_y_sign);

    vector<mpc_t> quotient_sign_bit(vec_size, 0);
    XorBit(shared_numer_sign, shared_denom_sign, quotient_sign_bit);
   
    vector<mpc_t> numerator_vec(vec_size, 0);
    vector<mpc_t> denominator_vec(vec_size, 0);
    DotProduct(shared_numerator_vec, shared_x_sign, numerator_vec);
    DotProduct(shared_denominator_vec, shared_y_sign, denominator_vec);

    vector<mpc_t> quotient_sign(vec_size, 0);
    Select1Of2(shared_sign_neg, shared_sign_pos, quotient_sign_bit, quotient_sign);

    vector<mpc_t> quotient_vec = shared_quotient_vec;

    //we should limit the denominator to expand the scope of computation and the accuracy.
    vector<mpc_t> SHARED_ONE(vec_size, 0);
    if(partyNum == PARTY_A) {
      SHARED_ONE = vector<mpc_t>(vec_size, FloatToMpcType(1, GetMpcContext()->FLOAT_PRECISION));
    }
    vector<mpc_t> SHARED_TEN(vec_size, 0);
    if(partyNum == PARTY_A) {
      SHARED_TEN = vector<mpc_t>(vec_size, FloatToMpcType(10, GetMpcContext()->FLOAT_PRECISION));
    }
    vector<mpc_t> SHARED_divTEN(vec_size, 0);
    if(partyNum == PARTY_A) {
      SHARED_divTEN = vector<mpc_t>(vec_size, FloatToMpcType(0.1, GetMpcContext()->FLOAT_PRECISION));
    }

    vector<mpc_t> judge_val_1(vec_size, 0);
    vector<mpc_t> judge_val_2(vec_size, 0);
    vector<mpc_t> judge_val_1_p(vec_size, 0);//msb of judge number
    vector<mpc_t> judge_val_2_p(vec_size, 0);
    vector<mpc_t> judge(vec_size, 0);
    vector<mpc_t> factor(vec_size, 0);//multiple factor
    vector<mpc_t> denominator_temp(vec_size, 0);
    vector<mpc_t> numerator_temp(vec_size, 0);

    for(int i = 0 ; i < 4 ; i++) {
      subtractVectors<mpc_t>(denominator_vec, SHARED_TEN, judge_val_1, vec_size);//x-10
      subtractVectors<mpc_t>(denominator_vec, SHARED_ONE, judge_val_2, vec_size);//x-1
      // Reconstruct2PC(judge_val_2, "judge_val_2");

      ComputeMSB(judge_val_1, judge_val_1_p);
      ComputeMSB(judge_val_2, judge_val_2_p);//positive or negative
      // Reconstruct2PC(judge_val_2_p, "judge_val_2_p");

      XorBit(judge_val_1_p, judge_val_2_p, judge);
      // Reconstruct2PC(judge, "judge");

      Select1Of2(SHARED_TEN, SHARED_divTEN, judge_val_1_p, factor);
      Select1Of2(SHARED_ONE, factor, judge, factor);
      // Reconstruct2PC(factor, "factor");
      // Reconstruct2PC(denominator_vec, "denominator_vec");	

      DotProduct(factor, denominator_vec, denominator_temp);
      DotProduct(factor, numerator_vec, numerator_temp);
      denominator_vec = denominator_temp;
      numerator_vec = numerator_temp;
      // Reconstruct2PC(denominator_vec, "denominator_vec");	
	}    

    vector<mpc_t> result(vec_size,0);//initial of 1/x
    vector<mpc_t> initial_temp(vec_size,0);
    vector<mpc_t> initial_exp(vec_size,0);

    /// compute the initial_value
    vector<mpc_t> SHARED_Factorialof3(vec_size, 0);
    if(partyNum == PARTY_A) {
      SHARED_Factorialof3 = vector<mpc_t>(vec_size, FloatToMpcType(0.16667, GetMpcContext()->FLOAT_PRECISION));
    }

    vector<mpc_t> NUM_ONE(vec_size, 0);
    NUM_ONE = vector<mpc_t>(vec_size, FloatToMpcType(1, GetMpcContext()->FLOAT_PRECISION));

    vector<mpc_t> NUM_Factorialof3(vec_size, 0);
    NUM_Factorialof3 = vector<mpc_t>(vec_size, FloatToMpcType(0.16667, GetMpcContext()->FLOAT_PRECISION));
			
    if (PRIMARY) {
      for (int i = 0; i < vec_size; ++i) {
        initial_temp[i] = denominator_vec[i] << 1;
      }
	  }

    subtractVectors<mpc_t>(SHARED_ONE, initial_temp, initial_exp, vec_size);
    // Reconstruct2PC(initial_exp, "1-2*den");

    vector<mpc_t> shared_beta(vec_size,0);
    vector<mpc_t> update(vec_size,0);
    ReluPrime(initial_exp, shared_beta);
    initial_exp = vector<mpc_t>(vec_size,0);
    if(partyNum == PARTY_A) {
      initial_exp = vector<mpc_t>(vec_size, FloatToMpcType(0.003, GetMpcContext()->FLOAT_PRECISION));
      update = vector<mpc_t>(vec_size, FloatToMpcType(4.074, GetMpcContext()->FLOAT_PRECISION));
    }
	
    // Reconstruct2PC(shared_beta, "shared_beta");
    SelectShares(update, shared_beta, update);
    // Reconstruct2PC(update, "update");
    addVectors<mpc_t>(initial_exp, update, result, vec_size);

    // Reconstruct2PC(result, "initial_value");
    vector<mpc_t> iteraion_temp_2A(vec_size,0);//2*A
    vector<mpc_t> iteraion_temp_AA(vec_size,0);//A^2
    vector<mpc_t> den_reprocial_temp(vec_size,0);
    vector<mpc_t> quo(vec_size,0);

	  ///get initial foriteration
    for(int i = 0; i <= ITERATION_TIME; i++) {
      DotProduct(result, NUM_ONE, iteraion_temp_2A);
      // Reconstruct2PC(iteraion_temp_2A, "iteraion_temp_2A");
      
      Square(result, iteraion_temp_AA);//A*A
      // Reconstruct2PC(iteraion_temp_AA, "iteraion_temp_AA");
      // Reconstruct2PC(denominator_vec, "denominator_vec");
      
      DotProduct(iteraion_temp_AA, denominator_vec, den_reprocial_temp);//A*A*SELF
      // Reconstruct2PC(den_reprocial_temp, "den_reprocial_temp");
      subtractVectors<mpc_t>(iteraion_temp_2A,den_reprocial_temp,result,vec_size);
    }

    DotProduct(numerator_vec, result, quo);
    DotProduct(quo, quotient_sign, shared_quotient_vec);

  }//threepc

  log_debug << "ReciprocalDivfor2 ok.";
  return 0;

}


}//snn
}//rosetta
