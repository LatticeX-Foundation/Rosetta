
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  Logger::Get().log_to_stdout(true);

  stringstream ssheader;
  // clang-format off
      ssheader << "Notes:" << endl;
      ssheader << "    OP: operator name. eg. 0.1.OpName: 0 means variable, 1 means constant." << endl;
      ssheader << "    avg-elap.(ms): elapsed/times " << endl;
      ssheader << "    shape size: k = r * c = m * K = K * n" << endl;
      ssheader << "+-------------------------+------------+---------------+---------------+----------------------+------------+------------+------------+------------+" << endl;
      ssheader << "|" << setw(25) << "OP "         << "|" << setw(12) << "times "
               << "|" << setw(15) << "elapsed(s) " << "|" << setw(15) << "average(ms) " << "|" << setw(22) << "shape size "
               << "|" << setw(12) << "sent data "  << "|" << setw(12) << "recv data "
               << "|" << setw(12) << "sent msgs "  << "|" << setw(12) << "recv msgs "
               << "|" << endl;
      ssheader << "+-------------------------+------------+---------------+---------------+----------------------+------------+------------+------------+------------+";
  // clang-format on
  stringstream ssender;
  // clang-format off
      ssender  << "+-------------------------+------------+---------------+---------------+----------------------+------------+------------+------------+------------+";
  // clang-format on

#define _perf_test_beg() do {
#define _perf_test_for(op, times)                                    \
  auto ntimes = times;                                               \
  stringstream ss;                                                   \
  ss.precision(11);                                                  \
  ss << "|" << setw(24) << string(#op) << " |" << setw(11) << times; \
  SimpleTimer timer;                                                 \
  for (int i = 0; i < times; i++) {
#define _perf_test_end(shape)                                             \
  }                                                                       \
  timer.stop();                                                           \
  ss << " |" << setw(14) << timer.elapse() << " |" << setw(14)            \
     << timer.us_elapse() / 1000.0 / ntimes << " |" << setw(21) << shape; \
  ss << " |";                                                             \
  cout << ss.str() << endl;                                               \
  }                                                                       \
  while (0)

#define binary_perf_test(lh, rh, sX, sY, op, times)                      \
  _perf_test_beg() attr_type attr;                                       \
  attr["lh_is_const"] = to_string(lh);                                   \
  attr["rh_is_const"] = to_string(rh);                                   \
  _perf_test_for(lh.rh.op, times) prot0.GetOps(msgid)->op(sX, sY, strZ); \
  _perf_test_end("k=" + to_string(sX.size()) + ",k=" + to_string(sY.size()))

#define unary_perf_test(op, times)                                      \
  _perf_test_beg() attr_type attr;                                      \
  _perf_test_for(op, times) prot0.GetOps(msgid)->op(strX, strZ, &attr); \
  _perf_test_end("k=" + to_string(strX.size()))

#define reduce_perf_test(op, times, r, c)                               \
  _perf_test_beg() attr_type attr;                                      \
  attr["rows"] = to_string(r);                                          \
  attr["cols"] = to_string(c);                                          \
  _perf_test_for(op, times) prot0.GetOps(msgid)->op(strX, strZ, &attr); \
  _perf_test_end("r=" + to_string(r) + ",c=" + to_string(c))

