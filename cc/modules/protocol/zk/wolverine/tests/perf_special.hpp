#include "wvr_test.h"

#include <iostream>
using namespace std;

#include "cc/modules/protocol/zk/wolverine/include/wolverine_internal.h"

void tempBatchNorm(
  rosetta::WolverineProtocol& WVR0,
  vector<int64_t> dim_size,
  vector<string> input_flat_x,
  vector<string> input_flat_mean,
  vector<string> input_flat_var,
  vector<string> input_flat_scale,
  vector<string> input_flat_offset,
  vector<string> output_x) {
  // dim_size(0,1,2,3)
  //vector<int64_t> dim_size = {1, 2, 2, 32};
  int64_t NumElements = dim_size[0] * dim_size[1] * dim_size[2] * dim_size[3];
  const int64_t depth = dim_size[3];
  const int64_t size = NumElements;
  const int64_t rest_size = size / depth;

  // Todo: these are for training:
  const int rest_size_minus_one = (rest_size > 1) ? (rest_size - 1) : 1;
  double rest_size_inv = static_cast<double>(1.0f / static_cast<double>(rest_size));
  // This adjustment is for Bessel's correction
  double rest_size_adjust =
    static_cast<double>(rest_size) / static_cast<double>(rest_size_minus_one);

  vector<string> inner_flat_x(size);
  vector<string> inner_flat_mean(size);
  vector<string> inner_flat_var(size);
  vector<string> inner_flat_scale(size);
  vector<string> inner_flat_offset(size);

  vector<string> x_flat;
  vector<string> mean_flat;
  vector<string> var_flat;
  vector<string> scale_flat;
  vector<string> offset_flat;

  msg_id_t msgid("performance ....");
  log_info << "msgid:" << msgid;
  auto ops = WVR0.GetOps(msgid);

  vector<string> scaling_factor(depth);
  float epsilon_ = 0.0001;
  {
    // stabilized variance
    vector<string> flat_var(depth);
    vector<string> flat_scale(depth);
    vector<string> flat_epsilon(depth, to_string(epsilon_));
    for (int i = 0; i < depth; ++i) {
      flat_var[i] = input_flat_var[i];
      flat_scale[i] = input_flat_scale[i];
    }

    vector<string> new_var(depth);
    ops->Add(flat_var, flat_epsilon, new_var);
    vector<string> norm_factor(depth);
    ops->Rsqrt(new_var, norm_factor);
    ops->Mul(norm_factor, flat_scale, scaling_factor);
  }

  vector<string> inner_scaling_factor(size);
  for (auto i = 0; i < size; ++i) {
    int d = i % depth;
    inner_flat_x[i] = input_flat_x[i];
    inner_flat_mean[i] = input_flat_mean[d];
    inner_scaling_factor[i] = scaling_factor[d];
    inner_flat_offset[i] = input_flat_offset[d];
  }

  vector<string> x_centered(size);
  ops->Sub(inner_flat_x, inner_flat_mean, x_centered);

  vector<string> x_scaled(size);
  ops->Mul(x_centered, inner_scaling_factor, x_scaled);

  //vector<string> output_x(size);
  ops->Add(x_scaled, inner_flat_offset, output_x);
}

