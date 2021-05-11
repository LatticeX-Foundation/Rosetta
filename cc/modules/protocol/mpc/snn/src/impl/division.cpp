// ==============================================================================
// Copyright 2020 The LatticeX Foundation
// This file is part of the Rosetta library.
//
// The Rosetta library is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// The Rosetta library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with the Rosetta library. If not, see <http://www.gnu.org/licenses/>.
// ==============================================================================
#include "cc/modules/protocol/mpc/snn/src/impl/op_impl.h"

namespace rosetta {
namespace snn {

int Div::funcDiv(const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size) {
  return GetMpcOpInner(FloorDivision)->Run(a, b, c, size);
}
int Truediv::funcTruediv(
  const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& c, size_t size) {
  return GetMpcOpInner(DivisionV2)->Run(a, b, c, size);
}

// All parties start with shares of a number in a and b and the quotient is in quotient.
// only support a < b
int Division::funcDivisionMPC(
  const vector<mpc_t>& a, const vector<mpc_t>& b, vector<mpc_t>& quotient, size_t size) {
  // LOGI("funcDivisionMPC");

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

    for (size_t looper = 1; looper < FLOAT_PRECISION_M + 1; ++looper) {
      if (PRIMARY) {
        for (size_t i = 0; i < size; ++i)
          input_1[i] = -b[i];

        funcTruncate2PC(input_1, looper, size, PARTY_A, PARTY_B);
        addVectors<mpc_t>(input_1, a, input_1, size);
        subtractVectors<mpc_t>(input_1, varP, input_1, size);
      }
      ////funcRELUPrime3PC(input_1, varB, size);
      GetMpcOpInner(ReluPrime)->Run3PC(input_1, varB, size);

      // Get the required shares of y/2^i and 2^FLOAT_PRECISION_M/2^i in input_1 and input_2
      for (size_t i = 0; i < size; ++i)
        input_1[i] = b[i];

      if (PRIMARY)
        funcTruncate2PC(input_1, looper, size, PARTY_A, PARTY_B);

      if (partyNum == PARTY_A)
        for (size_t i = 0; i < size; ++i)
          input_2[i] = (1 << FLOAT_PRECISION_M);

      if (partyNum == PARTY_B)
        for (size_t i = 0; i < size; ++i)
          input_2[i] = 0;

      if (PRIMARY)
        funcTruncate2PC(input_2, looper, size, PARTY_A, PARTY_B);

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
          &OpBase_::sendFourVectors<mpc_t>, this, ref(E_one), ref(F_one), ref(E_two), ref(F_two),
          adversary(partyNum), size, size, size, size);
        threads[1] = thread(
          &OpBase_::receiveFourVectors<mpc_t>, this, ref(temp_E_one), ref(temp_F_one),
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
        funcTruncate2PC(varD, FLOAT_PRECISION_M, size, PARTY_A, PARTY_B);

        addVectors<mpc_t>(varQ, C_two, varQ, size);
        funcTruncate2PC(varQ, FLOAT_PRECISION_M, size, PARTY_A, PARTY_B);
      }

      addVectors<mpc_t>(varP, varD, varP, size);
      addVectors<mpc_t>(quotient, varQ, quotient, size);
    }
  }

  if (FOUR_PC) {
    // I have removed 4PC, by yyl, 2020.03.12
    notYet();
  }
  return 0;
}

int Select1Of2::mpc_select_1_of_2(
  const vector<mpc_t> shared_x, const vector<mpc_t> shared_y, const vector<mpc_t> shared_bool,
  vector<mpc_t>& shared_result, size_t size) {
  vector<mpc_t> res_prod(size, 0);
  vector<mpc_t> x_minus_y(size, 0);
  subtractVectors(shared_x, shared_y, x_minus_y, size);
  GetMpcOpInner(DotProduct)->Run(x_minus_y, shared_bool, res_prod, size);
  addVectors<mpc_t>(shared_y, res_prod, shared_result, size);

  return 0;
}

