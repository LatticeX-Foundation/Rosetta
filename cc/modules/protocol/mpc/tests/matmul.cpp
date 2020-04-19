#include "snn.h"

namespace rosetta {
namespace mpc {
namespace debug {

void debugMatMul() {
  auto op = GetMpcOpDefault(MatMul);
  auto oprec = GetMpcOpWithKey(Reconstruct2PC, op->msg_id());

  size_t rows = 2;
  size_t common_dim = 3;
  size_t columns = 4;
  size_t transpose_a = 0, transpose_b = 0;

  vector<mpc_t> a(rows * common_dim);
  vector<mpc_t> b(common_dim * columns);
  vector<mpc_t> c(rows * columns);

  for (size_t i = 0; i < a.size(); ++i)
    a[i] = FloatToMpcType(i + 1);

  for (size_t i = 0; i < b.size(); ++i)
    b[i] = FloatToMpcType(i + 2);

  if (PRIMARY)
    oprec->Run(a, a.size(), "a");

  if (PRIMARY)
    oprec->Run(b, b.size(), "b");

  op->Run(a, b, c, rows, common_dim, columns, transpose_a, transpose_b);
  if (PRIMARY)
    oprec->Run(c, c.size(), "c");
}

void debugMatMul_tp() {
  auto op = GetMpcOpDefault(MatMul);
  auto oprec = GetMpcOpWithKey(Reconstruct2PC, op->msg_id());

  //---------------------------------------
  //transpose_a = 1 & transpose_b = 0
  //---------------------------------------
  size_t rows = 4;
  size_t common_dim = 2;
  size_t columns = 1;
  size_t transpose_a = 1, transpose_b = 0;

  vector<mpc_t> a(rows * common_dim);
  vector<mpc_t> b(common_dim * columns);
  vector<mpc_t> c(rows * columns);

  for (size_t i = 0; i < common_dim; ++i)
    for (size_t j = 0; j < rows; ++j)
      a[i * rows + j] = FloatToMpcType(i + j + 1);

  for (size_t i = 0; i < b.size(); ++i)
    b[i] = FloatToMpcType(i + 1);

  if (PRIMARY)
    oprec->Run(a, a.size(), "a");

  if (PRIMARY)
    oprec->Run(b, b.size(), "b");

  op->Run(a, b, c, rows, common_dim, columns, transpose_a, transpose_b);

  if (PRIMARY)
    oprec->Run(c, c.size(), "c");

  //----------------------------------------
  //transpose_a = 0 & transpose_b = 1
  //----------------------------------------
  rows = 2;
  common_dim = 2;
  columns = 3;
  transpose_a = 0, transpose_b = 1;
  a.resize(rows * common_dim);
  b.resize(common_dim * columns);
  c.resize(rows * columns);

  for (size_t i = 0; i < rows * common_dim; ++i)
    a[i] = FloatToMpcType(i + 1);

  b[0] = FloatToMpcType(1);
  b[1] = FloatToMpcType(4);
  b[2] = FloatToMpcType(2);
  b[3] = FloatToMpcType(5);
  b[4] = FloatToMpcType(3);
  b[5] = FloatToMpcType(6);

  if (PRIMARY)
    oprec->Run(a, a.size(), "a");

  if (PRIMARY)
    oprec->Run(b, b.size(), "b");

  op->Run(a, b, c, rows, common_dim, columns, transpose_a, transpose_b);

  if (PRIMARY)
    oprec->Run(c, c.size(), "c");

  //----------------------------------------
  //transpose_a = 1 & transpose_b = 1
  //----------------------------------------
  rows = 4;
  common_dim = 2;
  columns = 2;
  transpose_a = 1, transpose_b = 1;
  a.resize(rows * common_dim);
  b.resize(common_dim * columns);
  c.resize(rows * columns);

  for (size_t i = 0; i < common_dim; ++i)
    for (size_t j = 0; j < rows; ++j)
      a[i * rows + j] = FloatToMpcType(i + j + 1);

  b[0] = FloatToMpcType(1);
  b[1] = FloatToMpcType(3);
  b[2] = FloatToMpcType(2);
  b[3] = FloatToMpcType(4);

  if (PRIMARY)
    oprec->Run(a, a.size(), "a");

  if (PRIMARY)
    oprec->Run(b, b.size(), "b");

  op->Run(a, b, c, rows, common_dim, columns, transpose_a, transpose_b);

  if (PRIMARY)
    oprec->Run(c, c.size(), "c");
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
void testMatMul(size_t rows, size_t common_dim, size_t columns, size_t iter) {
  auto op = GetMpcOpDefault(MatMul);
  auto oprec = GetMpcOpWithKey(Reconstruct2PC, op->msg_id());

  vector<mpc_t> a(rows * common_dim, 1 * (1 << FLOAT_PRECISION));
  vector<mpc_t> b(common_dim * columns, 1 * (1 << FLOAT_PRECISION));
  vector<mpc_t> c(rows * columns);

  if (STANDALONE) {
    for (int runs = 0; runs < iter; ++runs) {
      matrixMultEigen(a, b, c, rows, common_dim, columns, 0, 0);
      dividePlainSA(c, (1 << FLOAT_PRECISION));
    }
  }

  if (MPC) {
    for (int runs = 0; runs < iter; ++runs)
      op->Run(a, b, c, rows, common_dim, columns, 0, 0);
  }

  if (PRIMARY)
    oprec->Run(c, DEBUG_CONST, "testMatMul");
}

void testConvolution(size_t iw, size_t ih, size_t fw, size_t fh, size_t C, size_t D, size_t iter) {
  size_t sx = 1, sy = 1, B = 32;
  vector<mpc_t> w(fw * fh * C * D, 0);
  vector<mpc_t> act(iw * ih * C * B, 0);
  size_t p_range = (ih - fh + 1);
  size_t q_range = (iw - fw + 1);
  size_t size_rw = fw * fh * C * D;
  size_t rows_rw = fw * fh * C;
  size_t columns_rw = D;

  for (int runs = 0; runs < iter; ++runs) {
    //Reshape weights
    vector<mpc_t> reshapedWeights(size_rw, 0);
    for (int i = 0; i < size_rw; ++i)
      reshapedWeights[(i % rows_rw) * columns_rw + (i / rows_rw)] = w[i];

    //reshape activations
    size_t size_convo = (p_range * q_range * B) * (fw * fh * C);
    vector<mpc_t> convShaped(size_convo, 0);
    convolutionReshape(act, convShaped, iw, ih, C, B, fw, fh, 1, 1);

    //Convolution multiplication
    vector<mpc_t> convOutput(p_range * q_range * B * D, 0);
    if (STANDALONE) {
      matrixMultEigen(
        convShaped, reshapedWeights, convOutput, (p_range * q_range * B), (fw * fh * C), D, 0, 0);
      dividePlainSA(convOutput, (1 << FLOAT_PRECISION));
    }

    if (MPC) {
      auto op = GetMpcOpDefault(MatMul);
      op->Run(
        convShaped, reshapedWeights, convOutput, (p_range * q_range * B), (fw * fh * C), D, 0, 0);
    }
  }
}

void testMatMul() {
  size_t iterations = 10;
  testMatMul(1, 1, 1, 1);
  testMatMul(2, 2, 2, 1);
  testMatMul(2, 3, 4, 1);
  testMatMul(4, 32, 64, 1);
  testMatMul(256, 64, 10, iterations);
}

void testConvolution() {
  size_t iterations = 10;
  testConvolution(28, 28, 5, 5, 1, 20, iterations);
  testConvolution(28, 28, 3, 3, 1, 20, iterations);
  testConvolution(8, 8, 5, 5, 16, 50, iterations);
}

void testMatMulThreads(int thread_nums) {
  auto f = [&](int tid) {
    msg_id_t key("MatMulSingle");
    if (thread_nums > 1) {
      key = msg_id_t("MatMulMulti" + std::to_string(tid));
    }
    auto op = std::make_shared<MatMul>(key);
    auto oprec = GetMpcOpWithKey(Reconstruct2PC, op->msg_id());
    cout << __FUNCTION__ << "tid:" << tid << " ==== " << key << ":op.msgid:" << op->msg_id()
         << ",oprec:msgid:" << oprec->msg_id() << endl;

    size_t rows = 20;
    size_t common_dim = 30;
    size_t columns = 40;
    size_t transpose_a = 0, transpose_b = 0;

    vector<mpc_t> a(rows * common_dim);
    vector<mpc_t> b(common_dim * columns);
    vector<mpc_t> c(rows * columns);

    for (size_t i = 0; i < a.size(); ++i)
      a[i] = FloatToMpcType(i + 1);

    for (size_t i = 0; i < b.size(); ++i)
      b[i] = FloatToMpcType(i + 2);

    // if (PRIMARY) {
    //   oprec->Run(a, a.size() > 20 ? 20 : a.size(), key.str() + " a");
    //   oprec->Run(b, b.size() > 20 ? 20 : b.size(), key.str() + " b");
    // }
    op->Run(a, b, c, rows, common_dim, columns, transpose_a, transpose_b);
    //if (PRIMARY) {
    //   oprec->Run(c, c.size() > 20 ? 20 : c.size(), key.str() + " c");
    //}
  };

  vector<thread> threads(thread_nums);
  for (int i = 0; i < threads.size(); i++) {
    threads[i] = thread(f, i);
  }
  for (int i = 0; i < threads.size(); i++) {
    threads[i].join();
  }
}
} // namespace debug
} // namespace mpc
} // namespace rosetta