void run(int partyid) {
  WVR_PROTOCOL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  // Also ref python/latticex/rosetta/secure/decorator/test_cases/conv2d_*.py

  msg_id_t msgid("performance ....");
  log_info << "msgid:" << msgid;

  /**
   * run 1000 times, get the average, all input is witness.
   * Fix2Float([1])
   * Float2Fix([1])
   * Sigmoid([1024])
   * Softmax([1024])
   * ReLU([1024])
   * BatchNorm([1,16,16,4])
   * BatchNorm([1,8,8,16])
   * BatchNorm([1,2,2,256])
   * BatchNorm([1,1,1,1024])
   * MatMul([512,512],[512,512])
   * MatMul([2048,2048],[2048,2048])
   */
  rosetta::PerfStats ps0, ps1, ps;

  // MaxPool
  auto test_max = [&](int64_t NX, int inp_rows, int inp_cols) {
    int rows = inp_rows;
    int cols = inp_cols;
    unordered_map<string,string> attrs;
    attrs.insert(std::pair<string, string>("rows", std::to_string(rows)));
    attrs.insert(std::pair<string, string>("cols", std::to_string(cols)));
    int size = rows * cols;

    msg_id_t msgid("performance Max....");
    vector<double> a;
    random_vector(a, size, -2.0, 2.0);
    vector<string> sa(a.size());
    vector<string> sc;
    WVR0.GetOps(msgid)->PrivateInput(ALICE - 1, a, sa);
    ps0 = WVR0.GetPerfStats();
    for (int64_t i = 0; i < NX; i++) {
      WVR0.GetOps(msgid)->Max(sa, sc, &attrs);
    }
    ps = WVR0.GetPerfStats() - ps0;
    double avg = ps.s.elapse / NX;
    cout << "PERFSTATS NX:" << NX << " per MaxPool([" << size << "]):" << setprecision(8) << avg
         << endl;
  };
  test_max(10, 4, 1);

  // Sigmoid
  auto test_sigmoid = [&](int64_t NX, int64_t size) {
    msg_id_t msgid("performance Sigmoid....");
    vector<double> a, b;
    random_vector(a, size, -2.0, 2.0);
    vector<string> sa(a.size());
    vector<string> sc;
    WVR0.GetOps(msgid)->PrivateInput(ALICE - 1, a, sa);
    ps0 = WVR0.GetPerfStats();
    for (int64_t i = 0; i < NX; i++) {
      WVR0.GetOps(msgid)->Sigmoid(sa, sc);
    }
    ps = WVR0.GetPerfStats() - ps0;
    double avg = ps.s.elapse / NX;
    cout << "PERFSTATS NX:" << NX << " per Sigmoid([" << size << "]):" << setprecision(8) << avg
         << endl;
  };
  test_sigmoid(10, 1);
  test_sigmoid(10, 1);

  // Fix2Float
  auto test_fix2float = [&](int64_t NX, int64_t size) {
    int SCALE = 16;
    ps0 = WVR0.GetPerfStats();
    for (int64_t i = 0; i < size; i++) {
      Integer a(62, 12, ALICE);
      Int62ToFloat(a, SCALE);
    }
    ps = WVR0.GetPerfStats() - ps0;
    double avg = ps.s.elapse / NX;
    cout << "PERFSTATS NX:" << NX << " per Int62ToFloat([" << size << "]):" << setprecision(8)
         << avg << endl;
  };
  test_fix2float(10, 1);

  // Float2Fix
  auto test_float2fix = [&](int64_t NX, int64_t size) {
    int SCALE = 16;
    ps0 = WVR0.GetPerfStats();
    for (int64_t i = 0; i < size; i++) {
      Float f1(1.23456789, ALICE);
      FloatToInt62(f1, SCALE);
    }
    ps = WVR0.GetPerfStats() - ps0;
    double avg = ps.s.elapse / NX;
    cout << "PERFSTATS NX:" << NX << " per FloatToInt62([" << size << "]):" << setprecision(8)
         << avg << endl;
  };
  test_float2fix(10, 1);
  sync_zk_bool<BoolIO<NetIO>>();

  // SoftMax
  auto test_softmax = [&](int64_t NX, int64_t batch_size, int64_t num_classes) {
    int64_t size = batch_size * num_classes;
    msg_id_t msgid("performance Softmax....");
    vector<double> a, b;
    random_vector(a, size, -2.0, 2.0);
    vector<string> sa(a.size());
    vector<string> sc;
    WVR0.GetOps(msgid)->PrivateInput(ALICE - 1, a, sa);
    ps0 = WVR0.GetPerfStats();
    attr_type attr;
    attr["rows"] = to_string(batch_size); //batch_size
    attr["cols"] = to_string(num_classes); //num_classes
    for (int64_t i = 0; i < NX; i++) {
      WVR0.GetOps(msgid)->Softmax(sa, sc, &attr);
    }
    ps = WVR0.GetPerfStats() - ps0;
    double avg = ps.s.elapse / NX;
    cout << "PERFSTATS NX:" << NX << " per Softmax([" << size << "]):" << setprecision(8) << avg
         << endl;
  };
  test_softmax(10, 10, 10);

  // ReLU
  auto test_relu = [&](int64_t NX, int64_t size) {
    msg_id_t msgid("performance Relu....");
    vector<double> a, b;
    random_vector(a, size, -2.0, 2.0);
    vector<string> sa(a.size());
    vector<string> sc;
    WVR0.GetOps(msgid)->PrivateInput(ALICE - 1, a, sa);
    ps0 = WVR0.GetPerfStats();
    for (int64_t i = 0; i < NX; i++) {
      WVR0.GetOps(msgid)->Relu(sa, sc);
    }
    ps = WVR0.GetPerfStats() - ps0;
    double avg = ps.s.elapse / NX;
    cout << "PERFSTATS NX:" << NX << " per Relu([" << size << "]):" << setprecision(8) << avg
         << endl;
  };
  test_relu(10, 1);

  // BatchNorm
  auto test_batchnorm = [&](int64_t NX, initializer_list<int64_t> idim_size) {
    if (idim_size.size() != 4)
      return;

    vector<int64_t> dim_size(idim_size);
    int64_t NumElements = dim_size[0] * dim_size[1] * dim_size[2] * dim_size[3];
    const int64_t depth = dim_size[3];
    const int64_t size = NumElements;
    const int64_t rest_size = size / depth;

    msg_id_t msgid("performance BatchNorm....");
    vector<double> a, b, c, d, e;
    random_vector(a, size, -2.0, 2.0);
    random_vector(b, depth, -1.0, 1.0);
    random_vector(c, depth, -1.0, 1.0);
    random_vector(d, depth, -1.0, 1.0);
    random_vector(e, depth, -1.0, 1.0);
    vector<string> sa(a.size());
    vector<string> sb(b.size());
    vector<string> sc(c.size());
    vector<string> sd(d.size());
    vector<string> se(e.size());
    vector<string> sf;
    WVR0.GetOps(msgid)->PrivateInput(ALICE - 1, a, sa);
    WVR0.GetOps(msgid)->PrivateInput(ALICE - 1, b, sb);
    WVR0.GetOps(msgid)->PrivateInput(ALICE - 1, c, sc);
    WVR0.GetOps(msgid)->PrivateInput(ALICE - 1, d, sd);
    WVR0.GetOps(msgid)->PrivateInput(ALICE - 1, e, se);
    vector<string> input_flat_x(sa);
    vector<string> input_flat_mean(sb);
    vector<string> input_flat_var(sc);
    vector<string> input_flat_scale(sd);
    vector<string> input_flat_offset(se);
    vector<string> output_x;

    ps0 = WVR0.GetPerfStats();
    for (int64_t i = 0; i < NX; i++) {
      tempBatchNorm(
        WVR0, dim_size, input_flat_x, input_flat_mean, input_flat_var, input_flat_scale,
        input_flat_offset, output_x);
    }
    ps = WVR0.GetPerfStats() - ps0;
    double avg = ps.s.elapse / NX;
    cout << "PERFSTATS NX:" << NX << " per BatchNorm([" << dim_size[0] << "," << dim_size[1] << ","
         << dim_size[2] << "," << dim_size[3] << "]):" << setprecision(8) << avg << endl;
  };
  test_batchnorm(10, {1, 16, 16, 4});
  test_batchnorm(10, {1, 8, 8, 16});
  test_batchnorm(10, {1, 2, 2, 256});
  test_batchnorm(10, {1, 1, 1, 1024});

  // Matmul
  auto test_matmul = [&](int64_t NX, int64_t m, int64_t k, int64_t n, bool rh_is_const = false) {
    msg_id_t msgid("performance Matmul....");
    vector<double> a, b;
    random_vector(a, m * k, -2.0, 2.0);
    random_vector(b, k * n, -2.0, 2.0);
    vector<string> sa(a.size());
    vector<string> sb(b.size());
    vector<string> sc;
    WVR0.GetOps(msgid)->PrivateInput(ALICE - 1, a, sa);
    if (rh_is_const) {
      for (int64_t i = 0; i < b.size(); i++) {
        sb[i] = to_string(b[i]);
      }
    }
    else
      WVR0.GetOps(msgid)->PrivateInput(ALICE - 1, b, sb);
    
    attr_type attr;
    attr["m"] = to_string(m);
    attr["k"] = to_string(k);
    attr["n"] = to_string(n);
    attr["rh_is_const"] = rh_is_const ? "1" : "0";

    ps0 = WVR0.GetPerfStats();
    for (int64_t i = 0; i < NX; i++) {
      WVR0.GetOps(msgid)->Matmul(sa, sb, sc, &attr);
    }
    ps = WVR0.GetPerfStats() - ps0;
    double avg = ps.s.elapse / NX;
    cout << "PERFSTATS NX:" << NX << " per Matmul([" << m << "x" << k << "],[" << k << "x" << n
         << "]):" << setprecision(8) << avg << endl;
  };
  test_matmul(10, 64, 64, 64);
  test_matmul(10, 512, 512, 512);
  test_matmul(10, 2048, 2048, 2048);

  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  WVR_PROTOCOL_TEST_UNINIT(partyid);
}




