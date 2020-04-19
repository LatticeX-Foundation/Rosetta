
#include "snn.h"
// clang-format off


#define Comp(x, y) (((x) > (y)) ? 1 : 0)

#define Comp_mytype(x, y) (((double)((int64_t)x) >= (double)((int64_t)y)) ? FloatToMpcType(1) : 0)
#define TestMulMytype(x, y) (dividePlainSA((x) * (y), 1 << FLOAT_PRECISION))

extern int partyNum;

static mpc_t plain_sigmoid_func_mytype(mpc_t x)
{
  return (TestMulMytype(Comp_mytype(x, FloatToMpcType(-4)),
                        (FloatToMpcType(0.5) + TestMulMytype(FloatToMpcType(0.15012), x) - TestMulMytype(TestMulMytype(FloatToMpcType(0.00159301), x), TestMulMytype(x, x)))) +
          TestMulMytype(Comp_mytype(x, FloatToMpcType(4)),
                        (FloatToMpcType(0.5) - TestMulMytype(FloatToMpcType(0.15012), x) + TestMulMytype(TestMulMytype(FloatToMpcType(0.00159301), x), TestMulMytype(x, x)))));
}

static void test_plain_sigmoid_g3x(const vector<mpc_t> &in, vector<mpc_t> &plain_sigmoid)
{
  plain_sigmoid.resize(in.size());
  for (size_t i = 0; i < in.size(); ++i)
  {
    plain_sigmoid[i] = plain_sigmoid_func_mytype(in[i]);
  }
}

static vector<mpc_t> get_fixs(double start, double end, int piece_count, vector<double> &dout, int fix_v = -1)
{
  vector<mpc_t> rands(piece_count);
  dout.resize(piece_count);
  double unit = (end - start) / piece_count;
  for (int i = 0; i < piece_count; i++)
  {
    rands[i] = (fix_v == -1) ? FloatToMpcType(start + unit * i) : FloatToMpcType(fix_v);
    dout[i] = (fix_v == -1) ? 2 * (start + unit * i) : 2 * (fix_v);
    //cout << "input: " << (start + unit*i)*2 /*<< " : " << MpcTypeToFloat(rands[i]) */<< endl;
  }

  return rands;
}

static void test_plain_sigmoid_g3x_double(const vector<double> &in, vector<double> &plain_sigmoid)
{
  plain_sigmoid.resize(in.size());
  auto plain_sigmoid_func_d = ([=](double x) { return Comp(x, -4) * (0.5 + 0.15012 * x - 0.00159301 * x * x * x) +
                                                      Comp(x, 4) * (0.5 - 0.15012 * x + 0.00159301 * x * x * x); });
  for (size_t i = 0; i < in.size(); ++i)
  {
    plain_sigmoid[i] = plain_sigmoid_func_d(in[i]);
  }
}

#define Cmp(x, y) (x >= y ? 1 : 0)
double static test_plain_standard_sigmoid_piece(double x)
{
  //(0.0484792, 0.1998976),  (0.1928931, 0.4761351), (0.1928931, 0.5238649),  (0.0484792, 0.8001024)
  double a1 = 0.0484792;
  double b1 = 0.1998976;
  double a2 = 0.1928931;
  double b2 = 0.4761351;
  double a3 = 0.1928931;
  double b3 = 0.5238649;
  double a4 = 0.0484792;
  double b4 = 0.8001024;

  double value = -10;
  //branch
  if (!Cmp(x, -4))
  {
    return 0;
  }
  else if (Cmp(x, -4) && !Cmp(x, -2))
  {
    return a1 * x + b1;
  }
  else if (Cmp(x, -2) && !Cmp(x, 0))
  {
    return a2 * x + b2;
  }
  else if (Cmp(x, 0) >= 0 && !Cmp(x, 2))
  {
    return a3 * x + b3;
  }
  else if (Cmp(x, 2) && !Cmp(x, 4))
  {
    return a4 * x + b4;
  }
  else
    return 1;
}

static double test_plain_tie_sigmoid_piece(double x)
{
  //(0.0484792, 0.1998976),  (0.1928931, 0.4761351), (0.1928931, 0.5238649),  (0.0484792, 0.8001024)
  double a1 = 0.0484792;
  double b1 = 0.1998976;
  double a2 = 0.1928931;
  double b2 = 0.4761351;
  double a3 = 0.1928931;
  double b3 = 0.5238649;
  double a4 = 0.0484792;
  double b4 = 0.8001024;

  //double value = -10;
  //branch

  double result = (Cmp(-4, x) * (0 - (a1 * x + b1))) + (Cmp(-2, x) * ((a1 * x + b1) - (a2 * x + b2))) + (Cmp(0, x) * ((a2 * x + b2) - (a3 * x + b3))) + (Cmp(2, x) * ((a3 * x + b3) - (a4 * x + b4))) + (Cmp(4, x) * ((a4 * x + b4) - 1)) + 1;

  double standard_piece_sig = test_plain_standard_sigmoid_piece(x);
  double ideal_sigmoid = 1.0 / (1.0 + exp(-x));
  //printf("approx-sigmoid(%lf)=%lf, branch-sigmoid: %lf, ideal-sigmoid:  %lf\n", x, result, standard_piece_sig, ideal_sigmoid);

  return result;
}

