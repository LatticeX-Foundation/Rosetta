// ==============================================================================
// Copyright 2020 The LatticeX Foundation
// This file is part of the Rosetta library.
//
// The Rosetta library is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// The Rosetta library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with the Rosetta library. If not, see <http://www.gnu.org/licenses/>.
// ==============================================================================
#include "cc/tf/secureops/secure_base_kernel.h"
#include "cc/modules/common/include/utils/logger.h"
#include "cc/modules/protocol/public/protocol_manager.h"

using rosetta::ProtocolManager;

// Binary OP: Add/Sub/Mul/Div/...
namespace tensorflow {

class SecureSigmoidOp : public StrUnaryOp {
 private:
  /* data */
 public:
  SecureSigmoidOp(OpKernelConstruction* context) : StrUnaryOp(context) {}
  ~SecureSigmoidOp() {}

  int UnaryCompute(const vector<string>& input, vector<string>& output) {
    log_debug << "--> Sigmoid OpKernel compute.";
    output.resize(input.size());
    ProtocolManager::Instance()->GetProtocol()->GetOps(msg_id().str())->Sigmoid(input, output, &attrs_);
    log_debug << "Sigmoid OpKernel compute ok. <--";
    return 0;
  }
};

class SecureSigmoidCrossEntropyOp : public SecureBinaryOp {
 private:
  /* data */
 public:
  SecureSigmoidCrossEntropyOp(OpKernelConstruction* context) : SecureBinaryOp(context) {}
  ~SecureSigmoidCrossEntropyOp() {}

  int BinaryCompute(const vector<string>& in1, const vector<string>& in2, vector<string>& output) {
    log_debug << "--> SigmoidCrossEntropy OpKernel compute.";
    output.resize(in1.size());
    ProtocolManager::Instance()->GetProtocol()->GetOps(msg_id().str())->SigmoidCrossEntropy(in1, in2, output, &attrs_);
    log_debug << "SigmoidCrossEntropy OpKernel compute ok. <--";
    return 0;
  }
};

class SecureReluOp : public StrUnaryOp {
 private:
  /* data */
 public:
  SecureReluOp(OpKernelConstruction* context) : StrUnaryOp(context) {}
  ~SecureReluOp() {}

  int UnaryCompute(const vector<string>& input, vector<string>& output) {
    log_debug << "--> Relu OpKernel compute.";
    ProtocolManager::Instance()->GetProtocol()->GetOps(msg_id().str())->Relu(input, output, &attrs_);
    log_debug << "Relu OpKernel compute ok. <--";
    return 0;
  }
};

class SecureReluPrimeOp : public StrUnaryOp {
 private:
  /* data */
 public:
  SecureReluPrimeOp(OpKernelConstruction* context) : StrUnaryOp(context) {}
  ~SecureReluPrimeOp() {}

  int UnaryCompute(const vector<string>& input, vector<string>& output) {
    log_debug << "--> ReluPrime OpKernel compute.";
    ProtocolManager::Instance()->GetProtocol()->GetOps(msg_id().str())->ReluPrime(input, output, &attrs_);
    log_debug << "ReluPrime OpKernel compute ok. <--";
    return 0;
  }
};

REGISTER_STR_CPU_KERNEL(SecureRelu, SecureReluOp);
REGISTER_STR_CPU_KERNEL(SecureReluPrime, SecureReluPrimeOp);
REGISTER_STR_CPU_KERNEL(SecureSigmoid, SecureSigmoidOp);
REGISTER_STR_CPU_KERNEL(SecureSigmoidCrossEntropy, SecureSigmoidCrossEntropyOp);

}// namespace tensorflow