/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

void run_maxpool(int partyid) {
  WVR_PROTOCOL_TEST_INIT(partyid);

  msg_id_t msgid("performance ....");
  log_info << "msgid:" << msgid;
  rosetta::PerfStats ps0, ps;

  // Max
  auto test_max = [&](int64_t NX, int inp_rows, int inp_cols) {
    int rows = inp_rows;
    int cols = inp_cols;
    unordered_map<string,string> attrs;
    attrs.insert(std::pair<string, string>("rows", std::to_string(rows)));
    attrs.insert(std::pair<string, string>("cols", std::to_string(cols)));
    int size = rows * cols;

    msg_id_t msgid("performance Max....");
    vector<double> a;
    random_vector(a, size, -2.0, 2.0);
    vector<string> sa(a.size());
    vector<string> sc;
    WVR0.GetOps(msgid)->PrivateInput(ALICE - 1, a, sa);
    ps0 = WVR0.GetPerfStats();
    for (int64_t i = 0; i < NX; i++) {
      WVR0.GetOps(msgid)->Max(sa, sc, &attrs);
    }
    ps = WVR0.GetPerfStats() - ps0;
    double avg = ps.s.elapse / NX;
    cout << "PERFSTATS NX:" << NX << " per MaxPool([" << size << "]):" << setprecision(8) << avg
         << endl;
  };
  test_max(250000, 2, 2);

  WVR_PROTOCOL_TEST_UNINIT(partyid);
}


