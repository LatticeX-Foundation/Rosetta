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
#include "cc/modules/protocol/public/include/protocol_manager.h"
#include "cc/modules/common/include/utils/rtt_logger.h"

using rosetta::ProtocolManager;

// Binary OP: Add/Sub/Mul/Div/...
namespace tensorflow {

class SecureAddOp : public SecureBinaryOp<BinaryOpState> {
 private:
  /* data */
 public:
  SecureAddOp(OpKernelConstruction* context) : SecureBinaryOp(context) {}
  ~SecureAddOp() {}
  int BinaryCompute(
    const vector<string>& in1,
    const vector<string>& in2,
    vector<string>& output,
    OpKernelContext* context) {
    log_debug << "--> Add OpKernel compute.";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Add);
    ProtocolManager::Instance()
      ->GetProtocol(
        ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->Add(in1, in2, output, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Add);
    log_debug << "Add OpKernel compute ok. <--";
    return 0;
  }
};

class SecureSubOp : public SecureBinaryOp<BinaryOpState> {
 private:
  /* data */
 public:
  SecureSubOp(OpKernelConstruction* context) : SecureBinaryOp(context) {}
  ~SecureSubOp() {}

  int BinaryCompute(
    const vector<string>& in1,
    const vector<string>& in2,
    vector<string>& output,
    OpKernelContext* context) {
    log_debug << "--> Sub OpKernel compute.";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Sub);
    ProtocolManager::Instance()
      ->GetProtocol(
        ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->Sub(in1, in2, output, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Sub);
    log_debug << "Sub OpKernel compute ok. <--";
    return 0;
  }
};

class SecureMulOp : public SecureBinaryOp<BinaryOpState> {
 private:
  /* data */
 public:
  SecureMulOp(OpKernelConstruction* context) : SecureBinaryOp(context) {}
  ~SecureMulOp() {}

  int BinaryCompute(
    const vector<string>& in1,
    const vector<string>& in2,
    vector<string>& output,
    OpKernelContext* context) {
    log_debug << "--> Mul OpKernel compute.";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Mul);
    ProtocolManager::Instance()
      ->GetProtocol(
        ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->Mul(in1, in2, output, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Mul);
    log_debug << "Mul OpKernel compute ok. <--";
    return 0;
  }
};

class SecureLessOp : public SecureBinaryOp<BinaryOpState> {
 private:
  /* data */
 public:
  SecureLessOp(OpKernelConstruction* context) : SecureBinaryOp(context) {}
  ~SecureLessOp() {}

  int BinaryCompute(
    const vector<string>& in1,
    const vector<string>& in2,
    vector<string>& output,
    OpKernelContext* context) {
    log_debug << "--> Less OpKernel compute.";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Less);
    ProtocolManager::Instance()
      ->GetProtocol(
        ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->Less(in1, in2, output, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Less);
    log_debug << "Less OpKernel compute ok. <--";
    return 0;
  }
};

class SecureLessEqualOp : public SecureBinaryOp<BinaryOpState> {
 private:
  /* data */
 public:
  SecureLessEqualOp(OpKernelConstruction* context) : SecureBinaryOp(context) {}
  ~SecureLessEqualOp() {}

