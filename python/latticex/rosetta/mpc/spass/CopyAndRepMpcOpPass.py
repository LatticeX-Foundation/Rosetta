# ==============================================================================
# Copyright 2020 The LatticeX Foundation
# This file is part of the Rosetta library.
#
# The Rosetta library is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The Rosetta library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with the Rosetta library. If not, see <http://www.gnu.org/licenses/>.
# =============================================================================="
import tensorflow as tf
from latticex.rosetta.mpc import decorator as mpcops
from tensorflow.python.framework import ops
from copy import deepcopy
from latticex.rosetta.mpc.utils.common import is_mpc_compare_tensor
from latticex.rosetta.mpc.ops.math_ops import mpc_cast

import logging
logging.basicConfig(format='%(asctime)s - %(pathname)s[line:%(lineno)d] - %(levelname)s: %(message)s', level=logging.DEBUG)



class CopyAndRepMpcOpPass():
    """
    A pass class for copy and replace tensorflow static graph, traverse the graph op,
    if the op is mpc op, we replace it with mpc op, otherwise, deepcopy it.

    For example:
    # Build a graph.
    a = tf.Variable(...)
    b = tf.Variable(...)
    c = a * b
    # The static graph as below:
            Mul:c
           /    \
        Var:a   Var:b
    # After run CopyAndRepMpcOpPass::run, the static graph look like this:
            MpcMul:c
           /      \
        Var:a    Var:b
    """

    # class variable, tf op type name map to mpc op type name
    support_mpc_op_type = { "add" :        "MpcAdd",
                            "sub" :        "MpcSub",
                            "multiply" :   "MpcMul",
                            "mul" :        "MpcMul",
                            "div" :        "MpcRealDiv",
                            "matmul" :     "MpcMatMul",
                            "log" :        "MpcLog",
                            "log1p" :      "MpcLog1p",
                            "pow" :        "MpcPow",
                            "sigmoid" :    "MpcSigmoid",
                            "truediv" :    "MpcTruediv",
                            "realdiv" :    "MpcRealDiv",
                            "max" :        "MpcMax",
                            "mean" :       "MpcMean",
                            "equal" :      "MpcEqual",
                            "less" :       "MpcLess",
                            "greater" :    "MpcGreater",
                            "lessequal":   "MpcLessEqual",
                            "greaterequal":"MpcGreaterEqual",
                            #"applyGradientDescentOp" : "MpcApplyGradientDescentOp",
                            #"saveV2" :   "MpcSaveV2"  
                            }

    # class variable, mpc namespace static index
    mpc_ns_idx = 0

    # class variable, map tf native graph to mpc rosetta graph
    tf_graph_mapto_mpc_graph = {}


    def __init__(self, mpc_scope="mpc"):
        """
        Creates a new CopyAndRepMpcOpPass instance.

        :param mpc_scope: mpc namespace
        # mpc_scope: The new instances are automatically inserted into the given 'scope'.
        # If mpc_scope='', it is inserted into the graph's global namespace.However,
        # to avoid naming conflicts, its better to provide a scope.
        # If the instance(s) happens to be a part of collection(s), they are
        #are added to the appropriate collections in to_graph as well.
        #For example, for collection 'grad' which the instance happens to be a
        #part of, given a namespace 'mpc', the new instance will be a part of
        #'grad/mpc' in to_graph.
        """

        # tf op type name map to mpc functors
        self.mpc_functors =   { "add" :         self._create_mpc_add_op,
                                "sub" :         self._create_mpc_sub_op,
                                "multiply" :    self._create_mpc_mul_op,
                                "mul" :         self._create_mpc_mul_op,
                                "div" :         self._create_mpc_realdiv_op,
                                "matmul" :      self._create_mpc_matmul_op,
                                "log" :         self._create_mpc_log_op,
                                "log1p" :       self._create_mpc_log1p_op,
                                "pow" :         self._create_mpc_pow_op,
                                "sigmoid" :     self._create_mpc_sigmoid_op,
                                "truediv" :     self._create_mpc_realdiv_op,
                                "realdiv" :     self._create_mpc_realdiv_op,
                                "max" :         self._create_mpc_max_op,
                                "mean" :        self._create_mpc_mean_op,
                                "equal" :       self._create_mpc_equal_op,
                                "less" :        self._create_mpc_less_op,
                                "greater" :     self._create_mpc_greater_op,
                                "lessequal":    self._create_mpc_lessequal_op,
                                "greaterequal": self._create_mpc_greaterequal_op, 
                                }

        # source graph variable dict, save the train variables
        self.train_vars = {}

        # source graph all variables dict, contain trainable variables
        # & no-trainable global variables & model variables
        self.all_vars = {}

        # mpc namespace
        self.mpc_scope = mpc_scope
        if (CopyAndRepMpcOpPass.mpc_ns_idx != 0):
            self.mpc_scope = mpc_scope + str(CopyAndRepMpcOpPass.mpc_ns_idx)
        CopyAndRepMpcOpPass.mpc_ns_idx += 1

        # src graph op must be replaced mpc op
        self.need_mpc_op_name = []

        # save deepcopy op info using dict
        self.op_info = {}

        # collect all variables, contain trainable variables
        # & no-trainable global variables & model variables
        self._collect_variables()


    def run(self, loss):
        """
        the static pass run function, the pass is deep copy or replace op based on the op data flow.

        :param loss: A `Tensor` containing the value to minimize.
        :return: the new loss tensor
        """

        # Judge the loss has be replace with mpc graph already
        # reture the mpc graph if the loss has be replace, 
        # otherwise we need to exec copy_and_replace_to_graph() function.
        if (loss in CopyAndRepMpcOpPass.tf_graph_mapto_mpc_graph.keys()):
            loss = CopyAndRepMpcOpPass.tf_graph_mapto_mpc_graph[loss]
            return loss

        # Get default graph
        to_graph = tf.get_default_graph()

        # Deep copy and replace source op
        mpc_loss = self.copy_and_replace_to_graph(loss, to_graph)

        # Save the mpc loss
        CopyAndRepMpcOpPass.tf_graph_mapto_mpc_graph[loss] = mpc_loss

        loss = mpc_loss
        return loss


    def copy_vars_to_graph(self, src_var, to_graph,
                           mpc_scope, copied_vars={}):
        """
        Copies the Variable instance 'src_var' into the graph
        'to_graph', under the given mpc_scope.
        The dict 'copied_vars', if provided, will be updated with
        mapping the new variable's name to the instance.

        :param src_var: source variable instance will be copyed
        :param to_graph: dest graph instance
        :param mpc_scope: mpc namespace
        :param copied_vars: Add the new variable to the dict
        :return: None
        """

        if not isinstance(src_var, tf.Variable):
            raise TypeError(str(src_var) + " is not a Variable")

        # The name of the new variable
        if mpc_scope != '':
            new_name = (mpc_scope + '/' + src_var.name[:src_var.name.index(':')])
        else:
            new_name = src_var.name[:src_var.name.index(':')]

        # Get the collections that the new instance needs to be added to.
        # The new collections will also be a part of the given scope,
        # except the special ones required for variable initialization and training.
        collections = []
        for name, collection in src_var.graph._collections.items():
            if src_var in collection:
                if (name == ops.GraphKeys.VARIABLES or
                    name == ops.GraphKeys.TRAINABLE_VARIABLES or
                        mpc_scope == ''):
                    collections.append(name)
                else:
                    collections.append(mpc_scope + '/' + name)

        # See if the variable is trainable var.
        trainable = (src_var in src_var.graph.get_collection(
                                ops.GraphKeys.TRAINABLE_VARIABLES))

        # Get the variable initial value
        with src_var.graph.as_default():
            temp_session = tf.Session()
            init_value = temp_session.run(src_var.initialized_value())

        # Initialize the new variable
        with to_graph.as_default():
            new_var = tf.Variable(init_value,
                                  trainable,
                                  name=new_name,
                                  collections=collections,
                                  validate_shape=False)

        # Add the new variable to the copied_vars dict
            copied_vars[new_var.name] = new_var


    def _collect_variables(self):
        """
        get all variables name, contain trainable variables
        & no-trainable global variables & model variables

        :return:None
        """

        # Get trainable variable
        for var in tf.trainable_variables():
            if self.mpc_scope != '':
                new_name = (self.mpc_scope + '/' + var.name)
            else:
                new_name = var.name
            self.all_vars[new_name] = var
            self.train_vars[new_name] = var

        # Get no-trainable global variable
        for var in tf.global_variables():
            if self.mpc_scope != '':
                new_name = (self.mpc_scope + '/' + var.name)
            else:
                new_name = var.name
            self.all_vars[new_name] = var

        #Get model varibale
        for var in tf.model_variables():
            if self.mpc_scope != '':
                new_name = (self.mpc_scope + '/' + var.name)
            else:
                new_name = var.name
            self.all_vars[new_name] = var


    def copy_and_replace_to_graph(self, src_inst, to_graph):
        """
        Makes a copy of the Operation/Tensor instance 'src_inst' for the graph 'to_graph',
        recursively. Therefore, all required structures linked to org_instance will
        be automatically copied.
        Note:Order of insertion into collections is not preserved

        :param src_inst: source instance, maybe tensor or op.
        :param to_graph: dest graph
        :return: the corresponding instance with respect to to_graph.
        """

        # The name of the new instance
        if self.mpc_scope != '':
            new_name = self.mpc_scope + '/' + src_inst.name
        else:
            new_name = src_inst.name

        # If a variable by the new name already exists,
        # return the correct tensor that will act as an input.
        if new_name in self.all_vars:
            return to_graph.get_tensor_by_name(self.all_vars[new_name].name)

        # If the op by the new name has already exists in op_info,
        # it mean the sub-graph branch has be already handle,
        # return the op immediately.
        if isinstance(src_inst, ops.Operation):
            if new_name in self.op_info:
                return self.op_info[new_name]

        # If the tensor is placeholder tensor, can't copy the op,
        # return it immediately.
        if isinstance(src_inst, ops.Tensor):
            if src_inst.op.op_def.name == "Placeholder":
                return src_inst

        # Get the collections that the new instance needs to be added to.
        # The new collections will also be a part of the given scope.
        collections = []
        for name, collection in src_inst.graph._collections.items():
            if src_inst in collection:
                if self.mpc_scope == '':
                    collections.append(name)
                else:
                    collections.append(self.mpc_scope + '/' + name)

        # Take action based on the class of the source instance
        if isinstance(src_inst, ops.Tensor):
            # If it's a Tensor, it is one of the outputs of the underlying op.
            # Therefore, copy the op itself and return the appropriate output.
            op = src_inst.op
            new_op = self.copy_and_replace_to_graph(op, to_graph)
            output_index = op.outputs.index(src_inst)
            new_tensor = new_op.outputs[output_index]

            # Add to collections if any
            for collection in collections:
                to_graph.add_to_collection(collection, new_tensor)

            return new_tensor

        elif isinstance(src_inst, ops.Operation):
            # If it's a Operation,we deep copy the op while it's not mpc op ,
            # Otherwise, we replace it with appropriate mpc op.
            op = src_inst

            # Take action based on the op, if the op must mpc op, we must
            # replace the op with mpc op, otherwise, deep copy it.
            if self.is_need_mpc_op(op) and self.is_support_mpc_op(op):
                self.need_mpc_op_name.append(new_name)
                return self.create_mpc_op(op, to_graph)
            else:
                return self.deep_copy_op(op, to_graph)

        else:
            # If the instance is not Tensor/Operation, then show error
            raise TypeError("Could not deal with the instance: " + str(src_inst))


    def is_need_mpc_op(self, op):
        """
        Replace the op with mpc op or deep copy the op based on the data flow.

        :param op: judge the op is mpc op or not based on the data flow
        :return: True is mpc op , otherwise is False
        """

        # The name of the new instance
        if self.mpc_scope != '':
            new_name = self.mpc_scope + '/' + op.name
        else:
            new_name = op.name

        # If a variable is trainable, so need mpc op to replace the op
        if new_name in self.all_vars:
            if new_name in self.train_vars:
                return True
            else:
                return False

        # if the op name is in self.need_mpc_op_name, 
        # so need mpc op to replace the op
        if new_name in self.need_mpc_op_name:
            return True

        # Take action based on the class of the source instance
        if isinstance(op, ops.Tensor):
            return self.is_need_mpc_op(op.op)

        elif isinstance(op, ops.Operation):
            # If the op is placeholder, return true
            if (op.op_def.name == "Placeholder"):
                return True

            # If it has inputs, call this function recursively on each.
            inputs_mpc_flag = [self.is_need_mpc_op(x)
                              for x in op.inputs]
            if True in inputs_mpc_flag:
                return True
            else:
                return False

        else:
            # If the op is not Tensor/Operation, return false.
            print("Unkown the op: " + str(op))
            return False

    @staticmethod
    def is_exist_mpc_op(op):
        """
        Does the data flow(subgraph) have one or more MPC node

        :param op: the node is one part of the subgraph
        :return: True mean exist mpc node, otherwise is False
        """

        # Take action based on the class of the source instance
        if isinstance(op, ops.Tensor):
            return CopyAndRepMpcOpPass.is_exist_mpc_op(op.op)

        elif isinstance(op, ops.Operation):
            # get the op def name
            op_def_name = op.op_def.name

            # If the op def name is mpc op name, return ture
            if op_def_name in CopyAndRepMpcOpPass.support_mpc_op_type.values():
                return True

            # If it has inputs, call this function recursively on each input.
            inputs_mpc_flag = [CopyAndRepMpcOpPass.is_exist_mpc_op(x)
                              for x in op.inputs]
            if True in inputs_mpc_flag:
                return True
            else:
                return False

        else:
            # If the op is not Tensor/Operation, return false.
            print("Unkown the op: %s at is_exist_mpc_op() function.", str(op))
            return False


    def is_support_mpc_op(self, op):
        """
        Determine whether the operation supports MPC functionality

        :param op: judge the op support mpc function
        :return: True is supported , otherwise is False
        """

        type_name = op.op_def.name
        type_name = type_name.lower()
        if type_name in CopyAndRepMpcOpPass.support_mpc_op_type:
            return True
        else:
            return False


    def create_mpc_op(self, src_op, to_graph):
        """
        Create a mpc op based on the source op, e.g.: Mul op => MpcMul op

        :param src_op: source op instance.
        :param to_graph: dest graph
        :return: mpc op instance
        """
        
        #get new dest op type name & create functor
        mpc_new_name = CopyAndRepMpcOpPass.support_mpc_op_type[src_op.op_def.name.lower()]
        assert mpc_new_name != ""
        mpc_create_func = self.mpc_functors[src_op.op_def.name.lower()]
        assert mpc_create_func != None
        return mpc_create_func(src_op, mpc_new_name, to_graph)    


    def deep_copy_op(self, src_op, to_graph):
        """
        Deep copy a another op based on the source op, e.g.:Add op => Add op

        :param src_op: source op instance.
        :param to_graph: dest graph
        :return: the deep copy op instance
        """

        # The name of the new instance
        if self.mpc_scope != '':
            new_name = self.mpc_scope + '/' + src_op.name
        else:
            new_name = src_op.name

        # If it has an original_op parameter, copy it
        if src_op._original_op is not None:
            new_original_op = self.copy_and_replace_to_graph(src_op._original_op, to_graph)
        else:
            new_original_op = None

        # If it has control inputs, call this function recursively on each.
        new_control_inputs = [self.copy_and_replace_to_graph(x, to_graph)
                              for x in src_op.control_inputs]

        # If it has inputs, call this function recursively on each.
        new_inputs = [self.copy_and_replace_to_graph(x, to_graph)
                      for x in src_op.inputs]

        # deep copy the tf native op
        return self._deepcopy_op_helper(src_op, new_name, new_inputs,
                        new_control_inputs, new_original_op, to_graph)


    def _deepcopy_op_helper(self, src_op, new_name, new_inputs,
                            new_ctrl_inputs, new_original_op, to_graph):
        """
        deepcopy op helper function

        :param src_op: source op instance.
        :param new_name: new op name.
        :param new_inputs: new op inputs.
        :param new_ctrl_inputs: new op control inputs.
        :param new_original_op: new original op, Used to associate the new `Operation` with an
        # existing `Operation` (for example, a replica with the op that was replicated).
        :param to_graph: dest graph
        :return: the deep copy op instance
        """

        # Make a new node_def based on that of the original.
        # An instance of tensorflow.core.framework.graph_pb2.NodeDef,
        # it stores String-based info such as name, device and type of the op.
        # Unique to every Operation instance.
        new_node_def = deepcopy(src_op.node_def)

        # Change the name
        new_node_def.name = new_name

        # Copy the other inputs needed for initialization
        output_types = src_op._output_types[:]
        input_types = src_op._input_types[:]

        # If new inputs is mpc compare op, we must change the input type to tf.float64
        # so we must create a cast op, the op src dtype == dest dtype == float64
        if (self._is_cast_op(src_op) and is_mpc_compare_tensor(new_inputs[0])):
            new_op = mpc_cast(new_inputs[0], tf.float64, name=new_name).op
            self.op_info[new_name] = new_op
            return new_op

        # Make a copy of the op_def too.
        # Its unique to every _type_ of Operation.
        op_def = deepcopy(src_op.op_def)

        # Initialize a new Operation instance
        new_op = ops.Operation(new_node_def,
                            to_graph,
                            new_inputs,
                            output_types,
                            new_ctrl_inputs,
                            input_types,
                            new_original_op,
                            op_def)

        # Save the op info
        self.op_info[new_name] = new_op

        return new_op


    def _create_mpc_add_op(self, src_op, dest_op_name, to_graph):
        """
        create MpcAdd op

        :param src_op: source op instance, it's not mpc op
        :param dest_op_name: MpcAdd op name.
        :param to_graph: dest graph
        :return: the mpc op instance
        """
        
        mpc_op_input_nums = 2
        mpc_op_creator = mpcops.MpcAdd
        return self._create_mpc_op_helper(src_op, dest_op_name, 
                    mpc_op_input_nums, mpc_op_creator, to_graph)

    
    def _create_mpc_sub_op(self, src_op, dest_op_name, to_graph):
        """
        create MpcSub op

        :param src_op: source op instance, it's not mpc op
        :param dest_op_name: MpcSub op name.
        :param to_graph: dest graph
        :return: the mpc op instance
        """
        
        mpc_op_input_nums = 2
        mpc_op_creator = mpcops.MpcSub
        return self._create_mpc_op_helper(src_op, dest_op_name, 
                    mpc_op_input_nums, mpc_op_creator, to_graph)


    def _create_mpc_mul_op(self, src_op, dest_op_name, to_graph):
        """
        create MpcMul op

        :param src_op: source op instance, it's not mpc op
        :param dest_op_name: MpcMul op name.
        :param to_graph: dest graph
        :return: the mpc op instance
        """
        
        mpc_op_input_nums = 2
        mpc_op_creator = mpcops.MpcMul
        return self._create_mpc_op_helper(src_op, dest_op_name, 
                    mpc_op_input_nums, mpc_op_creator, to_graph)


    def _create_mpc_matmul_op(self, src_op, dest_op_name, to_graph):
        """
        creatMul op

        :param src_op: source op instance, it's not mpc op
        :param dest_op_nameMul op name.
        :param to_graph: dest graph
        :return: the mpc op instance
        """
        
        mpc_op_input_nums = 2
        mpc_op_creator = mpcops.MpcMatMul
        return self._create_mpc_op_helper(src_op, dest_op_name, 
                    mpc_op_input_nums, mpc_op_creator, to_graph)


    def _create_mpc_log_op(self, src_op, dest_op_name, to_graph):
        """
        create MpcLog op

        :param src_op: source op instance, it's not mpc op
        :param dest_op_name: MpcLog op name.
        :param to_graph: dest graph
        :return: the mpc op instance
        """

        mpc_op_input_nums = 1
        mpc_op_creator = mpcops.MpcLog
        return self._create_mpc_op_helper(src_op, dest_op_name, 
                    mpc_op_input_nums, mpc_op_creator, to_graph)    


    def _create_mpc_log1p_op(self, src_op, dest_op_name, to_graph):
        """
        create MpcLog1p op

        :param src_op: source op instance, it's not mpc op
        :param dest_op_name: MpcLog1p op name.
        :param to_graph: dest graph
        :return: the mpc op instance
        """

        mpc_op_input_nums = 1
        mpc_op_creator = mpcops.MpcLog1p
        return self._create_mpc_op_helper(src_op, dest_op_name, 
                    mpc_op_input_nums, mpc_op_creator, to_graph)


    def _create_mpc_pow_op(self, src_op, dest_op_name, to_graph):
        """
        create MpcPow op

        :param src_op: source op instance, it's not mpc op
        :param dest_op_name: MpcPow op name.
        :param to_graph: dest graph
        :return: the mpc op instance
        """

        mpc_op_input_nums = 2
        mpc_op_creator = mpcops.MpcPow
        return self._create_mpc_op_helper(src_op, dest_op_name, 
                    mpc_op_input_nums, mpc_op_creator, to_graph)


    def _create_mpc_sigmoid_op(self, src_op, dest_op_name, to_graph):
        """
        create MpcSigmoid op

        :param src_op: source op instance, it's not mpc op
        :param dest_op_name: MpcSigmoid op name.
        :param to_graph: dest graph
        :return: the mpc op instance
        """

        mpc_op_input_nums = 1
        mpc_op_creator = mpcops.MpcSigmoid
        return self._create_mpc_op_helper(src_op, dest_op_name, 
                    mpc_op_input_nums, mpc_op_creator, to_graph)


    def _create_mpc_realdiv_op(self, src_op, dest_op_name, to_graph):
        """
        create MpcRealdiv op

        :param src_op: source op instance, it's not mpc op
        :param dest_op_name: MpcRealdiv op name.
        :param to_graph: dest graph
        :return: the mpc op instance
        """

        mpc_op_input_nums = 2
        mpc_op_creator = mpcops.MpcTruediv
        return self._create_mpc_op_helper(src_op, dest_op_name, 
                    mpc_op_input_nums, mpc_op_creator, to_graph)


    def _create_mpc_max_op(self, src_op, dest_op_name, to_graph):
        """
        create MpcMax op

        :param src_op: source op instance, it's not mpc op
        :param dest_op_name: MpcMax op name.
        :param to_graph: dest graph
        :return: the mpc op instance
        """
        
        mpc_op_input_nums = 2
        mpc_op_creator = mpcops.MpcMax
        return self._create_mpc_op_helper(src_op, dest_op_name, 
                    mpc_op_input_nums, mpc_op_creator, to_graph)


    def _create_mpc_mean_op(self, src_op, dest_op_name, to_graph):
        """
        create MpcMean op

        :param src_op: source op instance, it's not mpc op
        :param dest_op_name: MpcMean op name.
        :param to_graph: dest graph
        :return: the mpc op instance
        """

        mpc_op_input_nums = 2
        mpc_op_creator = mpcops.MpcMean
        return self._create_mpc_op_helper(src_op, dest_op_name, 
                    mpc_op_input_nums, mpc_op_creator, to_graph)
        

    def _create_mpc_equal_op(self, src_op, dest_op_name, to_graph):
        """
        create MpcEqual op

        :param src_op: source op instance, it's not mpc op
        :param dest_op_name: MpcEqual op name.
        :param to_graph: dest graph
        :return: the mpc op instance
        """

        mpc_op_input_nums = 2
        mpc_op_creator = mpcops.MpcEqual
        return self._create_mpc_op_helper(src_op, dest_op_name, 
                    mpc_op_input_nums, mpc_op_creator, to_graph)


    def _create_mpc_less_op(self, src_op, dest_op_name, to_graph):
        """
        create MpcLess op

        :param src_op: source op instance, it's not mpc op
        :param dest_op_name: MpcLess op name.
        :param to_graph: dest graph
        :return: the mpc op instance
        """

        mpc_op_input_nums = 2
        mpc_op_creator = mpcops.MpcLess
        return self._create_mpc_op_helper(src_op, dest_op_name, 
                    mpc_op_input_nums, mpc_op_creator, to_graph)


    def _create_mpc_greater_op(self, src_op, dest_op_name, to_graph):
        """
        create MpcGreater op

        :param src_op: source op instance, it's not mpc op
        :param dest_op_name: MpcGreater op name.
        :param to_graph: dest graph
        :return: the mpc op instance
        """

        mpc_op_input_nums = 2
        mpc_op_creator = mpcops.MpcGreater
        return self._create_mpc_op_helper(src_op, dest_op_name, 
                    mpc_op_input_nums, mpc_op_creator, to_graph)
    

    def _create_mpc_lessequal_op(self, src_op, dest_op_name, to_graph):
        """
        create MpcLessEqual op

        :param src_op: source op instance, it's not mpc op
        :param dest_op_name: MpcLessEqual op name.
        :param to_graph: dest graph
        :return: the mpc op instance
        """

        mpc_op_input_nums = 2
        mpc_op_creator = mpcops.MpcLessEqual
        return self._create_mpc_op_helper(src_op, dest_op_name, 
                    mpc_op_input_nums, mpc_op_creator, to_graph)


    def _create_mpc_greaterequal_op(self, src_op, dest_op_name, to_graph):
        """
        create MpcGreaterEqual op

        :param src_op: source op instance, it's not mpc op
        :param dest_op_name: MpcGreaterEqual op name.
        :param to_graph: dest graph
        :return: the mpc op instance
        """

        mpc_op_input_nums = 2
        mpc_op_creator = mpcops.MpcGreaterEqual
        return self._create_mpc_op_helper(src_op, dest_op_name, 
                    mpc_op_input_nums, mpc_op_creator, to_graph)



    def _create_mpc_op_helper(self, src_op, mpc_op_name, mpc_op_input_num,
                            mpc_op_creator, to_graph):
        """
        create mpc op helper function

        :param src_op: source op instance, it's not mpc op
        :param mpc_op_name: mpc op name, don't contain namespace
        :param mpc_op_input_num: mpc op input numbers
        :param mpc_op_creator: mpc op creator
        :param to_graph: dest graph
        :return: the mpc op instance
        """

        # The name of the new instance
        if self.mpc_scope != '':
            new_mpc_name = self.mpc_scope + '/' + mpc_op_name
            new_src_name = self.mpc_scope + '/' + src_op.name
        else:
            new_mpc_name = mpc_op_name
            new_src_name = src_op.name

        # If it has an original_op parameter, copy it
        # the original_op must be none.
        if src_op._original_op is not None:
            raise NotImplementedError("not supported")

        # If it has control inputs, call this function recursively on each.
        new_control_inputs = [self.copy_and_replace_to_graph(x, to_graph)
                              for x in src_op.control_inputs]
        assert  len(new_control_inputs) == 0, "%s don't have control input edges" %mpc_op_name

        # If it has inputs, call this function recursively on each.
        new_inputs = [self.copy_and_replace_to_graph(x, to_graph)
                      for x in src_op.inputs]
        assert  len(new_inputs) == mpc_op_input_num, "%s must have 2 input edges" %mpc_op_name

        # checked mpc op inputs
        self._checked_mpc_op_inputs(mpc_op_name, new_inputs)

        # create mpc op
        try:
            if mpc_op_input_num == 1:
                new_op = mpc_op_creator(new_inputs[0], name=new_mpc_name).op 
            elif mpc_op_input_num == 2:
                if self._is_op_unsupport_const_attr(src_op.op_def.name):
                    new_op = self._create_unsupport_const_op(src_op, mpc_op_creator, new_inputs, new_mpc_name)
                    #new_op = mpc_op_creator(new_inputs[0], new_inputs[1], name=new_mpc_name).op
                else:
                    new_op = mpc_op_creator(new_inputs[0], new_inputs[1], name=new_mpc_name,
                                        lh_is_const=self._mpc_input_is_const(new_inputs[0]), 
                                        rh_is_const=self._mpc_input_is_const(new_inputs[1])).op
            elif mpc_op_input_num == 3:
                new_op = mpc_op_creator(new_inputs[0], new_inputs[1], 
                                        new_inputs[2], name=new_mpc_name).op
            elif mpc_op_input_num == 4:
                new_op = mpc_op_creator(new_inputs[0], new_inputs[1], new_inputs[2], 
                                        new_inputs[3], name=new_mpc_name).op
            elif mpc_op_input_num == 5:
                new_op = mpc_op_creator(new_inputs[0], new_inputs[1], new_inputs[2], 
                                        new_inputs[3], new_inputs[4], name=new_mpc_name).op
            elif mpc_op_input_num == 6:
                new_op = mpc_op_creator(new_inputs[0], new_inputs[1], new_inputs[2], 
                                        new_inputs[3], new_inputs[4], new_inputs[5], name=new_mpc_name).op
            else:
                raise ValueError("the %s op don't have %d inputs" %mpc_op_name %mpc_op_input_num)
        except Exception as e:
            logging.exception(e)

        # Save the op info
        self.op_info[new_src_name] = new_op

        # Return the op
        return new_op


    def _checked_mpc_op_inputs(self, mpc_op_name, mpc_op_inputs):
        """
        checked mpc op inputs function

        :param mpc_op_name: mpc op name, don't contain namespace
        :param mpc_op_inputs: mpc op inputs
        :return: None
        """

        # Check the mpc pow parameters are correct
        if (mpc_op_name == "MpcPow"):
            if mpc_op_inputs[0].op.op_def.name != "Const" and mpc_op_inputs[1].op.op_def.name != "Const":
                raise NotImplementedError("only support MpcPow(var * const), MpcPow(var * var) not support")
        pass


    def _mpc_input_is_const(self, mpc_op_input):
        """
        get the mpc input type function

        :param mpc_op_input: mpc op input
        :return: True mean const, False mean non-const
        """
        
        assert mpc_op_input != None, "error:mpcop input is none"
        if mpc_op_input.op.op_def.name == "Const":
            return True
        else:
            return False


    def _is_op_unsupport_const_attr(self, op_def_name):
        """
        check the binary op is support lr/hr_is_const attrubute

        :param op_def_name: op define name
        :return: True mean unsupport lr/hr_is_const attrubute, False mean support
        """

        unsupport_const_attr_op = ("matmul", "mean", "max", "pow")
        op_name = op_def_name.lower()
        if (op_name in unsupport_const_attr_op):
            return True
        else:
            return False


    def _create_mpc_matmul_op_with_attr(self, tf_op, mpc_op, inputs, mpc_name):
        """
        create MpcMatMul op with attrs

        :param tf_op: the origin tensorflow operation
        :param mpc_op: mpc op mapping to MPC c++
        :param inputs: the input of the operation
        :param op_def_name: the origin name of tensorflow operation
        :param mpc_name: name of Mpc operation, eg. mpc/MpcMatMul

        :return: the new created Operation
        """
        
        attrs_map = {}
        for attr in tf_op.op_def.attr:
            attrs_map[attr.name] = attr.default_value.b
        
        return mpc_op(inputs[0], inputs[1], transpose_a=attrs_map['transpose_a'], transpose_b=attrs_map['transpose_b'], name=mpc_name).op


    def _create_unsupport_const_op(self, tf_op, mpc_op, inputs, mpc_name):
        """
        create operation not supports const attribute

        :param tf_op: the origin tensorflow operation
        :param mpc_op: mpc op mapping to MPC c++
        :param inputs: the input of the operation
        :param op_def_name: the origin name of tensorflow operation
        :param mpc_name: name of Mpc operation, eg. mpc/MpcMatMul

        :return: the new created Operation
        """
        op_def_name = tf_op.op_def.name
        op_name = op_def_name.lower()
        new_op = None
        if op_name == 'matmul':
            new_op = self._create_mpc_matmul_op_with_attr(tf_op, mpc_op, inputs, mpc_name)
        else:
            new_op = mpc_op(inputs[0], inputs[1], name=mpc_name).op
        
        return new_op


    def _is_cast_op(self, src_op):
        """
        whether the op is tf cast op

        :param src_op: source graph op
        :return: True mean tf cast op, otherwise is False
        """
        op_def_name = src_op.op_def.name
        op_name = op_def_name.lower()
        if op_name == 'cast':
            return True
        else:
            return False