static void test_plain_piece_sigmoid_double(const vector<mpc_t> &a, vector<mpc_t> &b)
{
  for (size_t i = 0; i < a.size(); i++)
  {
    b[i] = FloatToMpcType(test_plain_tie_sigmoid_piece(MpcTypeToFloat(a[i])));
  }
}

void static test_plain_sigmoid_piece_wise(const vector<mpc_t> &a, vector<mpc_t> &b)
{
  //origin sigmoid(x)=1/(1+exp(-x))
  vector<mpc_t> y = a;

  //(0.0484792, 0.1998976),  (0.1928931, 0.4761351), (0.1928931, 0.5238649),  (0.0484792, 0.8001024)
  mpc_t a1 = FloatToMpcType(0.0484792);
  mpc_t b1 = FloatToMpcType(0.1998976);
  mpc_t a2 = FloatToMpcType(0.1928931);
  mpc_t b2 = FloatToMpcType(0.4761351);
  mpc_t a3 = FloatToMpcType(0.1928931);
  mpc_t b3 = FloatToMpcType(0.5238649);
  mpc_t a4 = FloatToMpcType(0.0484792);
  mpc_t b4 = FloatToMpcType(0.8001024);

  //[-4,4]: (0.0484792, 0.1998976),  (0.1928931, 0.4761351), (0.1928931, 0.5238649),  (0.0484792, 0.8001024)
  mpc_t y1 = FloatToMpcType(-4);
  mpc_t y2 = FloatToMpcType(-2);
  mpc_t y3 = FloatToMpcType(0);
  mpc_t y4 = FloatToMpcType(2);
  mpc_t y5 = FloatToMpcType(4);
  mpc_t lastOne = FloatToMpcType(1);

  for (size_t i = 0; i < a.size(); i++)
  {
    b[i] = (Comp_mytype(y1, y[i]) * (0 - (TestMulMytype(a1, y[i]) + b1))) + (Comp_mytype(y2, y[i]) * ((TestMulMytype(a1, y[i]) + b1) - (TestMulMytype(a2, y[i]) + b2))) + (Comp_mytype(y3, y[i]) * ((TestMulMytype(a2, y[i]) + b2) - (TestMulMytype(a3, y[i]) + b3))) + (Comp_mytype(y4, y[i]) * ((TestMulMytype(a3, y[i]) + b3) - (TestMulMytype(a4, y[i]) + b4))) + (Comp_mytype(y5, y[i]) * ((TestMulMytype(a4, y[i]) + b4) - lastOne));

    b[i] = dividePlainSA(b[i], (1 << FLOAT_PRECISION));
    b[i] += lastOne;
  }
}

void test_plain_ideal_sigmoid(const vector<mpc_t> &in, vector<mpc_t> &plain_sigmoid)
{
  plain_sigmoid.resize(in.size());

  for (size_t i = 0; i < in.size(); ++i)
  {
    double x = MpcTypeToFloat(in[i]);
    x = 1.0 / (1.0 + exp(-x));
    plain_sigmoid[i] = FloatToMpcType(x);
  }
}

void test_plain_ideal_sigmoid_double(const vector<double> &in, vector<double> &plain_sigmoid)
{
  plain_sigmoid.resize(in.size());

  for (size_t i = 0; i < in.size(); ++i)
  {
    plain_sigmoid[i] = (1.0 / (1.0 + exp(-in[i])));
    cout << "sigmoid(" << in[i] << ") = " << plain_sigmoid[i] << endl;
  }
}


namespace rosetta{ namespace mpc {namespace debug { 

void testSigmoidPieceWise2()
{
  cout << "testSigmoidPieceWise2..." << endl;
  size_t size = 128;

  size = 50;
  vector<double> da;
  vector<mpc_t> a = get_fixs(-5, 5, size, da);

  vector<mpc_t> public_a(size);
  std::transform(a.begin(), a.end(), public_a.begin(), [](mpc_t v) { return (2 * v); });
  vector<mpc_t> plain_sigmoid(size);
  test_plain_sigmoid_piece_wise(public_a, plain_sigmoid);

  vector<mpc_t> double_piece_out(size);
  test_plain_piece_sigmoid_double(public_a, double_piece_out);

  vector<mpc_t> ideal_out(size);
  test_plain_ideal_sigmoid(public_a, ideal_out);

  vector<double> ideal_out_double(size);
  test_plain_ideal_sigmoid_double(da, ideal_out_double);

  vector<mpc_t> mpc_sigmoid(size);
  debugSigmoidPieceWise(a, mpc_sigmoid);

  vector<mpc_t> ali_sigmoid(size);
  debugSigmoidAliPieceWise(a, ali_sigmoid);

  cout << "--------------sigmoid_picee--------------    " << endl;
  cout << "input\t"
       << "juzix\t"
       << "ali\t"
       << "sigmoid\t" << endl;
  for (size_t i = 0; i < size; i++)
  {
    cout << da[i] /*MpcTypeToFloat(public_a[i])*/ << "\t"
         << MpcTypeToFloat(mpc_sigmoid[i]) << "\t"
         << MpcTypeToFloat(ali_sigmoid[i]) << "\t"
         //  << "plain_sigmoid: \t" << MpcTypeToFloat(plain_sigmoid[i]) << "\t"
         //  << "double_piece: \t" << MpcTypeToFloat(double_piece_out[i]) << "\t"
         << ideal_out_double[i] << "\t"
         << endl;
  }
  cout << endl;

  cout << "test_mpc_sigmoid_piece ok." << endl;
}

}}}

// clang-format on
