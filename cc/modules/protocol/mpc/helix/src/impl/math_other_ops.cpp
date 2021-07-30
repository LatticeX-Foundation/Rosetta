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
#include "helix_impl_util.h"

namespace rosetta {

int HelixOpsImpl::Pow(
  const vector<string>& a,
  const vector<string>& b,
  vector<string>& c,
  const attr_type* attr_info) {
  assert(a.size() == b.size());
  if (b.size() == 0) {
    c = a;
    return 0;
  }

  // only supports X^(Integer), so
  int size = a.size();
  vector<int> intB(size);
  vector<double> doubleB(size);
  helix_plain_string_to_double(b, doubleB);
  AUDIT("id:{}, PowV2 P{} input Y(double){}", _op_msg_id.get_hex(), hi->party_id(), Vector<double>(doubleB));

  for (int i = 0; i < size; i++) {
    intB[i] = (int)doubleB[i];
  }

  vector<Share> shareA, shareC;
  helix_convert_string_to_share(a, shareA);
  AUDIT("id:{}, PowV2 P{} input X(Share){}", _op_msg_id.get_hex(), hi->party_id(), Vector<Share>(shareA));

  hi->PowV2(shareA, intB, shareC);
  helix_convert_share_to_string(shareC, c);

  AUDIT("id:{}, PowV2 P{} ouput(Share){}", _op_msg_id.get_hex(), hi->party_id(), Vector<Share>(shareC));

  return 0;
}

int HelixOpsImpl::Matmul(
  const vector<string>& a,
  const vector<string>& b,
  vector<string>& c,
  const attr_type* attr_info) {
  assert(attr_info != nullptr);

  // c.shape = (m,k) x (k,n)
  int m = get_attr_value(attr_info, "m", 0);
  int k = get_attr_value(attr_info, "k", 0);
  int n = get_attr_value(attr_info, "n", 0);
  bool transpose_a = get_attr_value(attr_info, "transpose_a", 0) == 1;
  bool transpose_b = get_attr_value(attr_info, "transpose_b", 0) == 1;
  assert(m * k > 0);
  assert(k * n > 0);

  if ((m * k == 0) || (k * n == 0)) {
    tlog_error << "error m,k,n:" << m << " " << k << " " << n ;
  }

  vector<Share> shareA, shareB, shareC;
  helix_convert_string_to_share(a, shareA);
  helix_convert_string_to_share(b, shareB);
  AUDIT("id:{}, Matmul({},{},{}), P{} input X(Share){}", _op_msg_id.get_hex(), m, k, n, hi->party_id(), Vector<Share>(shareA));
  AUDIT("id:{}, Matmul({},{},{}), P{} input Y(Share){}", _op_msg_id.get_hex(), m, k, n, hi->party_id(), Vector<Share>(shareB));

  hi->MatMul(shareA, shareB, shareC, m, k, n, transpose_a, transpose_b);
  helix_convert_share_to_string(shareC, c);

  AUDIT("id:{}, Matmul({},{},{}), P{} output(Share){}", _op_msg_id.get_hex(), m, k, n, hi->party_id(), Vector<Share>(shareC));
  return 0;
}

int HelixOpsImpl::Square(const vector<string>& a, vector<string>& c, const attr_type* attr_info) {
  vector<Share> shareA, shareC;
  helix_convert_string_to_share(a, shareA);
  AUDIT("id:{}, Square P{} input(Share){}", _op_msg_id.get_hex(), hi->party_id(), Vector<Share>(shareA));

  hi->Square(shareA, shareC);
  helix_convert_share_to_string(shareC, c);

  AUDIT("id:{}, Square P{} output(Share){}", _op_msg_id.get_hex(), hi->party_id(), Vector<Share>(shareC));
  return 0;
}

int HelixOpsImpl::Negative(const vector<string>& a, vector<string>& c, const attr_type* attr_info) {
  vector<Share> shareA, shareC;
  helix_convert_string_to_share(a, shareA);
  AUDIT("id:{}, Negative P{} input(Share){}", _op_msg_id.get_hex(), hi->party_id(), Vector<Share>(shareA));

  hi->Negative(shareA, shareC);
  helix_convert_share_to_string(shareC, c);

  AUDIT("id:{}, Negative P{} output(Share){}", _op_msg_id.get_hex(), hi->party_id(), Vector<Share>(shareC));
  return 0;
}

/** 
 * |x| means:
 *   1 * x     if x >= 0
 *   (-1) * x  if x < 0
 */
int HelixOpsImpl::Abs(const vector<string>& a, vector<string>& c, const attr_type* attr_info) {
  int vec_size = a.size();
  vector<Share> shareA(vec_size), shareC(vec_size);
  helix_convert_string_to_share(a, shareA);
  AUDIT("id:{}, Abs P{} input(Share){}", _op_msg_id.get_hex(), hi->party_id(), Vector<Share>(shareA));

  vector<double> DOUBLE_ONE(vec_size, 1.0);
  vector<double> DOUBLE_NEG_ONE(vec_size, -1.0);
  vector<Share> a_sign(vec_size);
  vector<Share> sign_multiplier(vec_size);
  hi->DReLU(shareA, a_sign);
  hi->Select1Of2(DOUBLE_ONE, DOUBLE_NEG_ONE, a_sign, sign_multiplier);
  hi->Mul(sign_multiplier, shareA, shareC);
  helix_convert_share_to_string(shareC, c);

  AUDIT("id:{}, Abs P{} output(Share){}", _op_msg_id.get_hex(), hi->party_id(), Vector<Share>(shareC));
  return 0;
}

/** 
 * |x|' means:
 * 1     if x >= 0 [ in point 0 we set it to 1]
 * (-1)  if x < 0
 */
int HelixOpsImpl::AbsPrime(const vector<string>& a, vector<string>& c, const attr_type* attr_info) {
  int vec_size = a.size();
  vector<Share> shareA(vec_size), shareC(vec_size);
  helix_convert_string_to_share(a, shareA);
  AUDIT("id:{}, AbsPrime P{} input(Share){}", _op_msg_id.get_hex(), hi->party_id(), Vector<Share>(shareA));

  vector<double> DOUBLE_ONE(vec_size, 1.0);
  vector<double> DOUBLE_NEG_ONE(vec_size, -1.0);
  vector<Share> a_sign(vec_size);
  hi->DReLU(shareA, a_sign);
  hi->Select1Of2(DOUBLE_ONE, DOUBLE_NEG_ONE, a_sign, shareC);
  helix_convert_share_to_string(shareC, c);

  AUDIT("id:{}, AbsPrime P{} output(Share){}", _op_msg_id.get_hex(), hi->party_id(), Vector<Share>(shareC));
  return 0;
}

int HelixOpsImpl::Log(const vector<string>& a, vector<string>& c, const attr_type* attr_info) {
  int vec_size = a.size();
  vector<Share> shareA(vec_size), shareC(vec_size);
  helix_convert_string_to_share(a, shareA);
  AUDIT("id:{}, Log P{} input(Share){}", _op_msg_id.get_hex(), hi->party_id(), Vector<Share>(shareA));

  // use the version-2 implementation.
  hi->LogV2(shareA, shareC);
  helix_convert_share_to_string(shareC, c);

  AUDIT("id:{}, Log P{} output(Share){}", _op_msg_id.get_hex(), hi->party_id(), Vector<Share>(shareC));
  return 0;
}

int HelixOpsImpl::HLog(const vector<string>& a, vector<string>& c, const attr_type* attr_info) {
  int vec_size = a.size();
  vector<Share> shareA(vec_size), shareC(vec_size);
  helix_convert_string_to_share(a, shareA);
  AUDIT("id:{}, HLog P{} input(Share){}", _op_msg_id.get_hex(), hi->party_id(), Vector<Share>(shareA));

  hi->HLog(shareA, shareC);
  helix_convert_share_to_string(shareC, c);

  AUDIT("id:{}, HLog P{} output(Share){}", _op_msg_id.get_hex(), hi->party_id(), Vector<Share>(shareC));
  return 0;
}

int HelixOpsImpl::Log1p(const vector<string>& a, vector<string>& c, const attr_type* attr_info) {
  vector<string> CONST_ONE(a.size(), "1");
  vector<string> a_plus_one;
  Add(a, CONST_ONE, a_plus_one);
  return Log(a_plus_one, c, attr_info);
}

int HelixOpsImpl::Max(const vector<string>& a, vector<string>& c, const attr_type* attr_info) {
  int rows = get_attr_value(attr_info, "rows", 1);
  int cols = get_attr_value(attr_info, "cols", a.size() / rows);
  vector<Share> shareA, shareC;
  helix_convert_string_to_share(a, shareA);
  AUDIT("id:{}, Max({},{}) P{} input(Share){}", _op_msg_id.get_hex(), rows, cols, hi->party_id(), Vector<Share>(shareA));

  hi->Max(shareA, shareC, rows, cols);
  helix_convert_share_to_string(shareC, c);

  AUDIT("id:{}, Max({},{}) P{} output(Share){}", _op_msg_id.get_hex(), rows, cols, hi->party_id(), Vector<Share>(shareC));
  return 0;
}

int HelixOpsImpl::Min(const vector<string>& a, vector<string>& c, const attr_type* attr_info) {
  int rows = get_attr_value(attr_info, "rows", 1);
  int cols = get_attr_value(attr_info, "cols", a.size() / rows);
  vector<Share> shareA, shareC;
  helix_convert_string_to_share(a, shareA);
  AUDIT("id:{}, Min({},{}) P{} input(Share){}", _op_msg_id.get_hex(), rows, cols, hi->party_id(), Vector<Share>(shareA));

  hi->Min(shareA, shareC, rows, cols);
  helix_convert_share_to_string(shareC, c);

  AUDIT("id:{}, Min({},{}) P{} output(Share){}", _op_msg_id.get_hex(), rows, cols, hi->party_id(), Vector<Share>(shareC));
  return 0;
}

int HelixOpsImpl::Mean(const vector<string>& a, vector<string>& c, const attr_type* attr_info) {
  int rows = get_attr_value(attr_info, "rows", 1);
  int cols = get_attr_value(attr_info, "cols", a.size() / rows);
  vector<Share> shareA, shareC;
  helix_convert_string_to_share(a, shareA);
  AUDIT("id:{}, Mean({},{}) P{} input(Share){}", _op_msg_id.get_hex(), rows, cols, hi->party_id(), Vector<Share>(shareA));

  hi->Mean(shareA, shareC, rows, cols);
  helix_convert_share_to_string(shareC, c);

  AUDIT("id:{}, Mean({},{}) P{} output(Share){}", _op_msg_id.get_hex(), rows, cols, hi->party_id(), Vector<Share>(shareC));
  return 0;
}

int HelixOpsImpl::Sum(const vector<string>& a, vector<string>& c, const attr_type* attr_info) {
  int rows = get_attr_value(attr_info, "rows", 1);
  int cols = get_attr_value(attr_info, "cols", a.size() / rows);
  vector<Share> shareA, shareC;
  helix_convert_string_to_share(a, shareA);
  AUDIT("id:{}, Sum({},{}) P{} input(Share){}", _op_msg_id.get_hex(), rows, cols, hi->party_id(),Vector<Share>(shareA));

  hi->Sum(shareA, shareC, rows, cols);
  helix_convert_share_to_string(shareC, c);

  AUDIT("id:{}, Sum({},{}) P{} output(Share){}", _op_msg_id.get_hex(), rows, cols, hi->party_id(), Vector<Share>(shareC));
  return 0;
}

int HelixOpsImpl::Sum(const vector<string>& a, string& c, const attr_type* attr_info) {
  vector<Share> shareA, shareC(1);
  helix_convert_string_to_share(a, shareA);
  AUDIT("id:{}, Sum P{} input(Share){}", _op_msg_id.get_hex(), hi->party_id(), shareA);

  hi->Sum(shareA, shareC[0]);
  vector<string> cc(1);
  helix_convert_share_to_string(shareC, cc);
  c = cc[0];

  AUDIT("id:{}, Sum P{} output(Share){}", _op_msg_id.get_hex(), hi->party_id(), shareC);
  return 0;
}

int HelixOpsImpl::AddN(const vector<string>& a, vector<string>& c, const attr_type* attr_info) {
  int rows = get_attr_value(attr_info, "rows", 1);
  int cols = get_attr_value(attr_info, "cols", a.size() / rows);
  vector<Share> shareA(a.size()), shareC(cols);
  helix_convert_string_to_share(a, shareA);
  AUDIT("id:{}, AddN({},{}) P{} input(Share){}", _op_msg_id.get_hex(), rows, cols, hi->party_id(), Vector<Share>(shareA));

  hi->AddN(shareA, shareC, rows, cols);
  helix_convert_share_to_string(shareC, c);

  AUDIT("id:{}, AddN({},{}) P{} output(Share){}", _op_msg_id.get_hex(), rows, cols, hi->party_id(), Vector<Share>(shareC));
  return 0;
}

int HelixOpsImpl::Exp(const vector<string>& a, vector<string>& output, const attr_type* attr_info/* = nullptr*/) {
  int size = a.size();
  vector<Share> shareA(size), shareC(size);

  helix_convert_string_to_share(a, shareA);
  AUDIT("id:{}, Exp P{} input{}", _op_msg_id.get_hex(), hi->party_id(), Vector<Share>(shareA));
  hi->Exp(shareA, shareC);
  helix_convert_share_to_string(shareC, output);

  AUDIT("id:{}, Exp P{} output{}", _op_msg_id.get_hex(), hi->party_id(), Vector<Share>(shareC));
  return 0;
} 

int HelixOpsImpl::Rsqrt(const vector<string>& a, vector<string>& output, const attr_type* attr_info/* = nullptr*/) {
  int size = a.size();
  vector<Share> shareA(size), shareC(size);
  
  helix_convert_string_to_share(a, shareA);
  AUDIT("id:{}, Rsqrt P{} input{}", _op_msg_id.get_hex(), hi->party_id(), Vector<Share>(shareA));
  hi->Rsqrt(shareA, shareC);
  helix_convert_share_to_string(shareC, output);

  AUDIT("id:{}, Rsqrt P{} output{}", _op_msg_id.get_hex(), hi->party_id(), Vector<Share>(shareC));
  return 0;
} 

int HelixOpsImpl::Sqrt(const vector<string>& a, vector<string>& output, const attr_type* attr_info/* = nullptr*/) {
  int size = a.size();
  vector<Share> shareA(size), shareC(size);
  
  helix_convert_string_to_share(a, shareA);
  AUDIT("id:{}, Sqrt P{} input{}", _op_msg_id.get_hex(), hi->party_id(), Vector<Share>(shareA));
  hi->Sqrt(shareA, shareC);
  helix_convert_share_to_string(shareC, output);

  AUDIT("id:{}, Sqrt P{} output{}", _op_msg_id.get_hex(), hi->party_id(), Vector<Share>(shareC));
  return 0;
}

} // namespace rosetta