  int BinaryCompute(
    const vector<string>& in1,
    const vector<string>& in2,
    vector<string>& output,
    OpKernelContext* context) {
    log_debug << "--> LessEqual OpKernel compute.";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(LessEqual);
    ProtocolManager::Instance()
      ->GetProtocol(
        ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->LessEqual(in1, in2, output, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(LessEqual);
    log_debug << "LessEqual OpKernel compute ok. <--";
    return 0;
  }
};

class SecureEqualOp : public SecureBinaryOp<BinaryOpState> {
 private:
  /* data */
 public:
  SecureEqualOp(OpKernelConstruction* context) : SecureBinaryOp(context) {}
  ~SecureEqualOp() {}

  int BinaryCompute(
    const vector<string>& in1,
    const vector<string>& in2,
    vector<string>& output,
    OpKernelContext* context) {
    log_debug << "--> Equal OpKernel compute.";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Equal);
    ProtocolManager::Instance()
      ->GetProtocol(
        ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->Equal(in1, in2, output, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Equal);
    log_debug << "Equal OpKernel compute ok. <--";
    return 0;
  }
};

class SecureNotEqualOp : public SecureBinaryOp<BinaryOpState> {
 private:
  /* data */
 public:
  SecureNotEqualOp(OpKernelConstruction* context) : SecureBinaryOp(context) {}
  ~SecureNotEqualOp() {}

  int BinaryCompute(
    const vector<string>& in1,
    const vector<string>& in2,
    vector<string>& output,
    OpKernelContext* context) {
    log_debug << "--> NotEqual OpKernel compute.";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(NotEqual);
    ProtocolManager::Instance()
      ->GetProtocol(
        ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->NotEqual(in1, in2, output, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(NotEqual);
    log_debug << "NotEqual OpKernel compute ok. <--";
    return 0;
  }
};

class SecureGreaterOp : public SecureBinaryOp<BinaryOpState> {
 private:
  /* data */
 public:
  SecureGreaterOp(OpKernelConstruction* context) : SecureBinaryOp(context) {}
  ~SecureGreaterOp() {}

  int BinaryCompute(
    const vector<string>& in1,
    const vector<string>& in2,
    vector<string>& output,
    OpKernelContext* context) {
    log_debug << "--> Greater OpKernel compute.";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Greater);
    ProtocolManager::Instance()
      ->GetProtocol(
        ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->Greater(in1, in2, output, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Greater);
    log_debug << "Greater OpKernel compute ok. <--";
    return 0;
  }
};

class SecureGreaterEqualOp : public SecureBinaryOp<BinaryOpState> {
 private:
  /* data */
 public:
  SecureGreaterEqualOp(OpKernelConstruction* context) : SecureBinaryOp(context) {}
  ~SecureGreaterEqualOp() {}

  int BinaryCompute(
    const vector<string>& in1,
    const vector<string>& in2,
    vector<string>& output,
    OpKernelContext* context) {
    log_debug << "--> GreaterEqual OpKernel compute.";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(GreaterEqual);
    ProtocolManager::Instance()
      ->GetProtocol(
        ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->GreaterEqual(in1, in2, output, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(GreaterEqual);
    log_debug << "GreaterEqual OpKernel compute ok. <--";
    return 0;
  }
};

class SecureDivOp : public SecureBinaryOp<BinaryOpState> {
 private:
  /* data */
 public:
  SecureDivOp(OpKernelConstruction* context) : SecureBinaryOp(context) {}
  ~SecureDivOp() {}

  int BinaryCompute(
    const vector<string>& in1,
    const vector<string>& in2,
    vector<string>& output,
    OpKernelContext* context) {
    log_debug << "--> Div OpKernel compute.";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Div);
    ProtocolManager::Instance()
      ->GetProtocol(
        ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->Div(in1, in2, output, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Div);
    log_debug << "Div OpKernel compute ok. <--";
    return 0;
  }
};

class SecureReciprocaldivOp : public SecureBinaryOp<BinaryOpState> {
 private:
  /* data */
 public:
  SecureReciprocaldivOp(OpKernelConstruction* context) : SecureBinaryOp(context) {}
  ~SecureReciprocaldivOp() {}

  int BinaryCompute(
    const vector<string>& in1,
    const vector<string>& in2,
    vector<string>& output,
    OpKernelContext* context) {
    log_debug << "--> Reciprocaldiv OpKernel compute.";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Reciprocaldiv);
    ProtocolManager::Instance()
      ->GetProtocol(
        ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->Reciprocaldiv(in1, in2, output, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Reciprocaldiv);
    log_debug << "Reciprocaldiv OpKernel compute ok. <--";
    return 0;
  }
};

class SecureTruedivOp : public SecureBinaryOp<BinaryOpState> {
 private:
  /* data */
 public:
  SecureTruedivOp(OpKernelConstruction* context) : SecureBinaryOp(context) {}
  ~SecureTruedivOp() {}

  int BinaryCompute(
    const vector<string>& in1,
    const vector<string>& in2,
    vector<string>& output,
    OpKernelContext* context) {
    log_debug << "--> Truediv OpKernel compute.";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Truediv);
    ProtocolManager::Instance()
      ->GetProtocol(
        ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->Truediv(in1, in2, output, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Truediv);
    log_debug << "Truediv OpKernel compute ok. <--";
    return 0;
  }
};

class SecureFloordivOp : public SecureBinaryOp<BinaryOpState> {
 private:
  /* data */
 public:
  SecureFloordivOp(OpKernelConstruction* context) : SecureBinaryOp(context) {}
  ~SecureFloordivOp() {}

  int BinaryCompute(
    const vector<string>& in1,
    const vector<string>& in2,
    vector<string>& output,
    OpKernelContext* context) {
    log_debug << "--> Floordiv OpKernel compute.";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Floordiv);
    ProtocolManager::Instance()
      ->GetProtocol(
        ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->Floordiv(in1, in2, output, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Floordiv);
    log_debug << "Floordiv OpKernel compute ok. <--";
    return 0;
  }
};

// equivelent to SecureDivOp
class SecureRealdivOp : public SecureBinaryOp<BinaryOpState> {
 private:
  /* data */
 public:
  SecureRealdivOp(OpKernelConstruction* context) : SecureBinaryOp(context) {}
  ~SecureRealdivOp() {}

  int BinaryCompute(
    const vector<string>& in1,
    const vector<string>& in2,
    vector<string>& output,
    OpKernelContext* context) {
    log_debug << "--> Realdiv OpKernel compute.";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Div);
    ProtocolManager::Instance()
      ->GetProtocol(
        ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->Div(in1, in2, output, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Div);
    log_debug << "Realdiv OpKernel compute ok. <--";
    return 0;
  }
};

class SecurePowOp : public SecureBinaryOp<BinaryOpState> {
 private:
  /* data */
 public:
  SecurePowOp(OpKernelConstruction* context) : SecureBinaryOp(context) {}
  ~SecurePowOp() {}

  int BinaryCompute(
    const vector<string>& in1,
    const vector<string>& in2,
    vector<string>& output,
    OpKernelContext* context) {
    // lh_is_const, rh_is_const SHOULD set to False, True
    // if ((!lh_is_const_) && rh_is_const_)
    // {
    //   log_debug << "!! Pow OpKernel should set: lh_is_const=False, rh_is_const=True" ;
    //   return -1;
    // }

    log_debug << "--> Pow OpKernel compute.";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Pow);
    ProtocolManager::Instance()
      ->GetProtocol(
        ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->Pow(in1, in2, output, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Pow);
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
    // OP_REQUIRES_OK(context, context->GetAttr("rh_is_const", &rh_is_const_));
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
    // attrs_["rh_is_const"] = rh_is_const_ ? "1" : "0";

    log_debug << "**Matmul m: " << m << ", K: " << K << ", n: " << n
              << ", transpose_a: " << transpose_a_ << ", transpose_b: " << transpose_b_;

    // call protocol ops
    vector<string> outstr(m * n);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Matmul);
    ProtocolManager::Instance()
      ->GetProtocol(
        ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->Matmul(in1, in2, outstr, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Matmul);

    // set output
    TensorShape output_shape({m, n});
    Tensor* output = nullptr;
    OP_REQUIRES_OK(context, context->allocate_output(0, output_shape, &output));

    auto flat_out = output->flat<string>();
    for (int i = 0; i < outstr.size(); ++i) {
      flat_out(i) = outstr[i];
    }
#if PRINT_REVEAL
    debug_print_reveal(outstr, string(" mpc out").c_str(), context);
#endif
    log_debug << "Matmul OpKernel compute ok. <--";
    return;
  }

 private:
  bool transpose_a_ = false;
  bool transpose_b_ = false;
  bool rh_is_const_ = false;
};

class SecureSquareOp : public SecureUnaryOp {
 private:
  /* data */
 public:
  SecureSquareOp(OpKernelConstruction* context) : SecureUnaryOp(context) {}
  ~SecureSquareOp() {}

  int UnaryCompute(const vector<string>& input, vector<string>& output, OpKernelContext* context) {
    log_debug << "--> Square OpKernel compute.";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Square);
    ProtocolManager::Instance()
      ->GetProtocol(
        ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->Square(input, output, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Square);
    log_debug << "Square OpKernel compute ok. <--";
    return 0;
  }
};

class SecureNegativeOp : public SecureUnaryOp {
 private:
  /* data */
 public:
  SecureNegativeOp(OpKernelConstruction* context) : SecureUnaryOp(context) {}
  ~SecureNegativeOp() {}

  int UnaryCompute(const vector<string>& input, vector<string>& output, OpKernelContext* context) {
    log_debug << "--> Negative OpKernel compute.";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Negative);
    ProtocolManager::Instance()
      ->GetProtocol(
        ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->Negative(input, output, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Negative);
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
  string name() { return string("SecureReduceMeanOp"); }
  int ReduceCompute(
    const vector<string>& input,
    vector<string>& output,
    int rows,
    int cols,
    OpKernelContext* context) {
    log_debug << "--> ReduceMean OpKernel compute.";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Mean);
    ProtocolManager::Instance()
      ->GetProtocol(
        ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->Mean(input, output, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Mean);
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
  string name() { return string("SecureReduceSumOp"); }
  int ReduceCompute(
    const vector<string>& input,
    vector<string>& output,
    int rows,
    int cols,
    OpKernelContext* context) {
    log_debug << "--> ReduceSum OpKernel compute.";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Sum);
    ProtocolManager::Instance()
      ->GetProtocol(
        ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->Sum(input, output, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Sum);
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
  string name() { return string("SecureReduceMinOp"); }
  int ReduceCompute(
    const vector<string>& input,
    vector<string>& output,
    int rows,
    int cols,
    OpKernelContext* context) {
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Min);
    log_debug << "--> ReduceMin OpKernel compute.";
    ProtocolManager::Instance()
      ->GetProtocol(
        ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->Min(input, output, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Min);
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

  string name() { return string("SecureReduceMaxOp"); }
  int ReduceCompute(
    const vector<string>& input,
    vector<string>& output,
    int rows,
    int cols,
    OpKernelContext* context) {
    log_debug << "--> ReduceMax OpKernel compute.";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Max);
    ProtocolManager::Instance()
      ->GetProtocol(
        ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->Max(input, output, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Max);
    log_debug << "ReduceMax OpKernel compute ok. <--";
    return 0;
  }
};

class SecureArgMaxOp : public SecureOpKernel {
 public:
  explicit SecureArgMaxOp(OpKernelConstruction* context) : SecureOpKernel(context) {
    OP_REQUIRES_OK(context, context->GetAttr("Tidx", &index_type_));
  }

  void ComputeImpl(OpKernelContext* context) override {
    log_debug << "--> SecureArgMaxOp OpKernel compute.";
    // 获取输入张量
    const Tensor& input = context->input(0);
    const Tensor& dimension = context->input(1);

    // 验证dimension是标量
    OP_REQUIRES(
      context, TensorShapeUtils::IsScalar(dimension.shape()),
      errors::InvalidArgument("dimension must be scalar"));

    // 获取axis值
    int64 axis = 0;
    if (index_type_ == DT_INT32) {
      axis = static_cast<int64>(dimension.scalar<int32>()());
    } else {
      axis = dimension.scalar<int64>()();
    }

    const TensorShape& input_shape = input.shape();

    // 处理负数轴
    int64 real_axis = axis < 0 ? axis + input_shape.dims() : axis;

    // 验证轴的有效性
    OP_REQUIRES(
      context, real_axis >= 0 && real_axis < input_shape.dims(),
      errors::InvalidArgument(
        "axis must be in range [", -input_shape.dims(), ", ", input_shape.dims() - 1, "]"));

    // 计算输出形状
    TensorShape output_shape;
    for (int i = 0; i < input_shape.dims(); ++i) {
      if (i != real_axis) {
        output_shape.AddDim(input_shape.dim_size(i));
      }
    }

    // 分配输出张量
    Tensor* output = nullptr;
    OP_REQUIRES_OK(context, context->allocate_output(0, output_shape, &output));

    // 获取输入输出数据
    auto input_data = input.flat<string>();
    auto output_data = output->flat<string>();

    // 获取SNN操作集
    auto snn_ops = ProtocolManager::Instance()
                     ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(
                       context->device()->attributes().incarnation()))
                     ->GetOps(msg_id());

    // 计算各维度大小
    int64 outer_size = 1;
    for (int i = 0; i < real_axis; ++i) {
      outer_size *= input_shape.dim_size(i);
    }

    int64 inner_size = 1;
    for (int i = real_axis + 1; i < input_shape.dims(); ++i) {
      inner_size *= input_shape.dim_size(i);
    }

    int64 axis_dim_size = input_shape.dim_size(real_axis);

    // 执行argmax计算
    for (int64 i = 0; i < outer_size; ++i) {
      for (int64 j = 0; j < inner_size; ++j) {
        // 收集需要比较的值
        vector<string> values(axis_dim_size);
        for (int64 k = 0; k < axis_dim_size; ++k) {
          const int64 index = i * axis_dim_size * inner_size + k * inner_size + j;
          values[k] = input_data(index);
        }

        // 初始化最大值索引和最大值
        vector<string> max_index = {std::to_string(0)};
        vector<string> max_value = {values[0]};

        for (int64 k = 1; k < axis_dim_size; ++k) {
          vector<string> curr_value = {values[k]};
          vector<string> comparison_result;

          // Greater比较
          SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Greater);
          snn_ops->Greater(curr_value, max_value, comparison_result, &attrs_);
          SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Greater);

          // 条件更新逻辑
          vector<string> new_index = {std::to_string(k)};
          vector<string> keep_old;
          SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Mul);
          snn_ops->Mul(comparison_result, new_index, keep_old, &attrs_);
          SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Mul);

          vector<string> keep_new;
          vector<string> one = {std::to_string(1)};
          SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Sub);
          snn_ops->Sub(one, comparison_result, keep_new, &attrs_);
          SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Sub);

          vector<string> temp_max_index;
          SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Mul);
          snn_ops->Mul(keep_new, max_index, temp_max_index, &attrs_);
          SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Mul);

          SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Add);
          snn_ops->Add(temp_max_index, keep_old, max_index, &attrs_);
          SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Add);

          // 更新最大值
          vector<string> temp_value;
          SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Mul);
          snn_ops->Mul(comparison_result, curr_value, keep_old, &attrs_);
          SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Mul);

          SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Mul);
          snn_ops->Mul(keep_new, max_value, temp_value, &attrs_);
          SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Mul);

          SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Add);
          snn_ops->Add(temp_value, keep_old, max_value, &attrs_);
          SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Add);
        }

        // 存储结果
        auto out_flat = output->flat<string>();
        out_flat(i * inner_size + j) = std::move(max_index[0]);
      }
    }
    log_debug << "SecureOp OpKernel compute ok. <--";
  }

 private:
  DataType index_type_;
};

class SecureArgMinOp : public SecureOpKernel {
 public:
  explicit SecureArgMinOp(OpKernelConstruction* context) : SecureOpKernel(context) {
    OP_REQUIRES_OK(context, context->GetAttr("Tidx", &index_type_));
  }

  void ComputeImpl(OpKernelContext* context) override {
    log_debug << "--> SecureArgMinOp OpKernel compute.";
    // 获取输入张量
    const Tensor& input = context->input(0);
    const Tensor& dimension = context->input(1);

    // 验证dimension是标量
    OP_REQUIRES(
      context, TensorShapeUtils::IsScalar(dimension.shape()),
      errors::InvalidArgument("dimension must be scalar"));

    // std::cout << "input tensor contents:" << std::endl;
    // auto shape_flat = input.flat<string>();
    // for (int i = 0; i < shape_flat.size(); ++i) {
    //   std::cout << "Element" << i << ": " << shape_flat(i) << std::endl;
    // }

    // 获取axis值
    int64 axis = 0;
    if (index_type_ == DT_INT32) {
      axis = static_cast<int64>(dimension.scalar<int32>()());
    } else {
      axis = dimension.scalar<int64>()();
    }

    const TensorShape& input_shape = input.shape();

    // 处理负数轴
    int64 real_axis = axis < 0 ? axis + input_shape.dims() : axis;

    // 验证轴的有效性
    OP_REQUIRES(
      context, real_axis >= 0 && real_axis < input_shape.dims(),
      errors::InvalidArgument(
        "axis must be in range [", -input_shape.dims(), ", ", input_shape.dims() - 1, "]"));

    // 计算输出形状
    TensorShape output_shape;
    for (int i = 0; i < input_shape.dims(); ++i) {
      if (i != real_axis) {
        output_shape.AddDim(input_shape.dim_size(i));
      }
    }

    // 分配输出张量
    Tensor* output = nullptr;
    OP_REQUIRES_OK(context, context->allocate_output(0, output_shape, &output));

    // 获取输入输出数据
    auto input_data = input.flat<string>();
    auto output_data = output->flat<string>();

    // 获取SNN操作集
    auto snn_ops = ProtocolManager::Instance()
                     ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(
                       context->device()->attributes().incarnation()))
                     ->GetOps(msg_id());

