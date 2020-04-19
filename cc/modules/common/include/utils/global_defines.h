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

/*
about matrices' typedef
*/
#include <Eigen/Dense>
namespace Eigen {
typedef Eigen::Matrix<uint64_t, -1, -1> MatrixXul;
typedef Eigen::Matrix<int64_t, -1, -1> MatrixXl;
typedef Eigen::Matrix<uint32_t, -1, -1> MatrixXui;
} // namespace Eigen
using namespace Eigen;

#ifndef FLOAT_PRECISION
#define FLOAT_PRECISION 13
#endif

#ifndef SML_USE_INT64
#define SML_USE_INT64 0
#endif

#ifndef SML_USE_UINT64
#define SML_USE_UINT64 0
#endif

#if SML_USE_UINT32
typedef Eigen::MatrixXui MATRIX;
typedef uint32_t DType;
typedef int32_t DType2;
#elif SML_USE_INT64
typedef Eigen::MatrixXl MATRIX;
typedef int64_t DType;
typedef int64_t DType2;
#elif SML_USE_UINT64
typedef Eigen::MatrixXul MATRIX;
typedef uint64_t DType;
typedef int64_t DType2;
#else // default
#define SML_USE_DOUBLE 1
typedef Eigen::MatrixXd MATRIX;
typedef double DType;
typedef double DType2;
#undef FLOAT_PRECISION
#define FLOAT_PRECISION 0
#endif

#include <vector>
typedef std::vector<MATRIX> MATRIX3D;
typedef std::vector<std::vector<MATRIX>> MATRIX4D;

// clang-format off
#define USE_SNN 0
#if SML_USE_DOUBLE
#define floatToDType(a) (a)
#define dividePlainS(a) (a)
#define DTypeTofloat(a) (a)
#else
#define floatToDType(a) ((DType)(a * (1 << FLOAT_PRECISION)))
#define dividePlainS(a) ((DType)((double)((DType2)a) / (double)((DType2)(1 << FLOAT_PRECISION))))
#define DTypeTofloat(a) ((double)((DType2)a) / (double)((DType2)(1 << FLOAT_PRECISION)))
#endif
// clang-format on
