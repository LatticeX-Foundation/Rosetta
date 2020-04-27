#include "snn.h"

int main(int argc, char* argv[]) {
  using namespace rosetta::mpc::debug;

  if (argc < 3) {
    cout << "<party> <configuration file>" << endl;
    return 0;
  }

  Logger::Get().log_to_stdout();

  int party = atoi(argv[1]);
  string cfgfile(argv[2]);

  SecureParams params(party, cfgfile);
  initialize_mpc(params);

  {
    usage();
    testMyType();
    testTypeConvert();
    testTypeConvert2();
    testEachOpComm();
    testPRZS();
    testPrivateInput();

    // Basic
    debugDotProd();
    //debugDivision();
    //debugDivisionV2();
    debugComputeMSB();
    debugPC();
    debugSS();

    // Binary OP
    testBinaryOps();
    testMulThreads(6);

    // MatMul
    debugMatMul();
    debugMatMul_tp();
    testMatMul();
    testConvolution();
    testMatMulThreads();
    testMatMulThreads(8);
    if (false) {
      // todo: check later
      // use the following command
      // grep -E "MAIN|RUNMPCOP elapse" build/bin/log/protocol-mpc-newsnn-log0.txt |grep -E "::MatMul|MAIN"
      // grep -E "MAIN|RUNMPCOP communication" build/bin/log/protocol-mpc-newsnn-log0.txt |grep -E "::MatMul|MAIN"

      SimpleTimer timer;
      int n = 4;
      for (int i = 0; i < n; i++) {
        testMatMulThreads();
      }
      cout << "RUNMPCOP elapse(us): " << setw(10) << timer.us_elapse()
           << " MAIN single-thread (total " << n << " times)" << endl;

      timer.start();
      testMatMulThreads(n);
      cout << "RUNMPCOP elapse(us): " << setw(10) << timer.us_elapse()
           << " MAIN multi-thread (n=" << n << ")" << endl;
    }

    // Max/MaxPool
    debugMax();
    debugMaxIndex();

    // Relu
    debugReluPrime();
    testRelu();
    testReluPrime();
    // Log & Pow
    debug_mpc_power();
    debug_LOG();

    // Sigmoid
    testSigmoidG3();
    testSigmoidG3Prime();
    testSigmoidPieceWise();
    testSigmoidAliPieceWise();
    testSigmoidPieceWise2();

    // NN CrossEntropy
    debug_NN();

    // debug Mixed shared inputs and plaintext inputs.
    debug_Mix_OP();

    // TTT
    //testPow2();

    //testMatmul2();
    //testSigmoid2();
    //testMax2();
    //testMean2();
    //testRelu2();

    //testMSB2();
    //testReluPrime2();
    //testShareConvert2();
  }

  cout << "DONE 1!" << endl;
  msg_id_t msg_id("AhahahahaAhahahahaAhahahaha!~");
  synchronize(msg_id);
  cout << "DONE 2!" << endl;
  uninitialize_mpc();

  return 0;
}
