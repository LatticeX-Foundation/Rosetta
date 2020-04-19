#include "snn.h"
#include <cmath>
using namespace std;

namespace rosetta {
namespace mpc {
namespace debug {

template <typename BinaryOpCls>
void testBinaryOp(string keystr, int thread_nums) {
  output_function();

  vector<double> fa = {3, -1.1, -0.3, 0, 0.11, 0.77, 1.0, 1.002, 2};
  vector<double> fb = {2, 0.11, 0.77, 1.0, -1.1, -0.3, 1.002, 2, 0};
  size_t size = fa.size();

  vector<mpc_t> va, vb, vc(size);
  convert_double_to_mytype(fa, va);
  convert_double_to_mytype(fb, vb);

  auto f = [&](int tid) {
    msg_id_t key(keystr + "Single");
    if (thread_nums > 1) {
      key = msg_id_t(keystr + "Mult" + std::to_string(tid));
    }
    auto op = std::make_shared<BinaryOpCls>(key);
    auto oprec = GetMpcOpWithKey(Reconstruct2PC, op->msg_id());
    cout << __FUNCTION__ << "tid:" << tid << " ==== " << key << ":op.msgid:" << op->msg_id()
         << ",oprec:msgid:" << oprec->msg_id() << endl;

    vector<mpc_t> vc(size);
    op->Run(va, vb, vc, size);
    /*
    test_debug_print(oprec, vc, 16, keystr + " vc <mpc_t, mpc_t> with tid " + to_string(tid));
    op->Run(fa, vb, vc, size);
    test_debug_print(oprec, vc, 16, keystr + " vc <double, mpc_t> with tid " + to_string(tid));
    op->Run(va, fb, vc, size);
    test_debug_print(oprec, vc, 16, keystr + " vc <mpc_t, double> with tid " + to_string(tid));
    */
  };

  vector<thread> threads(thread_nums);
  for (int i = 0; i < threads.size(); i++) {
    threads[i] = thread(f, i);
  }
  for (int i = 0; i < threads.size(); i++) {
    threads[i].join();
  }
}

void testBinaryOps(int thread_nums) {
  using namespace rosetta::mpc;

  testBinaryOp<Add>("Add", thread_nums);
  testBinaryOp<Sub>("Sub", thread_nums);
  testBinaryOp<DotProduct>("DotProduct", thread_nums);
  testBinaryOp<Mul>("Mul", thread_nums);
  //testBinaryOp<Div>("Div", thread_nums);
  //testBinaryOp<Truediv>("Truediv", thread_nums);
} // namespace mpc

// TODO: more generally
// for testing mixed usage of shared value and real float value
void debug_Mix_OP() {
  auto op_true_div = GetMpcOpDefault(Truediv);
  auto op_rcs = GetMpcOpWithKey(Reconstruct2PC, op_true_div->msg_id());

  vector<mpc_t> shared_side = {FloatToMpcType(0.5), FloatToMpcType(1.0)};
  vector<double> plain_side = {1.0, 0.5};

  vector<mpc_t> right_side_expected = {FloatToMpcType(0.5), FloatToMpcType(2.0)};
  vector<mpc_t> left_side_expected = {FloatToMpcType(0.5), FloatToMpcType(0.125)};

  vector<mpc_t> result(shared_side.size(), 0);
  op_true_div->Run(shared_side, plain_side, result, shared_side.size());
  if (PRIMARY) {
    op_rcs->Run(shared_side, shared_side.size(), "TrueDiv Left shared side");
    op_rcs->Run(result, result.size(), "TrueDiv result ");
    op_rcs->Run(right_side_expected, right_side_expected.size(), "EXpected result");
  }

  op_true_div->Run(plain_side, shared_side, result, shared_side.size());
  if (PRIMARY) {
    op_rcs->Run(shared_side, shared_side.size(), "TrueDiv Right shared side");
    op_rcs->Run(result, result.size(), "TrueDiv result ");
    op_rcs->Run(left_side_expected, left_side_expected.size(), "EXpected result");
  }

  vector<mpc_t> debug_a = {FloatToMpcType(3), FloatToMpcType(-3), FloatToMpcType(3),
                           FloatToMpcType(-3)};
  vector<mpc_t> debug_b = {FloatToMpcType(2), FloatToMpcType(-2), FloatToMpcType(-2),
                           FloatToMpcType(2)};
  vector<mpc_t> res_a_div_b(4, 0);
  auto op_floor_div = GetMpcOpDefault(Div);
  op_rcs = GetMpcOpWithKey(Reconstruct2PC, op_floor_div->msg_id());
  op_floor_div->Run(debug_a, debug_b, res_a_div_b, res_a_div_b.size());
  if (PRIMARY) {
    op_rcs->Run(res_a_div_b, res_a_div_b.size(), "Floor Div debug");
  }
}

void testMulThreads(int thread_nums) {
  using namespace rosetta::mpc;

  testBinaryOp<Mul>("TMul", thread_nums);
}

} // namespace debug
} // namespace mpc
} // namespace rosetta
