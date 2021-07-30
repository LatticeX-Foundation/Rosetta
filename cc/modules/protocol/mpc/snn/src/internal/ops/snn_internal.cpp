#include "cc/modules/protocol/mpc/snn/include/snn_internal.h"
#include "cc/modules/common/include/utils/str_type_convert.h"
#include <thread>

using std::thread;

namespace rosetta {
namespace snn {

/*
partyA send key to partyB as the public key of A and B
eg:
kab: A --> B
kac: A --> C
kbc: B --> C
*/
int SnnInternal::SyncAesKey(
  int partyA, int partyB, std::string& key_send, std::string& key_recv) {
  if (GetRoleId() == partyA) {
    sendBuf(partyB, key_send.data(), key_send.length(), 0);
    AUDIT("id:{}, P{} SyncAesKey SEND to P{}, key_send{}", msg_id().get_hex(), context_->GetMyRole(), partyB, CStr(key_send.c_str(), key_send.length()));
  }
  if (GetRoleId() == partyB) {
    receiveBuf(partyA, (char*)key_recv.data(), key_recv.length(), 0);
    AUDIT("id:{}, P{} SyncAesKey RECV from P{}, key_recv{}", msg_id().get_hex(), context_->GetMyRole(), partyB, CStr(key_recv.c_str(), key_recv.length()));
  }
  return 0;
}


// zero sharing with Pseudorandom Zero-Sharing
int SnnInternal::PRZS(int party0, int party1, vector<double>& shares) {
  vector<mpc_t> ss(shares.size());
  PRZS(party0, party1, ss);
  convert_mpctype_to_double(ss, shares, GetMpcContext()->FLOAT_PRECISION);
  AUDIT("id:{}, P{} PRZS output(double){}", msg_id().get_hex(), context_->GetMyRole(), Vector<double>(shares));
  return 0;
}

int SnnInternal::PRZS(int party0, int party1, mpc_t& shares) {
  vector<mpc_t> ss(1);
  PRZS(party0, party1, ss);
  shares = ss[0];
  return 0;
}

int SnnInternal::PRZS(int party0, int party1, double& shares) {
  mpc_t ss;
  PRZS(party0, party1, ss);
  shares = MpcTypeToFloat(ss, GetMpcContext()->FLOAT_PRECISION);
  return 0;
}

int SnnInternal::PRZS(int party0, int party1, vector<mpc_t>& shares) {
  string r_type = "COMMON";
  if (
    ((party0 == PARTY_A) && (party1 == PARTY_B)) || ((party0 == PARTY_B) && (party1 == PARTY_A))) {
    r_type = "COMMON";
  } else if (
    ((party0 == PARTY_A) && (party1 == PARTY_C)) || ((party0 == PARTY_C) && (party1 == PARTY_A))) {
    r_type = "a_1";
  } else if (
    ((party0 == PARTY_B) && (party1 == PARTY_C)) || ((party0 == PARTY_C) && (party1 == PARTY_B))) {
    r_type = "a_2";
  } else {
    notYet();
  }

  AUDIT("id:{}, P{} PRZS, party0:{}, party1:{}", msg_id().get_hex(), context_->GetMyRole(), party0, party1);
  populateRandomVector2<mpc_t>(shares, shares.size(), r_type, "NEGATIVE");
  AUDIT("id:{}, P{} PRZS output(mpc_t){}", msg_id().get_hex(), context_->GetMyRole(), Vector<mpc_t>(shares));
  return 0;
}

int SnnInternal::PRZS(vector<mpc_t>& shares) {
  populateRandomVector3<mpc_t>(shares, shares.size());
  return 0;
}

int SnnInternal::PrivateInput(const string& node_id, const vector<double>& v, vector<mpc_t>& shares) {
  const int float_precision = GetMpcContext()->FLOAT_PRECISION;
  vector<mpc_t> vs(v.size(), 0);
  if (node_id == GetMpcContext()->NODE_ID) {
    convert_double_to_mpctype(v, vs, float_precision);
  }

  return PrivateInput(node_id, vs, shares);
}

int SnnInternal::PrivateInput(const string& node_id, const vector<mpc_t>& v, vector<mpc_t>& shares) {  
  shares.resize(v.size());
  string current_node_id = io->GetCurrentNodeId();
  int data_party = io->GetPartyId(node_id);
  tlog_debug << "PrivateInput node id:" << node_id << " owner party:" << data_party << "...";
  AUDIT("id:{}, P{} PrivateInput, node_id:{}, input{}", msg_id().get_hex(), context_->GetMyRole(), node_id, Vector<mpc_t>(shares));
  // the first condition denotes receiver, the second condition denotes sender
  if (PRIMARY || node_id == current_node_id) {
    // P0 or P1 input data
    if ((data_party == PARTY_A || data_party == PARTY_B) && PRIMARY) {
      PRZS(PARTY_A, PARTY_B, shares);
      
      if (partyNum == data_party) {
        for (size_t i = 0; i < v.size(); ++i)
          shares[i] = shares[i] + v[i];
      }
    }
    // P2 input data
    else if (data_party == PARTY_C && (PRIMARY || HELPER)) {
      PRZS(PARTY_B, PARTY_C, shares);
      if (partyNum == PARTY_C) {
        for (size_t i = 0; i < v.size(); ++i)
          shares[i] = shares[i] + v[i];

        // send C's to A
        sendVector<mpc_t>(shares, PARTY_A, shares.size());
        AUDIT("id:{}, PrivateInput P{} SEND to P{}{}", msg_id().get_hex(), partyNum, PARTY_A, Vector<mpc_t>(shares));
      } else if (partyNum == PARTY_A) {
        receiveVector<mpc_t>(shares, PARTY_C, shares.size());
        AUDIT("id:{}, PrivateInput P{} RECV from P{}{}", msg_id().get_hex(), partyNum, PARTY_C, Vector<mpc_t>(shares));
      }
    }
    // other nodes input data
    else if (current_node_id == node_id || PRIMARY) {
      if (current_node_id == node_id) {
        PRZS(shares);
        sendVector<mpc_t>(shares, PARTY_A, shares.size());
        AUDIT("id:{}, funPrivateInput P{} SEND to P{}{}", msg_id().get_hex(), partyNum, PARTY_A, Vector<mpc_t>(shares));
        
        for (size_t i = 0; i < v.size(); ++i) {
          shares[i] = v[i] - shares[i];
        }
        sendVector<mpc_t>(shares, PARTY_B, shares.size());
        AUDIT("id:{}, funPrivateInput P{} SEND to P{}{}", msg_id().get_hex(), partyNum, PARTY_B, Vector<mpc_t>(shares));
      } else if (PRIMARY) {
        receiveVector2<mpc_t>(shares, node_id, shares.size());
        AUDIT("id:{}, funPrivateInput P{} RECV from P{}{}", msg_id().get_hex(), partyNum, node_id, Vector<mpc_t>(shares));
      }
    }
  }

  AUDIT("id:{}, PrivateInput P{} output{}", msg_id().get_hex(), partyNum, Vector<mpc_t>(shares));
  tlog_debug << "PrivateInput ok.";
  return 0;
}


int SnnInternal::PrivateInput(const string& node_id, const vector<double>& v, vector<double>& shares) {
  vector<mpc_t> ss(shares.size());
  PrivateInput(node_id, v, ss);
  convert_mpctype_to_double(ss, shares, GetMpcContext()->FLOAT_PRECISION);
  return 0;
}

int SnnInternal::PrivateInput(const string& node_id, double v, mpc_t& shares) {
  vector<mpc_t> ss(1);
  vector<double> vv = {v};
  PrivateInput(node_id, vv, ss);
  shares = ss[0];
  return 0;
}

int SnnInternal::PrivateInput(const string& node_id, double v, double& shares) {
  mpc_t ss;
  PrivateInput(node_id, v, ss);
  shares = MpcTypeToFloat(ss, GetMpcContext()->FLOAT_PRECISION);
  return 0;
}




}//snn
}//rosetta