#define matmul_perf_test(op, times, m, K, n)                                  \
  _perf_test_beg() attr_type attr;                                            \
  attr["m"] = to_string(m);                                                   \
  attr["k"] = to_string(K);                                                   \
  attr["n"] = to_string(n);                                                   \
  _perf_test_for(op, times) prot0.GetOps(msgid)->op(strX, strY, strZ, &attr); \
  _perf_test_end("m=" + to_string(m) + ",K=" + to_string(K) + ",n=" + to_string(n))

  //////////////////////////////////////////////////////////////////
  vector<double> X, Y, Z; // variables
  vector<double> CX, CY; // constants
  vector<string> strX, strY, strZ;
  vector<string> strCX, strCY, positiveCY;
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

  // initialize
  random_vector(X, k, -3, 3);
  random_vector(Y, k, -3, 3);
  {
    CX.resize(k);
    CY.resize(k);
    strCX.resize(k);
    strCY.resize(k);
    positiveCY.resize(k);
    srand(1);
    for (int i = 0; i < k; i++) {
      CX[i] = (rand() % 300) / 100.0 + 3.0;
      strCX[i] = std::to_string(CX[i]);
    }
    srand(11);
    for (int i = 0; i < k; i++) {
      CY[i] = (rand() & 600) / 200.0 + 3.0;
      strCY[i] = std::to_string(CY[i]);
      positiveCY[i] = std::to_string(abs(CY[i]));
    }
  }
  prot0.GetOps(msgid)->PrivateInput(0, X, strX);
  prot0.GetOps(msgid)->PrivateInput(1, Y, strY);

  SimpleTimer all_tests_timer;

  // variable vs variable
  binary_perf_test(0, 0, strX, strY, Add, 23456);
  binary_perf_test(0, 0, strX, strY, Sub, 23456);
  binary_perf_test(0, 0, strX, strY, Mul, 3456);
  binary_perf_test(0, 0, strX, strY, Truediv, 3);
  binary_perf_test(0, 0, strX, strY, Floordiv, 4); ////////////////////
  binary_perf_test(0, 0, strX, strY, Less, 123);
  binary_perf_test(0, 0, strX, strY, LessEqual, 111);
  binary_perf_test(0, 0, strX, strY, Equal, 31);
  binary_perf_test(0, 0, strX, strY, NotEqual, 33);
  binary_perf_test(0, 0, strX, strY, Greater, 35);
  binary_perf_test(0, 0, strX, strY, GreaterEqual, 37); ////////////////////
  binary_perf_test(0, 0, strX, strY, SigmoidCrossEntropy, 13);
  // constant vs variable
  binary_perf_test(1, 0, strCX, strY, Add, 23456);
  binary_perf_test(1, 0, strCX, strY, Sub, 23456);
  binary_perf_test(1, 0, strCX, strY, Mul, 3456);
  binary_perf_test(1, 0, strCX, strY, Truediv, 3);
  binary_perf_test(1, 0, strCX, strY, Floordiv, 4); ////////////////////
  binary_perf_test(1, 0, strCX, strY, Less, 123);
  binary_perf_test(1, 0, strCX, strY, LessEqual, 111);
  binary_perf_test(1, 0, strCX, strY, Equal, 31);
  binary_perf_test(1, 0, strCX, strY, NotEqual, 33);
  binary_perf_test(1, 0, strCX, strY, Greater, 35);
  binary_perf_test(1, 0, strCX, strY, GreaterEqual, 37); ////////////////////
  binary_perf_test(1, 0, strCX, strY, SigmoidCrossEntropy, 13);
  // variable vs constant
  binary_perf_test(0, 1, strX, strCY, Add, 23456);
  binary_perf_test(0, 1, strX, strCY, Sub, 23456);
  binary_perf_test(0, 1, strX, strCY, Mul, 3456);
  binary_perf_test(0, 1, strX, strCY, Truediv, 3);
  binary_perf_test(0, 1, strX, strCY, Floordiv, 4); ////////////////////
  binary_perf_test(0, 1, strX, strCY, Less, 123);
  binary_perf_test(0, 1, strX, strCY, LessEqual, 111);
  binary_perf_test(0, 1, strX, strCY, Equal, 31);
  binary_perf_test(0, 1, strX, strCY, NotEqual, 33);
  binary_perf_test(0, 1, strX, strCY, Greater, 35);
  binary_perf_test(0, 1, strX, strCY, GreaterEqual, 37); ////////////////////
  binary_perf_test(0, 1, strX, strCY, SigmoidCrossEntropy, 13);
  binary_perf_test(0, 1, strX, positiveCY, Pow, 11);

  unary_perf_test(Square, 3210);
  unary_perf_test(Negative, 23456);
  unary_perf_test(Abs, 63);
  unary_perf_test(AbsPrime, 75);
  unary_perf_test(Log, 81);
  unary_perf_test(Log1p, 47);
  unary_perf_test(HLog, 3);
  unary_perf_test(Relu, 123);
  unary_perf_test(ReluPrime, 132);
  unary_perf_test(Sigmoid, 30);

  reduce_perf_test(Mean, 12345, r, c);
  reduce_perf_test(Sum, 12345, r, c);
  reduce_perf_test(AddN, 23456, r, c);
  reduce_perf_test(Max, 31, r, c);
  reduce_perf_test(Min, 31, r, c);

  matmul_perf_test(Matmul, 1234, m, K, n);
  cout << ssender.str() << endl;
  cout << "Total elapsed(s): " << all_tests_timer.elapse() << endl;
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////