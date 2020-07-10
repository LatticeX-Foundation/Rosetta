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

#include "cc/tf/dpass/tf_graph_writer.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "tensorflow/core/framework/attr_value_util.h"
#include "tensorflow/core/framework/graph.pb.h"
#include "tensorflow/core/framework/node_def_util.h"
#include "tensorflow/core/graph/graph.h"
#include "tensorflow/core/graph/graph_constructor.h"
#include "tensorflow/core/lib/strings/str_util.h"
#include "tensorflow/core/platform/default/logging.h"
#include "tensorflow/core/platform/env.h"
#include "tensorflow/core/platform/protobuf.h"

#include "rosetta_log.h"

using namespace std;

namespace tensorflow {

namespace rosetta {

//-----------------------------------------------------------------------------
// GraphToPbTextFile
//-----------------------------------------------------------------------------
void GraphToPbTextFile(Graph* graph, const string& filename) {
  GraphDef g_def;
  graph->ToGraphDef(&g_def);

  string graph_pb_str;
  protobuf::TextFormat::PrintToString(g_def, &graph_pb_str);
  std::ofstream ostrm_out(filename, std::ios_base::trunc);
  ostrm_out << graph_pb_str;
}

//-----------------------------------------------------------------------------
// GraphToDotFile
//-----------------------------------------------------------------------------
void GraphToDotFile(Graph* graph, const std::string& filename,
                    const std::string& title) {
  std::string dot = GraphToDot(graph, title);
  std::ofstream ostrm_out(filename, std::ios_base::trunc);
  ostrm_out << dot;
}

static std::string color_string(unsigned int color) {
  std::stringstream ss;
  ss << "#" << std::setfill('0') << std::setw(6) << std::hex
     << (color & 0xFFFFFF);
  return ss.str();
}

//-----------------------------------------------------------------------------
// GraphToDot
//-----------------------------------------------------------------------------
std::string GraphToDot(Graph* graph, const std::string& title) {
  //
  // We'll be assigning distinct colors to the nodes in each found cluster.
  //

  // Apparently these are called "Kelly's 22 colors of maximum contrast."
  static const int num_cluster_colors = 22;
  static unsigned int cluster_bg_colors[num_cluster_colors]{
      0xF3C300, 0x875692, 0xF38400, 0xA1CAF1, 0xBE0032, 0xC2B280,
      0x848482, 0x008856, 0xE68FAC, 0x0067A5, 0xF99379, 0x604E97,
      0xF6A600, 0xB3446C, 0xDCD300, 0x882D17, 0x8DB600, 0x654522,
      0xE25822, 0x2B3D26, 0xF2F3F4, 0x222222};

  // We want to color text either black or white, depending on what works best
  // with the chosen background color.
  //
  // Algorithm found at:
  //
  //   https://stackoverflow.com/questions/3942878/how-to-decide-font-color-in-white-or-black-depending-on-background-color
  static auto make_fg_color = [](unsigned int bg_color) {
    unsigned int red = (bg_color & 0xFF0000) >> 16;
    unsigned int green = (bg_color & 0x00FF00) >> 8;
    unsigned int blue = (bg_color & 0x0000FF);

    if (red * 0.299 + green * 0.587 + blue * 0.114 > 186) {
      return 0x000000;
    } else {
      return 0xFFFFFF;
    }
  };

  int seen_cluster_count = 0;
  std::map<int, unsigned int> cluster_color_map;

  //
  // Emit preamble.
  //
  std::ostringstream dot_string;
  dot_string << "digraph G {\n";
  dot_string << "labelloc=\"t\";\n";
  dot_string << "label=<<b>TensorFlow Graph: " << title << "</b><br/><br/>>;\n";

  //
  // Emit each node.
  //
  for (auto id = 0; id < graph->num_node_ids(); ++id) {
    const Node* node = graph->FindNodeId(id);

    // Skip deleted nodes.
    if (node == nullptr) continue;

    // Skip source and sink nodes.
    if (!node->IsOp()) continue;

    // TODO(amprocte): duplicated logic from ngraph_mark_for_clustering but
    // this file does not live inside src.
    bool is_marked;
    if (GetNodeAttr(node->attrs(), "_ngraph_marked_for_clustering",
                    &is_marked) != Status::OK()) {
      is_marked = false;
    }

    // Decide on colors, style.
    unsigned int bg_color = 0xf2f2f2;
    string style;

    // Clustered nodes get a color based on their cluster index.
    int cluster_idx;
    if (GetNodeAttr(node->attrs(), "_ngraph_cluster", &cluster_idx) ==
        Status::OK()) {
      if (cluster_color_map.find(cluster_idx) == cluster_color_map.end()) {
        bg_color = cluster_bg_colors[seen_cluster_count];
        cluster_color_map[cluster_idx] = bg_color;
        if (seen_cluster_count < num_cluster_colors - 1) {
          seen_cluster_count++;
        }
      } else {
        bg_color = cluster_color_map[cluster_idx];
      }
      style = "filled";
    }
    // Nodes marked for clustering get a solid border and an Intel blue
    // background.
    else if (is_marked) {
      style = "solid,filled";
      bg_color = 0x0071c5;
    }
    // Any other nodes are dashed.
    else {
      style = "dashed,filled";
    }

    unsigned int fg_color = make_fg_color(bg_color);
    unsigned int border_color = 0x000000;

    dot_string << "node_" << node;
    dot_string << " [label=<<b>" << node->type_string() << "</b><br/>";
    dot_string << node->name() << "<br/>";

    // Print the data type if this node an op node
    DataType datatype;
    if (GetNodeAttr(node->def(), "T", &datatype) == Status::OK()) {
      dot_string << DataTypeString(datatype) << "<br/>";
    }

    dot_string << ">";

    dot_string << ", shape=rect";
    dot_string << ", style=\"" << style << "\"";
    dot_string << ", fontcolor=\"" << color_string(fg_color) << "\"";
    dot_string << ", color=\"" << color_string(border_color) << "\"";
    dot_string << ", fillcolor=\"" << color_string(bg_color) << "\"";
    dot_string << " ];" << std::endl;

    //
    // Emit each of this node's input edges. (Graphviz does not care about
    // order here.)
    //
    for (const Edge* edge : node->in_edges()) {
      if (edge != nullptr) {
        const Node* src = edge->src();
        const Node* dst = edge->dst();

        // Skip edges from source and sink nodes.
        if (!src->IsOp()) continue;

        // Control edges are red, data edges are black.
        string arrow_color = edge->IsControlEdge() ? "#ff0000" : "#000000";

        dot_string << "node_" << src << " -> ";
        dot_string << "node_" << dst << " [color=\"" << arrow_color << "\"]\n";
      }
    }
  }

  //
  // Emit footer.
  //
  dot_string << "}\n";

  return dot_string.str();
}

//-----------------------------------------------------------------------------
// PbTextFileToDotFile
//-----------------------------------------------------------------------------
void PbTextFileToDotFile(const std::string& pbtxt_filename,
                         const std::string& dot_filename,
                         const std::string& title) {
  GraphDef gdef;
  auto status = ReadTextProto(Env::Default(), pbtxt_filename, &gdef);
  if (status != Status::OK()) {
    ROSETTA_VLOG(5) << "Can't read protobuf graph";
    return;
  }

  Graph input_graph(OpRegistry::Global());
  GraphConstructorOptions opts;
  opts.allow_internal_ops = true;
  status = ConvertGraphDefToGraph(opts, gdef, &input_graph);
  if (status != Status::OK()) {
    ROSETTA_VLOG(5) << "Can't convert graphdef to graph";
    return;
  }

  GraphToDotFile(&input_graph, dot_filename, title);
}

}  // namespace rosetta

}  // namespace tensorflow

