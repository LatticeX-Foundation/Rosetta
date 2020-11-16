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
#include <stdexcept>
#include "cc/tf/secureops/secure_base_kernel.h"
#include "cc/modules/protocol/public/protocol_manager.h"
#include "cc/modules/common/include/utils/logger.h"

using rosetta::ProtocolManager;

// Binary OP: Add/Sub/Mul/Div/...
namespace tensorflow {

class SecureAddOp : public SecureBinaryOp {
 private:
  /* data */
 public:
  SecureAddOp(OpKernelConstruction* context) : SecureBinaryOp(context) {}
  ~SecureAddOp() {}
  int BinaryCompute(
    const vector<string>& in1, const vector<string>& in2, vector<string>& output) {
    log_debug << "--> Add OpKernel compute.";
    ProtocolManager::Instance()->GetProtocol()->GetOps(msg_id().str())->Add(in1, in2, output, &attrs_);
    //cout << "DEBUG ADD END with output[0]: " << output[0] << endl;
    log_debug << "Add OpKernel compute ok. <--";
    return 0;
  }
};

class SecureSubOp : public SecureBinaryOp {
 private:
  /* data */
 public:
  SecureSubOp(OpKernelConstruction* context) : SecureBinaryOp(context) {}
  ~SecureSubOp() {}

  int BinaryCompute(
    const vector<string>& in1, const vector<string>& in2, vector<string>& output) {
    log_debug << "--> Sub OpKernel compute." << endl;;
    ProtocolManager::Instance()->GetProtocol()->GetOps(msg_id().str())->Sub(in1, in2, output, &attrs_);
    log_debug << "Sub OpKernel compute ok. <--";
    return 0;
  }
};

class SecureMulOp : public SecureBinaryOp {
 private:
  /* data */
 public:
  SecureMulOp(OpKernelConstruction* context) : SecureBinaryOp(context) {}
  ~SecureMulOp() {}

  int BinaryCompute(
    const vector<string>& in1, const vector<string>& in2, vector<string>& output) {
    log_debug << "--> Mul OpKernel compute.";
    ProtocolManager::Instance()->GetProtocol()->GetOps(msg_id().str())->Mul(in1, in2, output, &attrs_);
    log_debug << "Mul OpKernel compute ok. <--";
    return 0;
  }
};

class SecureLessOp : public SecureBinaryOp {
 private:
  /* data */
 public:
  SecureLessOp(OpKernelConstruction* context) : SecureBinaryOp(context) {}
  ~SecureLessOp() {}

  int BinaryCompute(
    const vector<string>& in1, const vector<string>& in2, vector<string>& output) {
    log_debug << "--> Less OpKernel compute.";
    ProtocolManager::Instance()->GetProtocol()->GetOps(msg_id().str())->Less(in1, in2, output, &attrs_);
    log_debug << "Less OpKernel compute ok. <--";
    return 0;
  }
};

class SecureLessEqualOp : public SecureBinaryOp {
 private:
  /* data */
 public:
  SecureLessEqualOp(OpKernelConstruction* context) : SecureBinaryOp(context) {}
  ~SecureLessEqualOp() {}

  int BinaryCompute(
    const vector<string>& in1, const vector<string>& in2, vector<string>& output) {
    log_debug << "--> LessEqual OpKernel compute.";
    ProtocolManager::Instance()->GetProtocol()->GetOps(msg_id().str())->LessEqual(in1, in2, output, &attrs_);
    log_debug << "LessEqual OpKernel compute ok. <--";
    return 0;
  }
};

class SecureEqualOp : public SecureBinaryOp {
 private:
  /* data */
 public:
  SecureEqualOp(OpKernelConstruction* context) : SecureBinaryOp(context) {}
  ~SecureEqualOp() {}

