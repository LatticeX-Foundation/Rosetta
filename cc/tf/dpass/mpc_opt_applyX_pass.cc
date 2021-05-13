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

// TF Optimizer type
typedef enum tagTF_OPT_TYPE {
  TF_OPT_UNKOWN = -1, // unkown the tf optimizer
  TF_OPT_SDG = 0,     // tf.train.GradientDescentOptimizer
  TF_OPT_PROX_GD,     // tf.train.ProximalGradientDescentOptimizer      
  TF_OPT_ADA_DALTA,   // tf.train.AdadeltaOptimizer
  TF_OPT_ADA_G,       // tf.train.AdagradOptimizer
  TF_OPT_PROX_ADA_G,  // tf.train.ProximalAdagradOptimizer
  TF_OPT_ADA_G_DA,    // tf.train.AdagradDAOptimizer
  TF_OPT_FTRL,        // tf.train.FtrlOptimizer
  TF_OPT_FTRL_V2,     // tf.train.FtrlOptimizer V2
  TF_OPT_MOM,         // tf.train.MomentumOptimizer
  TF_OPT_ADAM,        // tf.train.AdamOptimizer
  TF_OPT_RMSP,        // tf.train.RMSPropOptimizer
}E_TFOptType;


class MPCOptApplyXPass : public MpcBasePass {
  private:
  // Get tensorflow optimizer type.
  E_TFOptType GetOptType(const GraphOptimizationPassOptions& options) { 
    for (auto node : options.graph->get()->op_nodes()) {
        string NodeTypeName = node->type_string();
        if (NodeTypeName.find("Apply") != 0)
          continue;

        if (NodeTypeName == "ApplyGradientDescent")
          return E_TFOptType::TF_OPT_SDG;
        else if (NodeTypeName == "ApplyProximalGradientDescent")
          return E_TFOptType::TF_OPT_PROX_GD;
        else if (NodeTypeName == "ApplyAdadelta")
          return E_TFOptType::TF_OPT_ADA_DALTA;
        else if (NodeTypeName == "ApplyAdagrad")
          return E_TFOptType::TF_OPT_ADA_G;
        else if (NodeTypeName == "ApplyProximalAdagrad")
          return E_TFOptType::TF_OPT_PROX_ADA_G;
        else if (NodeTypeName == "ApplyAdagradDA")
          return E_TFOptType::TF_OPT_ADA_G_DA;
        else if (NodeTypeName == "ApplyFtrl")
          return E_TFOptType::TF_OPT_FTRL;
        else if (NodeTypeName == "ApplyFtrlV2")
          return E_TFOptType::TF_OPT_FTRL_V2;
        else if (NodeTypeName == "ApplyMomentum")
          return E_TFOptType::TF_OPT_MOM;
        else if (NodeTypeName == "ApplyAdam")
          return E_TFOptType::TF_OPT_ADAM;
        else if (NodeTypeName == "ApplyRMSProp")
          return E_TFOptType::TF_OPT_RMSP;
    }

    return E_TFOptType::TF_OPT_UNKOWN;
  }

