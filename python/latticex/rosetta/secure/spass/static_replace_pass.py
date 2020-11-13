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
from tensorflow.python.framework import ops
from copy import deepcopy
from latticex.rosetta.secure import decorator as secure_ops
from latticex.rosetta.secure import MsgIdGenerator
from latticex.rosetta.rtt.ops.rtt_math_ops import rtt_cast
from latticex.rosetta.controller.common_util import rtt_get_logger
import re


class StaticReplacePass():
    """
    A pass class for copy and replace tensorflow static graph, traverse the graph op,
    if the op with secure properties, we replace it with secure op, otherwise, deepcopy it.

    For example:
    # Build a graph.
    a = tf.Variable(...)
    b = tf.Variable(...)
    c = a * b
    # The static graph as below:
            Mul:c
           /    \
        Var:a   Var:b
    # After run StaticReplacePass::run, the static graph look like this:
          SecureMul:c
           /      \
        Var:a    Var:b
    """

    # class variable, rtt namespace static index
    rtt_ns_idx = 0

    # class variable, map tf native graph to secure rosetta graph
    tf_graph_mapto_secure_graph = {}

    # class variable, map tf native src op to rosetta secure dest op
    tf_op_mapto_secure_op = {}

    # class variable, Optimized Conversion ops(TfToRtt\RttToTf) flag
    enable_opt_conv = True
    support_conv_op_def_name = ("TfToRtt", "RttToTf")


    def __init__(self, rtt_scope="rtt"):
        """
        Creates a new StaticReplacePass instance.

        :param rtt_scope: rtt secure namespace
        # rtt_scope: The new instances are automatically inserted into the given 'scope'.
        # If rtt_scope='', it is inserted into the graph's global namespace.However,
        # to avoid naming conflicts, its better to provide a scope.
        # If the instance(s) happens to be a part of collection(s), they are
        #are added to the appropriate collections in to_graph as well.
        #For example, for collection 'grad' which the instance happens to be a
        #part of, given a namespace 'rtt', the new instance will be a part of
        #'grad/rtt' in to_graph.
        """

        # tf op type name map to secure op info
        # secure op info = [secure op name, secure op creator, secure op, is support constant attribute]
        self.SECURE_DEF_NAME_IDX   = 0
        self.SECURE_CREATOR_IDX    = 1
        self.SECURE_OP_IDX         = 2
        self.SECURE_CONST_ATTR_IDX = 3
        self.secure_ops_infos = {
                            # arithmetic binary operation
                            "rttadd" :        ["SecureAdd",      self._create_secure_binary_op,  secure_ops.SecureAdd,    True],
                            "rttsub" :        ["SecureSub",      self._create_secure_binary_op,  secure_ops.SecureSub,    True],
                            "rttmul" :        ["SecureMul",      self._create_secure_binary_op,  secure_ops.SecureMul,    True],
                            "rttdiv" :        ["SecureRealDiv",  self._create_secure_binary_op,  secure_ops.SecureTruediv,True],
                            "rtttruediv" :    ["SecureTruediv",  self._create_secure_binary_op,  secure_ops.SecureTruediv,True],
                            "rttrealdiv" :    ["SecureRealDiv",  self._create_secure_binary_op,  secure_ops.SecureTruediv,True],
                            "rttfloordiv":    ["SecureFloorDiv", self._create_secure_binary_op,  secure_ops.SecureFloorDiv,True],
                            "rttpow" :        ["SecurePow",      self._create_secure_binary_op,  secure_ops.SecurePow,    True],
                            "rttmatmul" :     ["SecureMatMul",   self._create_secure_binary_op,  secure_ops.SecureMatMul, False],

                            # arithmetic unary operation
                            "rttnegative" :   ["SecureNegative", self._create_secure_unary_op,   secure_ops.SecureNeg,    False],
                            "rttsquare" :     ["SecureSquare",   self._create_secure_unary_op,   secure_ops.SecureSquare, False],
                            "rttlog" :        ["SecureLog",      self._create_secure_unary_op,   secure_ops.SecureLog,    False],
                            "rttlog1p" :      ["SecureLog1p",    self._create_secure_unary_op,   secure_ops.SecureLog1p,  False],
                            "rttsigmoid" :    ["SecureSigmoid",  self._create_secure_unary_op,   secure_ops.SecureSigmoid,False],
                            "rttrelu" :       ["SecureRelu",     self._create_secure_unary_op,   secure_ops.SecureRelu,   False],
                            "rttabs" :        ["SecureAbs",      self._create_secure_unary_op,   secure_ops.SecureAbs,    False],

                            # relational operation
                            "rttequal" :      ["SecureEqual",    self._create_secure_relational_op,      secure_ops.SecureEqual,      True],
                            "rttnotequal":    ["SecureNotEqual", self._create_secure_relational_op,      secure_ops.SecureNotEqual,   True],
                            "rttless" :       ["SecureLess",     self._create_secure_relational_op,      secure_ops.SecureLess,       True],
                            "rttgreater" :    ["SecureGreater",  self._create_secure_relational_op,      secure_ops.SecureGreater,    True],
                            "rttlessequal":   ["SecureLessEqual",self._create_secure_relational_op,      secure_ops.SecureLessEqual,  True],
                            "rttgreaterequal":["SecureGreaterEqual", self._create_secure_relational_op,  secure_ops.SecureGreaterEqual,True],

                            # logical operation
                            "rttlogicaland" : ["SecureLogicalAnd",self._create_secure_logical_op, secure_ops.SecureLogicalAnd, True],
                            "rttlogicalor" :  ["SecureLogicalOr", self._create_secure_logical_op, secure_ops.SecureLogicalOr,  True],
                            "rttlogicalxor" : ["SecureLogicalXor",self._create_secure_logical_op, secure_ops.SecureLogicalXor, True],
                            "rttlogicalnot" : ["SecureLogicalNot",self._create_secure_unary_op,   secure_ops.SecureLogicalNot, False],

                            # reduce operation
                            "rttreducemin" :  ["SecureReduceMin", self._create_secure_reduce_op,  secure_ops.SecureMin,   False],
                            "rttreducemax" :  ["SecureReduceMax", self._create_secure_reduce_op,  secure_ops.SecureMax,   False],
                            "rttreducemean":  ["SecureReduceMean",self._create_secure_reduce_op,  secure_ops.SecureMean,  False],
                            "rttreducesum" :  ["SecureReduceSum", self._create_secure_reduce_op,  secure_ops.SecureSum,   False],
                        }


        # source graph variable dict, save the train variables
        self.train_vars = {}

        # source graph all variables dict, contain trainable variables
        # & no-trainable global variables & model variables
        self.all_vars = {}

        # rtt secure namespace
        self.rtt_scope = rtt_scope

        # Adjust rtt namespace index
        if (StaticReplacePass.rtt_ns_idx != 0):
            self.rtt_scope = self.rtt_scope + str(StaticReplacePass.rtt_ns_idx)
        StaticReplacePass.rtt_ns_idx += 1

        # src graph op must be replaced secure op
        self.need_secure_op_name = []

        # save deepcopy op info using dict
        self.dc_op_info = {}

        # collect all variables, contain trainable variables
        # & no-trainable global variables & model variables
        self._collect_variables()


    def run(self, loss):
        """
        the static pass run function, the pass is deep copy or replace op based on the op data flow.

        :param loss: A `Tensor` containing the value to minimize.
        :return: the new loss tensor
        """

        # Judge the loss has be replace with secure graph already
        # reture the secure graph if the loss has be replace, 
        # otherwise we need to exec copy_and_replace_to_graph() function.
        if (loss in StaticReplacePass.tf_graph_mapto_secure_graph.keys()):
            secure_loss = StaticReplacePass.tf_graph_mapto_secure_graph[loss]
            MsgIdGenerator().gen_msgid_and_notified(secure_loss)
            return secure_loss

        # Get default graph
        to_graph = tf.get_default_graph()

        # Deep copy and replace source op
        secure_loss = self.copy_and_replace_to_graph(loss, to_graph)

        # Save the secure loss
        StaticReplacePass.tf_graph_mapto_secure_graph[loss] = secure_loss

        # Generate message id and notified to player
        MsgIdGenerator().gen_msgid_and_notified(secure_loss)

        return secure_loss


    def copy_vars_to_graph(self, src_var, to_graph,
                           rtt_scope, copied_vars={}):
        """
        Copies the Variable instance 'src_var' into the graph
        'to_graph', under the given rtt_scope.
        The dict 'copied_vars', if provided, will be updated with
        mapping the new variable's name to the instance.

        :param src_var: source variable instance will be copyed
        :param to_graph: dest graph instance
        :param rtt_scope: secure namespace
        :param copied_vars: Add the new variable to the dict
        :return: None
        """

        if not isinstance(src_var, tf.Variable):
            raise TypeError(str(src_var) + " is not a Variable")

        # The name of the new variable
        if rtt_scope != '':
            new_name = (rtt_scope + '/' + src_var.name[:src_var.name.index(':')])
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
                        rtt_scope == ''):
                    collections.append(name)
                else:
                    collections.append(rtt_scope + '/' + name)

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
            if self.rtt_scope != '':
                new_name = (self.rtt_scope + '/' + var.name)
            else:
                new_name = var.name
            self.all_vars[new_name] = var
            self.train_vars[new_name] = var

        # Get no-trainable global variable
        for var in tf.global_variables():
            if self.rtt_scope != '':
                new_name = (self.rtt_scope + '/' + var.name)
            else:
                new_name = var.name
            self.all_vars[new_name] = var

        #Get model varibale
        for var in tf.model_variables():
            if self.rtt_scope != '':
                new_name = (self.rtt_scope + '/' + var.name)
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
        if self.rtt_scope != '':
            new_name = self.rtt_scope + '/' + src_inst.name
        else:
            new_name = src_inst.name

        # If a variable by the new name already exists,
        # return the correct tensor that will act as an input.
        if new_name in self.all_vars:
            return to_graph.get_tensor_by_name(self.all_vars[new_name].name)

        # Get the collections that the new instance needs to be added to.
        # The new collections will also be a part of the given scope.
        collections = []
        for name, collection in src_inst.graph._collections.items():
            if src_inst in collection:
                if self.rtt_scope == '':
                    collections.append(name)
                else:
                    collections.append(self.rtt_scope + '/' + name)

        # Take action based on the class of the source instance
        if isinstance(src_inst, ops.Tensor):
            # If the tensor is placeholder tensor, can't copy the op,
            # return it immediately.
            if src_inst.op.op_def.name == "Placeholder":
                return src_inst

            #If the tensor is tftortt/rtttotf tensor, and the next op is placeholder, skip the tftortt/rtttotf op, 
            #and can't copy the placeholder op, return it immediately.
            if src_inst.op.op_def.name in self.support_conv_op_def_name:
                conv_op = src_inst.op
                while(True):
                    if conv_op.op_def.name in self.support_conv_op_def_name:
                        assert len(conv_op.inputs) == 1, "Conversion operation must have only 1 edge"
                        conv_op = conv_op.inputs[0].op
                    elif conv_op.op_def.name == "Placeholder":
                        return src_inst
                    else:
                        break

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
            # If the op by the new name has already exists in dc_op_info,
            # it mean the sub-graph branch has be already handle,
            # return the op immediately.
            if new_name in self.dc_op_info:
                return self.dc_op_info[new_name]

            # If it's a Operation,we deep copy the op while it's not secure op ,
            # Otherwise, we replace it with appropriate secure op.
            op = src_inst

            # if enable optimization, skip the conversion ops.
            if (self.enable_opt_conv):
                op = self.skip_conversion_ops(op)

            # If the source op has been replaced, return the replaced op directly.
            if (op in StaticReplacePass.tf_op_mapto_secure_op.keys()):
                return StaticReplacePass.tf_op_mapto_secure_op[op]

            # If the source op is IteratorGetNext, reuse the tf native op. 
            if op.op_def.name == "IteratorGetNext":
                StaticReplacePass.tf_op_mapto_secure_op[op] = op
                return op

            # Take action based on the op, if the op must secure op, we must
            # replace the op with secure op, otherwise, deep copy it.
            if self._is_need_secure_op(op) and self._is_support_secure_op(op):
                self.need_secure_op_name.append(new_name)
                dest_op = self.create_secure_op(op, to_graph)
            else:
                dest_op = self.deep_copy_op(op, to_graph)

            # save the dest op, then return
            StaticReplacePass.tf_op_mapto_secure_op[op] = dest_op
            return dest_op

        else:
            # If the instance is not Tensor/Operation, then show error
            raise TypeError("Could not deal with the instance: " + str(src_inst))


    def _is_need_secure_op(self, op):
        """
        Replace the op with secure op or deep copy the op based on the data flow.

        :param op: judge the op is secure op or not based on the data flow
        :return: True is secure op , otherwise is False
        """

        # The name of the new instance
        if self.rtt_scope != '':
            new_name = self.rtt_scope + '/' + op.name
        else:
            new_name = op.name

        # If a variable is trainable, so need secure op to replace the op
        if new_name in self.all_vars:
            if new_name in self.train_vars:
                return True
            else:
                return False

        # if the op name is in self.need_secure_op_name, 
        # so need secure op to replace the op
        if new_name in self.need_secure_op_name:
            return True

        # Take action based on the class of the source instance
        if isinstance(op, ops.Tensor):
            return self._is_need_secure_op(op.op)

        elif isinstance(op, ops.Operation):
            # If the op is placeholder, return true
            if (op.op_def.name == "Placeholder"):
                return True

            # If the op is IteratorV2, subgraph data flow analysis, 
            # check the subgraph(root op is MakeIterator) has need secure op?
            if (op.op_def.name == "IteratorV2"):
                for unit_op in tf.get_default_graph().get_operations():
                    if (unit_op.op_def.name == "MakeIterator"):
                        assert len(unit_op.inputs) == 2, "MakeIterator op inputs is incorrect."
                        if (unit_op.inputs[0].op == op):
                            return self._is_need_secure_op(unit_op.inputs[1])
                        elif (unit_op.inputs[1].op == op):
                            return self._is_need_secure_op(unit_op.inputs[0])
            
            # If the op is BatchDatasetV2, return true
            if (op.op_def.name == "BatchDatasetV2"):
                return True

            # If it has inputs, call this function recursively on each.
            inputs_secure_flag = [self._is_need_secure_op(x)
                              for x in op.inputs]
            if True in inputs_secure_flag:
                return True
            else:
                return False

        else:
            # If the op is not Tensor/Operation, return false.
            rtt_get_logger().error("Unkown the op: " + str(op))
            return False


    def is_exist_secure_op(self, op):
        """
        Does the data flow(subgraph) have one or more Secure node

        :param op: the node is one part of the subgraph
        :return: True mean exist secure node, otherwise is False
        """

        # Take action based on the class of the source instance
        if isinstance(op, ops.Tensor):
            return self.is_exist_secure_op(op.op)

        elif isinstance(op, ops.Operation):
            # get the op def name
            op_def_name = op.op_def.name

            # If the op def name is secure op name, return true
            secure_info_val = list(self.secure_ops_infos.values())
            for item in secure_info_val:
                if op_def_name in item:
                    return True

            # If it has inputs, call this function recursively on each input.
            inputs_secure_flag = [self.is_exist_secure_op(x)
                              for x in op.inputs]
            if True in inputs_secure_flag:
                return True
            else:
                return False

        else:
            # If the op is not Tensor/Operation, return false.
            _errmsg = "Unkown the op {} at is_exist_secure_op() function.".format(str(op))
            rtt_get_logger().error(_errmsg)
            return False


    def _is_support_secure_op(self, op):
        """
        Determine whether the operation supports Secure functionality

        :param op: judge the op support secure function
        :return: True is supported , otherwise is False
        """

        tf_op_def_name = op.op_def.name.lower()
        if tf_op_def_name in self.secure_ops_infos.keys():
            return True
        else:
            return False


    def create_secure_op(self, src_op, to_graph):
        """
        Create a secure op based on the source op, e.g.: Mul op => SecureMul op

        :param src_op: source op instance.
        :param to_graph: dest graph
        :return: secure op instance
        """
        
        # get new dest op define name and create functor with the dest secure op define name.
        try:
            secure_def_name = self.secure_ops_infos[src_op.op_def.name.lower()][self.SECURE_DEF_NAME_IDX]
            secure_create_func = self.secure_ops_infos[src_op.op_def.name.lower()][self.SECURE_CREATOR_IDX]
            secure_op = self.secure_ops_infos[src_op.op_def.name.lower()][self.SECURE_OP_IDX]
            assert secure_create_func != None, "secure creator is none."
            return secure_create_func(src_op, secure_def_name, secure_op, to_graph)  
        except KeyError:
            _errmsg = "tf native op {} does not implemented Secure op".format(src_op.op_def.name.lower())
            rtt_get_logger().error(_errmsg)


    def deep_copy_op(self, src_op, to_graph):
        """
        Deep copy a another op based on the source op, e.g.:Add op => Add op

        :param src_op: source op instance.
        :param to_graph: dest graph
        :return: the deep copy op instance
        """

        # The name of the new instance
        if self.rtt_scope != '':
            new_name = self.rtt_scope + '/' + src_op.name
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

        # If the new_name op has already exist, find it and return
        if (new_name in self.dc_op_info.keys()):
            return self.dc_op_info[new_name]

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

        # Make a copy of the op_def too.
        # Its unique to every _type_ of Operation.
        op_def = deepcopy(src_op.op_def)

        # Initialize a new Operation instance
        try:
            new_op = ops.Operation(new_node_def,
                                to_graph,
                                new_inputs,
                                output_types,
                                new_ctrl_inputs,
                                input_types,
                                new_original_op,
                                op_def)
        except Exception as e:
            rtt_get_logger().error(str(e))

        # Save the op info
        self.dc_op_info[new_name] = new_op

        return new_op


    def _create_secure_binary_op(self, src_op, secure_op_name, secure_op, to_graph):
        """
        create secure binary op(eg: secureadd\securesub\securemul\securediv...)

        :param src_op: source op instance, it's not secure op, it's tensorflow native op
        :param secure_op_name: secure op name.
        :param secure_op: secure op
        :param to_graph: dest graph
        :return: the secure op instance
        """
        secure_op_input_nums = 2
        return self._create_secure_op_helper(src_op, secure_op_name, 
                    secure_op_input_nums, secure_op, to_graph)

    
    def _create_secure_unary_op(self, src_op, secure_op_name, secure_op, to_graph):
        """
        create secure unary op(eg: secureneg\securesquare\securelog\securelog1p...)

        :param src_op: source op instance, it's not secure op, it's tensorflow native op
        :param secure_op_name: secure op name.
        :param secure_op: secure op
        :param to_graph: dest graph
        :return: the secure op instance
        """
        secure_op_input_nums = 1
        return self._create_secure_op_helper(src_op, secure_op_name, 
                    secure_op_input_nums, secure_op, to_graph)

    
    def _create_secure_relational_op(self, src_op, secure_op_name, secure_op, to_graph):
        """
        create secure relational op(eg: secureless\securegreater\securelessequal...)

        :param src_op: source op instance, it's not secure op, it's tensorflow native op
        :param secure_op_name: secure op name.
        :param secure_op: secure op
        :param to_graph: dest graph
        :return: the secure op instance
        """
        secure_op_input_nums = 2
        return self._create_secure_op_helper(src_op, secure_op_name, 
                    secure_op_input_nums, secure_op, to_graph)


    def _create_secure_logical_op(self, src_op, secure_op_name, secure_op, to_graph):
        """
        create secure logical op(eg: securelogicaland\securelogicalor\securelogicalnot...)

        :param src_op: source op instance, it's not secure op, it's tensorflow native op
        :param secure_op_name: secure op name.
        :param secure_op: secure op
        :param to_graph: dest graph
        :return: the secure op instance
        """
        secure_op_input_nums = 2
        return self._create_secure_op_helper(src_op, secure_op_name, 
                    secure_op_input_nums, secure_op, to_graph)

    
    def _create_secure_reduce_op(self, src_op, secure_op_name, secure_op, to_graph):
        """
        create secure reduce op(eg: securemin\securemax\securemean\securesum...)

        :param src_op: source op instance, it's not secure op, it's tensorflow native op
        :param secure_op_name: secure op name.
        :param secure_op: secure op
        :param to_graph: dest graph
        :return: the secure op instance
        """
        secure_op_input_nums = 2
        return self._create_secure_op_helper(src_op, secure_op_name, 
                    secure_op_input_nums, secure_op, to_graph)


    def _create_secure_logic_binary_op(self, src_op, secure_op_name, secure_op, to_graph):
        """
        create secure relational op(eg: secureand\secureor\securexor...)

        :param src_op: source op instance, it's not secure op, it's tensorflow native op
        :param secure_op_name: secure op name.
        :param secure_op: secure op
        :param to_graph: dest graph
        :return: the secure op instance
        """
        secure_op_input_nums = 2
        return self._create_secure_op_helper(src_op, secure_op_name, 
                    secure_op_input_nums, secure_op, to_graph)


    def _create_secure_logic_unary_op(self, src_op, secure_op_name, secure_op, to_graph):
        """
        create secure relational op(eg: securenot...)

        :param src_op: source op instance, it's not secure op, it's tensorflow native op
        :param secure_op_name: secure op name.
        :param secure_op: secure op
        :param to_graph: dest graph
        :return: the secure op instance
        """
        secure_op_input_nums = 1
        return self._create_secure_op_helper(src_op, secure_op_name, 
                    secure_op_input_nums, secure_op, to_graph)


    def _create_secure_op_helper(self, src_op, secure_op_name, secure_op_input_num,
                            secure_op_creator, to_graph):
        """
        create secure op helper function

        :param src_op: source op instance, it's not secure op
        :param secure_op_name: secure op name, don't contain namespace
        :param secure_op_input_num: secure op input numbers
        :param secure_op_creator: secure op creator
        :param to_graph: dest graph
        :return: the secure op instance
        """

        # The name of the new instance
        if self.rtt_scope != '':
            new_secure_name = self.rtt_scope + '/' + secure_op_name
            new_src_name = self.rtt_scope + '/' + src_op.name
        else:
            new_secure_name = secure_op_name
            new_src_name = src_op.name

        # If it has an original_op parameter, copy it
        # the original_op must be none.
        if src_op._original_op is not None:
            raise NotImplementedError("not supported")

        # If it has control inputs, call this function recursively on each.
        new_control_inputs = [self.copy_and_replace_to_graph(x, to_graph)
                              for x in src_op.control_inputs]
        assert  len(new_control_inputs) == 0, "%s don't have control input edges" %secure_op_name

        # If it has inputs, call this function recursively on each.
        new_inputs = [self.copy_and_replace_to_graph(x, to_graph)
                      for x in src_op.inputs]
        assert  len(new_inputs) == secure_op_input_num, "{0} need {1} edges, but real edges is {2}".format(secure_op_name, 
                                                     secure_op_input_num, len(new_inputs))

        # checked secure op inputs
        self._checked_secure_op_inputs(secure_op_name, new_inputs)

        # create secure op
        try:
            if secure_op_input_num == 1:
                new_op = secure_op_creator(new_inputs[0], name=new_secure_name).op
            elif secure_op_input_num == 2:
                if self._is_bin_op_unsupport_const_attr(src_op.op_def.name):
                    new_op = self._create_unsupport_const_attr_secure_bin_op(src_op, secure_op_creator, new_inputs, new_secure_name)
                else:
                    new_op = secure_op_creator(new_inputs[0], new_inputs[1], name=new_secure_name,
                                        lh_is_const=self._secure_input_is_const(new_inputs[0]), 
                                        rh_is_const=self._secure_input_is_const(new_inputs[1])).op
            elif secure_op_input_num == 3:
                new_op = secure_op_creator(new_inputs[0], new_inputs[1], 
                                        new_inputs[2], name=new_secure_name).op
            elif secure_op_input_num == 4:
                new_op = secure_op_creator(new_inputs[0], new_inputs[1], new_inputs[2], 
                                        new_inputs[3], name=new_secure_name).op
            elif secure_op_input_num == 5:
                new_op = secure_op_creator(new_inputs[0], new_inputs[1], new_inputs[2], 
                                        new_inputs[3], new_inputs[4], name=new_secure_name).op
            elif secure_op_input_num == 6:
                new_op = secure_op_creator(new_inputs[0], new_inputs[1], new_inputs[2], 
                                        new_inputs[3], new_inputs[4], new_inputs[5], name=new_secure_name).op
            else:
                raise ValueError("the %s op don't have %d inputs" %secure_op_name %secure_op_input_num)
        except Exception as e:
            rtt_get_logger().error(str(e))

        # Save the op info
        self.dc_op_info[new_src_name] = new_op

        # Return the op
        return new_op


    def _checked_secure_op_inputs(self, secure_op_name, secure_op_inputs):
        """
        checked secure op inputs function

        :param secure_op_name: secure op name, don't contain namespace
        :param secure_op_inputs: secure op inputs
        :return: None
        """

        # Check the secure pow parameters are correct
        if (secure_op_name.lower() == "securepow"):
            if (not self._secure_input_is_const(secure_op_inputs[0])) and (not self._secure_input_is_const(secure_op_inputs[1])):
                raise NotImplementedError("only support SecurePow(var * const), SecurePow(var * var) not support")
        pass


    def _secure_input_is_const(self, secure_op_input):
        """
        get the secure input type function

        :param secure_op_input: secure op input
        :return: True mean const, False mean non-const
        """
        
        assert secure_op_input != None, "error:secure cop input is none"
        if self._is_need_secure_op(secure_op_input.op):
            return False
        else:
            return True


    def _is_bin_op_unsupport_const_attr(self, op_def_name):
        """
        check the binary op is support lr/hr_is_const attrubute

        :param op_def_name: op define name
        :return: True mean unsupport lr/hr_is_const attrubute, False mean support
        """

        # get rtt op info value
        info_val = self.secure_ops_infos[op_def_name.lower()]
        return not info_val[self.SECURE_CONST_ATTR_IDX]


    def _create_secure_matmul_op_with_attr(self, tf_op, secure_op, inputs, secure_name):
        """
        create SecureMatMul op with attrs

        :param tf_op: the origin tensorflow operation
        :param secure_op: secure op mapping to c++ layer secure op
        :param inputs: the input of the operation
        :param op_def_name: the origin name of tensorflow operation
        :param secure_name: name of Secure operation, eg. rtt/SecureMatMul

        :return: the new created Operation
        """
        
        attrs_map = {}
        attrs_map['transpose_a'] = tf_op.get_attr("transpose_a")
        attrs_map['transpose_b'] = tf_op.get_attr("transpose_b")
        
        return secure_op(inputs[0], inputs[1], transpose_a=attrs_map['transpose_a'], transpose_b=attrs_map['transpose_b'], name=secure_name).op


    def _create_unsupport_const_attr_secure_bin_op(self, tf_op, secure_op, inputs, secure_name):
        """
        create secure operation which is not supports const attribute

        :param tf_op: the origin tensorflow operation
        :param secure_op: secure op mapping to c++ layer secure op
        :param inputs: the input of the operation
        :param op_def_name: the origin name of tensorflow operation
        :param secure_name: name of Secure operation, eg. rtt/SecureMatMul

        :return: the new created Operation
        """
        op_def_name = tf_op.op_def.name
        op_name = op_def_name.lower()
        new_op = None
        if op_name == 'rttmatmul':
            new_op = self._create_secure_matmul_op_with_attr(tf_op, secure_op, inputs, secure_name)
        else:
            new_op = secure_op(inputs[0], inputs[1], name=secure_name).op
        
        return new_op


    def skip_conversion_ops(self, src_op):
        """
        Skip conversions ops(TfToRtt\RttToTf)

        :param src_op: source graph op
        :return: return to Optimized Op
        """
        dest_op = src_op
        assert isinstance(dest_op, ops.Operation), "Paramete type must be operation"

        while(True):
            if dest_op.op_def.name in self.support_conv_op_def_name:
                assert len(dest_op.inputs) == 1, "Conversion operation must have only 1 edge"
                dest_op = dest_op.inputs[0].op
            else:
                break

        return dest_op



def replace_tf_subgraph_with_secure_subgraph(fetch):
    """Replace tf subgraph with secure subgraph.

      fetch: A tensor from run param
      return: return secure fetch tensor

    Attention: It is not reasonable to check whether this is a gradient graph, 
    but the real usage scenario will not be a problem
    """

    def is_gradients_graph(grad_scope):
        if grad_scope.startswith('gradients/') or re.match("gradients_[0-9]+/", grad_scope) != None:
            return True
        return False

    if isinstance(fetch, ops.Tensor):
        # Check the subgraph exist secure op,  
        # if the subgraph has exist secure op, so we known the subgraph has be replaced, do nothing;
        # otherwise we must use StaticReplacePass obj to replace the tf static graph to secure static graph
        
        # Create StaticReplacePass object
        PassObj = StaticReplacePass()

        if not PassObj.is_exist_secure_op(fetch.op) and not is_gradients_graph(fetch.name):
            # run static pass
            fetch = PassObj.run(fetch)

    return fetch
    