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
from latticex.rosetta.rtt.framework import rtt_tensor
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
        # secure op info = [secure op name, secure op creator with build-in attribute, secure op creator without build-in attribute, secure op input numbers, is support constant attribute]
        self.SECURE_DEF_NAME_IDX                = 0
        self.SECURE_OP_CREATOR_WITH_ATTR_IDX    = 1
        self.SECURE_OP_CREATOR_WITHOUT_ATTR_IDX = 2
        self.SECURE_OP_INPUTS_IDX               = 3
        self.SECURE_CONST_ATTR_IDX              = 4
        self.secure_ops_infos = {
            # arithmetic binary operation
            "rttadd" :        ["SecureAdd",                         None,                   secure_ops.SecureAdd,           2,  True],
            "rttsub" :        ["SecureSub",                         None,                   secure_ops.SecureSub,           2,  True],
            "rttmul" :        ["SecureMul",                         None,                   secure_ops.SecureMul,           2,  True],
            "rttdiv" :        ["SecureRealDiv",                     None,                   secure_ops.SecureTruediv,       2,  True],
            "rttreciprocaldiv" : ["SecureReciprocaldiv",            None,                   secure_ops.SecureTruediv,       2,  True],
            "rtttruediv" :    ["SecureTruediv",                     None,                   secure_ops.SecureTruediv,       2,  True],
            "rttrealdiv" :    ["SecureRealDiv",                     None,                   secure_ops.SecureTruediv,       2,  True],
            "rttfloordiv":    ["SecureFloorDiv",                    None,                   secure_ops.SecureFloorDiv,      2,  True],
            "rttpow" :        ["SecurePow",                         None,                   secure_ops.SecurePow,           2,  True],
            "rttmatmul" :     ["SecureMatMul",   self._create_secure_matmul_op_with_attr,   secure_ops.SecureMatMul,        2,  False],

            # arithmetic unary operation
            "rttnegative" :   ["SecureNegative",                    None,                   secure_ops.SecureNeg,           1,  False],
            "rttsquare" :     ["SecureSquare",                      None,                   secure_ops.SecureSquare,        1,  False],
            "rttlog" :        ["SecureLog",                         None,                   secure_ops.SecureLog,           1,  False],
            "rttlog1p" :      ["SecureLog1p",                       None,                   secure_ops.SecureLog1p,         1,  False],
            "rttabs" :        ["SecureAbs",                         None,                   secure_ops.SecureAbs,           1,  False],

            "rttrsqrt" :     ["SecureRsqrt",                        None,                   secure_ops.SecureRsqrt,         1,  False],
            "rttsqrt" :      ["SecureSqrt",                         None,                   secure_ops.SecureSqrt,          1,  False],
            "rttexp" :       ["SecureExp",                          None,                   secure_ops.SecureExp,           1,  False],

            # relational operation
            "rttequal" :      ["SecureEqual",                       None,                   secure_ops.SecureEqual,         2,  True],
            "rttnotequal":    ["SecureNotEqual",                    None,                   secure_ops.SecureNotEqual,      2,  True],
            "rttless" :       ["SecureLess",                        None,                   secure_ops.SecureLess,          2,  True],
            "rttgreater" :    ["SecureGreater",                     None,                   secure_ops.SecureGreater,       2,  True],
            "rttlessequal":   ["SecureLessEqual",                   None,                   secure_ops.SecureLessEqual,     2,  True],
            "rttgreaterequal":["SecureGreaterEqual",                None,                   secure_ops.SecureGreaterEqual,  2,  True],

            # logical operation
            "rttlogicaland" : ["SecureLogicalAnd",                  None,                   secure_ops.SecureLogicalAnd,    2,  True],
            "rttlogicalor" :  ["SecureLogicalOr",                   None,                   secure_ops.SecureLogicalOr,     2,  True],
            "rttlogicalxor" : ["SecureLogicalXor",                  None,                   secure_ops.SecureLogicalXor,    2,  True],
            "rttlogicalnot" : ["SecureLogicalNot",                  None,                   secure_ops.SecureLogicalNot,    1,  False],

            # reduce operation
            "rttreducemin" :  ["SecureReduceMin", self._create_secure_reduce_op_with_attr,  secure_ops.SecureMin,           2,  False],
            "rttreducemax" :  ["SecureReduceMax", self._create_secure_reduce_op_with_attr,  secure_ops.SecureMax,           2,  False],
            "rttreducemean":  ["SecureReduceMean",self._create_secure_reduce_op_with_attr,  secure_ops.SecureMean,          2,  False],
            "rttreducesum" :  ["SecureReduceSum", self._create_secure_reduce_op_with_attr,  secure_ops.SecureSum,           2,  False],
            "rttargmax" :     ["SecureArgMax",                      None,                   secure_ops.SecureArgMax,        2,  False],

            # state operation
            "rttassignsub" :  ["SecureAssignSub",                   None,                   secure_ops.SecureAssignSub,     2,  False],

            # nn operation
            "rttsigmoid" :    ["SecureSigmoid",                     None,                   secure_ops.SecureSigmoid,       1,  False],
            "rttrelu" :       ["SecureRelu",                        None,                   secure_ops.SecureRelu,          1,  False],
            "rttbiasadd" :    ["SecureBiasAdd",                     None,                   secure_ops.SecureBiasAdd,       2,  False],
            "rttconv2d" :     ["SecureConv2D",   self._create_secure_conv2d_op_with_attr,   secure_ops.SecureConv2D,        2,  False],
            "rttl2loss" :     ["SecureL2Loss",                      None,                   secure_ops.SecureL2Loss,        1,  False],
            "rttfusedbatchnorm" : ["SecureFusedBatchNorm", self._create_secure_fusedbatchnorm_op_with_attr, secure_ops.SecureFusedBatchNorm, 5,  False],
            "rttavgpool" :    ["SecureAvgPool",  self._create_secure_avgpool_op_with_attr,  secure_ops.SecureAvgPool,       1,  False],
            "rttmaxpool" :    ["SecureMaxPool",  self._create_secure_maxpool_op_with_attr,  secure_ops.SecureMaxPool,       1,  False],
            "rttsoftmax" :    ["SecureSoftmax",                     None,                   secure_ops.SecureSoftmax,       1,  False],
        }


        # source graph variable dict, save the train variables
        self.train_vars = {}

        # source graph all variables dict, contain trainable variables
        # & no-trainable global variables & model variables
        self.all_vars = {}

        # rtt secure namespace
        self.rtt_scope = rtt_scope

        # save deepcopy op info using dict
        self.dc_op_info = {}

        # collect all variables, contain trainable variables
        # & no-trainable global variables & model variables
        self._collect_variables()

        # If the dataflow passes through any of these nodes(rtt/secure ops), 
        # then the tf/rtt native op need to replace with rtt secure op.
        self.need_secure_op_sets = []


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
        to_graph = tf.compat.v1.get_default_graph()

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
        for var in tf.compat.v1.trainable_variables():
            if self.rtt_scope != '':
                new_name = (self.rtt_scope + '/' + var.name)
            else:
                new_name = var.name
            self.all_vars[new_name] = var
            self.train_vars[new_name] = var

        # Get no-trainable global variable
        for var in tf.compat.v1.global_variables():
            if self.rtt_scope != '':
                new_name = (self.rtt_scope + '/' + var.name)
            else:
                new_name = var.name
            self.all_vars[new_name] = var

        #Get model varibale
        for var in tf.compat.v1.model_variables():
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
            if (len(collection) > 0):
                if isinstance(src_inst, ops.Operation) and isinstance(collection[0], rtt_tensor.RttTensor):
                    continue

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
                while(not conv_op.control_inputs):
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
            if self._is_support_secure_op(op) and self._is_need_secure_op(op):
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

        # If the op in need_secure_op_sets, return true.
        if op in self.need_secure_op_sets:
            return True

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

        # Take action based on the class of the source instance
        if isinstance(op, ops.Tensor):
            return self._is_need_secure_op(op.op)

        elif isinstance(op, ops.Operation):
            # Save the op to need_secure_op_sets
            if (op not in self.need_secure_op_sets):
                self.need_secure_op_sets.append(op)

            # If the op is placeholder, return true
            if (op.op_def.name == "Placeholder"):
                return True

            # If the op is IteratorV2, subgraph data flow analysis, 
            # check the subgraph(root op is MakeIterator) has need secure op?
            if (op.op_def.name == "IteratorV2"):
                for unit_op in tf.compat.v1.get_default_graph().get_operations():
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
            for x in op.inputs:
                the_input_secure_flag = self._is_need_secure_op(x)
                if (the_input_secure_flag):
                    return True
            
            # all the input edges don't need secure op, so remove the op from need_secure_op_sets,
            # and then return false
            self.need_secure_op_sets.remove(op)
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
            secure_op_creator_with_attr = self.secure_ops_infos[src_op.op_def.name.lower()][self.SECURE_OP_CREATOR_WITH_ATTR_IDX]
            secure_op_creator_without_attr = self.secure_ops_infos[src_op.op_def.name.lower()][self.SECURE_OP_CREATOR_WITHOUT_ATTR_IDX]
            secure_op_inputs = self.secure_ops_infos[src_op.op_def.name.lower()][self.SECURE_OP_INPUTS_IDX]

            return self._create_secure_op_helper(src_op, secure_def_name, secure_op_creator_with_attr,
                                                secure_op_creator_without_attr, secure_op_inputs, to_graph)
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


    def _create_secure_op_helper(self, src_op, secure_op_name, secure_op_creator_with_attr,
                                secure_op_creator_without_attr, secure_op_input_num, to_graph):
        """
        create secure op helper function

        :param src_op: source op instance, it's not secure op
        :param secure_op_name: secure op name, don't contain namespace
        :param secure_op_creator_with_attr: secure op creator with build-in attribute, eg: SecureMatmul's transpose_a and transpose_b attributes
        :param secure_op_creator_without_attr: secure op creator without build-in attribute, eg:SecureMul
        :param secure_op_input_num: secure op input numbers
        :param to_graph: dest graph
        :return: the secure op instance
        """

        # The name of the new instance
        if self.rtt_scope != '':
            new_secure_name = self.rtt_scope + '/' + secure_op_name + str(src_op._id)
            new_src_name = self.rtt_scope + '/' + src_op.name
        else:
            new_secure_name = secure_op_name + str(src_op._id)
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

        # create a secure op
        try:
            if self._is_op_support_const_attr(src_op.op_def.name):
                new_op = self._create_secure_op_with_const_attr(src_op, secure_op_creator_with_attr, 
                                                                secure_op_creator_without_attr, new_inputs,
                                                                secure_op_input_num, new_secure_name)
            else:
                new_op = self._create_secure_op_without_const_attr(src_op, secure_op_creator_with_attr, 
                                                                secure_op_creator_without_attr, new_inputs,
                                                                secure_op_input_num, new_secure_name)
        except Exception as e:
            rtt_get_logger().error(str(e))

        # Save the op info and return the op
        self.dc_op_info[new_src_name] = new_op

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


    def _is_op_support_const_attr(self, op_def_name):
        """
        Check if the op supports lr/hr_is_const attributes

        :param op_def_name: op define name
        :return: True mean support lr/hr_is_const attrubute, False mean not support
        """

        # get rtt op info value
        info_val = self.secure_ops_infos[op_def_name.lower()]
        return info_val[self.SECURE_CONST_ATTR_IDX]


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


    def _create_secure_conv2d_op_with_attr(self, tf_op, secure_op, inputs, secure_name):
        """
        create SecureConv2D op with attrs

        :param tf_op: the origin tensorflow operation
        :param secure_op: secure op mapping to c++ layer secure op
        :param inputs: the input of the operation
        :param op_def_name: the origin name of tensorflow operation
        :param secure_name: name of Secure operation, eg. rtt/SecureConv2D

        :return: the new created Operation
        """
        
        attrs_map = {}
        attrs_map['strides'] = tf_op.get_attr("strides")
        attrs_map['padding'] = tf_op.get_attr("padding")
        attrs_map['explicit_paddings'] = tf_op.get_attr("explicit_paddings")
        attrs_map['data_format'] = tf_op.get_attr("data_format")
        
        return secure_op(inputs[0], inputs[1], strides=attrs_map['strides'], padding=attrs_map['padding'], 
                        explicit_paddings=attrs_map['explicit_paddings'], data_format=attrs_map['data_format'], name=secure_name).op


    def _create_secure_fusedbatchnorm_op_with_attr(self, tf_op, secure_op, inputs, secure_name):
        """
        create SecureFusedBatchNorm op with attrs

        :param tf_op: the origin tensorflow operation
        :param secure_op: secure op mapping to c++ layer secure op
        :param inputs: the input of the operation
        :param op_def_name: the origin name of tensorflow operation
        :param secure_name: name of Secure operation, eg. SecureFusedBatchNorm

        :return: the new created Operation
        """
        
        attrs_map = {}
        attrs_map['epsilon'] = tf_op.get_attr("epsilon")
        attrs_map['is_training'] = tf_op.get_attr("is_training")
        
        return secure_op(inputs[0], inputs[1], inputs[2], inputs[3], inputs[4], 
                        epsilon=attrs_map['epsilon'], is_training=attrs_map['is_training'], name=secure_name).op


    def _create_secure_maxpool_op_with_attr(self, tf_op, secure_op, inputs, secure_name):
        """
        create SecureMaxPool op with attrs

        :param tf_op: the origin tensorflow operation
        :param secure_op: secure op mapping to c++ layer secure op
        :param inputs: the input of the operation
        :param op_def_name: the origin name of tensorflow operation
        :param secure_name: name of Secure operation, eg. SecureMaxPool

        :return: the new created Operation
        """
        
        attrs_map = {}
        attrs_map['ksize'] = tf_op.get_attr("ksize")
        attrs_map['strides'] = tf_op.get_attr("strides")
        attrs_map['padding'] = tf_op.get_attr("padding")
        attrs_map['data_format'] = tf_op.get_attr("data_format")
        
        return secure_op(inputs[0], ksize=attrs_map['ksize'], strides=attrs_map['strides'], 
                        padding=attrs_map['padding'], data_format=attrs_map['data_format'], name=secure_name).op


    def _create_secure_avgpool_op_with_attr(self, tf_op, secure_op, inputs, secure_name):
        """
        create SecureAvgPool op with attrs

        :param tf_op: the origin tensorflow operation
        :param secure_op: secure op mapping to c++ layer secure op
        :param inputs: the input of the operation
        :param op_def_name: the origin name of tensorflow operation
        :param secure_name: name of Secure operation, eg. SecureAvgPool

        :return: the new created Operation
        """
        
        attrs_map = {}
        attrs_map['ksize'] = tf_op.get_attr("ksize")
        attrs_map['strides'] = tf_op.get_attr("strides")
        attrs_map['padding'] = tf_op.get_attr("padding")
        attrs_map['data_format'] = tf_op.get_attr("data_format")
        
        return secure_op(inputs[0], ksize=attrs_map['ksize'], strides=attrs_map['strides'], 
                        padding=attrs_map['padding'], data_format=attrs_map['data_format'], name=secure_name).op

    
    def _create_secure_reduce_op_with_attr(self, tf_op, secure_op, inputs, secure_name):
        """
        create SecureReduce(sum/mean/min/max) op with attrs

        :param tf_op: the origin tensorflow operation
        :param secure_op: secure op mapping to c++ layer secure op
        :param inputs: the input of the operation
        :param op_def_name: the origin name of tensorflow operation
        :param secure_name: name of Secure operation, eg. SecureAvgPool

        :return: the new created Operation
        """
        attrs_map = {}
        attrs_map['keep_dims'] = tf_op.get_attr("keep_dims")
        return secure_op(inputs[0], inputs[1], keep_dims=attrs_map['keep_dims'], name=secure_name).op


    def _create_secure_op_without_const_attr(self, tf_op, secure_op_creator_with_attr, 
                                    secure_op_creator_without_attr, inputs, input_nums, secure_name):
        """
        create an secure operation which is not supports const attribute
        (Create an op without the const attribute)

        :param tf_op: the origin tensorflow operation
        :param secure_op_creator_with_attr: secure op creator with build-in attribute, eg: SecureMatmul's transpose_a and transpose_b attributes
        :param secure_op_creator_without_attr: secure op creator without build-in attribute, eg:SecureMul
        :param inputs: the input of the operation
        :param input_nums: the input numbers of the operation
        :param secure_name: name of Secure operation, eg. rtt/SecureMatMul

        :return: the new secure Operation
        """
        op_def_name = tf_op.op_def.name
        op_name = op_def_name.lower()
        new_op = None

        if secure_op_creator_with_attr != None:
            new_op = secure_op_creator_with_attr(tf_op, secure_op_creator_without_attr, inputs, secure_name)
        else:
            if (input_nums == 1):
                new_op = secure_op_creator_without_attr(inputs[0], name=secure_name).op
            elif (input_nums == 2):
                new_op = secure_op_creator_without_attr(inputs[0], inputs[1], name=secure_name).op
            elif (input_nums == 3):
                new_op = secure_op_creator_without_attr(inputs[0], inputs[1], inputs[2], name=secure_name).op
            elif (input_nums == 4):
                new_op = secure_op_creator_without_attr(inputs[0], inputs[1], inputs[2], inputs[3], name=secure_name).op
            elif (input_nums == 5):
                new_op = secure_op_creator_without_attr(inputs[0], inputs[1], inputs[2], inputs[3], inputs[4], name=secure_name).op
            elif (input_nums == 6):
                new_op = secure_op_creator_without_attr(inputs[0], inputs[1], inputs[2], inputs[3], inputs[4], inputs[5], name=secure_name).op
            else:
                raise ValueError("the %s op have %d input edges, and is not currently supported." %secure_name %input_nums)
        
        return new_op

    
    def _create_secure_op_with_const_attr(self, tf_op, secure_op_creator_with_attr, 
                                    secure_op_creator_without_attr, inputs, input_nums, secure_name):
        """
        Create an op with the const attribute

        :param tf_op: the origin tensorflow operation
        :param secure_op_creator_with_attr: secure op creator with build-in attribute, eg: SecureMatmul's transpose_a and transpose_b attributes
        :param secure_op_creator_without_attr: secure op creator without build-in attribute, eg:SecureMul
        :param inputs: the input of the operation
        :param input_nums: tthe input numbers of the operation
        :param secure_name: name of Secure operation, eg. rtt/SecureMatMul

        :return: the new secure Operation
        """
        if (input_nums == 2):
            new_op = secure_op_creator_without_attr(inputs[0], inputs[1], name=secure_name,
                            lh_is_const=self._secure_input_is_const(inputs[0]),
                            rh_is_const=self._secure_input_is_const(inputs[1])).op
        else:
            raise ValueError("Only binary secure operation has support const attribute.")
        
        return new_op


    def skip_conversion_ops(self, src_op):
        """
        Skip conversions ops(TfToRtt\RttToTf)

        :param src_op: source graph op
        :return: return to Optimized Op
        """
        dest_op = src_op
        assert isinstance(dest_op, ops.Operation), "Paramete type must be operation"

        while(not dest_op.control_inputs):
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

        if not is_gradients_graph(fetch.name):
            # run static pass
            fetch = PassObj.run(fetch)

    return fetch

