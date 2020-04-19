#include "snn.h"
#include <cmath>

namespace rosetta {
namespace mpc {
namespace debug {

void debug_NN() {
  LOGI("debug_NN begins!");
  auto op_sce = GetMpcOpDefault(SigmoidCrossEntropy);
  auto op_rcs = GetMpcOpWithKey(Reconstruct2PC, op_sce->msg_id());
  vector<mpc_t> test_logits_inputs = {
    FloatToMpcType(-1.5 / 2),   FloatToMpcType(-0.01 / 2), FloatToMpcType(-0.001 / 2),
    FloatToMpcType(-0.00013),   FloatToMpcType(-7.1 / 2),  FloatToMpcType(-10 / 2),
    FloatToMpcType(-10000 / 2), FloatToMpcType(1.5 / 2),   FloatToMpcType(0.01 / 2),
    FloatToMpcType(0.001 / 2),  FloatToMpcType(0.00013),   FloatToMpcType(5.0 / 2),
    FloatToMpcType(7.1 / 2),    FloatToMpcType(10 / 2),    FloatToMpcType(10000 / 2)};

  vector<mpc_t> test_labels_inputs_pos(test_logits_inputs.size(), FloatToMpcType(1.0 / 2));

  vector<mpc_t> test_labels_inputs_neg(test_logits_inputs.size(), FloatToMpcType(-1.0 / 2));

  vector<mpc_t> test_labels_inputs_zero(test_logits_inputs.size(), FloatToMpcType(0));

  vector<mpc_t> result(test_logits_inputs.size(), 0);
  op_sce->sigmoid_cross_entropy(
    test_logits_inputs, test_labels_inputs_pos, result, test_logits_inputs.size());

  if (PRIMARY) {
    op_rcs->Run(test_logits_inputs, test_logits_inputs.size(), "CrossEntropy logits inputs");
    op_rcs->Run(
      test_labels_inputs_pos, test_labels_inputs_pos.size(), "CrossEntropy labels inputs");
    op_rcs->Run(result, result.size(), "CrossEntropy outputs ");
  }
  cout << "expected:" << endl;
  for (int i = 0; i < test_logits_inputs.size(); ++i) {
    float curr_x = MpcTypeToFloat(test_logits_inputs[i]) * 2;
    float curr_z = MpcTypeToFloat(test_labels_inputs_pos[i]) * 2;
    float curr_res = 0.0;
    if (curr_x > 0.0) {
      curr_res = curr_x;
    }
    curr_res = curr_res - curr_x * curr_z;
    curr_z = log(1 + exp(-fabs(curr_x)));
    curr_res = curr_res + curr_z;
    cout << " " << curr_res << "  ";
  }
  cout << endl;
}
} // namespace debug
} // namespace mpc
} // namespace rosetta