  int BinaryCompute(
    const vector<string>& in1, const vector<string>& in2, vector<string>& output) {
    log_debug << "--> Equal OpKernel compute.";
    ProtocolManager::Instance()->GetProtocol()->GetOps(msg_id().str())->Equal(in1, in2, output, &attrs_);
    log_debug << "Equal OpKernel compute ok. <--";
    return 0;
  }
};

class SecureNotEqualOp : public SecureBinaryOp {
 private:
  /* data */
 public:
  SecureNotEqualOp(OpKernelConstruction* context) : SecureBinaryOp(context) {}
  ~SecureNotEqualOp() {}

  int BinaryCompute(
    const vector<string>& in1, const vector<string>& in2, vector<string>& output) {
    log_debug << "--> NotEqual OpKernel compute.";
    ProtocolManager::Instance()->GetProtocol()->GetOps(msg_id().str())->NotEqual(in1, in2, output, &attrs_);
    log_debug << "NotEqual OpKernel compute ok. <--";
    return 0;
  }
};

class SecureGreaterOp : public SecureBinaryOp {
 private:
  /* data */
 public:
  SecureGreaterOp(OpKernelConstruction* context) : SecureBinaryOp(context) {}
  ~SecureGreaterOp() {}

  int BinaryCompute(
    const vector<string>& in1, const vector<string>& in2, vector<string>& output) {
    log_debug << "--> Greater OpKernel compute.";
    ProtocolManager::Instance()->GetProtocol()->GetOps(msg_id().str())->Greater(in1, in2, output, &attrs_);
    log_debug << "Greater OpKernel compute ok. <--";
    return 0;
  }
};

class SecureGreaterEqualOp : public SecureBinaryOp {
 private:
  /* data */
 public:
  SecureGreaterEqualOp(OpKernelConstruction* context) : SecureBinaryOp(context) {}
  ~SecureGreaterEqualOp() {}

  int BinaryCompute(
    const vector<string>& in1, const vector<string>& in2, vector<string>& output) {
    log_debug << "--> GreaterEqual OpKernel compute.";
    ProtocolManager::Instance()->GetProtocol()->GetOps(msg_id().str())->GreaterEqual(in1, in2, output, &attrs_);
    log_debug << "GreaterEqual OpKernel compute ok. <--";
    return 0;
  }
};

class SecureDivOp : public SecureBinaryOp {
 private:
  /* data */
 public:
  SecureDivOp(OpKernelConstruction* context) : SecureBinaryOp(context) {}
  ~SecureDivOp() {}

  int BinaryCompute(
    const vector<string>& in1, const vector<string>& in2, vector<string>& output) {
    log_debug << "--> Div OpKernel compute.";
    ProtocolManager::Instance()->GetProtocol()->GetOps(msg_id().str())->Div(in1, in2, output, &attrs_);
    log_debug << "Div OpKernel compute ok. <--";
    return 0;
  }
};

class SecureTruedivOp : public SecureBinaryOp {
 private:
  /* data */
 public:
  SecureTruedivOp(OpKernelConstruction* context) : SecureBinaryOp(context) {}
  ~SecureTruedivOp() {}

  int BinaryCompute(
    const vector<string>& in1, const vector<string>& in2, vector<string>& output) {
    log_debug << "--> Truediv OpKernel compute.";
    ProtocolManager::Instance()->GetProtocol()->GetOps(msg_id().str())->Truediv(in1, in2, output, &attrs_);
    log_debug << "Truediv OpKernel compute ok. <--";
    return 0;
  }
};

class SecureFloordivOp : public SecureBinaryOp {
 private:
  /* data */
 public:
  SecureFloordivOp(OpKernelConstruction* context) : SecureBinaryOp(context) {}
  ~SecureFloordivOp() {}

  int BinaryCompute(
    const vector<string>& in1, const vector<string>& in2, vector<string>& output) {
    log_debug << "--> Floordiv OpKernel compute.";
    ProtocolManager::Instance()->GetProtocol()->GetOps(msg_id().str())->Floordiv(in1, in2, output, &attrs_);
    log_debug << "Floordiv OpKernel compute ok. <--";
    return 0;
  }
};

// equivelent to SecureDivOp
class SecureRealdivOp : public SecureBinaryOp {
 private:
  /* data */
 public:
  SecureRealdivOp(OpKernelConstruction* context) : SecureBinaryOp(context) {}
  ~SecureRealdivOp() {}

