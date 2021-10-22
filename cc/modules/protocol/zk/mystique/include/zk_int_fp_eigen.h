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
#pragma once

#include "cc/modules/protocol/zk/mystique/include/zk_int_fp.h"
#include "cc/third_party/eigen/Eigen/Core"
using namespace rosetta;

using rosetta::zk::ZkIntFp;

/* functions need to support for eigen new type   */
inline const ZkIntFp& conj(const ZkIntFp& x) {
  return x;
}
inline const ZkIntFp& real(const ZkIntFp& x) {
  return x;
}
inline ZkIntFp imag(const ZkIntFp& x) {
  return x;//0.
}
// inline ZkIntFp abs(const ZkIntFp& x) {
//   return x;// TODO: how to fix
//   // return x.select(ZkIntFp(0.0) > x, -x);// fabs(x);
// }
inline ZkIntFp abs2(const ZkIntFp& x) {
  return x * x;
}
/* functions need to support for eigen new type   */


namespace Eigen {
template <>
struct NumTraits<ZkIntFp>
    : NumTraits<double> // permits to get the epsilon, dummy_precision, lowest, highest functions
{
  typedef ZkIntFp Real;
  typedef ZkIntFp NonInteger;
  typedef ZkIntFp Nested;
  enum {
    IsComplex = 0,
    IsInteger = 0,
    IsSigned = 1,
    RequireInitialization = 1,
    ReadCost = 1,
    AddCost = 3,
    MulCost = 3
  };
};
} // namespace Eigen

#include "cc/modules/protocol/zk/mystique/include/zk_uint64_t.h"
using rosetta::zk::ZkUint64;

/* functions need to support for eigen new type   */
inline const ZkUint64& conj(const ZkUint64& x) {
  return x;
}
inline const ZkUint64& real(const ZkUint64& x) {
  return x;
}
inline ZkUint64 imag(const ZkUint64& x) {
  return x;//0.
}
// inline ZkUint64 abs(const ZkUint64& x) {
//   return x;// TODO: how to fix
//   // return x.select(ZkUint64(0.0) > x, -x);// fabs(x);
// }
inline ZkUint64 abs2(const ZkUint64& x) {
  return x * x;
}
/* functions need to support for eigen new type   */


namespace Eigen {
template <>
struct NumTraits<ZkUint64>
    : NumTraits<double> // permits to get the epsilon, dummy_precision, lowest, highest functions
{
  typedef ZkIntFp Real;
  typedef ZkIntFp NonInteger;
  typedef ZkIntFp Nested;
  enum {
    IsComplex = 0,
    IsInteger = 0,
    IsSigned = 1,
    RequireInitialization = 1,
    ReadCost = 1,
    AddCost = 3,
    MulCost = 3
  };
};
} // namespace Eigen


using ZkMatrix=Eigen::Matrix<ZkIntFp, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;
using U64Matrix=Eigen::Matrix<uint64_t, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;
using ZkU64Matrix=Eigen::Matrix<ZkUint64, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;
using DoubleMatrix=Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;
using FloatMatrix=Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

#define USE_MATRIX_ROW_CACHE 1
#if ! USE_MATRIX_ROW_CACHE
// U64Matrix * ZkMatrix
void static inline zk_eigen_const_matmul(const U64Matrix& plain_a, const ZkMatrix& b, ZkMatrix& c) {
  size_t M = plain_a.rows();
  size_t K = plain_a.cols();
  size_t N = b.cols();
  if (M*N != c.size())
    c.resize(M, N);
  //cout << "zk_eigen_const_matmul out: \n";
  for (size_t i = 0; i < M; i++)
  {
    for (size_t j = 0; j < N; j++)
    {
      //a(i*m)* [b(0)...b(mm)]
      ZkIntFp acc((uint64_t)0, PUBLIC);
      for (size_t l = 0; l < K; l++)
      {
        acc = acc + b(l, j) * plain_a(i, l);//a[i*K+l] * b[l*N+j];
      }
      c(i, j) = acc;// c[i*N+j] = acc;
      //cout << c(i,j) << ", \t";
    }
    //cout << endl;
  }
}

// U64Matrix * ZkMatrix
void static inline zk_eigen_const_matmul(const ZkU64Matrix& plain_a, const ZkMatrix& b, ZkMatrix& c) {
  size_t M = plain_a.rows();
  size_t K = plain_a.cols();
  size_t N = b.cols();
  if (M*N != c.size())
    c.resize(M, N);
  //cout << "zk_eigen_const_matmul out: \n";
  for (size_t i = 0; i < M; i++)
  {
    for (size_t j = 0; j < N; j++)
    {
      //a(i*m)* [b(0)...b(mm)]
      ZkIntFp acc((uint64_t)0, PUBLIC);
      for (size_t l = 0; l < K; l++)
      {
        acc = acc + b(l, j) * plain_a(i, l).value;//a[i*K+l] * b[l*N+j];
      }
      c(i, j) = acc;// c[i*N+j] = acc;
      //cout << c(i,j) << ", \t";
    }
    //cout << endl;
  }
}

