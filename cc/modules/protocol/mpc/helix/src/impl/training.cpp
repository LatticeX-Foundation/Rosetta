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

#include <iostream>
#include <string>
#include <vector>

using namespace std;
namespace rosetta {
int HelixOpsImpl::Reveal(const vector<string>& a, vector<string>& c, const attr_type* attr_info) {
  vector<double> fc;
  Reveal(a, fc, attr_info);
  helix_double_to_plain_string(fc, c);
  AUDIT("id:{}, P{} Reveal output(double, plain){}", _op_msg_id.get_hex(), hi->party_id(), Vector<double>(fc));

  return 0;
}

int HelixOpsImpl::Reveal(const vector<string>& a, vector<double>& c, const attr_type* attr_info) {
  string nodes = get_attr_value(attr_info, "receive_parties", "");
  vector<Share> shareA;
  helix_convert_string_to_share(a, shareA);
  AUDIT("id:{}, P{} Reveal input X(Share){}", _op_msg_id.get_hex(),  hi->party_id(), Vector<Share>(shareA));
  
  vector<string> result_nodes = io->GetResultNodes();
  vector<string> parties = decode_reveal_nodes(nodes, io->GetParty2Node(), result_nodes);

  for (auto iter = parties.begin(); iter != parties.end(); ) {
    if (std::find(result_nodes.begin(), result_nodes.end(), *iter) == result_nodes.end()) {
      tlog_error << "node " << *iter << " is not a valid result node!" ;
      iter = parties.erase(iter);
    } else {
      iter++;
    }
  }
  hi->Reveal(shareA, c, parties);
  AUDIT("id:{}, P{} Reveal output(double, plain){}", _op_msg_id.get_hex(), hi->party_id(), Vector<double>(c));

  return 0;
}

/**
 * @desc: This is for Tensorflow's SaveV2 Op.
 */
int HelixOpsImpl::ConditionalReveal(
  vector<string>& in_vec,
  vector<string>& out_cipher_vec,
  vector<double>& out_plain_vec) {
  // not to reveal plaintext by default
  const SaverModel& save_model = context_->SAVER_MODEL;

  // all ciphertext
  if (save_model.is_computation_mode()) {
    out_cipher_vec = in_vec;
    out_plain_vec.clear();
    return 0;
  } else if (save_model.is_ciphertext_mode()) {
    vector<Share> shared_input(in_vec.size());
    helix_convert_string_to_share(in_vec, shared_input);
    vector<Share> inner_out_vec = shared_input;
    const map<string, int>& ciphertext_nodes = save_model.get_ciphertext_nodes(); 
    hi->SyncCiphertext(shared_input, inner_out_vec, ciphertext_nodes);
    helix_convert_share_to_string(inner_out_vec, out_cipher_vec);
    out_plain_vec.clear();
    return 0;
  }
  // only plaintext for selected parties
  vector<Share> shareA;
  helix_convert_string_to_share(in_vec, shareA);
  AUDIT("id:{}, P{} Reveal, input(Share){}", _op_msg_id.get_hex(), hi->party_id(),  Vector<Share>(shareA));

  const vector<string>& plaintext_nodes = save_model.get_plaintext_nodes();
  vector<double> c;
  hi->Reveal(shareA, c, plaintext_nodes);
  string current_node_id = io->GetCurrentNodeId();
  if (std::find(plaintext_nodes.begin(), plaintext_nodes.end(), current_node_id) != plaintext_nodes.end()) {
    out_plain_vec.swap(c);
  } else {
    out_plain_vec.resize(in_vec.size());
  }
  out_cipher_vec.clear();

  AUDIT("id:{}, P{} Reveal, output(double, plain){}", _op_msg_id.get_hex(), hi->party_id(),  Vector<double>(out_plain_vec));
  //AUDIT("id:{}, Reveal nodes:{}, output{}", _op_msg_id.get_hex(), nodes, Vector<Share>(shareC));

  return 0;
}

} // namespace rosetta
