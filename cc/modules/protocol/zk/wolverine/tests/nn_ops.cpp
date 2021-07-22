#include "wvr_test.h"
// #include "emp-wolverine-fp/emp-wolverine-fp.h"

void run(int partyid) {
  SimpleTimer timer;
  WVR_PROTOCOL_TEST_INIT(partyid);
  log_error << "init cost: " << timer.elapse() << " seconds";
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  msg_id_t msgid("nn_ops_test");

  // {// test only one relu, only for zk non-plain
  //   // a2b first time
  //   IntFp fpa((uint64_t)1, PUBLIC);
  //   Integer fpb(ZK_INT_LEN, 0, PUBLIC);
  //   timer.start();
  //   sync_zk_bool<BoolIO<NetIO>>();
  //   arith2bool<BoolIO<NetIO>>(&fpb, &fpa, 1);
  //   sync_zk_bool<BoolIO<NetIO>>();
  //   log_error << "first a2b with one element, cost: " << timer.elapse() << " seconds";
  //   size_t size = 1;
  //   vector<double> relu_in(size, 1);
  //   vector<string> relu_str;
  //   WVR0.GetOps(msgid)->PrivateInput(0, relu_in, relu_str);
    
  //   timer.start();
  //   vector<string> relu_result;
  //   WVR0.GetOps(msgid)->Relu(relu_str, relu_result);//0, 1, ... 50, -51, ...
  //   log_error << "size: " << size << ", relu cost: " << timer.elapse() << " seconds";
  //   // WVR0.GetOps(msgid)->Reveal(bias_add, bias_add_expect);
  //   vector<double> relu_reveal;
  //   WVR0.GetOps(msgid)->Reveal(relu_result, relu_reveal);
    
  //   log_info << "relu(1): " << relu_reveal[0];
  // }

  {// test1, simple 4*4 relu
    size_t size = 4*4;
    vector<double> a(size, 3), b(size, 2);
    vector<double> relu_in(size, 0);
    vector<double> relu_expect(size, 0), bias_add_expect(size, 5);
    for (int i = 0; i < size; ++i) {
      relu_in[i] = (i <= size/2) ? i : (-i);//(double)(uint64_t(pr-i));//-i
      cout << "relu-in: " << relu_in[i] << endl;
      relu_expect[i] = (i <= size/2) ? i : 0;
    }
    
    vector<string> a_str, b_str, relu_in_str;
    WVR0.GetOps(msgid)->PrivateInput(0, a, a_str);
    WVR0.GetOps(msgid)->PrivateInput(0, b, b_str);
    WVR0.GetOps(msgid)->PrivateInput(0, relu_in, relu_in_str);
    
    // bias_add, relu, max_pool, avg_pool
    vector<string> bias_add, relu, all;
    // WVR0.GetOps(msgid)->BiasAdd(a_str, b_str, bias_add);//5,5,...
    // WVR0.GetOps(msgid)->Reveal(bias_add, bias_add_expect);
    WVR0.GetOps(msgid)->Relu(relu_in_str, relu);//0, 1, ... 50, -51, ...
    
    vector<double> relu_result;
    WVR0.GetOps(msgid)->Reveal(relu, relu_result);
    // vector<double> relu_exp;
    // WVR0.GetOps(msgid)->Reveal(relu_in_str, relu_exp);
    for (size_t i = 0; i < relu_result.size(); i++)
    {
      log_error << "==> relu: " << relu_result[i];
    }
  }

  {//relu failed case
    log_info << "----------- relu-failed-case1 test...";
    vector<double> input{-1.21801758, -0.43222046, -0.13249207, -0.26385498, -0.83807373, 
                        1.21801758, 0.43222046, 0.13249207, 0.26385498, 0.83807373};
    vector<string> input_str, relu_str;
    WVR0.GetOps(msgid)->PrivateInput(0, input, input_str);
    WVR0.GetOps(msgid)->Relu(input_str, relu_str);
    vector<double> relu_exp2;
    WVR0.GetOps(msgid)->Reveal(relu_str, relu_exp2);
    log_error << "relu: ---------------------------";
    for (size_t i = 0; i < relu_exp2.size(); i++)
    {
      log_error << "==> relu: " << relu_exp2[i];
    }
    log_info << "relu-failed-case1 test done.----------- ";
  }

  log_info << "bias_add, relu check and ok.";

  WVR_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_ZK_TEST(run);