  public:
  // Replace ApplyGradientDescent OP with MpcApplyGradientDescent OP
  Status ReplaceApplyGradientDescentOp(const GraphOptimizationPassOptions& options) {
    
    // Dump the graph before replace the ApplyGradientDescent OP
    if (DumpAllGraphs())
      DumpGraphs(options, 0, "BeforeRunMpcOptApplyXPass", "Before Run ApplyGradientDescent Pass");

    // Replace ApplyGradientDescent OP with MpcApplyGradientDescent OP
    std::vector<Node*> replaced_nodes;
    Node* MpcApplyGD = nullptr;
    int Idx = 0;
    std::string MpcApplyXOpName = "MpcApplyGradientDescent";
    for (auto node : options.graph->get()->op_nodes()) {
      if (node->type_string() == "ApplyGradientDescent") {
        ROSETTA_VLOG(4) << "Capturing: " << node->name();

        // Generate MpcApplyGradientDescent OP unique name
        if (Idx > 0) {
          char szBufTemp[64] = {0};
          snprintf(szBufTemp, sizeof(szBufTemp), "MpcApplyGradientDescent_%d", Idx);
          MpcApplyXOpName = szBufTemp;
        }
        Idx++;

        // Get data type, and use_locking
        DataType dtype;
        TF_RETURN_IF_ERROR(GetNodeAttr(node->attrs(), "T", &dtype));
        //bool use_locking;
        //TF_RETURN_IF_ERROR(GetNodeAttr(node->attrs(), "use_locking", &use_locking));

        // Get node non-control inputs
        NodeBuilder::NodeOut input_var;
        NodeBuilder::NodeOut input_alpha;
        NodeBuilder::NodeOut input_delta;
        std::vector<const Edge*> input_edges;
        TF_RETURN_IF_ERROR(node->input_edges(&input_edges));
        ROSETTA_VLOG(1) << "Num of the ApplyGradientDescent input edges:" << input_edges.size();
        input_var   = NodeBuilder::NodeOut(input_edges[0]->src(), input_edges[0]->src_output());
        input_alpha = NodeBuilder::NodeOut(input_edges[1]->src(), input_edges[1]->src_output());
        input_delta = NodeBuilder::NodeOut(input_edges[2]->src(), input_edges[2]->src_output());

        // Get node control inputs
        // std::vector<Node*> control_inputs;
        // control_inputs.reserve(node->in_edges().size() - node->num_inputs());
        // for (const Edge* in_edge : node->in_edges()) {
        //   if (in_edge->IsControlEdge()) {
        //     control_inputs.push_back(in_edge->src());
        //   }
        // }
        // ROSETTA_VLOG(1) << "Num of the ApplyGradientDescent control input edges:" << control_inputs.size();

        // Build mpc ApplyGradientDescent op
        TF_RETURN_IF_ERROR(NodeBuilder(MpcApplyXOpName.c_str(), "MpcApplyGradientDescent")
                               .Attr("T", dtype)
                               //.Attr("use_locking", use_locking)
                               .Device(node->assigned_device_name())
                               .Input(input_var)
                               .Input(input_alpha)
                               .Input(input_delta)
                               //.ControlInputs(control_inputs)
                               .Finalize(options.graph->get(), &MpcApplyGD));
        MpcApplyGD->set_assigned_device_name(node->assigned_device_name());

        // Print replace node edge info
        for (auto edge : MpcApplyGD->in_edges()) {
          ROSETTA_VLOG(4) << edge->DebugString();
        }
        ROSETTA_VLOG(4) << "MpcApplyGradientDescent inputs:" << MpcApplyGD->num_inputs(); 
        ROSETTA_VLOG(4) << "MpcApplyGradientDescent outputs:" << MpcApplyGD->num_outputs();

        // Add edge from the input nodes to the MpcApplyGradientDescent node 
        ROSETTA_VLOG(4) << "Replacing Node " << node->DebugString() << " with "
                              << MpcApplyGD->DebugString();

        TF_RETURN_IF_ERROR(ReplaceInputEdges(options.graph->get(), node, MpcApplyGD));
        TF_RETURN_IF_ERROR(ReplaceOutputEdges(options.graph->get(), node, MpcApplyGD));
        
        replaced_nodes.push_back(node);
      }
    }

    // Remove the ApplyGradientDescent node
    for (auto node : replaced_nodes) {
      ROSETTA_VLOG(4) << "Removing: " << node->name();
      options.graph->get()->RemoveNode(node);
    }

    // Dump the graph after replace the ApplyGradientDescent OP
    if (DumpAllGraphs())
      DumpGraphs(options, 0, "AfterRunMpcOptApplyXPass", "After Run MpcApplyGradientDescent Pass");

    return Status::OK();
  }

  // Replace ApplyProximalGradientDescent OP with MpcApplyProximalGradientDescent OP
  Status ReplaceApplyProximalGradientDescentOp(const GraphOptimizationPassOptions& options) {
    //do nothing now,implement at the next stage
    return Status::OK();
  }

  // Replace ApplyAdadelta OP with MpcApplyAdadelta OP
  Status ReplaceApplyAdadeltaOp(const GraphOptimizationPassOptions& options) {
    //do nothing now,implement at the next stage
    return Status::OK();
  }