// ZkMatrix * ZkU64Matrix
void static inline zk_eigen_const_matmul(const ZkMatrix& a, const ZkU64Matrix& plain_b, ZkMatrix& c) {
  size_t M = a.rows();
  size_t K = a.cols();
  size_t N = plain_b.cols();
  if (M*N != c.size())
    c.resize(M, N);
  //! cout << "zk_eigen_const_matmul out: \n";
  for (size_t i = 0; i < M; i++)
  {
    for (size_t j = 0; j < N; j++)
    {
      //a(i*m)* [b(0)...b(mm)]
      ZkIntFp acc((uint64_t)0, PUBLIC);
      for (size_t l = 0; l < K; l++)
      {
        acc = acc + a(i, l) * plain_b(l, j).value;
      }
      c(i, j) = acc;// c[i*N+j] = acc;
      //! cout << c(i,j) << ", \t";
    }
    //! cout << endl;
  }
}
// ZkMatrix * U64Matrix
void static inline zk_eigen_const_matmul(const ZkMatrix& a, const U64Matrix& plain_b, ZkMatrix& c) {
  size_t M = a.rows();
  size_t K = a.cols();
  size_t N = plain_b.cols();
  if (M*N != c.size())
    c.resize(M, N);
  //cout << "zk_eigen_const_matmul out: \n";
  for (size_t i = 0; i < M; i++)
  {
    for (size_t j = 0; j < N; j++)
    {
      //a(i*m)* [b(0)...b(mm)]
      ZkIntFp acc((uint64_t)0, PUBLIC);
      for (size_t l = 0; l < K; l++)
      {
        ZkIntFp tmp = ZkIntFp(plain_b(l, j), PUBLIC);
        acc = acc + a(i, l) * tmp;
      }
      c(i, j) = acc;// c[i*N+j] = acc;
      //cout << c(i,j) << ", \t";
    }
    //cout << endl;
  }
}

// U64Matrix * U64Matrix
void static inline zk_eigen_const_const_matmul(const U64Matrix& a, const U64Matrix& plain_b, U64Matrix& c) {
  size_t M = a.rows();
  size_t K = a.cols();
  size_t N = plain_b.cols();
  if (M*N != c.size())
    c.resize(M, N);
  // cout << "-------  zk_eigen_const_const_matmul out: \n";
  for (size_t i = 0; i < M; i++)
  {
    for (size_t j = 0; j < N; j++)
    {
      //a(i*m)* [b(0)...b(mm)]
      // ZkIntFp acc((uint64_t)0, PUBLIC);
      uint64_t acc = 0;
      for (size_t l = 0; l < K; l++)
      {
        // ZkIntFp tmp = ZkIntFp(plain_b(l, j), PUBLIC);
        uint64_t tmp = plain_b(l, j);
        acc = acc + a(i, l) * tmp;
      }
      c(i, j) = acc;// c[i*N+j] = acc;
      // cout << c(i,j) << ", \t";
    }
    // cout << endl;
  }
}

// ZkMatrix * U64Matrix
void static inline zk_eigen_const_const_matmul(const ZkU64Matrix& a, const ZkU64Matrix& plain_b, ZkU64Matrix& c) {
  size_t M = a.rows();
  size_t K = a.cols();
  size_t N = plain_b.cols();
  if (M*N != c.size())
    c.resize(M, N);
  // cout << "-------  zk_eigen_const_const_matmul out: \n";
  for (size_t i = 0; i < M; i++)
  {
    for (size_t j = 0; j < N; j++)
    {
      //a(i*m)* [b(0)...b(mm)]
      // ZkIntFp acc((uint64_t)0, PUBLIC);
      uint64_t acc = 0;
      for (size_t l = 0; l < K; l++)
      {
        // ZkIntFp tmp = ZkIntFp(plain_b(l, j), PUBLIC);
        uint64_t tmp = plain_b(l, j).value;
        acc = acc + a(i, l).value * tmp;
      }
      c(i, j) = acc;// c[i*N+j] = acc;
      // cout << c(i,j) << ", \t";
    }
    // cout << endl;
  }
}

