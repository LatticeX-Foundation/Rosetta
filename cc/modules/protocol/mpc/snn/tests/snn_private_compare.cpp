#include "snn__test.h"
#include <string>
#include "cc/modules/protocol/mpc/comm/include/mpc_defines.h"
#include <math.h>

using namespace std;
// using namespace rosetta::snn;

/* this example to check a issue when MPC_element 
is set to 128-bit long and PRIMER is less than p<=129, 
in this case, PrivateCompare gets overflow and invalid output
*/
void test_PrivateCompareMPC_big_value(SnnProtoType& snn0) {
  msg_id_t msgid("test_PrivateCompareMPC_big_value");
  cout << "-------------   PrivateCompare BigValue ---------------\n";
  size_t size = 17;
  
  vector<small_mpc_t> beta(size, 1);
  vector<mpc_t> x(size);
  for (auto i = 0; i < size; ++i)
    x[i] = i;
  mpc_t max_value = (MINUS_ONE >> 2) - 2;
  vector<mpc_t> r(size, max_value);
  vector<small_mpc_t> beta_prime(size, 0);
  vector<small_mpc_t> expect(size, 1);

  size_t elem_size = sizeof(mpc_t)*8;
  
  print_vec(x, size, "Input x: ");
  print_vec(r, size, "Input r: ");

  int partyNum = snn0.GetMpcContext()->GetMyRole();

  auto snn_internal = snn0.GetInternal(msgid);
  vector<small_mpc_t> x_share_bits(size*elem_size, 0);
  if (PRIMARY) {
    vector<small_mpc_t> x_share_bits0(size*elem_size, 0), x_share_bits1(size*elem_size, 0);
    snn_internal->sharesOfBits(x_share_bits0, x_share_bits1, x, size, "COMMON");
    if (partyNum == PARTY_A)
      x_share_bits.swap(x_share_bits0);
    else
      x_share_bits.swap(x_share_bits1);
  }
    
  // beta_prime = beta xor (x>r)
  // beta = 1, x < r+1, beta_prime = 1
  snn_internal->PrivateCompare(x_share_bits, r, beta, beta_prime, size, sizeof(mpc_t)*8);

  print_vec(beta_prime, size, "Beta_prime:");
  print_vec(expect, size, "Beta_prime expected:");
}

/**
 * basic test case of PrivateCompare
*/
void test_PrivateCompareMPC_basic(SnnProtoType& snn0) {
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
    beta[i] = rand() % 2;
    expect[i] = (x[i]>r[i]) xor (beta[i]==1) ? 1 : 0;// beta xor (x>r)
  }

  vector<small_mpc_t> beta_prime(size, 0);
  size_t elem_size = sizeof(mpc_t)*8;
  
  print_vec(x, size, "Input x: ");
  print_vec(r, size, "Input r: ");

  auto snn_internal = snn0.GetInternal(msgid);
  int partyNum = snn0.GetMpcContext()->GetMyRole();
  vector<small_mpc_t> x_share_bits(size*elem_size, 0);
  if (PRIMARY) {
    vector<small_mpc_t> x_share_bits0(size*elem_size, 0), x_share_bits1(size*elem_size, 0);
    snn_internal->sharesOfBits(x_share_bits0, x_share_bits1, x, size, "COMMON");
    if (partyNum == PARTY_A)
      x_share_bits.swap(x_share_bits0);
    else
      x_share_bits.swap(x_share_bits1);
  }
    
  // beta_prime = beta xor (x>r)
  // beta = 1, x < r+1, beta_prime = 1
  snn_internal->PrivateCompare(x_share_bits, r, beta, beta_prime, size, sizeof(mpc_t) * 8);

  print_vec(beta_prime, size, "Beta_prime:");
  print_vec(expect, size, "Beta_prime expected:");
}

void run(int partyid) {
  SNN_PROTOCOL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  /*        PrivateCompare() = belta_prime = belta xor (x > r)    */
  //////////////////////////////////////////////////////////////////
  srand(0);
  
  test_PrivateCompareMPC_big_value(snn0);
  test_PrivateCompareMPC_basic(snn0);

  printf("ending of test private-compare block.\n\n");
  //////////////////////////////////////////////////////////////////
  SNN_PROTOCOL_TEST_UNINIT(partyid);
  printf("The end.\n\n");
}

RUN_MPC_TEST(run);