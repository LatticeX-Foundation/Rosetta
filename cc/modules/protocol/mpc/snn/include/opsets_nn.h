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

/*
    Some high-level OPs and functionalities that are customized for neural network. 
*/
#pragma once

#include "cc/modules/protocol/mpc/snn/include/snn_opsets.h"

namespace rosetta {
namespace snn {
class SigmoidCrossEntropy : public OpBase {
  using OpBase::OpBase;

public:
    int Run(const vector<mpc_t>& shared_logits,
            const vector<mpc_t>& shared_labels,
            vector<mpc_t>& shared_result,
            size_t vec_size) {
            MPCOP_RETURN(sigmoid_cross_entropy(shared_logits, 
                                                shared_labels, 
                                                shared_result, vec_size));
        }

    int Run(const vector<mpc_t>& shared_logits,
            const vector<double>& common_labels,
            vector<mpc_t>& shared_result,
            size_t vec_size){
            // TODO
            return -1;
        }

    /**
        @Note: Just like the 'sigmoid_cross_entropy_with_logits' in python/ops/nn_impl.py of
            original Tensorflow source code, we implement formulation:
            max(logit, 0) - logit * label + log(1 + exp(-abs(x)))
    */
    int sigmoid_cross_entropy(const vector<mpc_t>& shared_logits,
            const vector<mpc_t>& shared_labels,
            vector<mpc_t>& shared_result,
            size_t vec_size);

    /**
        @brief: return |X|
    */
    void ABS(const vector<mpc_t>& shared_x,
               vector<mpc_t>& shared_y,
               size_t vec_size);
    /**
        @brief: 1 if X > 0, -1 if X < 0
    */
    void ABSPrime(const vector<mpc_t>& shared_x,
                    vector<mpc_t>& sahred_y,
                    size_t vec_size);
    
private:
    // get log(1 + exp(-x)) [x >= 0] with special polynommial
    void CELog(const vector<mpc_t>& shared_x,
               vector<mpc_t>& shared_y,
               size_t vec_size);

};

} // namespace mpc
} // namespace rosetta