// ZkMatrix * U64Matrix
void static inline zk_eigen_const_matmul(const ZkMatrix& a, const DoubleMatrix& plain_b, ZkMatrix& c) {
  size_t M = a.rows();
  size_t K = a.cols();
  size_t N = plain_b.cols();
  if (a.size() != c.size())
    c.resize(M, K);
  
  //cout << "zk_eigen_const_matmul out: \n";
  for (size_t i = 0; i < M; i++)
  {
    for (size_t j = 0; j < N; j++)
    {
      //a(i*m)* [b(0)...b(mm)]
      ZkIntFp acc((uint64_t)0, PUBLIC);
      for (size_t l = 0; l < K; l++)
      {
        acc = acc + a(i, l) * rosetta::zk::ZkIntFp::zk_fp_encode(plain_b(l, j));
      }
      c(i, j) = acc;// c[i*N+j] = acc;
      //cout << c(i,j) << ", \t";
    }
    //cout << endl;
  }
}

void static inline zk_eigen_const_mul(const ZkMatrix& a, const uint64_t& plain_b, ZkMatrix& c) {
  size_t M = a.rows();
  size_t K = a.cols();
  if (a.size() != c.size())
    c.resize(M, K);
  // cout << "const: " << plain_b << ", zk_eigen_const_matmul out: \n";
  for (size_t i = 0; i < M; i++)
  {
    for (size_t j = 0; j < K; j++)
    {
      c(i,j) = a(i, j) * plain_b;
      // cout << c(i,j) << ", \t";
    }
    // cout << endl;
  }
}
#else
// U64Matrix * ZkMatrix
void static inline zk_eigen_const_matmul(const U64Matrix& plain_a, const ZkMatrix& b, ZkMatrix& c) {
  size_t M = plain_a.rows();
  size_t K = plain_a.cols();
  size_t N = b.cols();
  if (M*N != c.size())
    c.resize(M, N);
  //cout << "zk_eigen_const_matmul out: \n";
  for (size_t i = 0; i < M; i++)
  {
    for (size_t l = 0; l < K; l++)
    {
      for (size_t j = 0; j < N; j++) {
        c(i, j) = c(i, j) + b(l, j) * plain_a(i, l);
      }
    }
  }
}

// U64Matrix * ZkMatrix
void static inline zk_eigen_const_matmul(const ZkU64Matrix& plain_a, const ZkMatrix& b, ZkMatrix& c) {
  size_t M = plain_a.rows();
  size_t K = plain_a.cols();
  size_t N = b.cols();
  if (M*N != c.size())
    c.resize(M, N);
  
  //cout << "zk_eigen_const_matmul out: \n";
  for (size_t i = 0; i < M; i++)
  {
    for (size_t l = 0; l < K; l++)
    {
      for (size_t j = 0; j < N; j++) {
        c(i, j) = c(i, j) + b(l, j) * plain_a(i, l).value;//a[i*K+l] * b[l*N+j];
      }
    }
  }
}

// ZkMatrix * ZkU64Matrix
void static inline zk_eigen_const_matmul(const ZkMatrix& a, const ZkU64Matrix& plain_b, ZkMatrix& c) {
  size_t M = a.rows();
  size_t K = a.cols();
  size_t N = plain_b.cols();
  if (M*N != c.size())
    c.resize(M, N);
  //! cout << "zk_eigen_const_matmul out: \n";
  for (size_t i = 0; i < M; i++)
  {
    for (size_t l = 0; l < K; l++)
    {
      for (size_t j = 0; j < N; j++) {
        c(i, j) = c(i, j) + a(i, l) * plain_b(l, j).value;//a[i*K+l] * b[l*N+j];
      }
    }
  }
}

// ZkMatrix * U64Matrix
void static inline zk_eigen_const_matmul(const ZkMatrix& a, const U64Matrix& plain_b, ZkMatrix& c) {
  size_t M = a.rows();
  size_t K = a.cols();
  size_t N = plain_b.cols();
  if (M*N != c.size())
    c.resize(M, N);
  //cout << "zk_eigen_const_matmul out: \n";
  for (size_t i = 0; i < M; i++)
  {
    for (size_t l = 0; l < K; l++)
    {
      for (size_t j = 0; j < N; j++) {
        c(i, j) = c(i, j) + a(i, l) * plain_b(l, j);//a[i*K+l] * b[l*N+j];
      }
    }
  }
}

// U64Matrix * U64Matrix
void static inline zk_eigen_const_const_matmul(const U64Matrix& plain_a, const U64Matrix& plain_b, U64Matrix& c) {
  size_t M = plain_a.rows();
  size_t K = plain_a.cols();
  size_t N = plain_b.cols();
  if (M*N != c.size())
    c.resize(M, N);
  // cout << "-------  zk_eigen_const_const_matmul out: \n";
  for (size_t i = 0; i < M; i++)
  {
    for (size_t l = 0; l < K; l++)
    {
      for (size_t j = 0; j < N; j++) {
        c(i, j) = c(i, j) + plain_a(i, l) * plain_b(l, j);//a[i*K+l] * b[l*N+j];
      }
    }
  }
}

