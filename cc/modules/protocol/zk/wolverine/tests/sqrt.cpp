#include "wvr_test.h"

#include <string>
#include <cmath>

using namespace emp;
using namespace rosetta;
using namespace rosetta::zk;


void run(int partyid) {
  WVR_PROTOCOL_TEST_INIT(partyid);


  msg_id_t msgid("log");

  // input a b
  vector<double> a = {100.5, 10.23, 4, 400, 40000, 0.04, 0.0004,  0.0001, 3432760.0, 0.999008};
  int SZ = a.size();
  vector<string> str_a(SZ);

  for(auto i = 0; i < SZ; ++i) {
    str_a[i] = to_string(a[i]);
  }

  vector<string> input_str, out_str;
  attr_type attr;
  
  vector<string> sqrt;
  vector<double> expect_sqrt{0.09975, 0.3127, 0.5,  0.05,  0.005,  5.0, 50.0,  100.0, 0.00054, 1.000496};
  vector<double> revealed_result(SZ);

  ///////////// The is a failed attempt that show we can not do the 1/X in field in our case,
  ///            1. the implementation of `egcd` seems wrong...
  ///            2. We encoded the negative numbers in the upper half, this is not compitable with x^{-1} in field.
  //             ...
  // vector<uint64_t> field_ele(SZ);
  // zk_encode(a, field_ele);
  // vector<uint64_t> rev_field_ele(SZ);
  // // testing reverse.
  // for(int i = 0; i < SZ; ++i) {
  //   auto temp = field_ele[i];
  //   auto temp2 = temp;
  //   auto temp3 = temp2;
  //   auto is_ok = egcd(temp, PR, temp2, temp3);
  //   cout << "testing EGCD " << is_ok << " x:" << field_ele[i] << ", x^{-1}:" << temp2 <<  "-->" << mult_mod(temp, temp2) << endl;
  //   rev_field_ele[i] = temp2;
  // }

  // vector<double> rev_a(SZ);
  // zk_decode(rev_field_ele, rev_a);
  // for(int i = 0; i < SZ; ++i) {
  //   cout << "original x:" << a[i] << endl;
  //   cout << "1/x: " << rev_a[i] << endl;
  // }

  WVR0.GetOps(msgid)->PrivateInput(ALICE - 1, a, input_str);
  print_vec(input_str, 10, "[Private] input_str a");
  int ITER = 2;
  print_vec(a, 20, "Original plain input:");
  print_vec(expect_sqrt, 20, "Expected RSqrt plain:");
  for (int i = 0; i < ITER; ++i) {
    cout << i << "-th iter test BEGIN: " << endl;
    /////// **************************RSQRT***************************
    WVR0.GetOps(msgid)->Rsqrt(input_str, sqrt);
    WVR0.GetOps(msgid)->Reveal(sqrt, revealed_result);
    print_vec(revealed_result, 10, "[Private] RSqrt result revealed:");
    cout << i << "-th iter test END: " << endl;
  }

  WVR_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_ZK_TEST(run);