  int BinaryCompute(
    const vector<string>& in1, const vector<string>& in2, vector<string>& output) {
    log_debug << "--> Realdiv OpKernel compute.";
    ProtocolManager::Instance()->GetProtocol()->GetOps(msg_id().str())->Div(in1, in2, output, &attrs_);
    log_debug << "Realdiv OpKernel compute ok. <--";
    return 0;
  }
};

class SecurePowOp : public SecureBinaryOp {
 private:
  /* data */
 public:
  SecurePowOp(OpKernelConstruction* context) : SecureBinaryOp(context) {}
  ~SecurePowOp() {}

  int BinaryCompute(
    const vector<string>& in1, const vector<string>& in2, vector<string>& output) {
    // lh_is_const, rh_is_const SHOULD set to False, True
    // if ((!lh_is_const_) && rh_is_const_)
    // {
    //   log_debug << "!! Pow OpKernel should set: lh_is_const=False, rh_is_const=True" << endl;
    //   return -1;
    // }

    log_debug << "--> Pow OpKernel compute.";
    ProtocolManager::Instance()->GetProtocol()->GetOps(msg_id().str())->Pow(in1, in2, output, &attrs_);
    log_debug << "Pow OpKernel compute ok. <--";
    return 0;
  }
};

class SecureMatmulOp : public SecureOpKernel {
 private:
  /* data */
 public:
  SecureMatmulOp(OpKernelConstruction* context) : SecureOpKernel(context) {
    OP_REQUIRES_OK(context, context->GetAttr("transpose_a", &transpose_a_));
    OP_REQUIRES_OK(context, context->GetAttr("transpose_b", &transpose_b_));
  }
  ~SecureMatmulOp() {}

  void ComputeImpl(OpKernelContext* context) {
    log_debug << "--> Matmul OpKernel compute.";
    const Tensor& x = context->input(0);
    const Tensor& y = context->input(1);

    // Check that the dimensions of the two matrices are valid.
    OP_REQUIRES(
      context, TensorShapeUtils::IsMatrix(x.shape()),
      errors::InvalidArgument(
        "In[0] is not a matrix. Instead it has shape ", x.shape().DebugString()));
    OP_REQUIRES(
      context, TensorShapeUtils::IsMatrix(y.shape()),
      errors::InvalidArgument(
        "In[1] is not a matrix. Instead it has shape ", y.shape().DebugString()));

    Eigen::array<Eigen::IndexPair<Eigen::DenseIndex>, 1> dim_pair;
    dim_pair[0].first = transpose_a_ ? 0 : 1;
    dim_pair[0].second = transpose_b_ ? 1 : 0;

    OP_REQUIRES(
      context, x.dim_size(dim_pair[0].first) == y.dim_size(dim_pair[0].second),
      errors::InvalidArgument(
        "Matrix size-incompatible: In[0]: ", x.shape().DebugString(),
        ", In[1]: ", y.shape().DebugString()));
    int a_dim_remaining = 1 - dim_pair[0].first;
    int b_dim_remaining = 1 - dim_pair[0].second;

    // (m,K) * (K,n) = (m,n)
    int m = x.dim_size(a_dim_remaining);
    int n = y.dim_size(b_dim_remaining);
    int K = x.dim_size(dim_pair[0].first);

    if (x.NumElements() == 0 && y.NumElements() == 0) {
      // If x has shape [x, 0] and y has shape [0, y], the
      // output shape is [x, y] where x and y are non-zero, so we fill
      // the output with zeros.
      return;
    }

    auto in_flatx = x.flat<string>();
    auto in_flaty = y.flat<string>();

    vector<string> in1(m * K);
    vector<string> in2(K * n);
    for (int k = 0; k <= K - 1; k++) {
      for (int i = 0; i <= m - 1; i++) {
        int idx = i * K + k;
        in1[idx] = in_flatx(idx);
      }
      for (int j = 0; j <= n - 1; j++) {
        int idx = k * n + j;
        in2[idx] = in_flaty(idx);
      }
    }

    // m, K, n attributes set
    attrs_["m"] = std::to_string(m);
    attrs_["k"] = std::to_string(K);
    attrs_["n"] = std::to_string(n);
    attrs_["transpose_a"] = transpose_a_ ? "1" : "0";
    attrs_["transpose_b"] = transpose_b_ ? "1" : "0";

    log_debug << "**Matmul m: " << m << ", K: " << K  << ", n: " << n << ", transpose_a: " << transpose_a_ << ", transpose_b: " << transpose_b_ << endl;
    
    // call protocol ops
    vector<string> outstr(m*n);
    ProtocolManager::Instance()->GetProtocol()->GetOps(msg_id().str())->Matmul(in1, in2, outstr, &attrs_);
    
    // set output
    TensorShape output_shape({m, n});
    Tensor* output = nullptr;
    OP_REQUIRES_OK(context, context->allocate_output(0, output_shape, &output));

    auto flat_out = output->flat<string>();
    for (int i = 0; i < outstr.size(); ++i) {
      flat_out(i) = outstr[i];
    }
#if PRINT_REVEAL
    debug_print_reveal(outstr, string(" mpc out").c_str());
#endif
    log_debug << "Matmul OpKernel compute ok. <--";
    return;
  }