    // 计算各维度大小
    int64 outer_size = 1;
    for (int i = 0; i < real_axis; ++i) {
      outer_size *= input_shape.dim_size(i);
    }

    int64 inner_size = 1;
    for (int i = real_axis + 1; i < input_shape.dims(); ++i) {
      inner_size *= input_shape.dim_size(i);
    }

    int64 axis_dim_size = input_shape.dim_size(real_axis);

    // 执行argmin计算
    for (int64 i = 0; i < outer_size; ++i) {
      for (int64 j = 0; j < inner_size; ++j) {
        // 收集需要比较的值
        vector<string> values(axis_dim_size);
        for (int64 k = 0; k < axis_dim_size; ++k) {
          const int64 index = i * axis_dim_size * inner_size + k * inner_size + j;
          values[k] = input_data(index);
        }

        // 初始化最大值索引和最大值
        vector<string> max_index = {std::to_string(0)};
        vector<string> max_value = {values[0]};

        for (int64 k = 1; k < axis_dim_size; ++k) {
          vector<string> curr_value = {values[k]};
          vector<string> comparison_result;

          // Greater比较
          SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Less);
          snn_ops->Less(curr_value, max_value, comparison_result, &attrs_);
          SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Less);

          // 条件更新逻辑
          vector<string> new_index = {std::to_string(k)};
          vector<string> keep_old;
          SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Mul);
          snn_ops->Mul(comparison_result, new_index, keep_old, &attrs_);
          SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Mul);

          vector<string> keep_new;
          vector<string> one = {std::to_string(1)};
          SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Sub);
          snn_ops->Sub(one, comparison_result, keep_new, &attrs_);
          SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Sub);

          vector<string> temp_max_index;
          SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Mul);
          snn_ops->Mul(keep_new, max_index, temp_max_index, &attrs_);
          SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Mul);

          SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Add);
          snn_ops->Add(temp_max_index, keep_old, max_index, &attrs_);
          SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Add);

          // 更新最大值
          vector<string> temp_value;
          SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Mul);
          snn_ops->Mul(comparison_result, curr_value, keep_old, &attrs_);
          SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Mul);

          SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Mul);
          snn_ops->Mul(keep_new, max_value, temp_value, &attrs_);
          SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Mul);

          SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Add);
          snn_ops->Add(temp_value, keep_old, max_value, &attrs_);
          SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Add);
        }

        // 存储结果
        auto out_flat = output->flat<string>();
        out_flat(i * inner_size + j) = std::move(max_index[0]);
      }
    }
    log_debug << "SecureArgMinOp OpKernel compute ok. <--";
  }

 private:
  DataType index_type_;
};

class SecureExpOp : public SecureUnaryOp {
 private:
  /* data */
 public:
  SecureExpOp(OpKernelConstruction* context) : SecureUnaryOp(context) {}
  ~SecureExpOp() {}

  int UnaryCompute(const vector<string>& input, vector<string>& output, OpKernelContext* context) {
    log_debug << "--> Exp OpKernel compute.";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Exp);
    ProtocolManager::Instance()
      ->GetProtocol(
        ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->Exp(input, output, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Exp);
    log_debug << "Exp OpKernel compute ok. <--";
    return 0;
  }
};

class SecureRsqrtOp : public SecureUnaryOp {
 private:
  /* data */
 public:
  SecureRsqrtOp(OpKernelConstruction* context) : SecureUnaryOp(context) {}
  ~SecureRsqrtOp() {}

  int UnaryCompute(const vector<string>& input, vector<string>& output, OpKernelContext* context) {
    log_debug << "--> Rsqrt OpKernel compute.";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Rsqrt);
    ProtocolManager::Instance()
      ->GetProtocol(
        ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->Rsqrt(input, output, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Rsqrt);
    log_debug << "Rsqrt OpKernel compute ok. <--";
    return 0;
  }
};

class SecureSqrtOp : public SecureUnaryOp {
 private:
  /* data */
 public:
  SecureSqrtOp(OpKernelConstruction* context) : SecureUnaryOp(context) {}
  ~SecureSqrtOp() {}

  int UnaryCompute(const vector<string>& input, vector<string>& output, OpKernelContext* context) {
    log_debug << "--> Sqrt OpKernel compute.";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Sqrt);
    ProtocolManager::Instance()
      ->GetProtocol(
        ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->Sqrt(input, output, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Sqrt);
    log_debug << "Sqrt OpKernel compute ok. <--";
    return 0;
  }
};

class SecureAbsOp : public SecureUnaryOp {
 private:
  /* data */
 public:
  SecureAbsOp(OpKernelConstruction* context) : SecureUnaryOp(context) {}
  ~SecureAbsOp() {}

  int UnaryCompute(const vector<string>& input, vector<string>& output, OpKernelContext* context) {
    log_debug << "--> Abs OpKernel compute.";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Abs);
    ProtocolManager::Instance()
      ->GetProtocol(
        ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->Abs(input, output, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Abs);
    log_debug << "Abs OpKernel compute ok. <--";
    return 0;
  }
};

class SecureAbsPrimeOp : public SecureUnaryOp {
 private:
  /* data */
 public:
  SecureAbsPrimeOp(OpKernelConstruction* context) : SecureUnaryOp(context) {}
  ~SecureAbsPrimeOp() {}