  // Replace ApplyAdagrad OP with MpcApplyAdagrad OP
  Status ReplaceApplyAdagradOp(const GraphOptimizationPassOptions& options) {
    //do nothing now,implement at the next stage
    return Status::OK();
  }

  // Replace ApplyProximalAdagrad OP with MpcApplyProximalAdagrad OP
  Status ReplaceApplyProximalAdagradOp(const GraphOptimizationPassOptions& options) {
    //do nothing now,implement at the next stage
    return Status::OK();
  }

  // Replace ApplyAdagradDA OP with MpcApplyAdagradDA OP
  Status ReplaceApplyAdagradDAOp(const GraphOptimizationPassOptions& options) {
    //do nothing now,implement at the next stage
    return Status::OK();
  }

  // Replace ApplyFtrl OP with MpcApplyFtrl OP
  Status ReplaceApplyFtrlOp(const GraphOptimizationPassOptions& options) {
    //do nothing now,implement at the next stage
    return Status::OK();
  }

  // Replace ApplyFtrlV2 OP with MpcApplyFtrlV2 OP
  Status ReplaceApplyFtrlV2Op(const GraphOptimizationPassOptions& options) {
    //do nothing now,implement at the next stage
    return Status::OK();
  }

  // Replace ApplyMomentum OP with MpcApplyMomentum OP
  Status ReplaceApplyMomentumOp(const GraphOptimizationPassOptions& options) {
    //do nothing now,implement at the next stage
    return Status::OK();
  }

  // Replace ApplyAdam OP with MpcApplyAdam OP
  Status ReplaceApplyAdamOp(const GraphOptimizationPassOptions& options) {
    //do nothing now,implement at the next stage
    return Status::OK();
  }

  // Replace ApplyRMSProp OP with MpcApplyRMSProp OP
  Status ReplaceApplyRMSPropOp(const GraphOptimizationPassOptions& options) {
    //do nothing now,implement at the next stage
    return Status::OK();
  }


  Status Run(const GraphOptimizationPassOptions& options) override {
    ROSETTA_VLOG(4) << "-----------------------" << endl;
    ROSETTA_VLOG(4) << "Run MPC Optimizate applyX Pass" << endl;
    ROSETTA_VLOG(4) << "-----------------------" << endl;

    // If we don't get a main graph, log that fact and bail.
    if (options.graph == nullptr) {
      ROSETTA_VLOG(0) << "GraphOptimizationPassOptions: options.graph == nullptr";
      return Status::OK();
    }

    // Get optimizer type and run pass
    Status status;
    E_TFOptType eOptType = GetOptType(options);
    switch(eOptType) {
      case TF_OPT_SDG:
        status = ReplaceApplyGradientDescentOp(options);
        break;

      case TF_OPT_PROX_GD:
        status = ReplaceApplyProximalGradientDescentOp(options);
        break;

      case TF_OPT_ADA_DALTA:
        status = ReplaceApplyAdadeltaOp(options);
        break;

      case TF_OPT_ADA_G:
        status = ReplaceApplyAdagradOp(options);
        break;

      case TF_OPT_PROX_ADA_G:
        status = ReplaceApplyProximalAdagradOp(options);
        break;

      case TF_OPT_ADA_G_DA:
        status = ReplaceApplyAdagradDAOp(options);
        break;

      case TF_OPT_FTRL:
        status = ReplaceApplyFtrlOp(options);
        break;

      case TF_OPT_FTRL_V2:
        status = ReplaceApplyFtrlV2Op(options);
        break;

      case TF_OPT_MOM:
        status = ReplaceApplyMomentumOp(options);
        break;

      case TF_OPT_ADAM:
        status = ReplaceApplyAdamOp(options);
        break;

      case TF_OPT_RMSP:
        status = ReplaceApplyRMSPropOp(options);
        break;

      default:
        ROSETTA_VLOG(4) << "unkown the tf optimizer.";
        break;
    }

    return status;
  }
};

}

// SGD apply gradient descent op (Var -= leraning_rate * delta_Var) must be local mul, so we don't registe MPCOptApplyXPass pass.
// REGISTER_OPTIMIZATION(OptimizationPassRegistry::POST_REWRITE_FOR_EXEC, 0,
//                       rosetta::MPCOptApplyXPass);

};



