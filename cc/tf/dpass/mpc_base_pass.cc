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

    void MpcBasePass::DumpGraphs(const GraphOptimizationPassOptions& options, int idx,
                                std::string filename_prefix, std::string title) {
    // If we have a "main" graph, dump that.
    if (options.graph != nullptr) {
      auto dot_filename = DotFilename(filename_prefix, idx);
      auto pbtxt_filename = PbtxtFilename(filename_prefix, idx);
      ROSETTA_VLOG(0) << "Dumping main graph to " << dot_filename;
      ROSETTA_VLOG(0) << "Dumping main graph to " << pbtxt_filename;

      GraphToDotFile(options.graph->get(), dot_filename, title);
      GraphToPbTextFile(options.graph->get(), pbtxt_filename);
    }

    // If we have partition graphs (we shouldn't), dump those.
    if (options.partition_graphs != nullptr) {
      int sub_idx = 0;

      for (auto& kv : *options.partition_graphs) {
        auto dot_filename = DotFilename(filename_prefix, idx, sub_idx);
        auto pbtxt_filename = PbtxtFilename(filename_prefix, idx, sub_idx);
        ROSETTA_VLOG(0) << "Dumping subgraph " << sub_idx << " to "
                              << dot_filename;
        ROSETTA_VLOG(0) << "Dumping subgraph " << sub_idx << " to "
                              << pbtxt_filename;

        Graph* pg = kv.second.get();

        GraphToDotFile(pg, dot_filename, title);
        GraphToPbTextFile(pg, pbtxt_filename);

        sub_idx++;
      }
    }
  }

  Status MpcBasePass::ReplaceInputEdges(Graph* graph, Node* node, Node* replacement) {
    std::vector<const Edge*> edges_to_remove;
    for (auto edge : node->in_edges()) {
      ROSETTA_VLOG(4) << "Replacing: " << edge->DebugString();
      if (!edge->IsControlEdge()) continue;
      //graph->AddEdge(edge->src(), edge->src_output(), replacement,
      //               edge->dst_input());
      edges_to_remove.push_back(edge);
    }

    // Though edges will be removed when we remove the node
    // we specifically remove the edges to be sure
    for (auto edge : edges_to_remove) {
      graph->RemoveEdge(edge);
    }

    //printf edge info
    for (auto edge : replacement->in_edges()) {
      ROSETTA_VLOG(4) << edge->DebugString();
    }
    ROSETTA_VLOG(4) << "the replacement inputs:" << replacement->num_inputs(); 
    ROSETTA_VLOG(4) << "the replacement outputs:" << replacement->num_outputs(); 

    return Status::OK();
  }

  Status MpcBasePass::ReplaceOutputEdges(Graph* graph, Node* node, Node* replacement) {
    std::vector<const Edge*> edges;
    for (auto edge : node->out_edges()) {
      edges.push_back(edge);
    }

    for (auto edge : edges) {
      ROSETTA_VLOG(4) << "Replacing: " << edge->DebugString();
      graph->AddEdge(replacement, edge->src_output(), edge->dst(),
                    edge->dst_input());
      graph->RemoveEdge(edge);
    }

    return Status::OK();
  }
} //namespace rosetta
};

