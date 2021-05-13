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

#include <iomanip>

#include "tensorflow/core/common_runtime/optimization_registry.h"
#include "tensorflow/core/framework/op.h"
#include "tensorflow/core/graph/graph.h"
#include "tensorflow/core/graph/node_builder.h"
#include "tensorflow/core/graph/graph_constructor.h"

#include "cc/tf/dpass/rosetta_log.h"
#include "cc/tf/dpass/tf_graph_writer.h"
#include "cc/tf/dpass/rosetta_utils.h"

namespace tensorflow {
namespace rosetta {
    
class MpcBasePass : public GraphOptimizationPass {
    public:
        virtual Status Run(const GraphOptimizationPassOptions& options) = 0;

    protected:
        // Dump tf graph
        void DumpGraphs(const GraphOptimizationPassOptions& options, int idx,
                        std::string filename_prefix, std::string title);

        // Checked if the graph has exist the name op node, 
        // exist return true, otherwise return false.
        bool IsExistTheOP(const GraphOptimizationPassOptions& options,
                        const std::string &op_type_name) {
            for (auto node : options.graph->get()->op_nodes()) {
                if (node->type_string() == op_type_name)
                    return true;
            }
            return false;
        }

        // Though edges will be removed when we remove the node
        // we specifically remove the edges to be sure
        Status ReplaceInputEdges(Graph* graph, Node* node, Node* replacement);

        // Though edges will be removed when we remove the node
        // we specifically remove the edges to be sure
        Status ReplaceOutputEdges(Graph* graph, Node* node, Node* replacement);
};

} // namespace rosetta
};



