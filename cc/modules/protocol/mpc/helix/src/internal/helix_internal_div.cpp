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
                  const vector<Share>& cond, vector<Share>& result) {
    vector<Share> res_prod(X.size());
    vector<Share> X_minus_Y(X.size());
    Sub(X, Y, X_minus_Y);
    // Note: the cond should be non-scaled.
    Mul(X_minus_Y, cond, res_prod, false);
    Add(Y, res_prod, result);
}

void HelixInternal::Select1Of2(const vector<Share>& X, const vector<Share>& Y,
                  const vector<BitShare>& cond, vector<Share>& result) {
    log_error << "ERROR! not implemented Yet! HelixInternal::Select1Of2 for BitShare." << endl;
}

void HelixInternal::Select1Of2(const vector<double>& X, const vector<double>& Y,
                  const vector<Share>& cond, vector<Share>& result) {
    vector<Share> res_prod(X.size());
    vector<double> X_minus_Y(X.size());
    for(int i = 0; i < X.size(); ++i) {
        X_minus_Y[i] = X[i] - Y[i];
    }
    vector<mpc_t> fpC;
    convert_plain_to_fixpoint(X_minus_Y, fpC);
    // Note the cond paramter is comparison result, no need to scale.
    Mul(cond, fpC, res_prod, false);
    Add(Y, res_prod, result);    
}

void HelixInternal::XORShare(const vector<Share>& X,
                        const vector<Share>& Y,
                        vector<Share>& Z) {
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
}

void HelixInternal::Div(const vector<Share>& X,
                        const vector<Share>& Y,
                        vector<Share>& Z) {    
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
    for (int i = LEN - 1 - FLOAT_PRECISION_M; i >= 0; --i) {
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
    for (int i = 1; i < FLOAT_PRECISION_M + 1; ++i) {
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
}

void HelixInternal::Div(const vector<Share>& X, 
                        const vector<double>& C,
                        vector<Share>& Z) {
    // cout << "calling HelixInternal::Div(SDS)" << endl;
    assert(X.size() == C.size());
    int vec_size = C.size();
    // local division and trunc: X * (1/C)
    // RevealAndPrint(X, "HelixInternal::Div(SDS) input X:");

    vector<double> tmp_v(vec_size);
    vector<size_t> power_list(vec_size, FLOAT_PRECISION_M);
    for(auto i = 0; i < vec_size; ++i) {
        tmp_v[i] = 1.0 / C[i];
        // This is for dealing with big constant denominator, part I:
        //  scale it up by left-shifting
        double abs_v = abs(C[i]);
        if(abs_v > 1) {
            power_list[i] = ceil(log2(abs_v));
            tmp_v[i] = tmp_v[i] * (1 << power_list[i]);
            power_list[i] = power_list[i] + FLOAT_PRECISION_M;
        }
        log_debug << "HelixInternal::Div " << C[i] << " -> " << tmp_v[i] << \
                " [" << power_list[i] << "]" << endl; 
    }

    vector<mpc_t> fp_C;
    convert_plain_to_fixpoint(tmp_v, fp_C);
    // We will do the batch truncation later.
    Mul(X, fp_C, Z, false);

    // This is for dealing with big constant denominator, part II:
    //  trunc it back in the final result
    vector<Share> single_share(1);
    Trunc_many(Z, Z.size(), power_list);
}

void HelixInternal::Div(const vector<double>& C,
                        const vector<Share>& X,
                        vector<Share>& Z) {
    // cout << "calling HelixInternal::Div(DSS)" << endl;
    int vec_size = C.size();
    vector<Share> SHARE_C(vec_size);
    // Input(0, C, SHARE_C);
    ConstCommonInput(C, SHARE_C);
    vector<Share> tmp_q(vec_size);
    Div(SHARE_C, X, Z);
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
    // Todo: bug for math.floor(-8/4) = -3!
    vector<Share> floor_offset(vec_size);
    // TODO: optimize this to use non-scaled version!
    vector<Share> neg_quotient_sign(vec_size);
    vector<Share> XX(vec_size), YY(vec_size);
    Scale(s_numer_sign, XX);
    Scale(s_denom_sign, YY);
    XORShare(XX, YY, neg_quotient_sign);
    vector<Share> neg_quotient_sign_trunced = neg_quotient_sign;
    Trunc(neg_quotient_sign_trunced, vec_size);
    
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
    for (int i = LEN - 1 - FLOAT_PRECISION_M; i >= 0; --i) {
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
    }
    // RevealAndPrint(curr_q, "abs Q:");
    Mul(quotient_sign_multiplier, curr_q, curr_Z);
    // RevealAndPrint(curr_Z, "signed Q:");
    Add(curr_Z, floor_offset, Z);
}

void HelixInternal::Floordiv(const vector<Share>& X,
                        const vector<double>& C,
                        vector<Share>& Z) {
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
}

void HelixInternal::Floordiv(const vector<double>& C,
                        const vector<Share>& X,
                        vector<Share>& Z) {
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
}

} // namespace helix
} // namespace rosetta