 private:
  bool transpose_a_ = false;
  bool transpose_b_ = false;
};

class SecureSquareOp : public StrUnaryOp {
 private:
  /* data */
 public:
  SecureSquareOp(OpKernelConstruction* context) : StrUnaryOp(context) {}
  ~SecureSquareOp() {}

  int UnaryCompute(const vector<string>& input, vector<string>& output) {
    log_debug << "--> Square OpKernel compute.";
    ProtocolManager::Instance()->GetProtocol()->GetOps(msg_id().str())->Square(input, output, &attrs_);
    log_debug << "Square OpKernel compute ok. <--";
    return 0;
  }
};

class SecureNegativeOp : public StrUnaryOp {
 private:
  /* data */
 public:
  SecureNegativeOp(OpKernelConstruction* context) : StrUnaryOp(context) {}
  ~SecureNegativeOp() {}

  int UnaryCompute(const vector<string>& input, vector<string>& output) {
    log_debug << "--> Negative OpKernel compute.";
    ProtocolManager::Instance()->GetProtocol()->GetOps(msg_id().str())->Negative(input, output, &attrs_);
    log_debug << "Negative OpKernel compute ok. <--";
    return 0;
  }
};

class SecureReduceMeanOp : public SecureReduceOp {
 private:
  /* data */
 public:
  SecureReduceMeanOp(OpKernelConstruction* context) : SecureReduceOp(context) {}
  ~SecureReduceMeanOp() {}
  string name() {
    return string("SecureReduceMeanOp");
  }
  int ReduceCompute(const vector<string>& input, vector<string>& output, int rows, int cols) {
    log_debug << "--> ReduceMean OpKernel compute.";
    ProtocolManager::Instance()->GetProtocol()->GetOps(msg_id().str())->Mean(input, output, &attrs_);
    log_debug << "ReduceMean OpKernel compute ok. <--";
    return 0;
  }
};

class SecureReduceSumOp : public SecureReduceOp {
 private:
  /* data */
 public:
  SecureReduceSumOp(OpKernelConstruction* context) : SecureReduceOp(context) {}
  ~SecureReduceSumOp() {}
  string name() {
    return string("SecureReduceSumOp");
  }
  int ReduceCompute(const vector<string>& input, vector<string>& output, int rows, int cols) {
    log_debug << "--> ReduceSum OpKernel compute.";
    ProtocolManager::Instance()->GetProtocol()->GetOps(msg_id().str())->Sum(input, output, &attrs_);
    log_debug << "ReduceSum OpKernel compute ok. <--";
    return 0;
  }
};

class SecureReduceMinOp : public SecureReduceOp {
 private:
  /* data */
 public:
  SecureReduceMinOp(OpKernelConstruction* context) : SecureReduceOp(context) {}
  ~SecureReduceMinOp() {}
  string name() {
    return string("SecureReduceMinOp");
  }
  int ReduceCompute(const vector<string>& input, vector<string>& output, int rows, int cols) {
    log_debug << "--> ReduceMin OpKernel compute.";
    ProtocolManager::Instance()->GetProtocol()->GetOps(msg_id().str())->Min(input, output, &attrs_);
    log_debug << "ReduceMin OpKernel compute ok. <--";
    return 0;
  }
};

class SecureReduceMaxOp : public SecureReduceOp {
 private:
  /* data */
 public:
  SecureReduceMaxOp(OpKernelConstruction* context) : SecureReduceOp(context) {}
  ~SecureReduceMaxOp() {}