int XorBit::mpc_xor_bit(
  const vector<mpc_t> shared_x, const vector<mpc_t> shared_y, vector<mpc_t>& shared_result,
  size_t size) {
  vector<mpc_t> shared_sum(size, 0);
  addVectors<mpc_t>(shared_x, shared_y, shared_sum, size);
  vector<mpc_t> shared_prod(size, 0);
  GetMpcOpInner(DotProduct)->Run(shared_x, shared_y, shared_prod, size);
  if (PRIMARY) {
    for (int i = 0; i < size; ++i) {
      shared_prod[i] = shared_prod[i] << 1;
    }
  }
  subtractVectors(shared_sum, shared_prod, shared_result, size);

  return 0;
}

  
  
//Reciprocal_div
//by LJF
int ReciprocalDiv::ReciprocalDivfor2(
  const vector<mpc_t>& shared_numerator_vec, const vector<mpc_t>& shared_denominator_vec,
  vector<mpc_t>& shared_quotient_vec, size_t vec_size, bool all_less) {
  if (all_less) {
    GetMpcOpInner(Division)->Run(shared_numerator_vec, shared_denominator_vec, shared_quotient_vec, vec_size);
    return 0;
  }//如果都是分子小于分母就可以加速处理 

  if (THREE_PC) {
    /*In the whole aspect,we are going to compute the reciprocial of shared_denominator_vec,
    then we can make the reciprocal to matmul the shared_denominator_vec.
    First, because we dicide to use the iteration function,
    so we should make sure the postivate/negativate of the den,and then get the sign of the final result, SJJ have done this part.
    Second,we should decide the iteration initial A by 3*exp(1-2*den)+0.003,the exp function we may use the polynimal to solve.
    Third,10 times for iteration is enough ,so we make each circle to compute:A=2*A-A*A*den.
    Finally,the A is 1/den,we realize num/den by num*（1/den).
    */

   /// PART 0: determine whether the values are positive or negative.
    vector<mpc_t> shared_numer_sign(vec_size, 0);
    GetMpcOpInner(ComputeMSB)->Run3PC(shared_numerator_vec, shared_numer_sign, vec_size);

    vector<mpc_t> shared_denom_sign(vec_size, 0);
    GetMpcOpInner(ComputeMSB)->Run3PC(shared_denominator_vec, shared_denom_sign, vec_size);

    vector<mpc_t> shared_sign_pos(vec_size, 0);
    if (partyNum == PARTY_A) {
      shared_sign_pos = vector<mpc_t>(vec_size, FloatToMpcType(1));//这个数据转换大概是类型转换的意思。但具体是什么意思？
    }
    vector<mpc_t> shared_sign_neg(vec_size, 0);
    if (partyNum == PARTY_A) {
      shared_sign_neg = vector<mpc_t>(vec_size, FloatToMpcType(-1));
    }
    vector<mpc_t> shared_x_sign(vec_size, 0);
    /*在这个以上，应该是解决了首位是正负号的问题*/

    // Note: actually we can do the three independent MPC calls together
    // in one call by using a triple-size vector.
    GetMpcOpInner(Select1Of2)->Run(shared_sign_neg, shared_sign_pos, shared_numer_sign, shared_x_sign, vec_size);

    vector<mpc_t> shared_y_sign(vec_size, 0);
    GetMpcOpInner(Select1Of2)->Run(shared_sign_neg, shared_sign_pos, shared_denom_sign, shared_y_sign, vec_size);

    vector<mpc_t> quotient_sign_bit(vec_size, 0);
    GetMpcOpInner(XorBit)->Run(shared_numer_sign, shared_denom_sign, quotient_sign_bit, vec_size);
    /*
    这里shared_numer_sign、shared_denom_sign这两个量在computeMSB的时候确定的就是分子分母的最高位（0、1），然后经过2选择1的之后将
    正负值转化为1和-1，靠选择shared_sign_neg和shared_sign_pos。然后确定分子分母的正负，确定系数的正负靠0和1异或，不同肯定是负数
    */

    // Note: actually we can do the three independent MPC calls together
    // in one call by using a triple-size vector.
    /*
    shared_numerator_vec:函数形参
    shared_denominator_vec:函数形参
    shared_quotient_vec：商的张量形参
    shared_x_sign:<vector>mpc_t格式的1or-1
    shared_y_sign:<vector>mpc_t格式的1or-1
    quotient_sign_bit：相同为0不同为1
    numerator_vec:新生成的分子
    denominator_vec:新生成的分母
    quotient_vec：新生成的商
    quotient_sign:商的张量的符号
    */
    vector<mpc_t> numerator_vec(vec_size, 0);
    vector<mpc_t> denominator_vec(vec_size, 0);

    GetMpcOpInner(DotProduct)->Run(shared_numerator_vec, shared_x_sign, numerator_vec, vec_size);
    GetMpcOpInner(DotProduct)->Run(shared_denominator_vec, shared_y_sign, denominator_vec, vec_size);

    vector<mpc_t> quotient_sign(vec_size, 0);
    GetMpcOpInner(Select1Of2)->Run(shared_sign_neg, shared_sign_pos, quotient_sign_bit, quotient_sign, vec_size);

    vector<mpc_t> quotient_vec = shared_quotient_vec;
    /*以下为非隐私计算下的除法运算
    result=3*exp(1-2*denominator_vec)+0.003; 
    进入迭代：
    for(i=0;i<=iteration_time;i++)
    {
      result=2*result-result*reault*denominator_vec;
    }
    
    den_reprocial=0;
    den_reprocial = result;
    quotient_vec=den_reprocial*numerator_vec*quotient_sign
    */
    
   //对于倍数关系，应该是对于share的值来说都是一样的，比如最终想要达到2x的效果那么我们需要x0和x1都2倍就可以满足，但是如果是加法的话，我们
   //就不能单纯的考虑一方，这个时候就就需要考虑share的和是整体即可，如果需要用到prg就用，不用到，一个是0，一个是C也是完全可以的
    
    vector<mpc_t> result(vec_size,0);//迭代的初值语义为初代的1/x
    vector<mpc_t> initial_temp(vec_size,0);//迭代初值计算的中间值
    vector<mpc_t> initial_exp(vec_size,0);//迭代初值的指数部分
    if (PRIMARY) {//这里的primary的条件具体含义是什么？是代表有任何一端的用户输入嘛？--是的，两个端口都是要进来的
    for (int i = 0; i < size; ++i) {
      initial_temp[i] = denominator_vec[i] << 1;
      //这里涉及到加减法，因此，我们需要share
      if (partyNum == PARTY_A) {
        initial_exp[i] = 0;
      }
      if (partyNum == PARTY_B) {
        initial_exp[i] = 1-initial_temp[i];
      }
      
    }//1-2*DEN
    for (int i = 0; i < size; ++i) {
      initial_temp[i] = initial_exp[i];
      initial_exp[i] = initial_exp[i] << 1;
      initial_temp[i]=initial_temp[i]+initial_exp[i]//这里实现了3倍
      //这里的result也需要share一下
      if(partyNum == PARTY_A) {
          result[i]=initial_temp[i]
      }
      if(partyNum == PARTY_B) {
          result[i]=result[i]+FloatToMpcType(0.003)
      }
      //result[i] = initial_exp[i] + initial_temp[i] + 0.003;//这里的数字不是十进制的，怎么实现加小数，是把0.003转化为mpc_t类型嘛？--已经解决
    }//3*exp+0.003
    //这里对于容器和常数的计算，就需要用这种形式进行每一位的运算，才能保证正确的处理，容器级的运算，就不需要这么麻烦。使用已有的算子即可。
    //这里的疑惑是，我用的是share的值，那我直接使用这种算法计算不太能保证最后得到的值就能对啊，这个算法并非经过SNN考量的。
    vector<mpc_t> iteraion_temp_2A(vec_size,0);//2*A
    vector<mpc_t> iteraion_temp_AA(vec_size,0);//A^2
    vector<mpc_t> den_reprocial(vec_size,0);//这个就是最后要和分子相乘的分母的倒数
    vector<mpc_t> quo(vec_size,0);
    for (int j = 0; j < size; ++j) {
      iteraion_temp[i] = result[i] << 1;//2*A
    }

    for(i=0;i<=iteration_time;i++)
    {
      GetMpcOpInner(DotProduct)->Run(result, result, iteraion_temp_AA, vec_size);
      GetMpcOpInner(DotProduct)->Run(iteraion_temp_AA, denominator_vec, den_reprocial, vec_size);
    }
    GetMpcOpInner(DotProduct)->Run(numerator_vec, den_reprocial, quo, vec_size);//这里是最后一步，但是还有一步，结果的正负没完成，结果的正负就存放在quotient_sign里面。
     GetMpcOpInner(DotProduct)->Run(quo, quotient_sign, shared_quotient_vec, vec_size);//这里可能可以完成？

    }
  }

  }

  }
  return 0;
}
  
