#include "snn__test.h"

/**
 * Binary OP(s)(VV/VC/CV)
 *  *Variable vs Variable; Variable vs Constant; Constant vs Constant
 *    - Add/Sub/Mul/Floordiv/Truediv
 *    - Equal/NotEqual/Less/LessEqual/Greater/GreaterEqual
 * 
 * Binary OP (Special)
 *    - SigmoidCrossEntropy
 *    - Pow
 * 
 * Unary OP(s)
 *    - Square/Negative/Abs/AbsPrime
 *    - Log/Log1p/HLog/
 *    - Relu/ReluPrime
 *    - Sigmoid
 * 
 * Reduce OP(s)
 *    - Mean/Sum/AddN/Max/Min
 * 
 * Special OP
 *    - MatMul
 */
void run(int partyid) {
  SNN_PROTOCOL_TEST_INIT(partyid);
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  rosetta::SnnProtocol& prot0 = snn0;
#include "cc/modules/protocol/mpc/comm/include/_test_check.cpp"
  //////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////
  SNN_PROTOCOL_TEST_UNINIT(partyid);
}

RUN_MPC_TEST(run);
