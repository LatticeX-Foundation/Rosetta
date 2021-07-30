#include "snn__test.h"
#include <string>
#include "cc/modules/protocol/mpc/comm/include/mpc_defines.h"

using namespace std;
// using namespace rosetta::snn;

/*
** unittest of ShareConvert protocol of SecureNN
*/
void test_ShareConvert(SnnProtoType& snn0, attr_type& reveal_attr) {
  msg_id_t msgid("test_ShareConvert basic protocol");
  size_t size = 10;
  
  // input from ShareConvert which outputs Z_{L-1} elements
  vector<mpc_t> x(size, 0);
  vector<mpc_t> x_share(size);
  for (mpc_t i = 2; i < size; ++i)
    x[i] = i+100;//(mpc_t(1)<<60) + 2*i;

  auto snn_internal = snn0.GetInternal(msgid);
  // private input
  snn_internal->PrivateInput(snn0.GetNetHandler()->GetNodeId(PARTY_A), x, x_share);
  print_vec(x, size, "Input x: ");
  print_vec(x_share, size, "x_share:");

  string reveal_dest = reveal_attr["receive_parties"];
  vector<mpc_t> reveal_x1;
  snn_internal->Reconstruct2PC_ex(x_share, reveal_x1, 7);//reveal_dest);
  
  print_vec(reveal_x1, size, "=======Input reveal x1: ");

  vector<mpc_t> expect = x;

  // share convert to Z_{L-1}
  snn_internal->ShareConvert(x_share);
  print_vec(x_share, size, "share_convert x_share converted:");

  vector<mpc_t> share_convert_reveal(size, 0);
  vector<string> receivers = {"P0", "P1", "P2"};
  snn_internal->Reconstruct2PC_ex_mod_odd_v2(x_share, share_convert_reveal, receivers);

  print_vec(share_convert_reveal, size, "share_convert reveal:");
  print_vec(expect, size, "share_convert expect:");
}

void run(int partyid) {
  SNN_PROTOCOL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  /*         ShareConvert X in Z_L to X_prime in Z_{L-1}          */
  //////////////////////////////////////////////////////////////////
  test_ShareConvert(snn0, reveal_attr);

  //////////////////////////////////////////////////////////////////
  SNN_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);