  int UnaryCompute(const vector<string>& input, vector<string>& output, OpKernelContext* context) {
    log_debug << "--> AbsPrime OpKernel compute.";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(AbsPrime);
    ProtocolManager::Instance()
      ->GetProtocol(
        ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->AbsPrime(input, output, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(AbsPrime);
    log_debug << "AbsPrime OpKernel compute ok. <--";
    return 0;
  }
};

class SecureLogOp : public SecureUnaryOp {
 private:
  /* data */
 public:
  SecureLogOp(OpKernelConstruction* context) : SecureUnaryOp(context) {}
  ~SecureLogOp() {}

  int UnaryCompute(const vector<string>& input, vector<string>& output, OpKernelContext* context) {
    log_debug << "--> Log OpKernel compute.";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Log);
    ProtocolManager::Instance()
      ->GetProtocol(
        ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->Log(input, output, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Log);
    log_debug << "Log OpKernel compute ok. <--";
    return 0;
  }
};

class SecureHLogOp : public SecureUnaryOp {
 private:
  /* data */
 public:
  SecureHLogOp(OpKernelConstruction* context) : SecureUnaryOp(context) {}
  ~SecureHLogOp() {}

  int UnaryCompute(const vector<string>& input, vector<string>& output, OpKernelContext* context) {
    log_debug << "--> HLog OpKernel compute.";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(HLog);
    ProtocolManager::Instance()
      ->GetProtocol(
        ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->HLog(input, output, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(HLog);
    log_debug << "HLog OpKernel compute ok. <--";
    return 0;
  }
};

class SecureLog1pOp : public SecureUnaryOp {
 private:
  /* data */
 public:
  SecureLog1pOp(OpKernelConstruction* context) : SecureUnaryOp(context) {}
  ~SecureLog1pOp() {}

  int UnaryCompute(const vector<string>& input, vector<string>& output, OpKernelContext* context) {
    log_debug << "--> Log1p OpKernel compute.";
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Log1p);
    ProtocolManager::Instance()
      ->GetProtocol(
        ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->Log1p(input, output, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Log1p);
    log_debug << "Log1p OpKernel compute ok. <--";
    return 0;
  }
};

class SecureAddNOp : public SecureOpKernel {
 public:
  SecureAddNOp(OpKernelConstruction* context) : SecureOpKernel(context) {}
  ~SecureAddNOp() {}

  void ComputeImpl(OpKernelContext* context) {
    if (!context->ValidateInputsAreSameShape(this))
      return;

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
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(AddN);
    ProtocolManager::Instance()
      ->GetProtocol(
        ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->AddN(inputs, output, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(AddN);

    auto out_flat = z->flat<string>();
    for (int i = 0; i < size; i++) {
      out_flat(i) = std::move(output[i]);
    }

#if PRINT_REVEAL
    debug_print_reveal(output, string(" mpc out").c_str(), context);
#endif
    log_debug << "AddN OpKernel compute ok. <--";
    return;
  }
};

class SecureRevealOp : public SecureUnaryOp {
 private:
  string receive_parties_;

 public:
  SecureRevealOp(OpKernelConstruction* context) : SecureUnaryOp(context) {
    context->GetAttr("receive_parties", &receive_parties_);
  }
  ~SecureRevealOp() {}

  int UnaryCompute(const vector<string>& input, vector<string>& output, OpKernelContext* context) {
    log_debug << "--> Reveal OpKernel compute.";
    attrs_["receive_parties"] = receive_parties_;
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Reveal);
    ProtocolManager::Instance()
      ->GetProtocol(
        ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->Reveal(input, output, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Reveal);
    log_debug << "Reveal OpKernel compute ok. <--";
    return 0;
  }
};

/////////////////////// logical ops //////////////////////////
class SecureLogicalAndOp : public SecureBinaryOp<BinaryOpState> {
 public:
  SecureLogicalAndOp(OpKernelConstruction* context) : SecureBinaryOp(context) {}
  ~SecureLogicalAndOp() {}

  int BinaryCompute(
    const vector<string>& in1,
    const vector<string>& in2,
    vector<string>& output,
    OpKernelContext* context) {
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(AND);
    ProtocolManager::Instance()
      ->GetProtocol(
        ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->AND(in1, in2, output, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(AND);
    return 0;
  }
};

class SecureLogicalOrOp : public SecureBinaryOp<BinaryOpState> {
 public:
  SecureLogicalOrOp(OpKernelConstruction* context) : SecureBinaryOp(context) {}
  ~SecureLogicalOrOp() {}

  int BinaryCompute(
    const vector<string>& in1,
    const vector<string>& in2,
    vector<string>& output,
    OpKernelContext* context) {
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(OR);
    ProtocolManager::Instance()
      ->GetProtocol(
        ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->OR(in1, in2, output, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(OR);
    return 0;
  }
};

class SecureLogicalXorOp : public SecureBinaryOp<BinaryOpState> {
 public:
  SecureLogicalXorOp(OpKernelConstruction* context) : SecureBinaryOp(context) {}
  ~SecureLogicalXorOp() {}

  int BinaryCompute(
    const vector<string>& in1,
    const vector<string>& in2,
    vector<string>& output,
    OpKernelContext* context) {
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(XOR);
    ProtocolManager::Instance()
      ->GetProtocol(
        ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->XOR(in1, in2, output, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(XOR);
    return 0;
  }
};

class SecureLogicalNotOp : public SecureUnaryOp {
 public:
  SecureLogicalNotOp(OpKernelConstruction* context) : SecureUnaryOp(context) {}
  ~SecureLogicalNotOp() {}

  int UnaryCompute(const vector<string>& input, vector<string>& output, OpKernelContext* context) {
    SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(NOT);
    ProtocolManager::Instance()
      ->GetProtocol(
        ProtocolManager::Instance()->QueryMappingID(context->device()->attributes().incarnation()))
      ->GetOps(msg_id())
      ->NOT(input, output, &attrs_);
    SECURE_OP_CALL_PROTOCOL_OP_STATS_END(NOT);
    return 0;
  }
};

class SecureOnesOp : public SecureOpKernel {
 public:
  explicit SecureOnesOp(OpKernelConstruction* context) : SecureOpKernel(context) {}
  // ~SecureOnesOp() {}

  void ComputeImpl(OpKernelContext* context) {
    log_debug << "--> SecureOnes OpKernel compute.";

    const Tensor& shape = context->input(0);
    OP_REQUIRES(
      context, shape.dims() == 1, errors::InvalidArgument("Shape input must be a 1D tensor."));

    auto shape_flat = shape.flat<string>();
    std::vector<int64> shape_values(shape_flat.size());
    for (int i = 0; i < shape_flat.size(); ++i) {
      // const std::string shape_element = shape_flat(i);
      // std::cout << "Parsing shape element: " << shape_element << std::endl;
      // 将每个字符串转换为 int64 类型
      // shape_values.push_back(std::stoll(shape_element));
      shape_values[i] = std::stoll((shape_flat(i)));
    }

    TensorShape output_shape;
    for (int64 dim : shape_values) {
      output_shape.AddDim(dim);
    }

    Tensor* output_tensor = nullptr;
    OP_REQUIRES_OK(context, context->allocate_output(0, output_shape, &output_tensor));

    // auto out_flat = output->flat<string>();
    // string ones_value = "1";
    // for (int i = 0; i < out_flat.size(); ++i) {
    //   out_flat(i) = ones_value;
    // }

    auto output_flat = output_tensor->flat<std::string>();
    std::fill(output_flat.data(), output_flat.data() + output_flat.size(), "1");

    log_debug << "SecureOnes OpKernel compute ok. <--";
  }
};

class SecureZerosOp : public SecureOpKernel {
 public:
  explicit SecureZerosOp(OpKernelConstruction* context) : SecureOpKernel(context) {}

  void ComputeImpl(OpKernelContext* context) {
    log_debug << "--> SecureZeros OpKernel compute.";

    const Tensor& shape = context->input(0);
    OP_REQUIRES(
      context, shape.dims() == 1, errors::InvalidArgument("Shape input must be a 1D tensor."));

    auto shape_flat = shape.flat<string>();
    std::vector<int64> shape_values(shape_flat.size());
    for (int i = 0; i < shape_flat.size(); ++i) {
      shape_values[i] = std::stoll((shape_flat(i)));
    }
    TensorShape output_shape;
    for (int64 dim : shape_values) {
      output_shape.AddDim(dim);
    }
    Tensor* output_tensor = nullptr;
    OP_REQUIRES_OK(context, context->allocate_output(0, output_shape, &output_tensor));
    auto output_flat = output_tensor->flat<std::string>();
    std::fill(output_flat.data(), output_flat.data() + output_flat.size(), "0");
    log_debug << "SecureOnes OpKernel compute ok. <--";
  }
};

class SecureCountUpToOp : public SecureOpKernel {
 public:
  explicit SecureCountUpToOp(OpKernelConstruction* context) : SecureOpKernel(context) {
    // OP_REQUIRES_OK(context, context->GetAttr("T", &T));
    OP_REQUIRES_OK(context, context->GetAttr("limit", &limit_));
  }

  void ComputeImpl(OpKernelContext* context) override {
    string before_increment;
    {
      // std::cout << " lock " << std::endl;
      // mutex_lock l(*context->input_ref_mutex(0));
      // Tensor ref_tensor = context->mutable_input(0, true);
      // Tensor* ref_tensor = nullptr;
      // OP_REQUIRES_OK(context, context->mutable_input(0, &ref_tensor));

      const Tensor& ref_tensor = context->input(0);

      OP_REQUIRES(
        context, TensorShapeUtils::IsScalar(ref_tensor.shape()),
        errors::InvalidArgument("ref must be a scalar Variable"));

      auto ref_scalar = ref_tensor.scalar<string>()();
      int64 ref_value = std::stoll(ref_scalar);
      before_increment = std::to_string(ref_value);

      if (ref_value >= limit_) {
        context->SetStatus(errors::OutOfRange("Reached limit of ", limit_));
        return;
      }

      int64 new_value = ref_value + 1;
      // ref_tensor.scalar<string>()() = std::to_string(new_value);
    }

    Tensor* output_tensor = nullptr;
    OP_REQUIRES_OK(context, context->allocate_output(0, TensorShape({}), &output_tensor));
    output_tensor->scalar<string>()() = before_increment;
  }

 private:
  int64 limit_;
};

class SecureSignOp : public SecureOpKernel {
 private:
  /*data*/
 public:
  SecureSignOp(OpKernelConstruction* context) : SecureOpKernel(context) {}
  ~SecureSignOp() {}

  void ComputeImpl(OpKernelContext* context) {
    log_debug << "--> SecureSignOp Opkernel Compute.";

    const Tensor& logits_in = context->input(0);
    OP_REQUIRES(
      context, TensorShapeUtils::IsVectorOrHigher(logits_in.shape()),
      errors::InvalidArgument(
        "logits must have >= 1 dimension, got ", logits_in.shape().DebugString()));

    Tensor* sign_out = nullptr;
    OP_REQUIRES_OK(
      context, context->forward_input_or_allocate_output({0}, 0, logits_in.shape(), &sign_out));

    std::cout << "Logits_in.NumElements(): " << logits_in.NumElements() << std::endl;
    const auto& logits_in_flat = logits_in.flat<string>();
    const TensorShape& logits_in_shape = logits_in.shape();
    std::cout << "Logits_in_shape.num_elements(): " << logits_in_shape.num_elements() << std::endl;

    // for (int64_t i = 0; i < logits_in_shape.dims(); ++i) {
    //   std::cout << "Logits_in dim" << i << ":" << logits_in_shape.dim_size(i) << std::endl;
    // }

    if (logits_in.NumElements() == 0) {
      return;
    }

    int64_t dims = logits_in_shape.dims();
    if (dims <= 0 || dims > 2) {
      // std::cout << "Not Supported dims: " << dims << std::endl;
      log_error << "not supported dims: " << dims;
      OP_REQUIRES(
        context, false,
        errors::InvalidArgument("not supported dims, got ", logits_in.shape().DebugString()));
    }

    int64_t rows = 0, cols = 0;
    if (dims == 1) {
      rows = 1;
      cols = logits_in_shape.dim_size(0);
    } else if (dims == 2) {
      rows = logits_in_shape.dim_size(0);
      cols = logits_in_shape.dim_size(1);
    }
    std::cout << "rows: " << rows << ", cols: " << cols << std::endl;

    auto snn_ops = ProtocolManager::Instance()
                     ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(
                       context->device()->attributes().incarnation()))
                     ->GetOps(msg_id());

    // vector<string> sign_value(rows * cols);
    for (int i = 0; i < rows; ++i) {
      for (int j = 0; j < cols; ++j) {
        int index = i * cols + j;
        // sign_value[index] = logits_in_flat(index);
        vector<string> sign_value = {logits_in_flat(index)};

        const vector<string> neg_one = {std::to_string(-1)};
        const vector<string> one_value = {std::to_string(1)};
        const vector<string> zero_value = {std::to_string(0)};
        vector<string> comp_res1, comp_res2;
        vector<string> diff1, diff2;
        vector<string> prod1, prod2, prod3;
        vector<string> final_res;

        SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Greater);
        snn_ops->Greater(sign_value, zero_value, comp_res1, &attrs_);
        SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Greater);

        SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Equal);
        snn_ops->Equal(sign_value, zero_value, comp_res2, &attrs_);
        SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Equal);

        SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Sub);
        snn_ops->Sub(one_value, comp_res1, diff1, &attrs_);
        SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Sub);

        SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Sub);
        snn_ops->Sub(one_value, comp_res2, diff2, &attrs_);
        SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Sub);

        SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Mul);
        snn_ops->Mul(diff1, diff2, prod1, &attrs_);
        SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Mul);

        SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Mul);
        snn_ops->Mul(neg_one, prod1, prod2, &attrs_);
        SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Mul);

        SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Mul);
        snn_ops->Mul(one_value, comp_res1, prod3, &attrs_);
        SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Mul);

        SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Add);
        snn_ops->Add(prod2, prod3, final_res, &attrs_);
        SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Add);

        auto out_flat = sign_out->flat<string>();
        out_flat(index) = std::move(final_res[0]);
      }
    }
    log_debug << "--> SecureSignOp Opkernel compute ok.";
  }
};

class SecureEluOp : public SecureOpKernel {
 private:
  /*data*/
 public:
  SecureEluOp(OpKernelConstruction* context) : SecureOpKernel(context) {}
  ~SecureEluOp() {}

  void ComputeImpl(OpKernelContext* context) {
    log_debug << "--> SecureEluOp Opkernel Compute.";

    const Tensor& logits_in = context->input(0);
    OP_REQUIRES(
      context, TensorShapeUtils::IsVectorOrHigher(logits_in.shape()),
      errors::InvalidArgument(
        "logits must have >= 1 dimension, got ", logits_in.shape().DebugString()));

    Tensor* elu_out = nullptr;
    OP_REQUIRES_OK(
      context, context->forward_input_or_allocate_output({0}, 0, logits_in.shape(), &elu_out));

    std::cout << "Logits_in.NumElements(): " << logits_in.NumElements() << std::endl;
    const auto& logits_in_flat = logits_in.flat<string>();
    const TensorShape& logits_in_shape = logits_in.shape();
    std::cout << "Logits_in_shape.num_elements(): " << logits_in_shape.num_elements() << std::endl;

    for (int64_t i = 0; i < logits_in_shape.dims(); ++i) {
      std::cout << "Logits_in dim" << i << ":" << logits_in_shape.dim_size(i) << std::endl;
    }

    if (logits_in.NumElements() == 0) {
      return;
    }

    int64_t dims = logits_in_shape.dims();
    if (dims <= 0 || dims > 2) {
      // std::cout << "Not Supported dims: " << dims << std::endl;
      log_error << "not supported dims: " << dims;
      OP_REQUIRES(
        context, false,
        errors::InvalidArgument("not supported dims, got ", logits_in.shape().DebugString()));
    }

    int64_t rows = 0, cols = 0;
    if (dims == 1) {
      rows = 1;
      cols = logits_in_shape.dim_size(0);
    } else if (dims == 2) {
      rows = logits_in_shape.dim_size(0);
      cols = logits_in_shape.dim_size(1);
    }
    std::cout << "rows: " << rows << ", cols: " << cols << std::endl;

    auto snn_ops = ProtocolManager::Instance()
                     ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(
                       context->device()->attributes().incarnation()))
                     ->GetOps(msg_id());