  string name() {
    return string("SecureReduceMaxOp");
  }
  int ReduceCompute(const vector<string>& input, vector<string>& output, int rows, int cols) {
    log_debug << "--> ReduceMax OpKernel compute.";
    ProtocolManager::Instance()->GetProtocol()->GetOps(msg_id().str())->Max(input, output, &attrs_);
    log_debug << "ReduceMax OpKernel compute ok. <--";
    return 0;
  }
};

class SecureAbsOp : public StrUnaryOp {
 private:
  /* data */
 public:
  SecureAbsOp(OpKernelConstruction* context) : StrUnaryOp(context) {}
  ~SecureAbsOp() {}

  int UnaryCompute(const vector<string>& input, vector<string>& output) {
    log_debug << "--> Abs OpKernel compute.";
    ProtocolManager::Instance()->GetProtocol()->GetOps(msg_id().str())->Abs(input, output, &attrs_);
    log_debug << "Abs OpKernel compute ok. <--";
    return 0;
  }
};

class SecureAbsPrimeOp : public StrUnaryOp {
 private:
  /* data */
 public:
  SecureAbsPrimeOp(OpKernelConstruction* context) : StrUnaryOp(context) {}
  ~SecureAbsPrimeOp() {}

  int UnaryCompute(const vector<string>& input, vector<string>& output) {
    log_debug << "--> AbsPrime OpKernel compute.";
    ProtocolManager::Instance()->GetProtocol()->GetOps(msg_id().str())->AbsPrime(input, output, &attrs_);
    log_debug << "AbsPrime OpKernel compute ok. <--";
    return 0;
  }
};

class SecureLogOp : public StrUnaryOp {
 private:
  /* data */
 public:
  SecureLogOp(OpKernelConstruction* context) : StrUnaryOp(context) {}
  ~SecureLogOp() {}

  int UnaryCompute(const vector<string>& input, vector<string>& output) {
    log_debug << "--> Log OpKernel compute.";
    ProtocolManager::Instance()->GetProtocol()->GetOps(msg_id().str())->Log(input, output, &attrs_);
    log_debug << "Log OpKernel compute ok. <--";
    return 0;
  }
};

class SecureHLogOp : public StrUnaryOp {
 private:
  /* data */
 public:
  SecureHLogOp(OpKernelConstruction* context) : StrUnaryOp(context) {}
  ~SecureHLogOp() {}

  int UnaryCompute(const vector<string>& input, vector<string>& output) {
    log_debug << "--> HLog OpKernel compute.";
    ProtocolManager::Instance()->GetProtocol()->GetOps(msg_id().str())->HLog(input, output, &attrs_);
    log_debug << "HLog OpKernel compute ok. <--";
    return 0;
  }
};

class SecureLog1pOp : public StrUnaryOp {
 private:
  /* data */
 public:
  SecureLog1pOp(OpKernelConstruction* context) : StrUnaryOp(context) {}
  ~SecureLog1pOp() {}

