#include "snn__test.h"
#include <string>
#include "cc/modules/protocol/mpc/snn/include/snn_opsets.h"
#include "cc/modules/protocol/mpc/comm/include/mpc_defines.h"

using namespace std;
// using namespace rosetta::snn;

/*
** unittest of ShareConvert protocol of SecureNN
*/
void test_ShareConvert(rosetta::SnnProtocol& snn0) {
  msg_id_t msgid("test_ShareConvert basic protocol");
  size_t size = 10;
  
  // input from ShareConvert which outputs Z_{L-1} elements
  vector<small_mpc_t> beta(size, 0);
  vector<mpc_t> x(size, 0);
  vector<mpc_t> x_share(size);
  for (mpc_t i = 2; i < size; ++i)
    x[i] = (mpc_t(1)<<62) + 2*i;
  
  print_vec(x, size, "Input x: ");

  // private input
  auto private_input = std::make_shared<rosetta::snn::PrivateInput>(msgid, snn0.GetNetHandler());
  private_input->Run(PARTY_A, x, x_share);
  print_vec(x_share, size, "x_share:");

  vector<mpc_t> expect = x;

  // share convert to Z_{L-1}
  auto share_convert = std::make_shared<rosetta::snn::ShareConvert>(msgid, snn0.GetNetHandler());
  auto x_share_converted = x_share;
  share_convert->Run(x_share_converted, size);
  print_vec(x_share_converted, size, "share_convert x_share_converted:");

  auto reveal = std::make_shared<rosetta::snn::Reconstruct2PC>(msgid, snn0.GetNetHandler());
  vector<mpc_t> share_convert_result(size, 0);
  reveal->RunModOdd(x_share_converted, share_convert_result, 0x07); // 111 to all parties

  print_vec(share_convert_result, size, "share_convert:");
  print_vec(expect, size, "share_convert expect:");
}

void run(int partyid) {
  SNN_PROTOCOL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  /*         ShareConvert X in Z_L to X_prime in Z_{L-1}          */
  //////////////////////////////////////////////////////////////////
  test_ShareConvert(snn0);

  //////////////////////////////////////////////////////////////////
  SNN_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);