void run_sigmoid(int partyid) {
  WVR_PROTOCOL_TEST_INIT(partyid);

  msg_id_t msgid("performance ....");
  log_info << "msgid:" << msgid;
  rosetta::PerfStats ps0, ps;

  // Sigmoid
  auto test_sigmoid = [&](int64_t NX, int64_t size) {
    msg_id_t msgid("performance Sigmoid....");
    vector<double> a, b;
    random_vector(a, size, -2.0, 2.0);
    vector<string> sa(a.size());
    vector<string> sc;
    WVR0.GetOps(msgid)->PrivateInput(ALICE - 1, a, sa);
    ps0 = WVR0.GetPerfStats();
    for (int64_t i = 0; i < NX; i++) {
      WVR0.GetOps(msgid)->Sigmoid(sa, sc);
    }
    ps = WVR0.GetPerfStats() - ps0;
    double avg = ps.s.elapse / NX;
    cout << "PERFSTATS NX:" << NX << " per Sigmoid([" << size << "]):" << setprecision(8) << avg
         << endl;
  };
  test_sigmoid(100000, 1);

  WVR_PROTOCOL_TEST_UNINIT(partyid);
}

void run_softmax(int partyid) {
  WVR_PROTOCOL_TEST_INIT(partyid);

  msg_id_t msgid("performance ....");
  log_info << "msgid:" << msgid;
  rosetta::PerfStats ps0, ps;

 // SoftMax
  auto test_softmax = [&](int64_t NX, int64_t batch_size, int64_t num_classes) {
    int64_t size = batch_size * num_classes;
    msg_id_t msgid("performance Softmax....");
    vector<double> a, b;
    random_vector(a, size, -2.0, 2.0);
    vector<string> sa(a.size());
    vector<string> sc;
    WVR0.GetOps(msgid)->PrivateInput(ALICE - 1, a, sa);
    ps0 = WVR0.GetPerfStats();
    attr_type attr;
    attr["rows"] = to_string(batch_size); //batch_size
    attr["cols"] = to_string(num_classes); //num_classes
    for (int64_t i = 0; i < NX; i++) {
      WVR0.GetOps(msgid)->Softmax(sa, sc, &attr);
    }
    ps = WVR0.GetPerfStats() - ps0;
    double avg = ps.s.elapse / NX;
    cout << "PERFSTATS NX:" << NX << " per Softmax([" << size << "]):" << setprecision(8) << avg
         << endl;
  };
  test_softmax(1000, 10, 10);

  WVR_PROTOCOL_TEST_UNINIT(partyid);
}

