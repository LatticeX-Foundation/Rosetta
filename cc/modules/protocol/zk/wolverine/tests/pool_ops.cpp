#include "wvr_test.h"
// #include "emp-wolverine-fp/emp-wolverine-fp.h"

void run(int partyid) {
  WVR_PROTOCOL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  msg_id_t msgid("pool_ops_test");
  
  int rows = 3;
  int cols = 3;
  unordered_map<string,string> attrs;
  attrs.insert(std::pair<string, string>("rows", std::to_string(rows)));
  attrs.insert(std::pair<string, string>("cols", std::to_string(cols)));
  {
    vector<double> max1{-9,-1,-4, 0, 3, -3, -2, 1, 2}, max_reveal1(rows);//{-1, 3, 2};//3*3
    vector<string> max_str1(max1.size()), max_output1(rows);
    WVR0.GetOps(msgid)->PrivateInput(0, max1, max_str1);
    // for (size_t i = 0; i < max1.size(); i++)
    // {
    //   log_info << "max1_str: " << max_str1[i];
    // }

    WVR0.GetOps(msgid)->Max(max_str1, max_output1, &attrs);
    WVR0.GetOps(msgid)->Reveal(max_output1, max_reveal1);
    log_info << "max1 check and ok. out: " << max_output1[0];
    print_vec(max1, max1.size(), "max1 input: ");
    print_vec(max_reveal1, max_reveal1.size(), "max1: ");
  }

  {
    vector<double> max2{0,0,1, 0,-2,1, -2,0,-1}, max_reveal2;//{1, 1, 0};
    vector<string> max_str2(max2.size()), max_output2;
    WVR0.GetOps(msgid)->PrivateInput(0, max2, max_str2);
    // for (size_t i = 0; i < max2.size(); i++)
    // {
    //   log_info << "max2_str: " << max_str2[i];
    // }

    WVR0.GetOps(msgid)->Max(max_str2, max_output2, &attrs);
    WVR0.GetOps(msgid)->Reveal(max_output2, max_reveal2);
    log_info << "max2 check and ok. out: " << max_output2[0];
    print_vec(max2, max2.size(), "max2 input: ");
    print_vec(max_reveal2, max_reveal2.size(), "max2: ");
  }

  {
    vector<double> avg{1,2,3, 1,4, 4, 1,9,2}, avg_reveal;//{2, 3, 4};
    vector<string> avg_str(avg.size()), avg_output(rows);
    WVR0.GetOps(msgid)->PrivateInput(0, avg, avg_str);
    // vector<double> avg_in_reveal;
    // WVR0.GetOps(msgid)->Reveal(avg_str, avg_in_reveal);
    WVR0.GetOps(msgid)->Mean(avg_str, avg_output, &attrs);
    WVR0.GetOps(msgid)->Reveal(avg_output, avg_reveal);
    log_info << "avg check and ok. cipher out: " << avg_output[0];
    
    print_vec(avg, avg.size(), "avg input: ");
    print_vec(avg_reveal, avg_reveal.size(), "avg: ");

    vector<double> max_expect{(1.0+2.0+3.0)/3, (1 + 4.0+4.0)/3, (1.0+9.0+2.0)/3};
    print_vec(max_expect, max_expect.size(), "avg expect: ");

    // print_vec(avg_in_reveal, avg_in_reveal.size(), "avg input reveal: ");
  }
  

  WVR_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_ZK_TEST(run);