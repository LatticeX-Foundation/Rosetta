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
#include "helix_impl_util.h"

namespace rosetta {

int HelixOpsImpl::Relu(const vector<string>& a, vector<string>& c, const attr_type* attr_info) {
  vector<Share> shareA, shareC;
  helix_convert_string_to_share(a, shareA);
  AUDIT("id:{}, Relu input X(Share){}", _op_msg_id.get_hex(), Vector<Share>(shareA));

  hi->ReLU(shareA, shareC);
  helix_convert_share_to_string(shareC, c);
  AUDIT("id:{}, Relu output(Share){}", _op_msg_id.get_hex(), Vector<Share>(shareC));

  return 0;
}

int HelixOpsImpl::ReluPrime(
  const vector<string>& a,
  vector<string>& c,
  const attr_type* attr_info) {
  vector<Share> shareA, shareC;
  helix_convert_string_to_share(a, shareA);
  AUDIT("id:{}, ReluPrime input X(Share){}", _op_msg_id.get_hex(), Vector<Share>(shareA));

  hi->DReLU(shareA, shareC);
  hi->Scale(shareC, context_->FLOAT_PRECISION);
  helix_convert_share_to_string(shareC, c);
  AUDIT("id:{}, ReluPrime output(Share){}", _op_msg_id.get_hex(), Vector<Share>(shareC));

  return 0;
}

int HelixOpsImpl::Sigmoid(const vector<string>& a, vector<string>& c, const attr_type* attr_info) {
  vector<Share> shareA, shareC;
  helix_convert_string_to_share(a, shareA);
  AUDIT("id:{}, Sigmoid input X(Share){}", _op_msg_id.get_hex(), Vector<Share>(shareA));

  hi->Sigmoid(shareA, shareC);
  helix_convert_share_to_string(shareC, c);
  AUDIT("id:{}, Sigmoid output(Share){}", _op_msg_id.get_hex(), Vector<Share>(shareC));

  return 0;
}

int HelixOpsImpl::SigmoidCrossEntropy(
  const vector<string>& a,
  const vector<string>& b,
  vector<string>& c,
  const attr_type* attr_info) {
  vector<Share> shareA, shareB, shareC;
  helix_convert_string_to_share(a, shareA);
  AUDIT("id:{}, SigmoidCrossEntropy input X(Share){}", _op_msg_id.get_hex(), Vector<Share>(shareA));

  helix_convert_string_to_share(b, shareB);
  AUDIT("id:{}, SigmoidCrossEntropy input Y(Share){}", _op_msg_id.get_hex(), Vector<Share>(shareB));

  // hi->SigmoidCrossEntropy(shareA, shareB, shareC);
  hi->SigmoidCrossEntropy_batch(shareA, shareB, shareC);
  helix_convert_share_to_string(shareC, c);
  AUDIT("id:{}, SigmoidCrossEntropy output(Share){}", _op_msg_id.get_hex(),  Vector<Share>(shareC));

  return 0;
}
} // namespace rosetta
