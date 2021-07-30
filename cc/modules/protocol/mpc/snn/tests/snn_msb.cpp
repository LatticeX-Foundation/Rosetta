#include "snn__test.h"
#include <string>
#include "cc/modules/protocol/mpc/comm/include/mpc_defines.h"

using namespace std;
// using namespace rosetta::snn;

/*
** MSB(X)=0
*/
void test_MSB_zero(shared_ptr<NET_IO> net_io, SnnProtoType& snn0, const string& receivers) {
  msg_id_t msgid("test_MSB basic protocol");
  size_t size = 10;
  
  // input from ShareConvert which outputs Z_{L-1} elements
  vector<small_mpc_t> beta(size, 0);
  vector<double> x(size, 0);
  vector<mpc_t> x_share(size);
  vector<mpc_t> x_msb(size);
  for (auto i = 2; i < size; ++i)
    x[i] = i;
  
  vector<small_mpc_t> expect(size, 0);
  print_vec(x, size, "Input x: ");

  // private input
  auto snn_internal = snn0.GetInternal(msgid);
  snn_internal->PrivateInput(snn0.GetNetHandler()->GetNodeId(PARTY_A), x, x_share);
  for (auto i = 0; i < size; ++i)
    x_share[i] *= 2;

  // share convert to Z_{L-1}
  snn_internal->ShareConvert(x_share);

  snn_internal->ComputeMSB(x_share, x_msb);

  vector<mpc_t> msb_reveal(size, 0);
  snn_internal->Reconstruct2PC_ex(x_msb, msb_reveal, receivers); // 0x111 to all parties

  print_vec(msb_reveal, size, "msb:");
  print_vec(expect, size, "msb expected:");
}

/*
** MSB(X)=1
*/
void test_MSB_one(shared_ptr<NET_IO> net_io, SnnProtoType& snn0, const string& receivers) {
  msg_id_t msgid("test_MSB basic protocol, all one in MSB");
  size_t size = 10;
  
  // input from ShareConvert which outputs Z_{L-1} elements
  vector<double> x(size);
  vector<mpc_t> x_share(size);
  vector<mpc_t> x_msb(size);
  for (auto i = 0; i < size; ++i)
    x[i] = -1-i;
  
  vector<double> all_ones(size, 1);
  vector<mpc_t> expect(size, 1);
  convert_double_to_mpctype(all_ones, expect, snn0.GetMpcContext()->FLOAT_PRECISION);

  print_vec(x, size, "Input x: ");

  auto snn_internal = snn0.GetInternal(msgid);
  // private input
  snn_internal->PrivateInput(snn0.GetNetHandler()->GetNodeId(PARTY_A), x, x_share);

  // share convert to Z_{L-1}
  snn_internal->ShareConvert(x_share);

  snn_internal->ComputeMSB(x_share, x_msb);

  vector<mpc_t> msb_reveal(size, 0);
  // reveal->RunEx(x_msb, msb_reveal, 0x07); // reveal to all parties
  snn_internal->Reconstruct2PC_ex(x_msb, msb_reveal, receivers); // reveal to all parties

  print_vec(msb_reveal, size, "msb:");
  print_vec(expect, size, "msb expected:");
}

void run(int partyid) {
  SNN_PROTOCOL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  /*                   MSB most significant bit                   */
  //////////////////////////////////////////////////////////////////
  vector<string> receivers = {"P0", "P1", "P2"};
  string receiver_parties = receiver_parties_pack(receivers);

  test_MSB_zero(net_io, snn0, receiver_parties);
  test_MSB_one(net_io, snn0, receiver_parties);

  //////////////////////////////////////////////////////////////////
  SNN_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);