// clang-format off
int DivisionV2::funcDivisionMPCV2(
  const vector<mpc_t>& shared_numerator_vec, const vector<mpc_t>& shared_denominator_vec,
  vector<mpc_t>& shared_quotient_vec, size_t vec_size, bool all_less) {
  if (all_less) {
    GetMpcOpInner(Division)->Run(shared_numerator_vec, shared_denominator_vec, shared_quotient_vec, vec_size);
    return 0;
  }

  if (THREE_PC) {
    mpc_t LEN = 8 * sizeof(mpc_t);

    /// PART 0: determine whether the values are positive or negative.
    vector<mpc_t> shared_numer_sign(vec_size, 0);
    GetMpcOpInner(ComputeMSB)->Run3PC(shared_numerator_vec, shared_numer_sign, vec_size);

    vector<mpc_t> shared_denom_sign(vec_size, 0);
    GetMpcOpInner(ComputeMSB)->Run3PC(shared_denominator_vec, shared_denom_sign, vec_size);

    // if (PRIMARY) {
    // 	funcReconstruct2PC(shared_numer_sign, vec_size, "x sign");
    // 	funcReconstruct2PC(shared_denom_sign, vec_size, "y sign");
    // }

    vector<mpc_t> shared_sign_pos(vec_size, 0);
    if (partyNum == PARTY_A) {
      shared_sign_pos = vector<mpc_t>(vec_size, FloatToMpcType(1));
    }
    vector<mpc_t> shared_sign_neg(vec_size, 0);
    if (partyNum == PARTY_A) {
      shared_sign_neg = vector<mpc_t>(vec_size, FloatToMpcType(-1));
    }
    vector<mpc_t> shared_x_sign(vec_size, 0);

    // Note: actually we can do the three independent MPC calls together
    // in one call by using a triple-size vector.
    GetMpcOpInner(Select1Of2)->Run(shared_sign_neg, shared_sign_pos, shared_numer_sign, shared_x_sign, vec_size);

    vector<mpc_t> shared_y_sign(vec_size, 0);
    GetMpcOpInner(Select1Of2)->Run(shared_sign_neg, shared_sign_pos, shared_denom_sign, shared_y_sign, vec_size);

    vector<mpc_t> quotient_sign_bit(vec_size, 0);
    GetMpcOpInner(XorBit)->Run(shared_numer_sign, shared_denom_sign, quotient_sign_bit, vec_size);

    // Note: actually we can do the three independent MPC calls together
    // in one call by using a triple-size vector.
    vector<mpc_t> numerator_vec(vec_size, 0);
    vector<mpc_t> denominator_vec(vec_size, 0);

    GetMpcOpInner(DotProduct)->Run(shared_numerator_vec, shared_x_sign, numerator_vec, vec_size);
    GetMpcOpInner(DotProduct)->Run(shared_denominator_vec, shared_y_sign, denominator_vec, vec_size);

    vector<mpc_t> quotient_sign(vec_size, 0);
    GetMpcOpInner(Select1Of2)->Run(shared_sign_neg, shared_sign_pos, quotient_sign_bit, quotient_sign, vec_size);

    vector<mpc_t> quotient_vec = shared_quotient_vec;

    // PART I: compute the compute the integer part.
    vector<mpc_t> curr_q(vec_size, 0);
    auto curr_x = numerator_vec;
    auto curr_y = denominator_vec;
    // this is just like the original algorithm 8 in PET's paper.
    for (int i = LEN - 1 - FLOAT_PRECISION_M; i >= 0; --i) {
      auto shared_z = curr_x;
      vector<mpc_t> shared_beta(vec_size, 0);
      if (PRIMARY) {
        funcTruncate2PC(shared_z, i, vec_size, PARTY_A, PARTY_B);
        subtractVectors<mpc_t>(shared_z, curr_y, shared_z, vec_size);
      }

      GetMpcOpInner(ReluPrime)->Run3PC(shared_z, shared_beta, vec_size);

      // selection base on beta
      vector<mpc_t> candidate_x_update(vec_size, 0);
      vector<mpc_t> candidate_q_update(vec_size, 0);
      if (PRIMARY) {
        for (auto j = 0; j < vec_size; ++j) {
          // TODO: check whether this is OK: direct local left shift.
          candidate_x_update[j] = curr_y[j] << i;
          if (partyNum == PARTY_A) {
            // to set it in the ring value!
            candidate_q_update[j] = (1 << i) << FLOAT_PRECISION_M;
          } else if (partyNum == PARTY_B) {
            candidate_q_update[j] = 0;
          }
        }
      }
      GetMpcOpInner(SelectShares)->Run3PC(candidate_x_update, shared_beta, candidate_x_update, vec_size);
      GetMpcOpInner(SelectShares)->Run3PC(candidate_q_update, shared_beta, candidate_q_update, vec_size);

      addVectors<mpc_t>(curr_q, candidate_q_update, curr_q, vec_size);
      subtractVectors<mpc_t>(curr_x, candidate_x_update, curr_x, vec_size);
    }
    // if(PRIMARY)
    //	  funcReconstruct2PC(curr_q, vec_size, "curr_q");

    // compute remainder: x = x -(q*y)
    vector<mpc_t> curr_prod(vec_size, 0);
    GetMpcOpInner(DotProduct)->Run(curr_y, curr_q, curr_prod, vec_size);
    subtractVectors(numerator_vec, curr_prod, curr_x, vec_size);

    // PART II: compute the fractional part,
    // if(PRIMARY)
    for (int i = 1; i < FLOAT_PRECISION_M + 1; ++i) {
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
      GetMpcOpInner(ReluPrime)->Run3PC(shared_z, shared_beta, vec_size);

      // selection base on beta
      if (PRIMARY) {
        for (auto j = 0; j < vec_size; ++j) {
          // share of 2^(f-i)
          if (partyNum == PARTY_A) {
            candidate_q_update[j] = 1 << (FLOAT_PRECISION_M - i);
          } else if (partyNum == PARTY_B) {
            candidate_q_update[j] = 0;
          }
        }
      }
      GetMpcOpInner(SelectShares)->Run3PC(candidate_x_update, shared_beta, candidate_x_update, vec_size);
      GetMpcOpInner(SelectShares)->Run3PC(candidate_q_update, shared_beta, candidate_q_update, vec_size);

      addVectors<mpc_t>(curr_q, candidate_q_update, curr_q, vec_size);
      subtractVectors<mpc_t>(curr_x, candidate_x_update, curr_x, vec_size);
    }
    quotient_vec = curr_q;
    ////funcDotProductMPC(quotient_vec, quotient_sign, shared_quotient_vec, vec_size);
    GetMpcOpInner(DotProduct)->Run(quotient_vec, quotient_sign, shared_quotient_vec, vec_size);
  }
  if (FOUR_PC) {
    // cout << "ERROR! not support yet!" <<endl;
    // TODO :throw exception
    notYet();
    return 1;
  }
  return 0;
}

