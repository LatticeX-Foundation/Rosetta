#include "wvr_test.h"

void run(int partyid) {
  WVR_PROTOCOL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  msg_id_t msgid("basic_ops_test");
  {// add
    size_t size = 2;
    vector<double> a(size, 1.234567);
    vector<double> b(size, -1.234567);
    vector<double> expect(size, 1.234567+(-1.234567));
    vector<string> a_str, b_str;
    vector<double> add;

    log_info << "test0 add(1.234567, -1.234567)...";
    WVR0.GetOps(msgid)->PrivateInput(0, a, a_str);
    WVR0.GetOps(msgid)->PrivateInput(0, b, b_str);
    vector<string> result;
    WVR0.GetOps(msgid)->Add(a_str, b_str, result);
    WVR0.GetOps(msgid)->Reveal(result, add);
    log_info << "test add(1.234567, -1.234567) ok. first: " << add[0];

    print_vec(expect, expect.size(), "test0 add(1.234567, -1.234567)expect: ");
    print_vec(add, add.size(), "test0 add(1.234567, -1.234567) add reveal: ");
  }

  {// sub
    size_t size = 2;
    vector<double> a(size, 3.234567);
    vector<double> b(size, 1.234567);
    vector<double> expect(size, 3.234567-1.234567);
    vector<string> a_str, b_str;
    vector<double> sub;

    log_info << "test1 sub(3.234567, 1.234567)...";
    WVR0.GetOps(msgid)->PrivateInput(0, a, a_str);
    WVR0.GetOps(msgid)->PrivateInput(0, b, b_str);
    vector<string> result;
    WVR0.GetOps(msgid)->Sub(a_str, b_str, result);
    WVR0.GetOps(msgid)->Reveal(result, sub);
    log_info << "test1 sub(3.234567, 1.234567) ok. first: " << sub[0];

    print_vec(expect, expect.size(), "test1 sub(3.234567, 1.234567)expect: ");
    print_vec(sub, sub.size(), "test1 sub(3.234567, 1.234567) sub reveal: ");
  }

  {// mul
    size_t size = 2;
    vector<double> a(size, 3.234567);
    vector<double> b(size, 1.234567);
    vector<double> expect(size, 3.234567*1.234567);
    vector<string> a_str, b_str;
    vector<double> mul;

    log_info << "test2 mul(3.234567, 1.234567)...";
    WVR0.GetOps(msgid)->PrivateInput(0, a, a_str);
    WVR0.GetOps(msgid)->PrivateInput(0, b, b_str);
    vector<string> result;
    WVR0.GetOps(msgid)->Mul(a_str, b_str, result);
    WVR0.GetOps(msgid)->Reveal(result, mul);
    log_info << "test mul(3.234567, 1.234567) ok. first: " << mul[0];

    print_vec(expect, expect.size(), "test2 mul(3.234567, 1.234567)expect: ");
    print_vec(mul, mul.size(), "test2 mul(3.234567, 1.234567) mul reveal: ");
  }

  {// mul
    size_t size = 2;
    vector<double> a(size, 3.234567);
    vector<double> b(size, -1.234567);
    vector<double> expect(size, 3.234567*(-1.234567));
    vector<string> a_str, b_str;
    vector<double> mul;

    log_info << "test3 mul(3.234567, -1.234567)...";
    WVR0.GetOps(msgid)->PrivateInput(0, a, a_str);
    WVR0.GetOps(msgid)->PrivateInput(0, b, b_str);
    vector<string> result;
    WVR0.GetOps(msgid)->Mul(a_str, b_str, result);
    WVR0.GetOps(msgid)->Reveal(result, mul);
    log_info << "test3 mul(3.234567, -1.234567) ok. first: " << mul[0];

    print_vec(expect, expect.size(), "test3 mul(3.234567, -1.234567)expect: ");
    print_vec(mul, mul.size(), "test3 mul(3.234567, -1.234567) mul reveal: ");
  }

  // {// mul const
  //   attr_type attr;
  //   attr["rh_is_const"]  = "1";

  //   size_t size = 2;
  //   double db = 1.234567;
  //   string db_str(sizeof(db)+1, '$');
  //   memcpy((char*)db_str.data(), &db, sizeof(db));
  //   vector<string> b_str_const(size, db_str);

  //   vector<double> a(size, 3.234567);
  //   vector<double> expect(size, 3.234567*1.234567);
  //   vector<string> a_str, b_str;
  //   vector<double> mul;

  //   log_info << "test4 mul_const(3.234567, 1.234567)...";
  //   WVR0.GetOps(msgid)->PrivateInput(0, a, a_str);
  //   vector<string> result;
  //   WVR0.GetOps(msgid)->Mul(a_str, b_str, result, &attr);
  //   WVR0.GetOps(msgid)->Reveal(result, mul);
  //   log_info << "test mul_const(3.234567, 1.234567) ok. first: " << mul[0];

  //   print_vec(expect, expect.size(), "test4 mul_const(3.234567, 1.234567)expect: ");
  //   print_vec(mul, mul.size(), "test4 mul_const(3.234567, 1.234567) mul reveal: ");
  // }
  
  // {// test a lot of multiplications
  //   size_t size = 49*49*64;
  //   log_info << "-------------   test of " << size << "  multiplications ....";
  //   vector<double> input1(size, 1), input2(size, 1.2);
  //   vector<string> input1_str, input2_str, mul_ret, square_ret;
  //   WVR0.GetOps(msgid)->PrivateInput(0, input1, input1_str);
  //   WVR0.GetOps(msgid)->PrivateInput(0, input2, input2_str);
  //   WVR0.GetOps(msgid)->Mul(input1_str, input2_str, mul_ret);
  //   WVR0.GetOps(msgid)->Square(input2_str, square_ret);
  //   vector<string> temp;
  //   WVR0.GetOps(msgid)->Reveal(mul_ret, temp);
  //   log_info << "mul[0]: " << temp[0];
  //   WVR0.GetOps(msgid)->Reveal(square_ret, temp);
  //   log_info << "square[0]: " << temp[0];
    
  //   log_info << "test of " << size << "  multiplications end. -------------   ";
  // }

  WVR_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_ZK_TEST(run);