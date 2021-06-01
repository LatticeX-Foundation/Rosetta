#include "snn__test.h"
#include <string>
#include "cc/modules/protocol/mpc/snn/include/snn_opsets.h"
#include "cc/modules/protocol/mpc/comm/include/mpc_defines.h"

using namespace std;
// using namespace rosetta::snn;

/*
** MSB(X)=0
*/
void test_MSB_zero(rosetta::SnnProtocol& snn0) {
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
  auto private_input = std::make_shared<rosetta::snn::PrivateInput>(msgid, snn0.GetNetHandler());
  private_input->Run(PARTY_A, x, x_share);
  for (auto i = 0; i < size; ++i)
    x_share[i] *= 2;

  // share convert to Z_{L-1}
  auto share_convert = std::make_shared<rosetta::snn::ShareConvert>(msgid, snn0.GetNetHandler());
  share_convert->Run(x_share, size);

  auto compute_msb = std::make_shared<rosetta::snn::ComputeMSB>(msgid, snn0.GetNetHandler());
  compute_msb->Run(x_share, x_msb, size);

  auto reveal = std::make_shared<rosetta::snn::Reconstruct2PC>(msgid, snn0.GetNetHandler());
  vector<mpc_t> msb_reveal(size, 0);
  reveal->RunEx(x_msb, msb_reveal, 0x07); // 111 to all parties
  // reveal->RunV2(x_msb, size, msb_reveal, 0); // 111 to all parties

  print_vec(msb_reveal, size, "msb:");
  print_vec(expect, size, "msb expected:");
}

/*
** MSB(X)=1
*/
void test_MSB_one(rosetta::SnnProtocol& snn0) {
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
  convert_double_to_mpctype(all_ones, expect);

  print_vec(x, size, "Input x: ");

  // private input
  auto private_input = std::make_shared<rosetta::snn::PrivateInput>(msgid, snn0.GetNetHandler());
  private_input->Run(PARTY_A, x, x_share);

  // share convert to Z_{L-1}
  auto share_convert = std::make_shared<rosetta::snn::ShareConvert>(msgid, snn0.GetNetHandler());
  share_convert->Run(x_share, size);

  auto compute_msb = std::make_shared<rosetta::snn::ComputeMSB>(msgid, snn0.GetNetHandler());
  compute_msb->Run(x_share, x_msb, size);

  auto reveal = std::make_shared<rosetta::snn::Reconstruct2PC>(msgid, snn0.GetNetHandler());
  vector<mpc_t> msb_reveal(size, 0);
  reveal->RunEx(x_msb, msb_reveal, 0x07); // reveal to all parties

  print_vec(msb_reveal, size, "msb:");
  print_vec(expect, size, "msb expected:");
}

void run(int partyid) {
  SNN_PROTOCOL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  /*                   MSB most significant bit                   */
  //////////////////////////////////////////////////////////////////
  test_MSB_zero(snn0);
  test_MSB_one(snn0);

  //////////////////////////////////////////////////////////////////
  SNN_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);