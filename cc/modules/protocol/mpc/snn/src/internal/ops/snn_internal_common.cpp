#include "cc/modules/protocol/mpc/snn/include/snn_internal.h"
#include "cc/modules/protocol/mpc/comm/include/mpc_helper.h"
#include <thread>


namespace rosetta {
namespace snn {

int SnnInternal::Broadcast(const string& from_node, const string& msg, string& result) {
  tlog_debug << "snn public input msg, size: " << msg.size();

  if (msg.size() != result.size())
    result.resize(msg.size());
  return Broadcast(from_node, msg.data(), &result[0], msg.size());
}
int SnnInternal::Broadcast(const string& from_node, const char* msg, char* result, size_t size) {
  map<string, int> computation_nodes = io->GetComputationNodes();
  vector<string> non_computation_nodes = io->GetNonComputationNodes();
  string current_node = io->GetCurrentNodeId();
  string node_c = io->GetNodeId(PARTY_C);
  if (from_node == current_node) {
    for (auto iter = computation_nodes.begin(); iter != computation_nodes.end(); iter++) {
      if (current_node != iter->first) {
        sendBuf(iter->first, msg, size, 0);
        memcpy(result, msg, size);
      }
    }
  } else if (computation_nodes.find(current_node) != computation_nodes.end()) {
    receiveBuf(from_node, result, size, 0);
  } else if (std::find(non_computation_nodes.begin(), non_computation_nodes.end(), current_node) != non_computation_nodes.end()) {
    receiveBuf(node_c, result, size, 0);
  }

  if (current_node == node_c) {
    for (auto iter = non_computation_nodes.begin(); iter != non_computation_nodes.end(); iter++) {
      if (*iter != from_node) {
        sendBuf(*iter, result, size, 0);
      }
    }
  }
  return 0;
}

//------------------------
int SnnInternal::Reconstruct2PC(const vector<mpc_t>& a, string str) {
  if (!PRIMARY)
    return 1;
  assert((partyNum == PARTY_A || partyNum == PARTY_B) && "Reconstruct called by spurious parties");

  int size = a.size();

  vector<mpc_t> out;
  Reconstruct2PC(a, out, PARTY_A);

  if (partyNum == PARTY_A) {
    // cout << str << "[shared]: ";
    // for (size_t i = 0; i < size; ++i) {
    //   cout << to_readable_dec((signed_mpc_t)out[i]) << " ";
    // }
    // cout << endl;
    cout << str << ": [plain] size [" << a.size() << "] ";
	  const int float_precision = GetMpcContext()->FLOAT_PRECISION;
    for (size_t i = 0; i < size; ++i) {
      cout << MpcTypeToFloat(out[i], float_precision) << " ";
    }
    cout << endl;
  }

  return 0;
}

int SnnInternal::Reconstruct2PC(
  const vector<mpc_t>& a, vector<mpc_t>& out, int recv_party) {
  if (!PRIMARY)
    return 1;
  assert((partyNum == PARTY_A || partyNum == PARTY_B) && "Reconstruct called by spurious parties");

  AUDIT("id:{}, P{} Reconstruct2PC, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(a));
  int size = a.size();

  //cout << "----  recons 2222" << endl;
  out.resize(size, 0);
  int tempPartyA = (recv_party == PARTY_A) ? PARTY_A : PARTY_B;
  int tempPartyB = (recv_party == PARTY_A) ? PARTY_B : PARTY_A;
  if (partyNum == tempPartyB) {
    sendVector<mpc_t>(a, tempPartyA, size);
    AUDIT("id:{}, P{} Reconstruct2PC SEND to P{}(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), tempPartyA, Vector<mpc_t>(a));
  }

  if (partyNum == tempPartyA) {
    receiveVector<mpc_t>(out, tempPartyB, size);
    AUDIT("id:{}, P{} Reconstruct2PC RECV from P{}(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), tempPartyB, Vector<mpc_t>(out));

    addVectors<mpc_t>(out, a, out, size);
    AUDIT("id:{}, P{} Reconstruct2PC output(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(out));
  }

  return 0;
}

/**
 * Reveal, arithmetic
 * 
 * \param plain the result (fixpoint for arithmetic), sets to p
 * \param p bit-wised, which party will get the plaintext
 * 
 * p --> 0x 0 1 1 1
 * P -->      2 1 0
 * eg.
 * p ==> 0x ... 0000 0001 --> P0
 * p ==> 0x ... 0000 0101 --> P2 & P0
 * p ==> 0x ... 0000 0111 --> P2 & P1 & P0
 * and so on.
 * 
 * for balancing traffic:
 * reveal P0: P1 sends A1 to P0
 * reveal P1: P2 sends A0 to P1
 * reveal P2: P0 sends delta to P2
 * 
 * @note
 * <T1,T2> ==>  <Share, mpc_t> or <BitShare, bit_t>
 */
int SnnInternal::Reconstruct2PC_ex(
  const vector<mpc_t>& a, vector<mpc_t>& out, int recv_party) {
  if (recv_party > 7 || recv_party <= 0)
  {
    cout << "!! bad receive_party, should be 1-7, Notice: one bit represent for one part\n" << endl;
    return -1;
  }

  size_t size = a.size();
  out.resize(size, 0);
  string recv_mask = encode_reveal_mask(recv_party);
  return Reconstruct2PC_ex(a, out, recv_mask);
}

/**
 * Reveal, arithmetic, the input is in Z_{L-1}
 * 
 * \param plain the result (fixpoint for arithmetic), sets to p
 * \param p bit-wised, which party will get the plaintext
 * 
 * p --> 0x 0 1 1 1
 * P -->      2 1 0
 * eg.
 * p ==> 0x ... 0000 0001 --> P0
 * p ==> 0x ... 0000 0101 --> P2 & P0
 * p ==> 0x ... 0000 0111 --> P2 & P1 & P0
 * and so on.
 * 
 * for balancing traffic:
 * reveal P0: P1 sends A1 to P0
 * reveal P1: P2 sends A0 to P1
 * reveal P2: P0 sends delta to P2
 * 
 * @note
 * <T1,T2> ==>  <Share, mpc_t> or <BitShare, bit_t>
 */
int SnnInternal::Reconstruct2PC_ex_mod_odd(
  const vector<mpc_t>& a, vector<mpc_t>& out, int recv_party) {
  if (recv_party > 7 || recv_party <= 0)
  {
    cout << "!! bad receive_party, should be 1-7, Notice: one bit represent for one part\n" << endl;
    return -1;
  }

  AUDIT("id:{}, P{} Reconstruct2PC_ex_mod_odd, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(a));
  size_t size = a.size();

  out.resize(size, 0);
 
  bool reveal_a = recv_party & 0x00000001 ? true : false;
  bool reveal_b = recv_party & 0x00000002 ? true : false;
  bool reveal_c = recv_party & 0x00000004 ? true : false;

  if (reveal_a) {
    if (partyNum == PARTY_A) {
      receiveVector<mpc_t>(out, PARTY_B, size);
      AUDIT("id:{}, P{} Reconstruct2PC_ex_mod_odd RECV from P{}(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), PARTY_B, Vector<mpc_t>(out));

      addModuloOdd<mpc_t, mpc_t>(out, a, out, size);
      AUDIT("id:{}, P{} Reconstruct2PC_ex_mod_odd compute: out=out+X, out(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(out));
    }
    if (partyNum == PARTY_B) {
      sendVector<mpc_t>(a, PARTY_A, size);
      AUDIT("id:{}, P{} Reconstruct2PC_ex_mod_odd SEND to P{}(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), PARTY_A, Vector<mpc_t>(a));
    }
  }

  if (reveal_b) {
    if (partyNum == PARTY_A) {
      sendVector<mpc_t>(a, PARTY_B, size);
      AUDIT("id:{}, P{} Reconstruct2PC_ex_mod_odd SEND to P{}(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), PARTY_B, Vector<mpc_t>(a));
    }
    if (partyNum == PARTY_B) {
      receiveVector<mpc_t>(out, PARTY_A, size);
      AUDIT("id:{}, P{} Reconstruct2PC_ex_mod_odd RECV from P{}(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), PARTY_A, Vector<mpc_t>(out));

      addModuloOdd<mpc_t, mpc_t>(out, a, out, size);
      AUDIT("id:{}, P{} Reconstruct2PC_ex_mod_odd compute: out=out+X, out(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(out));
    }
  }

  if (reveal_c) {
    if (partyNum == PARTY_A) {
      sendVector<mpc_t>(a, PARTY_C, size);
      AUDIT("id:{}, P{} Reconstruct2PC_ex_mod_odd SEND to P{}(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), PARTY_C, Vector<mpc_t>(a));
    }
    if (partyNum == PARTY_B) {
      sendVector<mpc_t>(a, PARTY_C, size);
      AUDIT("id:{}, P{} Reconstruct2PC_ex_mod_odd SEND to P{}(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), PARTY_C, Vector<mpc_t>(a));
    }
    if (partyNum == PARTY_C) {
      receiveVector<mpc_t>(out, PARTY_A, size);
      AUDIT("id:{}, P{} Reconstruct2PC_ex_mod_odd RECV from P{}, A(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), PARTY_A, Vector<mpc_t>(out));

      vector<mpc_t> b_secret(size, 0);
      receiveVector<mpc_t>(b_secret, PARTY_B, size);
      AUDIT("id:{}, P{} Reconstruct2PC_ex_mod_odd RECV from P{}, B(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), PARTY_B, Vector<mpc_t>(b_secret));
      addModuloOdd<mpc_t, mpc_t>(out, b_secret, out, size);
      AUDIT("id:{}, P{} Reconstruct2PC_ex_mod_odd compute: out=A+B, output(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(out));
    }
  }

  return 0;
}

int SnnInternal::Reconstruct2PC_ex_mod_odd_v2(
    const vector<mpc_t>& a,
    vector<mpc_t>& out,
    vector<string>& receivers) {
  if (receivers.empty())
  {
    cout << "!! receivers is empty !!" << endl;
    return -1;
  }

  size_t size = a.size();
  out.resize(size, 0);

  auto is_compute_node = [&](const string& node_id, const map<string,int>& node_ids) -> bool {
    if (node_ids.find(node_id) != node_ids.end())
      return true;
    else
      return false;
  };

  auto is_receiver = [&](const string& node_id, const vector<string>& receivers) -> bool {
    for (auto &receiver : receivers) {
      if (receiver == node_id)
        return true;
    }
    return false;
  };

  AUDIT("id:{}, P{} Reconstruct2PC_ex_mod_odd_v2, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(a));
  string my_node_id = GetMpcContext()->NODE_ID;
  // non-computation nodes, can only be receiver
  auto &node_ids = GetMpcContext()->NODE_ROLE_MAPPING;
  if (!is_compute_node(my_node_id, node_ids)) {
    if (is_receiver(my_node_id, receivers)) {
      vector<mpc_t> shares(size, 0);
      receiveVector<mpc_t>(shares, PARTY_A, shares.size());
      AUDIT("id:{}, P{} Reconstruct2PC_ex_mod_odd_v2 RECV from P{}, shares(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), PARTY_A, Vector<mpc_t>(a));

      receiveVector<mpc_t>(out, PARTY_B, out.size());
      AUDIT("id:{}, P{} Reconstruct2PC_ex_mod_odd_v2 RECV from P{}, out(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), PARTY_B, Vector<mpc_t>(out));

      addModuloOdd<mpc_t>(out, shares, out, size);
      AUDIT("id:{}, P{} Reconstruct2PC_ex_mod_odd_v2 compute: out=out+shares, output(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(out));
    }
    return 0;
  }
  
  // computation-nodes
  int my_role_id = GetMpcContext()->ROLE_ID;
  // 1. send sharing to receiver
  for (auto &node_id : receivers) {
    if (node_id != my_node_id && my_role_id != PARTY_C) {
      sendVector2<mpc_t>(a, node_id, a.size());
      AUDIT("id:{}, P{} Reconstruct2PC_ex_mod_odd_v2 SEND to P{}, shares(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), node_id, Vector<mpc_t>(a));
    }
  }//for

  // 2. receive sharing from receiver
  if (is_receiver(my_node_id, receivers)) {
    if (my_role_id == PARTY_A || my_role_id == PARTY_B) {
      receiveVector<mpc_t>(out, adversary(partyNum), out.size());
      AUDIT("id:{}, P{} Reconstruct2PC_ex_mod_odd_v2 RECV from P{}, out(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), adversary(partyNum), Vector<mpc_t>(out));

      addModuloOdd<mpc_t>(out, a, out, out.size());
      AUDIT("id:{}, P{} Reconstruct2PC_ex_mod_odd_v2 compute: out=out+a, output(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(out));
    } else {// party_c
      vector<mpc_t> shares(size, 0);
      receiveVector<mpc_t>(shares, PARTY_A, shares.size());
      AUDIT("id:{}, P{} Reconstruct2PC_ex_mod_odd_v2 RECV from P{}, shares(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), PARTY_A, Vector<mpc_t>(a));

      receiveVector<mpc_t>(out, PARTY_B, out.size());
      AUDIT("id:{}, P{} Reconstruct2PC_ex_mod_odd_v2 RECV from P{}, out(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), PARTY_A, Vector<mpc_t>(a));

      addModuloOdd<mpc_t>(out, shares, out, size);
      AUDIT("id:{}, P{} Reconstruct2PC_ex_mod_odd_v2 compute: out=out+a, output(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(out));
    }
  }//for

  return 0;
}

int SnnInternal::Reconstruct2PC(
  const vector<mpc_t>& a, vector<mpc_t>& out, const vector<string>& recv_nodes) {
  string curr_node = io->GetCurrentNodeId();
  tlog_debug << "curr node:" << curr_node ;
  AUDIT("id:{}, P{} Reconstruct2PC, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(a));
  int size = a.size();
  if (PRIMARY) {
    for (int i = 0; i < recv_nodes.size(); i++) {
      if (curr_node != recv_nodes[i]) {
        sendVector2<mpc_t>(a, recv_nodes[i], a.size());
        AUDIT("id:{}, P{} Reconstruct2PC SEND to P{}, a(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), adversary(partyNum), Vector<mpc_t>(a));
      } else {
        receiveVector<mpc_t>(out, adversary(partyNum), out.size());
        AUDIT("id:{}, P{} Reconstruct2PC RECV from P{}, out(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), adversary(partyNum), Vector<mpc_t>(a));

        addVectors<mpc_t>(out, a, out, out.size());
        AUDIT("id:{}, P{} Reconstruct2PC compute: out=out+a, output(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(out));
      }
    }
  } else {
    for (int i = 0; i < recv_nodes.size(); i++) {
      if (curr_node == recv_nodes[i]) {
        vector<mpc_t> shares(size, 0);
        receiveVector<mpc_t>(shares, PARTY_A, shares.size());
        AUDIT("id:{}, P{} Reconstruct2PC RECV from P{}, shares(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), PARTY_A, Vector<mpc_t>(shares));

        receiveVector<mpc_t>(out, PARTY_B, out.size());
        AUDIT("id:{}, P{} Reconstruct2PC RECV from P{}, out(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), PARTY_B, Vector<mpc_t>(out));

        addVectors<mpc_t>(out, shares, out, size);
        AUDIT("id:{}, P{} Reconstruct2PC compute: out=out+shares, output(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(out));
      }
    }
  }
  return 0;
}

int SnnInternal::Reconstruct2PC_ex(
  const vector<mpc_t>& a, vector<mpc_t>& out, const string& recv_parties) {
  if (recv_parties.empty())
  {
    cout << "!! bad receive_parties, at least one node should be provided\n" << endl;
    return -1;
  }

  out.resize(a.size(), 0);
  vector<string> result_nodes = io->GetResultNodes();
  vector<string> nodes = decode_reveal_nodes(recv_parties, io->GetParty2Node(),result_nodes);

  return Reconstruct2PC(a, out, nodes);
}

// Added by SJJ
int SnnInternal::Reconstruct2PC_general(
  const vector<mpc_t>& shared_v, vector<mpc_t>& plaintext_v, int recv_party) {
  // Note: [HGF] fix this
  if (recv_party > PARTY_C || recv_party < PARTY_A) {
    recv_party = PARTY_A;
  }

  return Reconstruct2PC_ex(shared_v, plaintext_v, 0x00000001 << recv_party);
}

int SnnInternal::ReconstructBit2PC(
  const vector<small_mpc_t>& a, size_t size, string str) {
  if (!PRIMARY)
    return 1;
  assert((partyNum == PARTY_A || partyNum == PARTY_B) && "Reconstruct called by spurious parties");
  AUDIT("id:{}, P{} ReconstructBit2PC, input X(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<small_mpc_t>(a));

  vector<small_mpc_t> temp(size);
  if (partyNum == PARTY_B) {
    sendVector<small_mpc_t>(a, PARTY_A, size);
    AUDIT("id:{}, P{} ReconstructBit2PC SEND to P{}, a(small_mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), PARTY_A, Vector<small_mpc_t>(a));
  }

  if (partyNum == PARTY_A) {
    receiveVector<small_mpc_t>(temp, PARTY_B, size);
    AUDIT("id:{}, P{} ReconstructBit2PC RECV from P{}, temp(small_mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), PARTY_B, Vector<small_mpc_t>(temp));

    XORVectors(temp, a, temp, size);
    AUDIT("id:{}, P{} ReconstructBit2PC compute: temp=temp+a, temp(small_mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<small_mpc_t>(temp));

    cout << str << ": ";
    for (size_t i = 0; i < size; ++i)
      cout << (int)temp[i] << " ";
    cout << endl;
  }
  return 0;
}



}//snn
}//rosetta