  int UnaryCompute(const vector<string>& input, vector<string>& output) {
    log_debug << "--> Log1p OpKernel compute.";
    ProtocolManager::Instance()->GetProtocol()->GetOps(msg_id().str())->Log1p(input, output, &attrs_);
    log_debug << "Log1p OpKernel compute ok. <--";
    return 0;
  }
};

class SecureAddNOp : public SecureOpKernel {
 public:
  SecureAddNOp(OpKernelConstruction* context) : SecureOpKernel(context) {
  }
  ~SecureAddNOp() {}

  void ComputeImpl(OpKernelContext* context) {
    if (!context->ValidateInputsAreSameShape(this)) return;

    const Tensor& input0 = context->input(0);
    const int num = context->num_inputs();

    if (num == 1) {
      context->set_output(0, input0);
      return;
    }

    // log_debug << "=====  add_n input_nums: " << context->num_inputs();
    // log_debug << "          in0.dims():" << input0.dims();
    // log_debug << "    in0_num_elements:" << input0.NumElements();

    Tensor* z = nullptr;
    OP_REQUIRES_OK(context, context->allocate_output(0, input0.shape(), &z));

    int size = input0.NumElements();
    // vectorize with element-wise
    vector<string> inputs;
    for (int idx = 0; idx < num; idx++) {
      auto flat_idx = context->input(idx).flat<string>();
      for (int i = 0; i < size; i++) {
        inputs.push_back(flat_idx(i));
      }
    }

    // fill reduce op attributes
    attrs_["rows"] = std::to_string(num);
    attrs_["cols"] = std::to_string(size);

    // // compute with protocol
    vector<string> output(size);
    ProtocolManager::Instance()->GetProtocol()->GetOps(msg_id().str())->AddN(inputs, output, &attrs_);
    
    auto out_flat = z->flat<string>();
    for (int i = 0; i < size; i++) {
      out_flat(i) = output[i];
    }

#if PRINT_REVEAL
    debug_print_reveal(output, string(" mpc out").c_str());
#endif
    log_debug << "AddN OpKernel compute ok. <--";
    return;
  }
};


class SecureRevealOp : public StrUnaryOp {
 private:
  int receive_party_ = 0;
 public:
  SecureRevealOp(OpKernelConstruction* context) : StrUnaryOp(context) {
    context->GetAttr("receive_party", &receive_party_);
  }
  ~SecureRevealOp() {}

  int UnaryCompute(const vector<string>& input, vector<string>& output) {
    log_debug << "--> Reveal OpKernel compute.";
    attrs_["receive_party"] = std::to_string(receive_party_);
    ProtocolManager::Instance()->GetProtocol()->GetOps(msg_id().str())->Reveal(input, output, &attrs_);
    log_debug << "Reveal OpKernel compute ok. <--";
    return 0;
  }
};

/////////////////////// logical ops //////////////////////////
class SecureLogicalAndOp : public SecureBinaryOp {
 public:
  SecureLogicalAndOp(OpKernelConstruction* context) : SecureBinaryOp(context) {}
  ~SecureLogicalAndOp() {}

  int BinaryCompute(
    const vector<string>& in1, const vector<string>& in2, vector<string>& output) {
    ProtocolManager::Instance()->GetProtocol()->GetOps(msg_id().str())->AND(in1, in2, output, &attrs_);
    return 0;
  }
};

class SecureLogicalOrOp : public SecureBinaryOp {
 public:
  SecureLogicalOrOp(OpKernelConstruction* context) : SecureBinaryOp(context) {}
  ~SecureLogicalOrOp() {}

  int BinaryCompute(
    const vector<string>& in1, const vector<string>& in2, vector<string>& output) {
    ProtocolManager::Instance()->GetProtocol()->GetOps(msg_id().str())->OR(in1, in2, output, &attrs_);
    return 0;
  }
};

class SecureLogicalXorOp : public SecureBinaryOp {
 public:
  SecureLogicalXorOp(OpKernelConstruction* context) : SecureBinaryOp(context) {}
  ~SecureLogicalXorOp() {}

