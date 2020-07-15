
  Logger::Get().log_to_stdout(true);
  //////////////////////////////////////////////////////////////////

  stringstream ssheader;
  // clang-format off
      ssheader << "+---------------------+------------+------------+----------------------+------------+------------+------------+------------+" << endl;
      ssheader << "|" << setw(21) << "OP "         << "|" << setw(12) << "times "
               << "|" << setw(12) << "elapsed(s) " << "|" << setw(22) << "shape size "
               << "|" << setw(12) << "sent data "  << "|" << setw(12) << "recv data "
               << "|" << setw(12) << "sent msgs "  << "|" << setw(12) << "recv msgs "
               << "|" << endl;
      ssheader << "+---------------------+------------+------------+----------------------+------------+------------+------------+------------+";
  // clang-format on
  stringstream ssender;
  // clang-format off
      ssender  << "+---------------------+------------+------------+----------------------+------------+------------+------------+------------+";
  // clang-format on

#define _perf_test_beg(op, times)                                      \
  do {                                                                 \
    stringstream ss;                                                   \
    ss << "|" << setw(20) << string(#op) << " |" << setw(11) << times; \
    SimpleTimer timer;                                                 \
    for (int i = 0; i < times; i++) {
#define _perf_test_end(shape)                                            \
  }                                                                      \
  timer.stop();                                                          \
  ss << " |" << setw(11) << timer.elapse() << " |" << setw(21) << shape; \
  ss << " |";                                                            \
  cout << ss.str() << endl;                                              \
  }                                                                      \
  while (0)

#define binary_perf_test(op, times)                                    \
  _perf_test_beg(op, times) prot0.GetOps(msgid)->op(strX, strY, strZ); \
  _perf_test_end("k=" + to_string(strX.size()) + ",k=" + to_string(strY.size()))

#define unary_perf_test(op, times)                               \
  _perf_test_beg(op, times) prot0.GetOps(msgid)->op(strX, strZ); \
  _perf_test_end("k=" + to_string(strX.size()))

#define reduce_perf_test(op, times, r, c)     \
  _perf_test_beg(op, times) attr_type attr;   \
  attr["rows"] = to_string(r);                \
  attr["cols"] = to_string(c);                \
  prot0.GetOps(msgid)->op(strX, strZ, &attr); \
  _perf_test_end("r=" + to_string(r) + ",c=" + to_string(c))

#define matmul_perf_test(op, times, m, K, n)        \
  _perf_test_beg(op, times) attr_type attr;         \
  attr["m"] = to_string(m);                         \
  attr["k"] = to_string(K);                         \
  attr["n"] = to_string(n);                         \
  prot0.GetOps(msgid)->op(strX, strY, strZ, &attr); \
  _perf_test_end("m=" + to_string(m) + ",K=" + to_string(K) + ",n=" + to_string(n))

  //////////////////////////////////////////////////////////////////
  vector<double> X, Y, Z;
  vector<string> strX, strY, strZ;
  string msgid("Helix performance test");
  cout << __FUNCTION__ << " " << msgid << endl;
  cout << ssheader.str() << endl;
  /**
   * for convenience,
   * k = r * c = m * K = K * n
   * so, only need set m and K.
   */
  int k, r, c, m, K, n = 1;

  // case 1
  m = K = 1;
  // case 2
  m = 33;
  K = 44;
  // .....
  r = n = m;
  c = K;
  k = r * c;

  random_vector(X, k, -3, 3);
  random_vector(Y, k, -3, 3);
  prot0.GetOps(msgid)->PrivateInput(0, X, strX);
  prot0.GetOps(msgid)->PrivateInput(1, Y, strY);

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

  //binary_perf_test(Pow, 2);
  binary_perf_test(SigmoidCrossEntropy, 2);

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

  reduce_perf_test(Mean, 100, r, c);
  reduce_perf_test(Sum, 1000, r, c);
  reduce_perf_test(AddN, 1000, r, c);
  reduce_perf_test(Max, 2, r, c);
  reduce_perf_test(Min, 2, r, c);

  matmul_perf_test(Matmul, 100, m, K, n);
  cout << ssender.str() << endl;