int FloorDivision::mpc_floor_division(
  const vector<mpc_t>& ori_numerator_vec, const vector<mpc_t>& ori_denominator_vec,
  vector<mpc_t>& ori_quotient_vec, size_t common_vec_size, bool all_positive) {
  if (THREE_PC) {
    mpc_t LEN = 8 * sizeof(mpc_t);
    vector<mpc_t> shared_numerator_vec = ori_numerator_vec;
    vector<mpc_t> shared_denominator_vec = ori_denominator_vec;
    vector<mpc_t>& shared_quotient_vec = ori_quotient_vec;
    vector<mpc_t> quotient_sign(common_vec_size, 0);
    vector<mpc_t> shared_offset(common_vec_size, 0);
    if (!all_positive) {
      /// PART 0: determine whether the values are positive or negative.
      vector<mpc_t> shared_numer_sign(common_vec_size, 0);
      GetMpcOpInner(ComputeMSB)->Run3PC(shared_numerator_vec, shared_numer_sign, common_vec_size);

      vector<mpc_t> shared_denom_sign(common_vec_size, 0);
      GetMpcOpInner(ComputeMSB)->Run3PC(shared_denominator_vec, shared_denom_sign, common_vec_size);

      vector<mpc_t> shared_sign_pos(common_vec_size, 0);
      if (partyNum == PARTY_A) {
        shared_sign_pos = vector<mpc_t>(common_vec_size, FloatToMpcType(1));
      }
      vector<mpc_t> shared_sign_neg(common_vec_size, 0);
      if (partyNum == PARTY_A) {
        shared_sign_neg = vector<mpc_t>(common_vec_size, FloatToMpcType(-1));
      }
      // Note: this is for negative case floor:
      //    eg : math.floor(-6/4) = -math.floor(6/4) + (-1)
      vector<mpc_t> shared_zero(common_vec_size, 0);
      vector<mpc_t> shared_neg_one(common_vec_size, 0);
      if(partyNum == PARTY_A) {
        shared_neg_one = vector<mpc_t>(common_vec_size, FloatToMpcType(-1));
      }
      
      vector<mpc_t> shared_x_sign(common_vec_size, 0);
      // Note: actually we can do the three independent MPC calls together
      // in one call by using a double-size vector.
      GetMpcOpInner(Select1Of2)->Run(shared_sign_neg, shared_sign_pos, shared_numer_sign, shared_x_sign, common_vec_size);
      
      vector<mpc_t> shared_y_sign(common_vec_size, 0);
      GetMpcOpInner(Select1Of2)->Run(shared_sign_neg, shared_sign_pos, shared_denom_sign, shared_y_sign, common_vec_size);
     
      vector<mpc_t> quotient_sign_bit(common_vec_size, 0);
      GetMpcOpInner(XorBit)->Run(shared_numer_sign, shared_denom_sign, quotient_sign_bit, common_vec_size);

      // Note: actually we can do the three independent MPC calls together
      // in one call by using a double-size vector.
      vector<mpc_t> numerator_vec(common_vec_size, 0);
      vector<mpc_t> denominator_vec(common_vec_size, 0);
      GetMpcOpInner(DotProduct)->Run(shared_numerator_vec, shared_x_sign, numerator_vec, common_vec_size);
      GetMpcOpInner(DotProduct)->Run(shared_denominator_vec, shared_y_sign, denominator_vec, common_vec_size);

      GetMpcOpInner(Select1Of2)->Run(shared_sign_neg, shared_sign_pos, quotient_sign_bit, quotient_sign, common_vec_size);
      GetMpcOpInner(Select1Of2)->Run(shared_neg_one, shared_zero, quotient_sign_bit, shared_offset, common_vec_size); 
      vector<mpc_t> quotient_vec = shared_quotient_vec;
      shared_numerator_vec = numerator_vec;
      shared_denominator_vec = denominator_vec;
    }

    vector<mpc_t> curr_q(common_vec_size, 0);
    auto curr_x = shared_numerator_vec;
    auto curr_y = shared_denominator_vec;
    // this is just like the original algorithm 8 in PET's paper.
    for (int i = LEN - 1 - FLOAT_PRECISION_M; i >= 0; --i) {
      auto shared_z = curr_x;
      vector<mpc_t> shared_beta(common_vec_size, 0);
      if (PRIMARY) {
        funcTruncate2PC(shared_z, i, common_vec_size, PARTY_A, PARTY_B);
        subtractVectors<mpc_t>(shared_z, curr_y, shared_z, common_vec_size);
      }

      GetMpcOpInner(ReluPrime)->Run3PC(shared_z, shared_beta, common_vec_size);
      
      // selection base on beta
      vector<mpc_t> candidate_x_update(common_vec_size, 0);
      vector<mpc_t> candidate_q_update(common_vec_size, 0);
      if (PRIMARY) {
        for (auto j = 0; j < common_vec_size; ++j) {
          // TODO: check whether this is OK: direct local left shift.
          candidate_x_update[j] = curr_y[j] << i;
          if (partyNum == PARTY_A) {
            // to set it in the ring value!
            candidate_q_update[j] = (1 << i) << FLOAT_PRECISION_M;
          } else if (partyNum == PARTY_B) {
            candidate_q_update[j] = 0;
          }
        }
      }
      GetMpcOpInner(SelectShares)->Run3PC(candidate_x_update, shared_beta, candidate_x_update, common_vec_size);
      GetMpcOpInner(SelectShares)->Run3PC(candidate_q_update, shared_beta, candidate_q_update, common_vec_size);
      
      addVectors<mpc_t>(curr_q, candidate_q_update, curr_q, common_vec_size);
      subtractVectors<mpc_t>(curr_x, candidate_x_update, curr_x, common_vec_size);
    }
    if (all_positive) {
      shared_quotient_vec = curr_q;
    } else {
      GetMpcOpInner(DotProduct)->Run(curr_q, quotient_sign, shared_quotient_vec, common_vec_size);
      addVectors<mpc_t>(shared_quotient_vec, shared_offset, shared_quotient_vec, common_vec_size); 
    }
  }
  return 0;
}

