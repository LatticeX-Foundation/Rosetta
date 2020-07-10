
  vector<double> X = {-1.01, -2.00, 0, 1.3, 2.02, 3.14, +2, -0.01};
  vector<double> Y = {-1.00, -2.01, 0, 1.3, 2.03, 3.12, -2, +0.01};
  size_t size = X.size();
  print_vec(X, 10, "X");
  print_vec(Y, 10, "Y");

  string msgid("Helix performance test");
  cout << __FUNCTION__ << " " << msgid << endl;

  vector<string> strX, strY, strZ;
  prot0.GetOps(msgid)->PrivateInput(0, X, strX);
  print_vec(strX, 10, "strX");
  prot0.GetOps(msgid)->PrivateInput(1, Y, strY);
  print_vec(strY, 10, "strY");

#define _perf_test_beg(op, times)                                                                  \
  do {                                                                                             \
    string tag = string("TIMER:") + string(#op) + " (times=" + to_string(times) + ") elapsed(s) "; \
    SimpleTimer timer;                                                                             \
    for (int i = 0; i < times; i++) {
#define _perf_test_end()                 \
  }                                      \
  cout << tag << timer.elapse() << endl; \
  }                                      \
  while (0)

#define binary_perf_test(op, times)                                     \
  _perf_test_beg(op, times) prot0.GetOps(msgid)->op(strX, strY, strZ); \
  _perf_test_end()

#define unary_perf_test(op, times)                                \
  _perf_test_beg(op, times) prot0.GetOps(msgid)->op(strX, strZ); \
  _perf_test_end()

#define reduce_perf_test(op, times, r, c)      \
  _perf_test_beg(op, times) attr_type attr;    \
  attr["rows"] = to_string(r);                 \
  attr["cols"] = to_string(c);                 \
  prot0.GetOps(msgid)->op(strX, strZ, &attr); \
  _perf_test_end()

  binary_perf_test(Add, 1000);
  binary_perf_test(Sub, 1000);
  binary_perf_test(Mul, 1000);
  binary_perf_test(Truediv, 2);
  binary_perf_test(Floordiv, 2);

  binary_perf_test(Less, 2);
  binary_perf_test(LessEqual, 2);
  binary_perf_test(Equal, 2);
  binary_perf_test(NotEqual, 2);
  binary_perf_test(Greater, 2);
  binary_perf_test(GreaterEqual, 2);

  unary_perf_test(Square, 2);
  unary_perf_test(Negative, 1000);
  unary_perf_test(Abs, 2);
  unary_perf_test(AbsPrime, 2);
  unary_perf_test(Log, 2);
  unary_perf_test(Log1p, 2);
  unary_perf_test(HLog, 2);
  unary_perf_test(Relu, 10);
  unary_perf_test(ReluPrime, 10);
  unary_perf_test(Sigmoid, 2);

  reduce_perf_test(Mean, 100, 2, 4);
  reduce_perf_test(Sum, 1000, 2, 4);
  reduce_perf_test(AddN, 1000, 2, 4);

  {
    // MatMul
    SimpleTimer timer;
    {
      int times = 1000;
      string tag = string("TIMER:Matmul (times=") + to_string(times) + ") elapsed(s) ";
      attr_type attr;
      attr["m"] = "2";
      attr["k"] = "4";
      attr["n"] = "2";
      timer.start();
      for (int i = 0; i < 5; i++) {
        prot0.GetOps(msgid)->Matmul(strX, strY, strZ, &attr);
      }
      cout << tag << timer.elapse() << endl;
    }
  }

  /*
Pow

Max
Min

Reveal
*/