// ZkMatrix * U64Matrix
void static inline zk_eigen_const_const_matmul(const ZkU64Matrix& plain_a, const ZkU64Matrix& plain_b, ZkU64Matrix& c) {
  size_t M = plain_a.rows();
  size_t K = plain_a.cols();
  size_t N = plain_b.cols();
  if (M*N != c.size())
    c.resize(M, N);
  // cout << "-------  zk_eigen_const_const_matmul out: \n";
  for (size_t i = 0; i < M; i++)
  {
    for (size_t l = 0; l < K; l++)
    {
      for (size_t j = 0; j < N; j++) {
        c(i, j) = c(i, j) + plain_a(i, l).value * plain_b(l, j).value;//a[i*K+l] * b[l*N+j];
      }
    }
  }
}

// ZkMatrix * U64Matrix
void static inline zk_eigen_const_matmul(const ZkMatrix& a, const DoubleMatrix& plain_b, ZkMatrix& c) {
  size_t M = a.rows();
  size_t K = a.cols();
  size_t N = plain_b.cols();
  if (a.size() != c.size())
    c.resize(M, K);
  
  for (size_t i = 0; i < M; i++)
  {
    for (size_t l = 0; l < K; l++)
    {
      for (size_t j = 0; j < N; j++) {
        c(i, j) = c(i, j) + a(i, l) * rosetta::zk::ZkIntFp::zk_fp_encode(plain_b(l, j));//a[i*K+l] * b[l*N+j];
      }
    }
  }
}

void static inline zk_eigen_const_mul(const ZkMatrix& a, const uint64_t& plain_b, ZkMatrix& c) {
  size_t M = a.rows();
  size_t K = a.cols();
  if (a.size() != c.size())
    c.resize(M, K);
  // cout << "const: " << plain_b << ", zk_eigen_const_matmul out: \n";
  for (size_t i = 0; i < M; i++)
  {
    for (size_t j = 0; j < K; j++)
    {
      c(i,j) = a(i, j) * plain_b;
      // cout << c(i,j) << ", \t";
    }
    // cout << endl;
  }
}

// zk,zk matmul with extend inner-product optimization
// a(i) * b^T[j] = c[i][j]
void static inline zk_eigen_matmul_with_inner_prdt_opt(const ZkMatrix& a, const ZkMatrix& b, ZkMatrix& c) {
  size_t M = a.rows();
  size_t K = a.cols();
  size_t N = b.cols();
  if (M*N != c.size())
    c.resize(M, N);

  ZkMatrix &c_private_input = c;//(M, N);
  if (zk_party_id == ALICE) {
    ZkU64Matrix a_fp(M, K), b_fp(K, N);
    ZkIntFp::zk_batch_get_fp(a.data(), a.size(), (uint64_t*)a_fp.data());
    ZkIntFp::zk_batch_get_fp(b.data(), b.size(), (uint64_t*)b_fp.data());
    ZkU64Matrix c_fp = a_fp * b_fp;
    batch_feed(c_private_input.data(), (uint64_t*)c_fp.data(), c_private_input.size());
  } else {
    batch_feed(c_private_input.data(), nullptr, c_private_input.size());
  }

  // cout << "-------  zk_eigen_matmul_with_inner_opt: m,k,n: " << M << ", " << K << ", " << N << "\n";
  vector<ZkIntFp> extend_a(K+1);
  vector<ZkIntFp> extend_b(K+1);
  vector<uint64_t> zeros(c.size(), 0ull);

  for (size_t i = 0; i < M; i++)
  {
    for (size_t j = 0; j < N; j++)
    {
      // assign
      size_t k = 0;
      for (k = 0 ; k < K; k++)
      {
        extend_a[k] = a(i, k);
        extend_b[k] = b(k, j);
      }
      extend_a[k] = ZkIntFp((uint64_t)1, PUBLIC);
      extend_b[k] = ZkIntFp::zk_fp_neg(c_private_input(i,j), zk_party_id);
      
      ZkIntFp tmp((uint64_t)0, PUBLIC);
      for (k = 0; k < extend_a.size(); k++)
      {
        tmp = tmp + extend_a[k] * extend_b[k];
      }
      
      fp_zkp_inner_prdt<BoolIO<ZKNetIO>>(extend_a.data(), extend_b.data(), zeros[i*N+j], extend_a.size());
    }
  }
}

#endif//