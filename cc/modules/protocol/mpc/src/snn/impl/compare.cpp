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
#include "op_impl.h"

namespace rosetta {
namespace mpc {

// clang-format off
int Equal::funcEqual(const vector<mpc_t> &a, const vector<mpc_t> &b, vector<mpc_t> &c, size_t size)
{
  if (a.size() == 0)
  {
    cout << "-----  error, input with bad size ------" << endl;
    return 1;
  }

  if (a.size() != b.size() || a.size() != c.size())
  {
    cout << "-----  error, input with size not match ------" << endl;
    return 1;
  }

  if (a.size() != size)
  {
    cout << "-----  error, input size not match with specified size------" << endl;
    return 1;
  }

  // compare all the elements
  //sub then compare with zero
  //1. calculate greaterEqual ==> A
  vector<mpc_t> A(a.size(), 0);
  ////GreaterEqual(a, b, A, a.size());
  GetMpcOpInner(GreaterEqual)->Run(a, b, A, a.size());

  //2. calculate lessEqual ==> B
  vector<mpc_t> B(a.size(), 0);
  ////LessEqual(a, b, B, a.size());
  GetMpcOpInner(LessEqual)->Run(a, b, B, a.size());

  //3. result c = A + B - 1
  for (size_t i = 0; i < a.size(); ++i)
  {
    c[i] = A[i] + B[i] - FloatToMpcType(1)/2;
  }

  //cout << ":) mpc equal ok." << endl;
  return 0;
}


int Greater::funcGreater(const vector<mpc_t> &a, const vector<mpc_t> &b, vector<mpc_t> &c, size_t size)
{
  if (a.size() == 0)
  {
    cout << "-----  error, input with bad size ------" << endl;
    return 1;
  }

  if (a.size() != b.size() || a.size() != c.size())
  {
    cout << "-----  error, input with size not match ------" << endl;
    return 1;
  }

  if (a.size() != size)
  {
    cout << "-----  error, input size not match with specified size------" << endl;
    return 1;
  }

  // compare all the elements
  //sub then compare with zero
  // //1. calculate greaterEqual ==> A
  // vector<mpc_t> A(a.size(), 0);
  // GreaterEqual(a, b, A, a.size());

  //2. calculate lessEqual ==> B
  vector<mpc_t> B(a.size(), 0);
  //sub then compare with zero
  vector<mpc_t> cmp(a.size(), 0);
  for (size_t i = 0; i < a.size(); ++i)
  {
    cmp[i] = b[i] - a[i];
  }

  ////funcRELUPrime3PC(cmp, B, size);
  GetMpcOpInner(ReluPrime)->Run3PC(cmp, B, size);

  // //3. calculate equal ==>  E = A + B - 1
  // vector<mpc_t> E(a.size(), 0);
  // for (size_t i = 0; i < a.size(); ++i)
  // {
  //     E[i] = A[i] + B[i] - 1;
  // }

  //4. calculate result ==> c = A - E = A - (A + B - 1) = 1 - B
  for (size_t i = 0; i < a.size(); ++i)
  {
    c[i] = FloatToMpcType(1)/2 - B[i];
  }

  //cout << ":) mpc Greater ok." << endl;
  return 0;
}

int Less::funcLess(const vector<mpc_t> &a, const vector<mpc_t> &b, vector<mpc_t> &c, size_t size)
{
  if (a.size() == 0)
  {
    cout << "-----  error, MpcLess input with bad size ------" << endl;
    return 1;
  }

  if (a.size() != b.size() || a.size() != c.size())
  {
    cout << "-----  error, MpcLess input with size not match ------" << endl;
    return 1;
  }

  if (a.size() != size)
  {
    cout << "-----  error, MpcLess input size not match with specified size------" << endl;
    return 1;
  }

  // compare all the elements
  //sub then compare with zero
  //1. calculate greaterEqual ==> A
  vector<mpc_t> A(a.size(), 0);
  vector<mpc_t> cmp(a.size(), 0);
  for (size_t i = 0; i < a.size(); ++i)
  {
    cmp[i] = a[i] - b[i];
  }

  ////funcRELUPrime3PC(cmp, A, size);
  GetMpcOpInner(ReluPrime)->Run3PC(cmp, A, size);

  //2. calculate lessEqual ==> B

  // //3. calculate equal ==>  E = A + B - 1
  // vector<mpc_t> E(a.size(), 0);
  // for (size_t i = 0; i < a.size(); ++i)
  // {
  //     E[i] = A[i] + B[i] - 1;
  // }

  //4. calculate result ==> c = B - E = B - (A + B - 1) = 1 - A
  for (size_t i = 0; i < a.size(); ++i)
  {
    c[i] = FloatToMpcType(1)/2 - A[i];
  }

  //cout << ":) mpc Less ok." << endl;
  return 0;
}

int GreaterEqual::funcGreaterEqual(const vector<mpc_t> &a, const vector<mpc_t> &b, vector<mpc_t> &c, size_t size)
{
  if (a.size() == 0)
  {
    cout << "-----  error, MpcGreaterEqual input with bad size ------" << endl;
    return 1;
  }

  if (a.size() != b.size() || a.size() != c.size())
  {
    cout << "-----  error, MpcGreaterEqual input with size not match ------" << endl;
    return 1;
  }

  if (a.size() != size)
  {
    cout << "-----  error, MpcGreaterEqual input size not match with specified size------" << endl;
    return 1;
  }

  // compare all the elements
  //sub then compare with zero
  vector<mpc_t> cmp(a.size(), 0);
  for (size_t i = 0; i < a.size(); ++i)
  {
    cmp[i] = a[i] - b[i];
  }

  ////funcRELUPrime3PC(cmp, c, size);
  GetMpcOpInner(ReluPrime)->Run3PC(cmp, c, size);

  //cout << "------> :) => mpc GreaterEqual ok. <= <------" << endl;
  return 0;
}


int LessEqual::funcLessEqual(const vector<mpc_t> &a, const vector<mpc_t> &b, vector<mpc_t> &c, size_t size)
{
    if (a.size() == 0)
    {
        cout << "-----  error, MpcLessEqual input with bad size ------" << endl;
        return 1;
    }

    if (a.size() != b.size() || a.size() != c.size())
    {
        cout << "-----  error, MpcLessEqual input with size not match ------" << endl;
        return 1;
    }

    if (a.size() != size)
    {
        cout << "-----  error, MpcLessEqual input size not match with specified size------" << endl;
        return 1;
    }

    // compare all the elements
    //sub then compare with zero
    vector<mpc_t> cmp(a.size(), 0);
    for (size_t i = 0; i < a.size(); ++i)
    {
        cmp[i] = b[i] - a[i];
    }

    ////funcRELUPrime3PC(cmp, c, size);
    GetMpcOpInner(ReluPrime)->Run3PC(cmp, c, size);

    //cout << "------> :) => mpc LessEqual ok. <= <------" << endl;
  return 0;
}
// clang-format on

} // namespace mpc
} // namespace rosetta
