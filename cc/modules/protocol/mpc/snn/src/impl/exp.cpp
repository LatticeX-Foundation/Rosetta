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
#include "cc/modules/protocol/mpc/snn/src/impl/op_impl.h"

//Computes the inverse square root of the input using the Newton-Raphson method.
namespace rosetta {
namespace snn {

int Exp::funcExp(const vector<mpc_t>& a ,vector<mpc_t>& b,size_t size)
{
    const int exp_iterations = 8;
    vector<mpc_t> n(size,FloatToMpcType(2^exp_iterations));//n为实际迭代次数
    vector<mpc_t> m(size , 0) ; //
    vector<mpc_t> result(size , 0);
    const vector<mpc_t> one(size,FloatToMpcType(1));
    GetMpcOpInner(DotProduct)->Run(a, n, m, size);//m=a/2的exp_iterations次方
    addVectors<mpc_t>(m, one, result, size);//result = 1+m
    for(int i=0;i<exp_iterations;++i)
        GetMpcOpInner(Square)->Run(result,result,size);
    b = result;//result = (1 + x / n) ^ n,其中n = 2的8次方
    return 0;
}
}
}