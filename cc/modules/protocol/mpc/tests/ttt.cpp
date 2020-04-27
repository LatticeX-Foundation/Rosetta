#include "snn.h"

namespace rosetta {
namespace mpc {
namespace debug {

void testPow2() {
  output_function();

  vector<mpc_t> vecx = {2048, 4096, 8192, 16384};
  size_t n = 1;
  vector<mpc_t> vecy(4);

  auto op = GetMpcOpDefault(Pow);
  auto oprec = GetMpcOpWithKey(Reconstruct2PC, op->msg_id());

  test_debug_print(oprec, vecx, 16, "Pow 0 vecx");

  op->Run(vecx, 0, vecy, 4);
  test_debug_print(oprec, vecy, 16, "Pow 0 vecy");

  op->Run(vecx, 1, vecy, 4);
  test_debug_print(oprec, vecy, 16, "Pow 1 vecy");

  op->Run(vecx, 2, vecy, 4);
  test_debug_print(oprec, vecy, 16, "Pow 2 vecy");

  op->Run(vecx, 3, vecy, 4);
  test_debug_print(oprec, vecy, 16, "Pow 3 vecy");
}

void testMatmul2() {
  output_function();
  auto op = GetMpcOpDefault(MatMul);
  auto oprec = GetMpcOpWithKey(Reconstruct2PC, op->msg_id());

  vector<mpc_t> veca = {8192, 4096, 8192, 4096};
  vector<mpc_t> vecb = {4096, 8192, 4096, 8192};
  vector<mpc_t> vecc(4);

  test_debug_print(oprec, veca, 16, "MatMul veca");
  test_debug_print(oprec, vecb, 16, "MatMul vecb");
  op->Run(veca, vecb, vecc, 2, 2, 2, 0, 0);
  test_debug_print(oprec, vecc, 16, "MatMul vecc");
}

void testSigmoid2() {
  output_function();
  auto op = GetMpcOpDefault(Sigmoid);
  auto oprec = GetMpcOpWithKey(Reconstruct2PC, op->msg_id());

  vector<double> rd = {-2.02, -1, -0.88, -0.5, -0.25, 0, 0.25, 0.5, 0.897, 1, 1.2};
  vector<mpc_t> r(rd.size());
  vector<mpc_t> o(rd.size());
  convert_double_to_mpctype(rd, r);

  test_debug_print(oprec, r, 16, "Sigmoid 0");
  op->Run(r, o, rd.size());
  test_debug_print(oprec, o, 16, "Sigmoid 1");
}

void testMax2() {
  output_function();
  auto op = GetMpcOpDefault(Max);
  auto oprec = GetMpcOpWithKey(Reconstruct2PC, op->msg_id());

  vector<double> a = {-0.01, 0.45, -0.34, 1.23, -0.72, 0.99, 1, -0.1, -0.5, 0, 2.3, 0.7}; // 3 x 4
  vector<mpc_t> va(a.size());
  convert_double_to_mpctype(a, va);
  vector<mpc_t> vmax(va.size());
  vector<mpc_t> vmaxi(va.size());

  test_debug_print(oprec, va, 16, "Max va");
  op->Run(va, vmax, vmaxi, 3, 4);
  test_debug_print(oprec, vmax, 16, "Max vmax");
  test_debug_print(oprec, vmaxi, 16, "Max vmaxi");
}
void testMean2() {
  output_function();
  auto op = GetMpcOpDefault(Mean);
  auto oprec = GetMpcOpWithKey(Reconstruct2PC, op->msg_id());

  vector<double> a = {-0.01, 0.45, -0.34, 1.23, -0.72, 0.99, -1, -0.1, -0.5, 0, 2.3, 0.7}; // 3 x 4
  //vector<double> a = {0.1, 0.2, 0.5, 0.4, -0.72, 0.99, 1, -0.1, -0.5, 0, 2.3, 0.7}; // 3 x 4
  vector<mpc_t> va(a.size());
  convert_double_to_mpctype(a, va);
  vector<mpc_t> vmean(va.size());

  test_debug_print(oprec, va, 16, "Mean va");
  op->Run(va, vmean, 3, 4);
  test_debug_print(oprec, vmean, 16, "Mean vmean");
}
void testRelu2() {
  output_function();
  auto op = GetMpcOpDefault(Relu);
  auto oprec = GetMpcOpWithKey(Reconstruct2PC, op->msg_id());

  vector<double> a = {-0.01, 0.45, -0.34, 1.23, -0.72, 0.99, 1, -0.1, -0.5, 0, 2.3, 0.7}; // 3 x 4
  vector<mpc_t> va(a.size());
  convert_double_to_mpctype(a, va);
  vector<mpc_t> vb(va.size());

  test_debug_print(oprec, va, 16, "Relu va");
  op->Run(va, vb, 3 * 4);
  test_debug_print(oprec, vb, 16, "Relu vb");

  {
    output_function();
    vector<double> a = {-0.01, 0.45, -0.34, 1.23, -0.72, 0.99, 1, -0.1, -0.5, 0, 2.3, 0.7}; // 3 x 4
    vector<mpc_t> va(a.size());
    convert_double_to_mpctype(a, va);
    vector<mpc_t> vb(va.size());

    test_debug_print(oprec, va, 16, "Relu va");
    op->Run(va, vb, 3 * 4);
    test_debug_print(oprec, vb, 16, "Relu vb");
  }

  {
    output_function();
    vector<double> a = {-0.01, 0, 0, 0, 0, 0.99, 1, -0.1, -0.5, 0, 2.3, 0.7}; // 3 x 4
    vector<mpc_t> va(a.size());
    convert_double_to_mpctype(a, va);
    vector<mpc_t> vb(va.size());

    test_debug_print(oprec, va, 16, "Relu2 va");
    op->Run(va, vb, 3 * 4);
    test_debug_print(oprec, vb, 16, "Relu2 vb");
  }
}

static mpc_t FloatToMpcTypeLess1(double a) {
  if (a >= 0) {
    return FloatToMpcType(a) % (0xFFFFFFFFFFFFFFFE);
  } else
    return (FloatToMpcType(a) - 1) % (0xFFFFFFFFFFFFFFFE);
}

void testMSB2() {
  size_t size = 2;
  size_t mid = 0;
  vector<mpc_t> zero_a(size, 0);

  output_function();

  cout << "zero inputs:\n";
  for (size_t i = 0; i < size; ++i)
    cout << zero_a[i];
  cout << endl;

  vector<mpc_t> mock_sc_a(size, 0);
  //manual data from the output of share-convert of zero-input-relu-prime (three zeros relu-prime example)
  //first input is 1, second is -1 in Z_{2^64-1}
  if (partyNum == PARTY_A) {
    //1165826204827014250 4539033683812076943
    mock_sc_a[0] = 11165826204827014250ULL;
    mock_sc_a[1] = 4539033683812076944ULL;
  } else {
    //7280917868882537366 13907710389897474671
    mock_sc_a[0] = 7280917868882537366ULL;
    mock_sc_a[1] = 13907710389897474672ULL;
  }

  cout << "msb input: ";
  for (size_t i = 0; i < size; ++i)
    cout << mock_sc_a[i];
  cout << endl;

  /*
  //mock of a_0 == 1 and a_1 == -1 in Z_{2^64-1}
  if (partyNum == PARTY_A)
  {
      for (size_t i = 0; i < size; ++i)
      {
          mock_sc_a[i] = FloatToMpcTypeLess1(1);
      }
  }
  else
  {
      for (size_t i = 0; i < size; ++i)
      {
          mock_sc_a[i] = FloatToMpcTypeLess1(-1);
      }
  }
  */

  auto opmsb = GetMpcOpDefault(ComputeMSB);
  auto oprec = GetMpcOpWithKey(Reconstruct2PC, opmsb->msg_id());
  if (THREE_PC) {
    vector<mpc_t> zero_c(size);
    opmsb->Run3PC(zero_a, zero_c, size);
    if (PRIMARY)
      oprec->Run(zero_c, size, "zero-sc msb: =>");

    vector<mpc_t> mock_sc_c(size);
    opmsb->Run3PC(mock_sc_a, mock_sc_c, size);
    if (PRIMARY)
      oprec->Run(mock_sc_c, size, "mock-relu-sc msb =>");
  }

  cout << "floatToMytype(-1): " << FloatToMpcType(-1)
       << ", FloatToMpcTypeLess1(-1): " << FloatToMpcTypeLess1(-1) << endl;
  cout << "testMsb ok." << endl;
}

void testReluPrime2() {
  output_function();
  size_t size = 1;

  auto print_inputs = [&](const string& name, const vector<mpc_t>& inputs) {
    cout << name << "-inputs: " << endl;
    for (auto& elem : inputs)
      cout << elem << " ";
    cout << endl;
  };

  auto opRePr = GetMpcOpDefault(ReluPrime);
  auto oprec = GetMpcOpWithKey(Reconstruct2PC, opRePr->msg_id());
  auto mpc_relu_prime = [&](const string& name, const vector<mpc_t>& inputs) {
    size_t size = inputs.size();
    vector<mpc_t> outputs(size, 0);
    opRePr->Run3PC(inputs, outputs, size);
    if (PRIMARY) {
      oprec->Run(outputs, size, name);
    }

    // //print
    // cout << name << "-relu_prime_output: " << endl;
    // for (size_t i = 0; i < size; i++)
    // {
    //     cout << outputs[i] << " ";
    // }
    cout << endl;
  };

  mpc_t pos_in = FloatToMpcType(1); //real value is 2
  mpc_t neg_in = FloatToMpcType(-1); //real value is -2

  //1. one zero
  string test_name = "one-zero";
  vector<mpc_t> input1(size);
  for (size_t i = 0; i < size; ++i) {
    input1[i] = 0;
  }
  print_inputs(test_name, input1);
  mpc_relu_prime(test_name, input1);

  //2. two zero
  test_name = "two-zero";
  size = 2;
  vector<mpc_t> input2(size);
  for (size_t i = 0; i < size; ++i) {
    input2[i] = 0;
  }
  print_inputs(test_name, input2);
  mpc_relu_prime(test_name, input2);

  //3. three zero
  test_name = "three-zero";
  size = 6;
  vector<mpc_t> input3(size);
  for (size_t i = 0; i < size; ++i) {
    input3[i] = 0;
  }
  print_inputs(test_name, input3);
  mpc_relu_prime(test_name, input3);

  //4. two zero and other input
  test_name = "two-zero-and-others";
  size = 4;
  vector<mpc_t> input4(size);
  for (size_t i = 0; i < size; ++i) {
    if (i >= 2)
      input4[i] = neg_in;
    else
      input4[i] = 0;
  }
  print_inputs(test_name, input4);
  mpc_relu_prime(test_name, input4);

  //5. one zero and other input
  test_name = "zero-positive-vector";
  size = 8;
  vector<mpc_t> input5(size);
  for (size_t i = 0; i < size; ++i) {
    if (i % 2 == 0)
      input5[i] = 0;
    else
      input5[i] = pos_in;
  }
  print_inputs(test_name, input5);
  mpc_relu_prime(test_name, input5);

  //6. one zero and other input
  test_name = "rand-pos-and-neg-others";
  size = 8;
  vector<mpc_t> input6(size);
  for (size_t i = 0; i < size; ++i) {
    if (i % 2 == 0)
      input6[i] = FloatToMpcType(rand() % 1000);
    else
      input6[i] = FloatToMpcType(-(rand() % 1000));
  }
  print_inputs(test_name, input6);
  mpc_relu_prime(test_name, input6);

  cout << "test_mpc_relu_prime OK." << endl;
}

void testShareConvert2() {
  output_function();
  size_t size = 1;

  auto print_inputs = [&](const string& name, const vector<mpc_t>& inputs) {
    cout << name << "-inputs: " << endl;
    for (auto& elem : inputs)
      cout << elem << " ";
    cout << endl;
  };

  auto opSC = GetMpcOpDefault(ShareConvert);
  auto oprec = GetMpcOpWithKey(Reconstruct2PC, opSC->msg_id());
  auto mpc_share_convert = [&](const string& name, const vector<mpc_t>& inputs) {
    size_t size = inputs.size();
    vector<mpc_t> outputs = inputs;
    opSC->Run(outputs, size);
    if (PRIMARY) {
      oprec->Run(outputs, size, name);
      //funcReconstruct2PC(outputs, size, outputs);
    }

    // //print
    // cout << name << "-relu_prime_output: " << endl;
    // for (size_t i = 0; i < size; i++)
    // {
    //     cout << outputs[i] << " ";
    // }
    cout << endl;
  };

  mpc_t neg_in = FloatToMpcType(1); //real value is 2
  mpc_t pos_in = FloatToMpcType(-1); //real value is -2

  //1. one zero
  string test_name = "sc-one-zero";
  vector<mpc_t> input1(size);
  for (size_t i = 0; i < size; ++i) {
    input1[i] = 0;
  }
  print_inputs(test_name, input1);
  mpc_share_convert(test_name, input1);

  //2. two zero
  test_name = "sc-two-zero";
  size = 2;
  vector<mpc_t> input2(size);
  for (size_t i = 0; i < size; ++i) {
    input2[i] = 0;
  }
  print_inputs(test_name, input2);
  mpc_share_convert(test_name, input2);

  //3. three zero
  test_name = "sc-three-zero";
  size = 6;
  vector<mpc_t> input3(size);
  for (size_t i = 0; i < size; ++i) {
    if (i < 3)
      input3[i] = 0;
    else {
      if (partyNum == PARTY_A)
        input3[i] = pos_in;
      else if (partyNum == PARTY_B)
        input3[i] = neg_in;
    }
  }
  print_inputs(test_name, input3);
  mpc_share_convert(test_name, input3);

  //4. two zero and other input
  test_name = "sc-two-zero-and-others";
  size = 4;
  vector<mpc_t> input4(size);
  for (size_t i = 0; i < size; ++i) {
    if (i >= 2)
      input4[i] = pos_in;
    else
      input4[i] = 0;
  }
  print_inputs(test_name, input4);
  mpc_share_convert(test_name, input4);

  //5. one zero and other input
  test_name = "sc-two-zero-and-others";
  size = 4;
  vector<mpc_t> input5(size);
  for (size_t i = 0; i < size; ++i) {
    if (i % 2 == 0)
      input5[i] = pos_in;
    else
      input5[i] = 0;
  }
  print_inputs(test_name, input5);
  mpc_share_convert(test_name, input5);

  cout << "testShareConvert OK." << endl;
}

} // namespace debug
} // namespace mpc
} // namespace rosetta