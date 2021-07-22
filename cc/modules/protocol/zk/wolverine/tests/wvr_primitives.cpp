#include "wvr_test.h"

#include <string>
#include <cmath>

using namespace emp;
using namespace rosetta;
using namespace rosetta::zk;


void run(int partyid) {
  WVR_PROTOCOL_TEST_INIT(partyid);
  msg_id_t msgid("log");

// **********************Note[georgeShi] BEGIN: Please DO NOT remove me!
// ********************case 0 : test truncation with float-point input!
  // vector<double> ori_a = {-213.521, 213.521};
  // int size = ori_a.size(); 

  // print_vec(ori_a, 10, "original float value:");
  // vector<int64_t> bar_x(size);
  // vector<uint64_t> field_x(size);

  // for(auto i = 0; i < size; ++i) {
  //   bar_x[i] = FloatToFixPoint(ori_a[i]);
  // }

  // zk_encode(bar_x, field_x);
  // print_vec(field_x, 10, "field element:");
  
  //////// ********** F_p operations ******************

  ////// original method, borrowed from MPC paper.
  // 2^{k-1}
  // int64_t U = (1LL << (ZK_K-1));
  // vector<uint64_t> B(size);
  // for(int i =0; i < size; ++i) {
  //   B[i] = mod(U + field_x[i]);

  //   uint64_t b1 = common_mod(B[i], 1 << ZK_M);
  //   uint64_t b2 = common_mod(bar_x[i], 1 << ZK_M);
  //   cout << "bar:" << bar_x[i] << ", field_x:" << field_x[i] << endl;
  //   cout << "They should be equal, b1:" << b1 << " VS " << "b2:" << b2 << endl;
    
  // }
	// block rr;
	// PRG prg;
	// prg.random_block(&rr, 1);
	// uint64_t *temp_a = (uint64_t*)&rr;
  // // randome r < 2^{58}
  // uint64_t R = (*temp_a) & (__uint128_t)0x03FFFFFFFFFFFFFFULL;
  // uint64_t R_prime = common_mod(R, 1UL << ZK_M);



  // vector<uint64_t> C(size);
  // vector<uint64_t> C_prime(size);
  // vector<uint64_t> A_prime(size);
  // vector<uint64_t> D(size);
  // uint64_t temp = TWO_INV;
  // uint64_t temp2 = mult_mod(2, TWO_INV);
  // cout << "2^{-1}=" << temp << " vs " << temp2 << endl;

  // temp = uint64_t(1ULL << ZK_M);
  // temp2 = uint64_t(1ULL << (MERSENNE_PRIME_EXP - ZK_M));
  // uint64_t temp3 = mult_mod(temp, temp2);
  // cout << "2^{-M}=" << temp2 << " vs " << temp3 << endl;

  // // testing reverse.
  // auto is_ok = egcd(temp, PR, temp2, temp3);
  // cout << "testing EGCD " << is_ok << " x:" << temp << ", x^{-1}:" << temp2 << endl;

  // for(int i =0; i < size; ++i) {
  //   C[i] = mod(B[i] + R);
  //   C_prime[i] = common_mod(C[i], 1ULL << ZK_M);
  //   A_prime[i] = common_mod(C_prime[i] - R_prime, PR);
  //   D[i] = common_mod(field_x[i] - A_prime[i], PR);
  //   D[i] = mult_mod(D[i], temp2);
  // }

  // vector<int64_t> back_bar_x(size);
  // // zk_decode(field_x, back_bar_x);
  // zk_decode(D, back_bar_x);
  // vector<double> back_float(size);
  // for(auto i = 0; i < size; ++i) {
  //   back_float[i] = FixToFloatPoint(back_bar_x[i]);
  // }

  // print_vec(back_float, 10, "recovered float value:");

  // /////// ZK-adapted simplified method 
  // vector<uint64_t> trunc_x(size);
  // field_truncation(field_x, trunc_x);
  
  // zk_decode(trunc_x, back_bar_x);
  // for(auto i = 0; i < size; ++i) {
  //   back_float[i] = FixToFloatPoint(back_bar_x[i]);
  // }

  // print_vec(back_float, 10, "NEW recovered float value:");

// **********************Note[georgeShi] END.

// test a lot of multiplications
  // size_t size = 31*49*64;

  // size_t size = 2*49*64;
  // for (size_t size = 99999; size <= 100002; ++size) {
  //   log_info << "-------------   test of " << size << "  multiplications ....";
  //   vector<double> input1(size, 1), input2(size, 1.2);
  //   vector<string> input1_str, input2_str, mul_ret, mul_const_ret;
  //   vector<string> const_input2(size, string("1.2"));
  //   WVR0.GetOps(msgid)->PrivateInput(0, input1, input1_str);
  //   WVR0.GetOps(msgid)->PrivateInput(0, input2, input2_str);
  //   WVR0.GetOps(msgid)->Mul(input1_str, input2_str, mul_ret);
  //   WVR0.GetOps(msgid)->Mul(input1_str, const_input2, mul_const_ret);
  //   vector<string> temp, temp_const;
  //   WVR0.GetOps(msgid)->Reveal(mul_ret, temp);
  //   WVR0.GetOps(msgid)->Reveal(mul_const_ret, temp_const);
  //   print_vec(temp, 50, "[PP] large-size MUL result plain str:");
  //   print_vec(temp_const, 50, "[PP] large-size MUL-const result plain str:");
  //   log_info << "test of " << size << "  multiplications end. -------------";
  // }

  // input a b
  vector<double> a = {10.5, 0.23, 8, 7, 6, -10.73, -9,  8, 0, -10000,   263.662};//, -6};
  vector<double> b = { 1.2, 6.32, 4, 4, 12,  1.42, -2, -3, 4, 0.0004, 343276.00};//,  0};
  // vector<double> a = {10, 9, 8, 7, 6, -10, -9,  8, 0, -6};
  // vector<double> b = { 1, 3, 4, 4, 12,   1, -2, -3, 4,  0};
  vector<string> str_a(a.size());
  vector<string> str_b(b.size());

  for(auto i = 0; i < a.size(); ++i) {
    str_a[i] = to_string(a[i]);
    str_b[i] = to_string(b[i]);
  }

  vector<string> input_str[2], out_str;
  attr_type attr;
  
  //add, sub, mul, div
  vector<string> add, sub, mul, div;
  vector<double> expect_add{  11.7,   6.55, 12,   11,  18,    -9.31, -11,      5,  4,  -9999.9996,   343539.662};//, -6};
  vector<double> expect_sub{   9.3,  -6.08,  4,    3,  -6,   -12.15,  -7,     11, -4, -10000.0004,  -343012.338};//, -6};
  vector<double> expect_mul{  12.6, 1.4536, 32,   28,  72, -15.2366,  18,    -24,  0,        -4.0, 90508836.712};//,  0};
  vector<double> expect_div{  8.75, 0.0364,  2, 1.75, 0.5,  -7.5563, 4.5,-2.6667,  0, -25000000.0,     0.000768};//,  0};   // the last one is illegal, set to 0
  
  // vector<double> expect_add{11, 12, 12, 11, 18,  -9, -11,   5,  4, -6};
  // vector<double> expect_sub{ 9,  6,  4,  3, -6, -11,  -7,  11, -4, -6};
  // vector<double> expect_mul{10, 27, 32, 28, 72, -10,  18, -24,  0,  0};
  vector<string> revealed_str(a.size());

  WVR0.GetOps(msgid)->PrivateInput(ALICE - 1, a, input_str[0]);
  WVR0.GetOps(msgid)->PrivateInput(ALICE - 1, b, input_str[1]);

  print_vec(a, 20, "original input a:");
  print_vec(b, 20, "original input b:");
  print_vec(expect_add, 20, "Expected add:");
  print_vec(expect_sub, 20, "Expected sub:");
  print_vec(expect_mul, 20, "Expected mul:");

/// *********************** case 1: Private VS private ************************************
  print_vec(input_str[0], 20, "[PP] input_str a");
  print_vec(input_str[1], 20, "[PP] input_str b");

  WVR0.GetOps(msgid)->Add(input_str[0], input_str[1], add);
  WVR0.GetOps(msgid)->Sub(input_str[0], input_str[1], sub);


  WVR0.GetOps(msgid)->Reveal(add, revealed_str);
  WVR0.GetOps(msgid)->Reveal(sub, expect_sub);


  print_vec(revealed_str, 20, "[PP] Add result plain str:");
  print_vec(expect_sub, 20, "[PP] sub result plain:");

  log_info << "PP PASS!";

  /// *********************** case 2: Private VS Const ************************************
  print_vec(input_str[0], 20, "[PC] input_str a");
  print_vec(str_b, 20, "[PC] input_str b");

  attr["rh_is_const"] = "1";
  WVR0.GetOps(msgid)->Add(input_str[0], str_b, add, &attr);
  WVR0.GetOps(msgid)->Sub(input_str[0], str_b, sub, &attr);

  WVR0.GetOps(msgid)->Reveal(add, expect_add);
  WVR0.GetOps(msgid)->Reveal(sub, expect_sub);

  print_vec(expect_add, 20, "[PC] Add result plain:");
  print_vec(expect_sub, 20, "[PC] sub result plain:");

  log_info << "PC PASS!";

  /// *********************** case 3: Const VS Private ************************************
  print_vec(str_a, 20, "[CP] input_str a");
  print_vec(input_str[1], 20, "[CP] input_str b");

  WVR0.GetOps(msgid)->Add(str_a, input_str[1], add);
  WVR0.GetOps(msgid)->Sub(str_a, input_str[1], sub);

  WVR0.GetOps(msgid)->Reveal(add, expect_add);
  WVR0.GetOps(msgid)->Reveal(sub, expect_sub);

  print_vec(expect_add, 20, "[CP] Add result plain:");
  print_vec(expect_sub, 20, "[CP] sub result plain:");

  log_info << "CP PASS!";

  /// *********************** case 4: Const VS Const ************************************
  print_vec(str_a, 20, "[CC] input_str a");
  print_vec(str_b, 20, "[CC] input_str b");

  WVR0.GetOps(msgid)->Add(str_a, str_b, add);
  WVR0.GetOps(msgid)->Sub(str_a, str_b, sub);


  WVR0.GetOps(msgid)->Reveal(add, expect_add);
  WVR0.GetOps(msgid)->Reveal(sub, expect_sub);


  print_vec(expect_add, 20, "[CC] Add result plain:");
  print_vec(expect_sub, 20, "[CC] sub result plain:");

  log_info << "CC PASS!";

  ///// **************************MUL***************************
  WVR0.GetOps(msgid)->Mul(input_str[0], input_str[1], mul);
  WVR0.GetOps(msgid)->Reveal(mul, expect_mul);
  print_vec(expect_mul, 20, "[PP] mul result plain:");

  attr.clear();
  attr["rh_is_const"] = "1";
  WVR0.GetOps(msgid)->Mul(input_str[0], str_b, mul, &attr);
  WVR0.GetOps(msgid)->Reveal(mul, expect_mul);
  print_vec(expect_mul, 20, "[PC] mul result plain:");

  WVR0.GetOps(msgid)->Mul(str_a, input_str[1], mul);
  WVR0.GetOps(msgid)->Reveal(mul, expect_mul);
  print_vec(expect_mul, 20, "[CP] mul result plain:");

  WVR0.GetOps(msgid)->Mul(str_a, str_b, mul);
  WVR0.GetOps(msgid)->Reveal(mul, expect_mul);
  print_vec(expect_mul, 20, "[CC] mul result plain:");

// #if !LOCAL_SIMULATE
  // /////// **************************Div***************************
  WVR0.GetOps(msgid)->Div(input_str[0], input_str[1], div);
  WVR0.GetOps(msgid)->Reveal(div, expect_div);
  print_vec(expect_div, 20, "[PP] div result plain:");

#if !LOCAL_SIMULATE
  attr.clear();
  attr["rh_is_const"] = "1";
  WVR0.GetOps(msgid)->Div(input_str[0], str_b, div, &attr);
  WVR0.GetOps(msgid)->Reveal(div, expect_div);
  print_vec(expect_div, 20, "[PC] div result plain:");
#endif

  WVR0.GetOps(msgid)->Div(str_a, input_str[1], div);
  WVR0.GetOps(msgid)->Reveal(div, expect_div);
  print_vec(expect_div, 20, "[CP] div result plain:");

  WVR0.GetOps(msgid)->Div(str_a, str_b, div);
  WVR0.GetOps(msgid)->Reveal(div, expect_div);
  print_vec(expect_div, 20, "[CC] div result plain:");

  // ////////////////    max, avg, relu, matmul  check plain/zk   ////////////
  attr.clear();
  int rows = 3, cols = 3;
  attr["rows"] = std::to_string(rows);
  attr["cols"] = std::to_string(cols);
  vector<double> test_max = {-6.234, -1.8, -3.889, 2.345, 7.126, 5.129, -6.678, 0, -9.6789};
  vector<string> max_out, max_revealed;
  WVR0.GetOps(msgid)->PrivateInput(ALICE - 1, test_max, input_str[0]);
  WVR0.GetOps(msgid)->Max(input_str[0], max_out, &attr);
  WVR0.GetOps(msgid)->Reveal(max_out, max_revealed);
  print_vec(max_revealed, 20, "Max result plain:");

  vector<double> test_avg = {-6.234, -1.8, -3.889, 2.345, 7.126, 5.129, -6.678, 0, -9.6789};
  vector<string> avg_out, avg_revealed;
  WVR0.GetOps(msgid)->PrivateInput(ALICE - 1, test_avg, input_str[0]);
  WVR0.GetOps(msgid)->Mean(input_str[0], avg_out, &attr);
  WVR0.GetOps(msgid)->Reveal(avg_out, avg_revealed);
  vector<double> input_reveal;
  WVR0.GetOps(msgid)->Reveal(input_str[0], input_reveal);
  print_vec(input_reveal, input_reveal.size(), "input_reveal: ");
  print_vec(avg_revealed, avg_revealed.size(), "avg result plain:");

  vector<double> test_relu = {-6.234, -1.8, -3.889, 2.345, 7.126, 5.129, -6.678, 0, -9.6789};
  vector<string> relu_out, relu_revealed;
  WVR0.GetOps(msgid)->PrivateInput(ALICE - 1, test_relu, input_str[0]);
  WVR0.GetOps(msgid)->Relu(input_str[0], relu_out);
  WVR0.GetOps(msgid)->Reveal(relu_out, relu_revealed);
  print_vec(relu_revealed, 20, "relu result plain:");

  vector<double> test_matmul1 = {-6.234, -1.8, -3.889, 2.345, 7.126, 5.129, -6.678, 0, -9.6789};
  vector<double> test_matmul2 = {-16.234, -11.8, -31.889, 21.345, 71.126, 51.129, -61.678, 10, -19.6789};
  vector<string> matmul_out, matmul_revealed;
  /* expect: 
  array([[ 302.647498  ,  -93.3556    ,  183.2950681 ],
       [-202.310722  ,  530.462876  ,  188.6324709 ],
       [ 705.3858462 ,  -17.9886    ,  403.42484721]])
      */
  attr.clear();
  attr["m"] = std::to_string(rows);
  attr["k"] = std::to_string(rows);
  attr["n"] = std::to_string(rows);
  WVR0.GetOps(msgid)->PrivateInput(ALICE - 1, test_matmul1, input_str[0]);
  WVR0.GetOps(msgid)->PrivateInput(ALICE - 1, test_matmul2, input_str[1]);
  WVR0.GetOps(msgid)->Matmul(input_str[0], input_str[1], matmul_out, &attr);
  WVR0.GetOps(msgid)->Reveal(matmul_out, matmul_revealed);
  print_vec(matmul_revealed, 20, "matmul result plain:");

  /////////////   mul with high resolution  ////////////////
  size_t size = 1;
  vector<double> sa(size, 3.234567);
  vector<double> sb(size, 1.234567);
  vector<double> expect(size, 3.234567*1.234567);
  vector<string> sa_str, sb_str;
  vector<double> issue_mul;

  log_info << "test mul(3.234567, 1.234567)...";
  WVR0.GetOps(msgid)->PrivateInput(0, sa, sa_str);
  WVR0.GetOps(msgid)->PrivateInput(0, sb, sb_str);
  vector<string> result;
  WVR0.GetOps(msgid)->Mul(sa_str, sb_str, result);
  WVR0.GetOps(msgid)->Reveal(result, issue_mul);
  log_info << "test mul(3.234567, 1.234567) ok. first: " << issue_mul[0];

  print_vec(expect, expect.size(), "test mul(3.234567, 1.234567)expect: ");
  print_vec(issue_mul, issue_mul.size(), "test mul(3.234567, 1.234567) mul reveal: ");

// #endif
  WVR_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_ZK_TEST(run);
