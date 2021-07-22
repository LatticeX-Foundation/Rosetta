#include "perf_special.hpp"
#include <iostream>

#define MAXPOOL 0
#define SIGMOID 1
#define SOFTMAX 2
#define RELU  3
#define BN_1 4
#define MATMUL_1 5
#define MATMUL_2 6
#define MATMUL_3 7


int main(int argc, char* argv[]) {
  if (argc < 3) {
    cerr << "Usage:\n\t" << argv[0] << " <partyid> <ml function>" << endl;
    exit(0);
  }
  // run(atoi(argv[1]));

  int pid = atoi(argv[1]);
  int type = atoi(argv[2]);
  switch (type) {
    case MAXPOOL:
    run_maxpool(pid);
    break;

    case SIGMOID:
    run_sigmoid(pid);
    break;

    case SOFTMAX:
    run_softmax(pid);
    break;

    case RELU:
    run_relu(pid);
    break;

    case BN_1:
    run_bn_1(pid);
    break;

    case MATMUL_1:
    run_matmul_1(pid);
    break;

    case MATMUL_2:
    run_matmul_2(pid);
    break;

    case MATMUL_3:
    run_matmul_3(pid);
    break;
  }


  std::cout << "DONE!\n";
  return 0;
}