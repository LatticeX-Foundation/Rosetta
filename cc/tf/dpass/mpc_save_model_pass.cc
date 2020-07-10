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
#include "cc/tf/dpass/mpc_base_pass.h"

namespace tensorflow {
namespace rosetta {

class MPCSaveModelPass : public MpcBasePass {
  public:
  Status Run(const GraphOptimizationPassOptions& options) override {
    ROSETTA_VLOG(4) << "-----------------------" << endl;
    ROSETTA_VLOG(4) << "Run MPC Save Model Pass" << endl;
    ROSETTA_VLOG(4) << "-----------------------" << endl;

    // If we don't get a main graph, log that fact and bail.
    if (options.graph == nullptr) {
      ROSETTA_VLOG(0) << "GraphOptimizationPassOptions: options.graph == nullptr";
      return Status::OK();
    }

    // Check if the SaveV2 OP be repalce with MpcSaveV2 OP
    // If the model is kept on one side, we need to replace the SaveV2 OP,
    // otherwise do nothing.

    // Check if the graph has exist the SaveV2 OP
    if (!IsExistTheOP(options, "SaveV2"))
      return Status::OK();

    // Dump the graph before replace the SaveV2 OP
    if (DumpAllGraphs())
      DumpGraphs(options, 0, "BeforeRunMpcSaveModelPass", "Before Run MpcSaveV2 Pass");

    // Replace SaveV2 OP with MpcSaveV2 OP
    std::vector<Node*> replaced_nodes;
    Node* MpcSaveV2 = nullptr;
    for (auto node : options.graph->get()->op_nodes()) {
      if (node->type_string() == "SaveV2") {
        ROSETTA_VLOG(4) << "Capturing: " << node->name();

        // Get data type,has_minimum,minimum attribute values
        std::vector<DataType> input_dtypes;
        TF_RETURN_IF_ERROR(GetNodeAttr(node->attrs(), "dtypes", &input_dtypes));

        // Get node inputs
        NodeBuilder::NodeOut input_prefix;
        NodeBuilder::NodeOut input_tensorname;
        NodeBuilder::NodeOut input_shape_and_slices;
        std::vector<NodeBuilder::NodeOut> input_tensors(input_dtypes.size());
        std::vector<const Edge*> input_edges;
        TF_RETURN_IF_ERROR(node->input_edges(&input_edges));
        ROSETTA_VLOG(1) << "Num of the SaveV2 input edges:" << input_edges.size();
        input_prefix          = NodeBuilder::NodeOut(input_edges[0]->src(), input_edges[0]->src_output());
        input_tensorname      = NodeBuilder::NodeOut(input_edges[1]->src(), input_edges[1]->src_output());
        input_shape_and_slices= NodeBuilder::NodeOut(input_edges[2]->src(), input_edges[2]->src_output());
        const int nStartEdgeIdx = 3;
        for (int i = nStartEdgeIdx; i < (nStartEdgeIdx+input_dtypes.size()); i++)
          input_tensors[i-nStartEdgeIdx] = NodeBuilder::NodeOut(input_edges[i]->src(), input_edges[i]->src_output());

        // Build mpc savev2 op
        TF_RETURN_IF_ERROR(NodeBuilder("MpcSaveV2", "MpcSaveV2")
                               .Attr("dtypes", input_dtypes)
                               .Device(node->assigned_device_name())
                               .Input(input_prefix)
                               .Input(input_tensorname)
                               .Input(input_shape_and_slices)
                               .Input(input_tensors)
                               .Finalize(options.graph->get(), &MpcSaveV2));
        MpcSaveV2->set_assigned_device_name(node->assigned_device_name());

        // Print replace node edge info
        for (auto edge : MpcSaveV2->in_edges()) {
          ROSETTA_VLOG(4) << edge->DebugString();
        }
        ROSETTA_VLOG(4) << "MpcSaveV2 inputs:" << MpcSaveV2->num_inputs(); 
        ROSETTA_VLOG(4) << "MpcSaveV2 outputs:" << MpcSaveV2->num_outputs();

        // Add edge from the input nodes to the MpcSaveV2 node 
        ROSETTA_VLOG(4) << "Replacing Node " << node->DebugString() << " with "
                              << MpcSaveV2->DebugString();

        TF_RETURN_IF_ERROR(ReplaceInputEdges(options.graph->get(), node, MpcSaveV2));
        TF_RETURN_IF_ERROR(ReplaceOutputEdges(options.graph->get(), node, MpcSaveV2));
        
        replaced_nodes.push_back(node);
      }
    }

    // Remove the SaveV2 node
    for (auto node : replaced_nodes) {
      ROSETTA_VLOG(4) << "Removing: " << node->name();
      options.graph->get()->RemoveNode(node);
    }

    // After remove SaveV2 node, prinf edge info
    for (auto edge : MpcSaveV2->in_edges()) {
      ROSETTA_VLOG(4) << edge->DebugString();
    }
    ROSETTA_VLOG(4) << "MpcSaveV2 inputs:" << MpcSaveV2->num_inputs(); 
    ROSETTA_VLOG(4) << "MpcSaveV2 outputs:" << MpcSaveV2->num_outputs();

    // Dump the graph after replace the SaveV2 OP
    if (DumpAllGraphs())
      DumpGraphs(options, 0, "AfterRunMpcSaveModelPass", "After Run MpcSaveV2 Pass");

    return Status::OK();
  }
  
};

}

// REGISTER_OPTIMIZATION(OptimizationPassRegistry::POST_REWRITE_FOR_EXEC, 0,
//                       rosetta::MPCSaveModelPass);
};