int FracDivision::mpc_frac_division(
  const vector<mpc_t>& ori_numerator_vec, const vector<mpc_t>& ori_denominator_vec,
  vector<mpc_t>& ori_quotient_vec, size_t common_vec_size, bool all_positive) {
  if (THREE_PC) {
    // mpc_t LEN = 8 * sizeof(mpc_t);
    vector<mpc_t> shared_numerator_vec = ori_numerator_vec;
    vector<mpc_t> shared_denominator_vec = ori_denominator_vec;
    vector<mpc_t>& shared_quotient_vec = ori_quotient_vec;
    vector<mpc_t> quotient_sign(common_vec_size, 0);
    if (!all_positive) {
      /// PART 0: determine whether the values are positive or negative.
      vector<mpc_t> shared_numer_sign(common_vec_size, 0);
      GetMpcOpInner(ComputeMSB)->Run3PC(shared_numerator_vec, shared_numer_sign, common_vec_size);
      
      vector<mpc_t> shared_denom_sign(common_vec_size, 0);
      GetMpcOpInner(ComputeMSB)->Run3PC(shared_denominator_vec, shared_denom_sign, common_vec_size);

      vector<mpc_t> shared_sign_pos(common_vec_size, 0);
      if (partyNum == PARTY_A) {
        shared_sign_pos = vector<mpc_t>(common_vec_size, FloatToMpcType(1));
      }
      vector<mpc_t> shared_sign_neg(common_vec_size, 0);
      if (partyNum == PARTY_A) {
        shared_sign_neg = vector<mpc_t>(common_vec_size, FloatToMpcType(-1));
      }
      vector<mpc_t> shared_x_sign(common_vec_size, 0);

      // Note: actually we can do the three independent MPC calls together
      // in one call by using a double-size vector.
      GetMpcOpInner(Select1Of2)->Run(shared_sign_neg, shared_sign_pos, shared_numer_sign, shared_x_sign, common_vec_size);
      
      vector<mpc_t> shared_y_sign(common_vec_size, 0);
      GetMpcOpInner(Select1Of2)->Run(shared_sign_neg, shared_sign_pos, shared_denom_sign, shared_y_sign, common_vec_size);
     
      vector<mpc_t> quotient_sign_bit(common_vec_size, 0);
      GetMpcOpInner(XorBit)->Run(shared_numer_sign, shared_denom_sign, quotient_sign_bit, common_vec_size);

      // Note: actually we can do the three independent MPC calls together
      // in one call by using a double-size vector.
      vector<mpc_t> numerator_vec(common_vec_size, 0);
      vector<mpc_t> denominator_vec(common_vec_size, 0);
      GetMpcOpInner(DotProduct)->Run(shared_numerator_vec, shared_x_sign, numerator_vec, common_vec_size);
      GetMpcOpInner(DotProduct)->Run(shared_denominator_vec, shared_y_sign, denominator_vec, common_vec_size);

      GetMpcOpInner(Select1Of2)->Run(shared_sign_neg, shared_sign_pos, quotient_sign_bit, quotient_sign, common_vec_size);

      vector<mpc_t> quotient_vec = shared_quotient_vec;
      shared_numerator_vec = numerator_vec;
      shared_denominator_vec = denominator_vec;
    }
    vector<mpc_t> curr_q(common_vec_size, 0);
    auto curr_x = shared_numerator_vec;
    auto curr_y = shared_denominator_vec;
    for (int i = 1; i < FLOAT_PRECISION_M + 1; ++i) {
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
      GetMpcOpInner(ReluPrime)->Run3PC(shared_z, shared_beta, common_vec_size);
      // selection base on beta

      if (PRIMARY) {
        for (auto j = 0; j < common_vec_size; ++j) {
          // share of 2^(f-i)
          if (partyNum == PARTY_A) {
            candidate_q_update[j] = 1 << (FLOAT_PRECISION_M - i);
          } else if (partyNum == PARTY_B) {
            candidate_q_update[j] = 0;
          }
        }
      }
      GetMpcOpInner(SelectShares)->Run3PC(candidate_x_update, shared_beta, candidate_x_update, common_vec_size);
      GetMpcOpInner(SelectShares)->Run3PC(candidate_q_update, shared_beta, candidate_q_update, common_vec_size);

      addVectors<mpc_t>(curr_q, candidate_q_update, curr_q, common_vec_size);
      subtractVectors<mpc_t>(curr_x, candidate_x_update, curr_x, common_vec_size);
    }

    if (all_positive) {
      shared_quotient_vec = curr_q;
    } else {
      GetMpcOpInner(DotProduct)->Run(curr_q, quotient_sign, shared_quotient_vec, common_vec_size);
    }
  }
  return 0;
}
// clang-format on
} // namespace mpc
} // namespace rosetta
