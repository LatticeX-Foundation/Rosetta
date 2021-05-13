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

#include <fstream>
#include <ostream>
#include <sstream>

#include "tensorflow/core/common_runtime/dma_helper.h"
#include "tensorflow/core/framework/op_kernel.h"
#include "tensorflow/core/graph/graph.h"
#include "tensorflow/core/platform/tensor_coding.h"
#include "tensorflow/core/util/saved_tensor_slice_util.h"


using namespace std;
namespace tensorflow {

namespace rosetta {

std::string DotFilename(std::string, int);

std::string DotFilename(std::string kind, int idx, int sub_idx);

std::string PbtxtFilename(std::string, int);

std::string PbtxtFilename(std::string kind, int idx, int sub_idx);

std::string GraphFilenamePrefix(std::string, int);

std::string GraphFilenamePrefix(std::string, int, int);

bool DumpAllGraphs();
}  // namespace rosetta

};  // namespace tensorflow