  int BinaryCompute(
    const vector<string>& in1, const vector<string>& in2, vector<string>& output) {
    ProtocolManager::Instance()->GetProtocol()->GetOps(msg_id().str())->XOR(in1, in2, output, &attrs_);
    return 0;
  }
};

class SecureLogicalNotOp : public StrUnaryOp {
 private:
  int receive_party_ = 0;
 public:
  SecureLogicalNotOp(OpKernelConstruction* context) : StrUnaryOp(context) {}
  ~SecureLogicalNotOp() {}

  int UnaryCompute(const vector<string>& input, vector<string>& output) {
    ProtocolManager::Instance()->GetProtocol()->GetOps(msg_id().str())->NOT(input, output, &attrs_);
    return 0;
  }
};

//////////////    register kernels    //////////////
REGISTER_STR_CPU_KERNEL(SecureAdd, SecureAddOp);
REGISTER_STR_CPU_KERNEL(SecureSub, SecureSubOp);
REGISTER_STR_CPU_KERNEL(SecureMul, SecureMulOp);
REGISTER_STR_CPU_KERNEL(SecureDiv, SecureDivOp);
REGISTER_STR_CPU_KERNEL(SecureTruediv, SecureTruedivOp);
REGISTER_STR_CPU_KERNEL(SecureRealdiv, SecureRealdivOp);
REGISTER_STR_CPU_KERNEL(SecureFloordiv, SecureFloordivOp);
REGISTER_STR_CPU_KERNEL(SecureGreater, SecureGreaterOp);
REGISTER_STR_CPU_KERNEL(SecureLess, SecureLessOp);
REGISTER_STR_CPU_KERNEL(SecureEqual, SecureEqualOp);
REGISTER_STR_CPU_KERNEL(SecureNotEqual, SecureNotEqualOp);
REGISTER_STR_CPU_KERNEL(SecureGreaterEqual, SecureGreaterEqualOp);
REGISTER_STR_CPU_KERNEL(SecureLessEqual, SecureLessEqualOp);
REGISTER_STR_CPU_KERNEL(SecurePow, SecurePowOp);

REGISTER_STR_CPU_KERNEL(SecureMatmul, SecureMatmulOp);
REGISTER_STR_CPU_KERNEL(SecureNegative, SecureNegativeOp);
REGISTER_STR_CPU_KERNEL(SecureSquare, SecureSquareOp);
REGISTER_STR_CPU_KERNEL(SecureReduceMean, SecureReduceMeanOp);
REGISTER_STR_CPU_KERNEL(SecureReduceMax, SecureReduceMaxOp);
REGISTER_STR_CPU_KERNEL(SecureReduceMin, SecureReduceMinOp);
REGISTER_STR_CPU_KERNEL(SecureReduceSum, SecureReduceSumOp);
REGISTER_STR_CPU_KERNEL(SecureAbs, SecureAbsOp);
REGISTER_STR_CPU_KERNEL(SecureAbsPrime, SecureAbsPrimeOp);
REGISTER_STR_CPU_KERNEL(SecureLog, SecureLogOp);
REGISTER_STR_CPU_KERNEL(SecureHLog, SecureHLogOp);
REGISTER_STR_CPU_KERNEL(SecureLog1p, SecureLog1pOp);
REGISTER_STR_CPU_KERNEL(SecureReveal, SecureRevealOp);
//REGISTER_STR_CPU_KERNEL(SecureAddN, SecureAddNOp);

REGISTER_KERNEL_BUILDER(Name("SecureAddN").Device(DEVICE_CPU).TypeConstraint<std::string>("T"), \
                        SecureAddNOp);

// logical ops
REGISTER_STR_CPU_KERNEL(SecureLogicalAnd, SecureLogicalAndOp);
REGISTER_STR_CPU_KERNEL(SecureLogicalOr, SecureLogicalOrOp);
REGISTER_STR_CPU_KERNEL(SecureLogicalXor, SecureLogicalXorOp);
REGISTER_STR_CPU_KERNEL(SecureLogicalNot, SecureLogicalNotOp);
} // namespace tensorflow
