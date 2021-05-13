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

#include <ostream>

#include "tensorflow/core/framework/op_kernel.h"
#include "tensorflow/core/graph/graph.h"
#include "tensorflow/core/platform/tensor_coding.h"
#include "tensorflow/core/util/saved_tensor_slice_util.h"

namespace tensorflow {

namespace rosetta {

// GraphToDot
// Transforms a TensorFlow graph to a DOT file for rendering with graphviz
std::string GraphToDot(Graph* graph, const std::string& title);

// GraphToDotFile
// Saves a TensorFlow graph into a DOT file for rendering with graphviz
void GraphToDotFile(Graph* graph, const std::string& filename,
                    const std::string& title);

// GraphToPbTextFile
// Saves a TensorFlow graph into a protobuf text
void GraphToPbTextFile(Graph* graph, const std::string& filename);

// PbTextFileToDotFile
// Saves a protobuf text into a DOT file
void PbTextFileToDotFile(const std::string& pbtxt_filename,
                         const std::string& dot_filename,
                         const std::string& title);

}  // namespace rosetta

}  // namespace tensorflow

