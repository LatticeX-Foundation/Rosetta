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

#include "cc/modules/protocol/mpc/helix/include/helix_internal.h"

#include <iostream>
#include <vector>
#include <cassert>
#include <cmath>
#include <string>

using namespace std;

namespace rosetta {
namespace helix {
void HelixInternal::Select1Of2(const vector<Share>& X, const vector<Share>& Y,
                  const vector<Share>& cond, vector<Share>& result, bool is_scaled) {
    AUDIT("id:{}, P{} Select1Of2, compute: Z=cond*X = (1-cond)*Y = cond*(X-Y)+Y where cond is 0 or 1, input X(Share){}", msgid.get_hex(), player, Vector<Share>(X));
    AUDIT("id:{}, P{} Select1Of2, compute: Z=cond*X = (1-cond)*Y = cond*(X-Y)+Y where cond is 0 or 1, input Y(Share){}", msgid.get_hex(), player, Vector<Share>(Y));
    AUDIT("id:{}, P{} Select1Of2, compute: Z=cond*X = (1-cond)*Y = cond*(X-Y)+Y where cond is 0 or 1, input cond(Share){}", msgid.get_hex(), player, Vector<Share>(cond));
    vector<Share> res_prod(X.size());
    vector<Share> X_minus_Y(X.size());
    Sub(X, Y, X_minus_Y);
    // Note: the cond should be non-scaled.
    Mul(X_minus_Y, cond, res_prod, is_scaled);
    Add(Y, res_prod, result);

    AUDIT("id:{}, P{} Select1Of2, compute: Z=cond*X = (1-cond)*Y = cond*(X-Y)+Y where cond is 0 or 1, output Z(Share){}", msgid.get_hex(), player, Vector<Share>(result));
}

void HelixInternal::Select1Of2(const vector<Share>& X, const vector<Share>& Y,
                  const vector<BitShare>& cond, vector<Share>& result) {
    //tlog_error << "ERROR! not implemented Yet! HelixInternal::Select1Of2 for BitShare." ;
    ERROR("ERROR! not implemented Yet! HelixInternal::Select1Of2 for BitShare.");
}

void HelixInternal::Select1Of2(const vector<double>& X, const vector<double>& Y,
                  const vector<Share>& cond, vector<Share>& result) {
    AUDIT("id:{}, P{} Select1Of2, compute: Z=cond*X = (1-cond)*Y = cond*(X-Y)+Y where cond is 0 or 1, input X(double){}", msgid.get_hex(), player, Vector<double>(X));
    AUDIT("id:{}, P{} Select1Of2, compute: Z=cond*X = (1-cond)*Y = cond*(X-Y)+Y where cond is 0 or 1, input Y(double){}", msgid.get_hex(), player, Vector<double>(Y));
    AUDIT("id:{}, P{} Select1Of2, compute: Z=cond*X = (1-cond)*Y = cond*(X-Y)+Y where cond is 0 or 1, input cond(Share){}", msgid.get_hex(), player, Vector<Share>(cond));
    vector<Share> res_prod(X.size());
    vector<double> X_minus_Y(X.size());
    for(int i = 0; i < X.size(); ++i) {
        X_minus_Y[i] = X[i] - Y[i];
    }
    vector<mpc_t> fpC;
    convert_plain_to_fixpoint(X_minus_Y, fpC, GetMpcContext()->FLOAT_PRECISION);
    // Note the cond paramter is comparison result, no need to scale.
    Mul(cond, fpC, res_prod, false);
    Add(Y, res_prod, result);    

    AUDIT("id:{}, P{} Select1Of2, compute: Z=cond*X = (1-cond)*Y = cond*(X-Y)+Y where cond is 0 or 1, output Z(Share){}", msgid.get_hex(), player, Vector<Share>(result));
}

void HelixInternal::XORShare(const vector<Share>& X,
                        const vector<Share>& Y,
                        vector<Share>& Z) {
    AUDIT("id:{}, P{} XORShare, compute: Z=(X+Y)-(X*Y*2), input X(Share){}", msgid.get_hex(), player, Vector<Share>(X));
    AUDIT("id:{}, P{} XORShare, compute: Z=(X+Y)-(X*Y*2), input Y(Share){}", msgid.get_hex(), player, Vector<Share>(Y));
    int vec_size = X.size();
    vector<Share> sum(vec_size);
    Add(X, Y, sum);
    vector<Share> prod(vec_size);
    Mul(X, Y, prod);
    // optimized for locally multipling constants (no-scaled) without needing truncating.
    vector<mpc_t> two(vec_size, (mpc_t)2L);
    vector<Share> tmp(vec_size);
    Mul(prod, two, tmp, false);
    Sub(sum, tmp, Z);

    AUDIT("id:{}, P{} XORShare, compute: Z=(X+Y)-(X*Y*2), output Z(Share){}", msgid.get_hex(), player, Vector<Share>(Z));
}

void HelixInternal::Div(const vector<Share>& X,
                        const vector<Share>& Y,
                        vector<Share>& Z) {    
    AUDIT("id:{}, P{} Div(Share), input X(Share){}", msgid.get_hex(), player, Vector<Share>(X));
    AUDIT("id:{}, P{} Div(Share), input Y(Share){}", msgid.get_hex(), player, Vector<Share>(Y));
    // cout << "calling HelixInternal::Div(SSS) " << endl;
    size_t vec_size = X.size();
    // Note this is actually a constant: 64
    int LEN = 8 * sizeof(mpc_t); 

    /** 
     * PART 0: 
     *  Determine whether the values are positive or negative.
     *  And change numerator and denominator to positive one.
    */
    vector<Share> s_numer_sign(vec_size);
    vector<Share> s_denom_sign(vec_size);
    DReLU(X, s_numer_sign);
    DReLU(Y, s_denom_sign);
    
    vector<double> CONST_ONE(vec_size, 1.0);
    vector<double> CONST_MINUS_ONE(vec_size, -1.0);

    vector<Share> numer_sign_multiplier(vec_size);
    vector<Share> denom_sign_multiplier(vec_size);
    Select1Of2(CONST_ONE, CONST_MINUS_ONE, s_numer_sign, numer_sign_multiplier);
    Select1Of2(CONST_ONE, CONST_MINUS_ONE, s_denom_sign, denom_sign_multiplier);
    // TODO: optimized with XOR of MSB(X) and MSB(Y) and B2A
    vector<Share> quotient_sign_multiplier(vec_size);
    Mul(numer_sign_multiplier, denom_sign_multiplier, quotient_sign_multiplier);

    vector<Share> numerator_vec(vec_size);
    vector<Share> denominator_vec(vec_size);
    Mul(X, numer_sign_multiplier, numerator_vec);
    Mul(Y, denom_sign_multiplier, denominator_vec);

    /** 
     * PART 1: 
     *  get the integer part.
    */
    vector<Share> curr_q(vec_size);
    auto curr_x = numerator_vec;
    auto abs_y = denominator_vec;
    // traditional Long division approach
    // define some common used temp variables
    vector<double> CONST_ZERO_D(vec_size, 0.0);
    vector<Share> CONST_ZERO_SHARE(vec_size);
    vector<Share> shared_beta(vec_size);
    vector<Share> x_update(vec_size);
    vector<Share> q_update(vec_size);
    int float_precision = GetMpcContext()->FLOAT_PRECISION;
    for (int i = LEN - 1 - float_precision; i >= 0; --i) {
        auto shared_z = curr_x;
        vector<Share> shared_beta(vec_size);
        // we Trunc X rather than scale Y to avoid the error occurred if scaling Y leads its overflow.      
        Trunc(shared_z, vec_size, i);
        Sub(shared_z, abs_y);
        DReLU(shared_z, shared_beta);
        vector<Share> candidate_x_update = abs_y;
        // y << i
        Scale(candidate_x_update, i);

        vector<double> candidate_q_update(vec_size, double(1 << i));

        Select1Of2(candidate_x_update, CONST_ZERO_SHARE, shared_beta, x_update);
        Select1Of2(candidate_q_update, CONST_ZERO_D, shared_beta, q_update);
        Add(curr_q, q_update);
        Sub(curr_x, x_update);
    }
    // PART 2: compute the fractional part,
    for (int i = 1; i < float_precision + 1; ++i) {
        vector<Share> shared_z(vec_size);
        // each time test z = (x << 1) - y
        Scale(curr_x, 1);
        Sub(curr_x, abs_y, shared_z);
        vector<Share> shared_beta(vec_size);
        DReLU(shared_z, shared_beta);
        //Scale(shared_beta, shared_beta_scaled);

        vector<double> candidate_q_update(vec_size, double(1.0 /(1 << i)));

        Select1Of2(abs_y, CONST_ZERO_SHARE, shared_beta, x_update);
        Select1Of2(candidate_q_update, CONST_ZERO_D, shared_beta, q_update);
        Add(curr_q, q_update);
        Sub(curr_x, x_update);
    }
    Mul(quotient_sign_multiplier, curr_q, Z);

    AUDIT("id:{}, P{} Div(Share), output Z{}", msgid.get_hex(), player, Vector<Share>(Z));
}

void HelixInternal::Div(const vector<Share>& X, 
                        const vector<double>& C,
                        vector<Share>& Z) {
    AUDIT("id:{}, P{} Div(C,S), compute: Z=X/Y, input X(Share){}", msgid.get_hex(), player, Vector<Share>(X));
    AUDIT("id:{}, P{} Div(C,S), compute: Z=X/Y, input C(double){}", msgid.get_hex(), player, Vector<double>(C));
    // cout << "calling HelixInternal::Div(SDS)" << endl;
    assert(X.size() == C.size());
    int vec_size = C.size();
    // local division and trunc: X * (1/C)
    // RevealAndPrint(X, "HelixInternal::Div(SDS) input X:");

    int float_precision = GetMpcContext()->FLOAT_PRECISION;
    vector<double> tmp_v(vec_size);
    vector<size_t> power_list(vec_size, float_precision);
    for(auto i = 0; i < vec_size; ++i) {
        tmp_v[i] = 1.0 / C[i];
        // This is for dealing with big constant denominator, part I:
        //  scale it up by left-shifting
        double abs_v = abs(C[i]);
        if(abs_v > 1) {
            power_list[i] = ceil(log2(abs_v));
            tmp_v[i] = tmp_v[i] * (1 << power_list[i]);
            power_list[i] = power_list[i] + float_precision;
        }
        tlog_debug << "HelixInternal::Div " << C[i] << " -> " << tmp_v[i] << \
                " [" << power_list[i] << "]" ; 
    }

    vector<mpc_t> fp_C;
    convert_plain_to_fixpoint(tmp_v, fp_C, GetMpcContext()->FLOAT_PRECISION);
    // We will do the batch truncation later.
    Mul(X, fp_C, Z, false);

    // This is for dealing with big constant denominator, part II:
    //  trunc it back in the final result
    vector<Share> single_share(1);
    Trunc_many(Z, Z.size(), power_list);

    AUDIT("id:{}, P{} Div(C,S), compute: Z=X/Y, output Z(Share){}", msgid.get_hex(), player, Vector<Share>(Z));
}

void HelixInternal::Div(const vector<double>& C,
                        const vector<Share>& X,
                        vector<Share>& Z) {
    AUDIT("id:{}, P{} Div(C,S), compute: Z=X/Y, input X(double){}", msgid.get_hex(), player, Vector<double>(C));
    AUDIT("id:{}, P{} Div(C,S), compute: Z=X/Y, input Y(Share){}", msgid.get_hex(), player, Vector<Share>(X));
    // cout << "calling HelixInternal::Div(DSS)" << endl;
    int vec_size = C.size();
    vector<Share> SHARE_C(vec_size);
    // Input(0, C, SHARE_C);
    ConstCommonInput(C, SHARE_C);
    vector<Share> tmp_q(vec_size);
    Div(SHARE_C, X, Z);

    AUDIT("id:{}, P{} Div(C,S), compute: Z=X/Y, output Z(Share){}", msgid.get_hex(), player, Vector<Share>(Z));
}

void HelixInternal::Truediv(const vector<Share>& X,
                        const vector<Share>& Y,
                        vector<Share>& Z) {
    Div(X, Y, Z);
}

void HelixInternal::Truediv(const vector<Share>& X,
                        const vector<double>& C,
                        vector<Share>& Z) {
    Div(X, C, Z);
}

void HelixInternal::Truediv(const vector<double>& C,
                        const vector<Share>& X,
                        vector<Share>& Z) {
    Div(C, X, Z);
}

// Note: you can see this a simplified version of Div.
void HelixInternal::Floordiv(const vector<Share>& X,
                        const vector<Share>& Y, vector<Share>& Z) {
    AUDIT("id:{}, P{} FloorDiv(S,S), compute: Z=X/Y, input X(Share){}", msgid.get_hex(), player, Vector<Share>(X));
    AUDIT("id:{}, P{} FloorDiv(S,S), compute: Z=X/Y, input Y(Share){}", msgid.get_hex(), player, Vector<Share>(Y));
    // cout << "calling HelixInternal::FloorDiv(SSS)" << endl;
    size_t vec_size = X.size();
    // Note this is actually a constant: 64
    int LEN = 8 * sizeof(mpc_t); 
    vector<Share> curr_Z(vec_size);
    /** 
     * PART 0: 
     *  Determine whether the values are positive or negative.
     *  And change numerator and denominator to positive one.
    */
    vector<Share> s_numer_sign(vec_size);
    vector<Share> s_denom_sign(vec_size);
    DReLU(X, s_numer_sign);
    DReLU(Y, s_denom_sign);
    // RevealAndPrint(X, "FLOORDIV input X:");
    // RevealAndPrint(Y, "FLOORDIV input Y:");

    vector<double> CONST_ONE(vec_size, 1.0);
    vector<double> CONST_MINUS_ONE(vec_size, -1.0);
    vector<double> CONST_ZERO_D(vec_size, 0.0);
    vector<Share> CONST_ZERO_SHARE(vec_size);

    vector<Share> numer_sign_multiplier(vec_size);
    vector<Share> denom_sign_multiplier(vec_size);
    Select1Of2(CONST_ONE, CONST_MINUS_ONE, s_numer_sign, numer_sign_multiplier);
    Select1Of2(CONST_ONE, CONST_MINUS_ONE, s_denom_sign, denom_sign_multiplier);
    // vector<Share> quotient_sign_multiplier(vec_size);
    // Mul(numer_sign_multiplier, denom_sign_multiplier, quotient_sign_multiplier);
    
    // Note: this is for negative case floor:
    //    eg : math.floor(-6/4) = -math.floor(6/4) + (-1)
    //         math.floor(-6/-4) = +math.floor(6/4) + 0
    vector<Share> floor_offset(vec_size);
    // TODO: optimize this to use non-scaled version!
    vector<Share> neg_quotient_sign(vec_size);
    vector<Share> XX(vec_size), YY(vec_size);
	int float_precision = GetMpcContext()->FLOAT_PRECISION;
    Scale(s_numer_sign, XX, float_precision);
    Scale(s_denom_sign, YY, float_precision);
    XORShare(XX, YY, neg_quotient_sign);
    vector<Share> neg_quotient_sign_trunced = neg_quotient_sign;
    Trunc(neg_quotient_sign_trunced, vec_size, float_precision);
    
    Sub(CONST_ZERO_D, neg_quotient_sign, floor_offset);
    // RevealAndPrint(neg_quotient_sign, "quotient_sign:");
    // RevealAndPrint(floor_offset, "floor_offset:");
    vector<Share> quotient_sign_multiplier(vec_size);
    Select1Of2(CONST_MINUS_ONE, CONST_ONE, neg_quotient_sign_trunced, quotient_sign_multiplier);

    vector<Share> numerator_vec(vec_size);
    vector<Share> denominator_vec(vec_size);
    Mul(X, numer_sign_multiplier, numerator_vec);
    Mul(Y, denom_sign_multiplier, denominator_vec);

    // RevealAndPrint(numerator_vec, "PosDiv X:");
    // RevealAndPrint(denominator_vec, "PosDiv Y:");
    vector<Share> curr_q(vec_size);
    auto curr_x = numerator_vec;
    auto abs_y = denominator_vec;
    // traditional Long division approach
    // define some common used temp variables
    vector<Share> shared_beta(vec_size);
    vector<Share> shared_beta_scaled(vec_size);
    vector<Share> x_update(vec_size);
    vector<Share> q_update(vec_size);
    for (int i = LEN - 1 - float_precision; i >= 0; --i) {
        auto shared_z = curr_x;
        vector<Share> shared_beta(vec_size);
        Trunc(shared_z, vec_size, i);
        Sub(shared_z, abs_y);
        DReLU(shared_z, shared_beta);
        //Scale(shared_beta, shared_beta_scaled);

        vector<Share> candidate_x_update = abs_y;
        // y << i
        Scale(candidate_x_update, i);

        vector<double> candidate_q_update(vec_size, double(1 << i));

        Select1Of2(candidate_x_update, CONST_ZERO_SHARE, shared_beta, x_update);
        Select1Of2(candidate_q_update, CONST_ZERO_D, shared_beta, q_update);
        Add(curr_q, q_update);
        Sub(curr_x, x_update);
        // fix bug for math.floor(-8/4) = -3!
        if(i == 0) {
            // 20210721: part I to fix bug such as (-2)/2, we got - (2/2) - 1 = - 2, not -1
            //  Note that if curr_x is still bigger than 0, it means actually it has decimal part 
            //  if we are using RealDivision.
            vector<Share> indivisable(vec_size);
            // RevealAndPrint(curr_x, "curr_x val:");
            NotEqual(curr_x, CONST_ZERO_SHARE, indivisable);
            Scale(indivisable, float_precision);
            // RevealAndPrint(indivisable, "indivisable val:");
            // if neg, we minus 1 only when it is indivisiable;
            vector<Share> tmp_vec(vec_size);
            Mul(floor_offset, indivisable, tmp_vec);
            floor_offset.swap(tmp_vec);
            // RevealAndPrint(floor_offset, "floor_offset val 2:");
        }
    }
    // RevealAndPrint(curr_q, "abs Q:");
    Mul(quotient_sign_multiplier, curr_q, curr_Z);
    // RevealAndPrint(curr_Z, "signed Q:");
    Add(curr_Z, floor_offset, Z);

    AUDIT("id:{}, P{} FloorDiv(S,S), compute: Z=X/Y, output Z{}", msgid.get_hex(), player, Vector<Share>(Z));
}

void HelixInternal::Floordiv(const vector<Share>& X,
                        const vector<double>& C,
                        vector<Share>& Z) {
    AUDIT("id:{}, P{} FloorDiv(S,D), input X(Share){}", msgid.get_hex(), player, Vector<Share>(X));
    AUDIT("id:{}, P{} FloorDiv(S,D), input C(double){}", msgid.get_hex(), player, Vector<double>(C));
    // cout << "calling HelixInternal::FloorDiv(SDS)" << endl;
    int vec_size = X.size();  
    vector<Share> shareC(vec_size);
    // (0, C, shareC);
    ConstCommonInput(C, shareC);
    Floordiv(X, shareC, Z);
    
    // optimized version:
    // int vec_size = C.size();
    // vector<Share> curr_z(vec_size);
    // // local division and trunc: X * (1/C)
    // vector<double> tmp_v(vec_size);
    // for(auto i = 0; i < vec_size; ++i) {
    //     tmp_v[i] = 1.0 / C[i];
    // }
    // vector<mpc_t> fp_C;
    // convert_plain_to_fixpoint(tmp_v, fp_C);
    // Mul(X, fp_C, curr_z);
    // RevealAndPrint(curr_z, "curr_z:");
    // Trunc(curr_z, vec_size);
    // RevealAndPrint(curr_z, "truncated curr_z:");
    // Scale(curr_z);
    // RevealAndPrint(curr_z, "scaled curr_z:");
    // Z.swap(curr_z);

    AUDIT("id:{}, P{} FloorDiv(S,D), compute: Z=X/Y, output Z(Share){}", msgid.get_hex(), player, Vector<Share>(Z));
}

void HelixInternal::Floordiv(const vector<double>& C,
                        const vector<Share>& X,
                        vector<Share>& Z) {
    AUDIT("id:{}, P{} FloorDiv(C, S), compute: Z=X/Y, input X(double){}", msgid.get_hex(), player, Vector<double>(C));
    AUDIT("id:{}, P{} FloorDiv(C, S), compute: Z=X/Y, input Y(Share){}", msgid.get_hex(), player, Vector<Share>(X));
    // cout << "calling HelixInternal::FloorDiv(DSS)" << endl;
    int vec_size = X.size();  
    vector<Share> shareC(vec_size);
    // Input(0, C, shareC);
    ConstCommonInput(C, shareC);
    Floordiv(shareC, X, Z);

    // Optimized version
    // vector<double> CONST_ONE(vec_size, 1.0);
    // vector<Share> curr_z(vec_size);
    // Div(CONST_ONE, X, curr_z);
    // vector<mpc_t> fp_C;
    // convert_plain_to_fixpoint(C, fp_C);
    // Mul(X, fp_C, curr_z);
    // RevealAndPrint(curr_z, "curr_z:");
    // Trunc(curr_z, vec_size);
    // RevealAndPrint(curr_z, "truncated curr_z:");
    // Scale(curr_z);
    // RevealAndPrint(curr_z, "scaled curr_z:");
    // Z.swap(curr_z); 
    AUDIT("id:{}, P{} FloorDiv(C, S), compute: Z=X/Y, output Z(Share){}", msgid.get_hex(), player, Vector<Share>(Z));   
}

/*
In the whole aspect,we are going to compute the reciprocal of shared_denominator_vec,
  then we can make the reciprocal to matmul the shared_numerator_vec.
  First, because we decide to use the iteration function,
  so we should make sure the postivate/negate of the den,and then get the sign of the final result, SJJ have done this part.
  Second, we should decide the iteration initial A by 3*exp(1-2*den)+0.003,but in this method we will use our experience 
  to set two initial value. but we should notice the formula above is more precise,but more exhaustive.
  Third,15 times for iteration is enough ,and we make each circle to compute:A=2*A-A*A*den.
  Finally,the A is 1/den,we realize num/den by num*ги1/den).
*/
void HelixInternal::Reciprocaldiv(
  const vector<Share>& shared_numerator_vec,
  const vector<Share>& shared_denominator_vec,
  vector<Share>& shared_quotient_vec) {
  size_t vec_size = shared_numerator_vec.size();
  log_debug << "Reciprocaldiv ...";
  AUDIT("id:{}, P{} Reciprocaldiv, compute: Z=X/Y, input X(share){}", msgid.get_hex(), player, Vector<Share>(shared_numerator_vec));
  AUDIT("id:{}, P{} Reciprocaldiv, compute: Z=X/Y, input Y(Share){}", msgid.get_hex(), player, Vector<Share>(shared_denominator_vec));
  /// determine whether the values are positive or negative.
  vector<Share> shared_numer_sign(vec_size);

  auto get_MSB_share = [=](const vector<Share>& x, vector<Share>& y) {
    vector<BitShare> bit_x(x.size());
    MSB(x, bit_x);
    B2A(bit_x, y);
    Scale(y, GetMpcContext()->FLOAT_PRECISION);
  };
  
  get_MSB_share(shared_numerator_vec, shared_numer_sign);

  vector<Share> shared_denom_sign(vec_size);
  get_MSB_share(shared_denominator_vec, shared_denom_sign);

  vector<Share> shared_sign_pos(vec_size);//, 0);
  ConstCommonInput(vector<double>(vec_size, 1), shared_sign_pos);
  
  vector<Share> shared_sign_neg(vec_size);
  ConstCommonInput(vector<double>(vec_size, -1), shared_sign_neg);

  vector<Share> shared_x_sign(vec_size);
  Select1Of2(shared_sign_neg, shared_sign_pos, shared_numer_sign, shared_x_sign, true);

  vector<Share> shared_y_sign(vec_size);
  Select1Of2(shared_sign_neg, shared_sign_pos, shared_denom_sign, shared_y_sign, true);

  vector<Share> quotient_sign_bit(vec_size);
  XOR(shared_numer_sign, shared_denom_sign, quotient_sign_bit);

  vector<Share> quotient_sign(vec_size);
  Select1Of2(shared_sign_neg, shared_sign_pos, quotient_sign_bit, quotient_sign, true);
  // RevealAndPrint(quotient_sign, "quotient_sign");

  vector<Share> numerator_vec(vec_size);
  vector<Share> denominator_vec(vec_size);
  Mul(shared_numerator_vec, shared_x_sign, numerator_vec);
  Mul(shared_denominator_vec, shared_y_sign, denominator_vec);
  
  vector<Share> quotient_vec = shared_quotient_vec;

  //we should limit the denominator to expand the scope of computation and the accuracy.
  vector<Share> SHARED_ONE(vec_size);
  ConstCommonInput(vector<double>(vec_size, 1), SHARED_ONE);
  
  vector<Share> SHARED_TEN(vec_size);
  ConstCommonInput(vector<double>(vec_size, 10), SHARED_TEN);
  
  vector<Share> SHARED_divTEN(vec_size);
  ConstCommonInput(vector<double>(vec_size, 0.1), SHARED_divTEN);
 
  vector<Share> judge_val_1(vec_size);
  vector<Share> judge_val_2(vec_size);
  vector<Share> judge_val_1_p(vec_size); //msb of judge number
  vector<Share> judge_val_2_p(vec_size);
  vector<Share> judge(vec_size);
  vector<Share> factor(vec_size); //multiple factor
  vector<Share> denominator_temp(vec_size);
  vector<Share> numerator_temp(vec_size);

  for (int i = 0; i < 4; i++) {
    Sub(denominator_vec, SHARED_TEN, judge_val_1); //x-10
    Sub(denominator_vec, SHARED_ONE, judge_val_2); //x-1

    get_MSB_share(judge_val_1, judge_val_1_p);
    get_MSB_share(judge_val_2, judge_val_2_p); //positive or negative

    XOR(judge_val_1_p, judge_val_2_p, judge);

    Select1Of2(SHARED_TEN, SHARED_divTEN, judge_val_1_p, factor, true);
    Select1Of2(SHARED_ONE, factor, judge, factor, true);

    Mul(factor, denominator_vec, denominator_temp);
    Mul(factor, numerator_vec, numerator_temp);
    denominator_vec = denominator_temp;
    numerator_vec = numerator_temp;
  }

  vector<Share> result(vec_size); //initial of 1/x
  vector<Share> initial_temp(vec_size);
  vector<Share> initial_exp(vec_size);

  /// compute the initial_value
  vector<Share> SHARED_Factorialof3(vec_size);
  ConstCommonInput(vector<double>(vec_size, 0.16667), SHARED_Factorialof3);
  
  // vector<Share> SHARED_HALF(vec_size);
  // ConstCommonInput(vector<double>(vec_size, 0.5), SHARED_HALF);
  
  // vector<Share> NUM_HALF(vec_size);
  // ConstCommonInput(vector<double>(vec_size, 2*0.5), NUM_HALF);

  vector<Share> NUM_ONE(vec_size);
  vector<double> float_NUM_ONE(vec_size, 1.0);
  ConstCommonInput(vector<double>(vec_size, 2*1.0), NUM_ONE);

  // vector<Share> NUM_TWO(vec_size);
  // ConstCommonInput(vector<double>(vec_size, 2*2.0), NUM_TWO);

  vector<Share> NUM_Factorialof3(vec_size);
  ConstCommonInput(vector<double>(vec_size, 2*0.16667), NUM_Factorialof3);

  Mul(denominator_vec, vector<mpc_t>(vec_size, 2), initial_temp, false);// just multiply 2, no truncation

  Sub(SHARED_ONE, initial_temp, initial_exp);
  // RevealAndPrint(initial_exp, "1-2*den");

  vector<Share> shared_beta(vec_size);
  vector<Share> update(vec_size);
  DReLU(initial_exp, shared_beta);
  Scale(shared_beta, GetMpcContext()->FLOAT_PRECISION);

  // initial_exp = vector<Share>(vec_size);
  ConstCommonInput(vector<double>(vec_size, 0.003), initial_exp);

  ConstCommonInput(vector<double>(vec_size, 4.074), update);
  
  Mul(update, shared_beta, update);
  Add(initial_exp, update, result);

  // Reconstruct2PC(result, "initial_value");
  vector<Share> iteraion_temp_2A(vec_size); //2*A
  vector<Share> iteraion_temp_AA(vec_size); //A^2
  vector<Share> den_reprocial_temp(vec_size);
  vector<Share> quo(vec_size);

  ///get initial foriteration
  for (int i = 0; i <= ITERATION_TIME; i++) {
    Mul(result, NUM_ONE, iteraion_temp_2A);
    Square(result, iteraion_temp_AA); //A*A
    Mul(iteraion_temp_AA, denominator_vec, den_reprocial_temp); //A*A*SELF
    Sub(iteraion_temp_2A, den_reprocial_temp, result);
  }

  Mul(numerator_vec, result, quo);
  Mul(quo, quotient_sign, shared_quotient_vec);
  
  AUDIT("id:{}, P{} Reciprocaldiv, compute: Z=X/Y, output Z(Share){}", msgid.get_hex(), player, Vector<Share>(shared_quotient_vec)); 
  log_debug << "Reciprocaldiv ok.";
}

void HelixInternal::Reciprocaldiv(const vector<Share>& X, const vector<double>& C, vector<Share>& Z) {
  AUDIT("id:{}, P{} Reciprocaldiv(S, C), compute: Z=X/Y, input X(share){}", msgid.get_hex(), player, Vector<Share>(X));
  AUDIT("id:{}, P{} Reciprocaldiv(S, C), compute: Z=X/Y, input Y(double){}", msgid.get_hex(), player, Vector<double>(C));
  vector<double> temp(C.size(), 0);
  for (size_t i = 0; i < C.size(); ++i) {
    temp[i] = 1.0/C[i];
  }

  Mul(X, temp, Z);
  AUDIT("id:{}, P{} Reciprocaldiv(S, C), compute: Z=X/Y, output Z(Share){}", msgid.get_hex(), player, Vector<Share>(Z));
}

void HelixInternal::Reciprocaldiv(const vector<double>& C, const vector<Share>& X, vector<Share>& Z) {
  AUDIT("id:{}, P{} Reciprocaldiv(C, S), compute: Z=X/Y, input X(double){}", msgid.get_hex(), player, Vector<double>(C));
  AUDIT("id:{}, P{} Reciprocaldiv(C, S), compute: Z=X/Y, input Y(Share){}", msgid.get_hex(), player, Vector<Share>(X));
  vector<Share> C_;
  ConstCommonInput(C, C_);

  Reciprocaldiv(C_, X, Z);
}

} // namespace helix
} // namespace rosetta
