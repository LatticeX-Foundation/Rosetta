#include "snn.h"

namespace rosetta {
namespace mpc {
namespace debug {

void debug_LOG() {
  LOGI("debug_LOG begins!");
  //cout << FloatToMpcType(-3) <<endl;
  //cout << "i" << FloatToMpcType(-4.2) <<endl;
  auto op_log = GetMpcOpDefault(Log);
  auto op_rcs = GetMpcOpWithKey(Reconstruct2PC, op_log->msg_id());
  cout << "V1:" << endl;
  //mpc_t input = FloatToMpcType(2.71828/2);
  mpc_t input = FloatToMpcType(1.5 / 2);
  mpc_t output = 0;
  op_log->mpc_log_v1(input, output);

  if (PRIMARY) {
    vector<mpc_t> result(1, output);
    cout << "LOG(" << MpcTypeToFloat(input * 2) << "):" << endl;
    op_rcs->Run(result, 1, "V1 Log result ");
  }
  cout << "V2:" << endl;
  op_log->mpc_log_v2(input, output);
  if (PRIMARY) {
    cout << "LOG(" << MpcTypeToFloat(input * 2) << "):" << endl;
    vector<mpc_t> result(1, output);
    op_rcs->Run(result, 1, "V2 Log result ");
  }
  input = FloatToMpcType(2.71828 / 2);
  op_log->mpc_log_v2(input, output);
  if (PRIMARY) {
    cout << "LOG(" << MpcTypeToFloat(input * 2) << "):" << endl;
    vector<mpc_t> result(1, output);
    op_rcs->Run(result, 1, "V2 Log result ");
  }

  // vector<mpc_t> inputs_common(10, 0);
  // vector<mpc_t> tfe_outputs(10, 0);
  // for (int i = 0; i < 10 + 1; ++i) {
  // 	inputs_common[i] = FloatToMpcType( (0.2 + i * (1.5-0.2)/10) / 2);
  // 	op_log->mpc_log_v1(inputs_common[i], tfe_outputs[i]);
  // }

  // vector<mpc_t> hd_outputs(10, 0);
  // op_log->mpc_log_hd(inputs_common, hd_outputs, inputs_common.size());
  // if(PRIMARY) {
  // 	op_rcs->Run(inputs_common, inputs_common.size(), "Inputs");
  // 	op_rcs->Run(tfe_outputs, tfe_outputs.size(), "TFE Log result ");
  // 	op_rcs->Run(hd_outputs, tfe_outputs.size(), "HD Log result ");
  // }

  // cout << "expected:" << endl;
  // for(int i = 0; i < inputs_common.size(); ++i) {
  // 	float curr = log(MpcTypeToFloat(inputs_common[i]) * 2);
  // 	cout << " " << curr << "  ";
  // }
  // cout << endl;

  long long ai = 500000;
  auto aj = ai * 10;
  vector<mpc_t> inputs = {
    FloatToMpcType(1.5 / 2),    FloatToMpcType(0.01 / 2), FloatToMpcType(0.001 / 2),
    FloatToMpcType(0.00013),    FloatToMpcType(10 / 2),   FloatToMpcType(10000 / 2),
    FloatToMpcType(100000 / 2), FloatToMpcType(ai),       FloatToMpcType(aj)};
  vector<mpc_t> outputs(inputs.size(), 0);
  op_log->mpc_log_hd(inputs, outputs, inputs.size());
  if (PRIMARY) {
    op_rcs->Run(inputs, inputs.size(), "HD inputs");
    op_rcs->Run(outputs, outputs.size(), "HD Log result ");
  }
  cout << "expected:" << endl;
  for (int i = 0; i < inputs.size(); ++i) {
    float curr = log(MpcTypeToFloat(inputs[i]) * 2);
    cout << " " << curr << "  ";
  }
  cout << endl;
}

} // namespace debug
} // namespace mpc
} // namespace rosetta