void run_relu(int partyid) {
  WVR_PROTOCOL_TEST_INIT(partyid);

  msg_id_t msgid("performance ....");
  log_info << "msgid:" << msgid;
  rosetta::PerfStats ps0, ps;

  // ReLU
  auto test_relu = [&](int64_t NX, int64_t size) {
    msg_id_t msgid("performance Relu....");
    vector<double> a, b;
    random_vector(a, size, -2.0, 2.0);
    vector<string> sa(a.size());
    vector<string> sc;
    WVR0.GetOps(msgid)->PrivateInput(ALICE - 1, a, sa);
    ps0 = WVR0.GetPerfStats();
    for (int64_t i = 0; i < NX; i++) {
      WVR0.GetOps(msgid)->Relu(sa, sc);
    }
    ps = WVR0.GetPerfStats() - ps0;
    double avg = ps.s.elapse / NX;
    cout << "PERFSTATS NX:" << NX << " per Relu([" << size << "]):" << setprecision(8) << avg
         << endl;
  };
  test_relu(1000000, 1);

  WVR_PROTOCOL_TEST_UNINIT(partyid);
}

// BatchNorm
void test_batchnorm(rosetta::WolverineProtocol &WVR0, rosetta::PerfStats ps0, rosetta::PerfStats ps, int64_t NX, initializer_list<int64_t> idim_size) {
    if (idim_size.size() != 4)
      return;

    vector<int64_t> dim_size(idim_size);
    int64_t NumElements = dim_size[0] * dim_size[1] * dim_size[2] * dim_size[3];
    const int64_t depth = dim_size[3];
    const int64_t size = NumElements;
    const int64_t rest_size = size / depth;

    msg_id_t msgid("performance BatchNorm....");
    vector<double> a, b, c, d, e;
    random_vector(a, size, -2.0, 2.0);
    random_vector(b, depth, -1.0, 1.0);
    random_vector(c, depth, -1.0, 1.0);
    random_vector(d, depth, -1.0, 1.0);
    random_vector(e, depth, -1.0, 1.0);
    vector<string> sa(a.size());
    vector<string> sb(b.size());
    vector<string> sc(c.size());
    vector<string> sd(d.size());
    vector<string> se(e.size());
    vector<string> sf;
    WVR0.GetOps(msgid)->PrivateInput(ALICE - 1, a, sa);
    WVR0.GetOps(msgid)->PrivateInput(ALICE - 1, b, sb);
    WVR0.GetOps(msgid)->PrivateInput(ALICE - 1, c, sc);
    WVR0.GetOps(msgid)->PrivateInput(ALICE - 1, d, sd);
    WVR0.GetOps(msgid)->PrivateInput(ALICE - 1, e, se);
    vector<string> input_flat_x(sa);
    vector<string> input_flat_mean(sb);
    vector<string> input_flat_var(sc);
    vector<string> input_flat_scale(sd);
    vector<string> input_flat_offset(se);
    vector<string> output_x;

    ps0 = WVR0.GetPerfStats();
    for (int64_t i = 0; i < NX; i++) {
      tempBatchNorm(
        WVR0, dim_size, input_flat_x, input_flat_mean, input_flat_var, input_flat_scale,
        input_flat_offset, output_x);
    }
    ps = WVR0.GetPerfStats() - ps0;
    double avg = ps.s.elapse / NX;
    cout << "PERFSTATS NX:" << NX << " per BatchNorm([" << dim_size[0] << "," << dim_size[1] << ","
         << dim_size[2] << "," << dim_size[3] << "]):" << setprecision(8) << avg << endl;
};

