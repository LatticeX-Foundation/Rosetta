#include "snn__test.h"
#include <string>
#include "cc/modules/protocol/mpc/snn/include/snn_opsets.h"
#include "cc/modules/protocol/mpc/comm/include/mpc_defines.h"

using namespace std;
// using namespace rosetta::snn;

/* this example to check a issue when MPC_element 
is set to 128-bit long and PRIMER is less than p<=129, 
in this case, PrivateCompare gets overflow and invalid output
*/
void test_PrivateCompareMPC_big_value(rosetta::SnnProtocol& snn0) {
  msg_id_t msgid("test_PrivateCompareMPC_big_value");
  cout << "-------------   PrivateCompare BigValue ---------------\n";
  size_t size = 17;
  
  vector<small_mpc_t> beta(size, 0);
  vector<mpc_t> x(size);
  for (auto i = 0; i < size; ++i)
    x[i] = i;
  mpc_t max_value = (MINUS_ONE >> 1) - 2;
  vector<mpc_t> r(size, max_value);
  vector<small_mpc_t> beta_prime(size, 0);
  vector<small_mpc_t> expect(size, 0);

  size_t elem_size = sizeof(mpc_t)*8;
  
  print_vec(x, size, "Input x: ");
  print_vec(r, size, "Input r: ");

  auto private_cmp = std::make_shared<rosetta::snn::PrivateCompare>(msgid, snn0.GetNetHandler());
  vector<small_mpc_t> x_share_bits(size*elem_size, 0);
  if (PRIMARY) {
    vector<small_mpc_t> x_share_bits0(size*elem_size, 0), x_share_bits1(size*elem_size, 0);
    private_cmp->sharesOfBits(x_share_bits0, x_share_bits1, x, size, "COMMON");
    if (partyNum == PARTY_A)
      x_share_bits.swap(x_share_bits0);
    else
      x_share_bits.swap(x_share_bits1);
  }
    
  // beta_prime = beta xor (x>r)
  // beta = 1, x < r+1, beta_prime = 1
  private_cmp->Run(x_share_bits, r, beta, beta_prime, size, sizeof(mpc_t)*8);

  print_vec(beta_prime, size, "Beta_prime:");
  print_vec(expect, size, "Beta_prime expected:");
}

/**
 * basic test case of PrivateCompare
*/
void test_PrivateCompareMPC_basic(rosetta::SnnProtocol& snn0) {
  msg_id_t msgid("test_PrivateCompareMPC_basic");
  cout << "-------------   PrivateCompare Basic ---------------\n";
  size_t size = 10;
  
  vector<small_mpc_t> beta(size, 0);
  vector<mpc_t> x(size);
  vector<mpc_t> r(size);
  vector<small_mpc_t> expect(size, 1);
  for (auto i = 0; i < size; ++i){//x > r
    x[i] = 10000+i;
    r[i] = x[i]-1;
    beta[i] = 0;
    expect[i] = x[i]>r[i] ? 1 : 0;// beta xor (x>r)
  }

  vector<small_mpc_t> beta_prime(size, 0);
  size_t elem_size = sizeof(mpc_t)*8;
  
  print_vec(x, size, "Input x: ");
  print_vec(r, size, "Input r: ");

  auto private_cmp = std::make_shared<rosetta::snn::PrivateCompare>(msgid, snn0.GetNetHandler());
  vector<small_mpc_t> x_share_bits(size*elem_size, 0);
  if (PRIMARY) {
    vector<small_mpc_t> x_share_bits0(size*elem_size, 0), x_share_bits1(size*elem_size, 0);
    private_cmp->sharesOfBits(x_share_bits0, x_share_bits1, x, size, "COMMON");
    if (partyNum == PARTY_A)
      x_share_bits.swap(x_share_bits0);
    else
      x_share_bits.swap(x_share_bits1);
  }
    
  // beta_prime = beta xor (x>r)
  // beta = 1, x < r+1, beta_prime = 1
  private_cmp->Run(x_share_bits, r, beta, beta_prime, size, sizeof(mpc_t)*8);

  print_vec(beta_prime, size, "Beta_prime:");
  print_vec(expect, size, "Beta_prime expected:");
}

void run(int partyid) {
  SNN_PROTOCOL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  /*        PrivateCompare() = belta_prime = belta xor (x > r)    */
  //////////////////////////////////////////////////////////////////
  test_PrivateCompareMPC_big_value(snn0);
  test_PrivateCompareMPC_basic(snn0);
  //////////////////////////////////////////////////////////////////
  SNN_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);