    for (int i = 0; i < rows; ++i) {
      for (int j = 0; j < cols; ++j) {
        int index = i * cols + j;

        vector<string> elu_value = {logits_in_flat(index)};

        const vector<string> one_value = {std::to_string(1)};
        const vector<string> zero_value = {std::to_string(0)};
        vector<string> comp_res;
        vector<string> exp_value;
        vector<string> diff1, diff2;
        vector<string> prod1, prod2;
        vector<string> final_res;

        SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Less);
        snn_ops->Less(elu_value, zero_value, comp_res, &attrs_);
        SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Less);

        SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Exp);
        snn_ops->Exp(elu_value, exp_value, &attrs_);
        SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Exp);

        SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Sub);
        snn_ops->Sub(exp_value, one_value, diff1, &attrs_);
        SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Sub);

        SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Sub);
        snn_ops->Sub(one_value, comp_res, diff2, &attrs_);
        SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Sub);

        SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Mul);
        snn_ops->Mul(diff1, comp_res, prod1, &attrs_);
        SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Mul);

        SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Mul);
        snn_ops->Mul(diff2, elu_value, prod2, &attrs_);
        SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Mul);

        SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Add);
        snn_ops->Add(prod1, prod2, final_res, &attrs_);
        SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Add);

        auto out_flat = elu_out->flat<string>();
        out_flat(index) = std::move(final_res[0]);
      }
    }
    log_debug << "--> SecureEluOp Opkernel compute ok.";
  }
};

class SecureRelu6Op : public SecureOpKernel {
 private:
  /*data*/
 public:
  SecureRelu6Op(OpKernelConstruction* context) : SecureOpKernel(context) {}
  ~SecureRelu6Op() {}

  void ComputeImpl(OpKernelContext* context) {
    log_debug << "--> SecureRelu6Op Opkernel Compute.";

    const Tensor& logits_in = context->input(0);
    OP_REQUIRES(
      context, TensorShapeUtils::IsVectorOrHigher(logits_in.shape()),
      errors::InvalidArgument(
        "logits must have >= 1 dimension, got ", logits_in.shape().DebugString()));

    Tensor* relu6_out = nullptr;
    OP_REQUIRES_OK(
      context, context->forward_input_or_allocate_output({0}, 0, logits_in.shape(), &relu6_out));

    std::cout << "Logits_in.NumElements(): " << logits_in.NumElements() << std::endl;
    const auto& logits_in_flat = logits_in.flat<string>();
    const TensorShape& logits_in_shape = logits_in.shape();
    std::cout << "Logits_in_shape.num_elements(): " << logits_in_shape.num_elements() << std::endl;

    for (int64_t i = 0; i < logits_in_shape.dims(); ++i) {
      std::cout << "Logits_in dim" << i << ":" << logits_in_shape.dim_size(i) << std::endl;
    }

    if (logits_in.NumElements() == 0) {
      return;
    }

    int64_t dims = logits_in_shape.dims();
    if (dims <= 0 || dims > 2) {
      // std::cout << "Not Supported dims: " << dims << std::endl;
      log_error << "not supported dims: " << dims;
      OP_REQUIRES(
        context, false,
        errors::InvalidArgument("not supported dims, got ", logits_in.shape().DebugString()));
    }

    int64_t rows = 0, cols = 0;
    if (dims == 1) {
      rows = 1;
      cols = logits_in_shape.dim_size(0);
    } else if (dims == 2) {
      rows = logits_in_shape.dim_size(0);
      cols = logits_in_shape.dim_size(1);
    }
    std::cout << "rows: " << rows << ", cols: " << cols << std::endl;

    auto snn_ops = ProtocolManager::Instance()
                     ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(
                       context->device()->attributes().incarnation()))
                     ->GetOps(msg_id());

    for (int i = 0; i < rows; ++i) {
      for (int j = 0; j < cols; ++j) {
        int index = i * cols + j;

        vector<string> relu6_value = {logits_in_flat(index)};

        const vector<string> one_value = {std::to_string(1)};
        const vector<string> zero_value = {std::to_string(0)};
        const vector<string> six_value = {std::to_string(6)};
        vector<string> comp_res1, comp_res2;
        vector<string> diff1, diff2;
        vector<string> prod1, prod2, prod3, prod4;
        vector<string> sum1, sum2;

        SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Less);
        snn_ops->Less(relu6_value, zero_value, comp_res1, &attrs_);
        SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Less);

        SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Greater);
        snn_ops->Greater(relu6_value, six_value, comp_res2, &attrs_);
        SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Greater);

        SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Sub);
        snn_ops->Sub(one_value, comp_res1, diff1, &attrs_);
        SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Sub);

        SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Sub);
        snn_ops->Sub(one_value, comp_res2, diff2, &attrs_);
        SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Sub);

        SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Mul);
        snn_ops->Mul(diff1, diff2, prod1, &attrs_);
        SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Mul);

        SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Mul);
        snn_ops->Mul(relu6_value, prod1, prod2, &attrs_);
        SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Mul);

        SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Mul);
        snn_ops->Mul(zero_value, comp_res1, prod3, &attrs_);
        SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Mul);

        SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Mul);
        snn_ops->Mul(six_value, comp_res2, prod4, &attrs_);
        SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Mul);

        SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Add);
        snn_ops->Add(prod3, prod4, sum1, &attrs_);
        SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Add);

        SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Add);
        snn_ops->Add(prod2, sum1, sum2, &attrs_);
        SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Add);

        auto out_flat = relu6_out->flat<string>();
        out_flat(index) = std::move(sum2[0]);
      }
    }
    log_debug << "--> SecureRelu6Op Opkernel compute ok.";
  }
};

class SecureLeakyReluOp : public SecureOpKernel {
 private:
  /*data*/
 public:
  SecureLeakyReluOp(OpKernelConstruction* context) : SecureOpKernel(context) {}
  ~SecureLeakyReluOp() {}

  void ComputeImpl(OpKernelContext* context) {
    log_debug << "--> SecureLeakyReluOp Opkernel Compute.";

    const Tensor& logits_in = context->input(0);
    OP_REQUIRES(
      context, TensorShapeUtils::IsVectorOrHigher(logits_in.shape()),
      errors::InvalidArgument(
        "logits must have >= 1 dimension, got ", logits_in.shape().DebugString()));

    Tensor* leaky_relu_out = nullptr;
    OP_REQUIRES_OK(
      context,
      context->forward_input_or_allocate_output({0}, 0, logits_in.shape(), &leaky_relu_out));

    std::cout << "Logits_in.NumElements(): " << logits_in.NumElements() << std::endl;
    const auto& logits_in_flat = logits_in.flat<string>();
    const TensorShape& logits_in_shape = logits_in.shape();
    std::cout << "Logits_in_shape.num_elements(): " << logits_in_shape.num_elements() << std::endl;

    for (int64_t i = 0; i < logits_in_shape.dims(); ++i) {
      std::cout << "Logits_in dim" << i << ":" << logits_in_shape.dim_size(i) << std::endl;
    }

    if (logits_in.NumElements() == 0) {
      return;
    }

    int64_t dims = logits_in_shape.dims();
    if (dims <= 0 || dims > 2) {
      // std::cout << "Not Supported dims: " << dims << std::endl;
      log_error << "not supported dims: " << dims;
      OP_REQUIRES(
        context, false,
        errors::InvalidArgument("not supported dims, got ", logits_in.shape().DebugString()));
    }

    int64_t rows = 0, cols = 0;
    if (dims == 1) {
      rows = 1;
      cols = logits_in_shape.dim_size(0);
    } else if (dims == 2) {
      rows = logits_in_shape.dim_size(0);
      cols = logits_in_shape.dim_size(1);
    }
    std::cout << "rows: " << rows << ", cols: " << cols << std::endl;

    auto snn_ops = ProtocolManager::Instance()
                     ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(
                       context->device()->attributes().incarnation()))
                     ->GetOps(msg_id());

    const vector<string> one_value = {std::to_string(1)};
    const vector<string> zero_value = {std::to_string(0)};
    const vector<string> alpha = {std::to_string(0.2)};

    for (int i = 0; i < rows; ++i) {
      for (int j = 0; j < cols; ++j) {
        int index = i * cols + j;

        vector<string> leaky_relu_value = {logits_in_flat(index)};
        vector<string> comp_res;
        vector<string> diff;
        vector<string> prod1, prod2, prod3;
        vector<string> final_res;

        SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Greater);
        snn_ops->Greater(leaky_relu_value, zero_value, comp_res, &attrs_);
        SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Greater);

        SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Sub);
        snn_ops->Sub(one_value, comp_res, diff, &attrs_);
        SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Sub);

        SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Mul);
        snn_ops->Mul(alpha, leaky_relu_value, prod1, &attrs_);
        SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Mul);

        SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Mul);
        snn_ops->Mul(prod1, diff, prod2, &attrs_);
        SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Mul);

        SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Mul);
        snn_ops->Mul(leaky_relu_value, comp_res, prod3, &attrs_);
        SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Mul);

        SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG(Add);
        snn_ops->Add(prod2, prod3, final_res, &attrs_);
        SECURE_OP_CALL_PROTOCOL_OP_STATS_END(Add);

        auto out_flat = leaky_relu_out->flat<string>();
        out_flat(index) = std::move(final_res[0]);
      }
    }
    log_debug << "--> SecureLeakyReluOp Opkernel compute ok.";
  }
};

//////////////    register kernels    //////////////
REGISTER_STR_CPU_KERNEL(SecureAdd, SecureAddOp);
REGISTER_STR_CPU_KERNEL(SecureSub, SecureSubOp);
REGISTER_STR_CPU_KERNEL(SecureMul, SecureMulOp);
REGISTER_STR_CPU_KERNEL(SecureDiv, SecureDivOp);
REGISTER_STR_CPU_KERNEL(SecureReciprocaldiv, SecureReciprocaldivOp);
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
REGISTER_STR_CPU_KERNEL(SecureExp, SecureExpOp);
REGISTER_STR_CPU_KERNEL(SecureRsqrt, SecureRsqrtOp);
REGISTER_STR_CPU_KERNEL(SecureSqrt, SecureSqrtOp);

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

REGISTER_STR_CPU_KERNEL(SecureArgMax, SecureArgMaxOp);
REGISTER_STR_CPU_KERNEL(SecureArgMin, SecureArgMinOp);

REGISTER_STR_CPU_KERNEL(SecureOnes, SecureOnesOp);
REGISTER_STR_CPU_KERNEL(SecureZeros, SecureZerosOp);

REGISTER_STR_CPU_KERNEL(SecureSign, SecureSignOp);
REGISTER_STR_CPU_KERNEL(SecureElu, SecureEluOp);
REGISTER_STR_CPU_KERNEL(SecureRelu6, SecureRelu6Op);
REGISTER_STR_CPU_KERNEL(SecureLeakyRelu, SecureLeakyReluOp);
REGISTER_STR_CPU_KERNEL(SecureCountUpTo, SecureCountUpToOp);

REGISTER_KERNEL_BUILDER(
  Name("SecureAddN").Device(DEVICE_CPU).TypeConstraint<std::string>("T"),
  SecureAddNOp);

// logical ops
REGISTER_STR_CPU_KERNEL(SecureLogicalAnd, SecureLogicalAndOp);
REGISTER_STR_CPU_KERNEL(SecureLogicalOr, SecureLogicalOrOp);
REGISTER_STR_CPU_KERNEL(SecureLogicalXor, SecureLogicalXorOp);
REGISTER_STR_CPU_KERNEL(SecureLogicalNot, SecureLogicalNotOp);
} // namespace tensorflow