void run_bn_1(int partyid) {
  WVR_PROTOCOL_TEST_INIT(partyid);

  msg_id_t msgid("performance ....");
  log_info << "msgid:" << msgid;
  rosetta::PerfStats ps0, ps;
  test_batchnorm(WVR0, ps0, ps, 1000, {1, 16, 16, 4});

  WVR_PROTOCOL_TEST_UNINIT(partyid);
}

// Matmul
void test_matmul(rosetta::WolverineProtocol &WVR0, rosetta::PerfStats ps0, rosetta::PerfStats ps, int64_t NX, int64_t m, int64_t k, int64_t n, bool rh_is_const = false) {
    msg_id_t msgid("performance Matmul....");
    vector<double> a, b;
    random_vector(a, m * k, -2.0, 2.0);
    random_vector(b, k * n, -2.0, 2.0);
    vector<string> sa(a.size());
    vector<string> sb(b.size());
    vector<string> sc;
    WVR0.GetOps(msgid)->PrivateInput(ALICE - 1, a, sa);
    if (rh_is_const) {
      for (int64_t i = 0; i < b.size(); i++) {
        sb[i] = to_string(b[i]);
      }
    }
    else
      WVR0.GetOps(msgid)->PrivateInput(ALICE - 1, b, sb);
    
    attr_type attr;
    attr["m"] = to_string(m);
    attr["k"] = to_string(k);
    attr["n"] = to_string(n);
    attr["rh_is_const"] = rh_is_const ? "1" : "0";

    ps0 = WVR0.GetPerfStats();
    for (int64_t i = 0; i < NX; i++) {
      WVR0.GetOps(msgid)->Matmul(sa, sb, sc, &attr);
    }
    ps = WVR0.GetPerfStats() - ps0;
    double avg = ps.s.elapse / NX;
    cout << "PERFSTATS NX:" << NX << " per Matmul([" << m << "x" << k << "],[" << k << "x" << n
         << "]):" << setprecision(8) << avg << endl;
};

void run_matmul_1(int partyid) {
  WVR_PROTOCOL_TEST_INIT(partyid);

  msg_id_t msgid("performance ....");
  log_info << "msgid:" << msgid;
  rosetta::PerfStats ps0, ps;
  test_matmul(WVR0, ps0, ps, 1000, 512, 512, 512);

  WVR_PROTOCOL_TEST_UNINIT(partyid);
}

void run_matmul_2(int partyid) {
  WVR_PROTOCOL_TEST_INIT(partyid);

  msg_id_t msgid("performance ....");
  log_info << "msgid:" << msgid;
  rosetta::PerfStats ps0, ps;
  test_matmul(WVR0, ps0, ps, 15, 2048, 2048, 2048);

  WVR_PROTOCOL_TEST_UNINIT(partyid);
}

void run_matmul_3(int partyid) {
  WVR_PROTOCOL_TEST_INIT(partyid);

  msg_id_t msgid("performance ....");
  log_info << "msgid:" << msgid;
  rosetta::PerfStats ps0, ps;
  test_matmul(WVR0, ps0, ps, 150, 1024, 1024, 1024);

  WVR_PROTOCOL_TEST_UNINIT(partyid);
}
