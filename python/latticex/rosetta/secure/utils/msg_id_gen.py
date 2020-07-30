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
from latticex.rosetta.controller.controller_base_ import _rtt
from latticex.rosetta.controller.common_util import rtt_get_logger


class MsgIdGenerator():
    """
    generate message id for rosetta graph
    message id format:
    "rosetta op name" +  "\t" + message id index  + "\n"
    "rosetta op name2" + "\t" + message id index2 + "\n"
    
    For example:
    a = tf.Variable(...)
    b = tf.Variable(...)
    c = a * b

    generate the message id:(index start with 0)
    "Mpc/MpcMul 0 \n"
    """

    # class variable, map rosetta graph to message id string
    rtt_graph_mapto_msgid = {}


    def __init__(self, regenerate=False):
        """
        Create MsgIdGenerator.
        """
        self.regen = regenerate
        pass


    def _is_privacy_op_name(self, op_name):
        """
        Check the operation name is private operation name

        :param op_name: operation name(contain namespaces)
        return: True = private op name, otherwise is native op name
        """
        pure_op_name = op_name.lower()
        pos = pure_op_name.rfind("/")
        if (pos != -1):
            pure_op_name = pure_op_name[pos + 1:]

        return pure_op_name.startswith("secure")


    def _generate(self, privacy_tensor):
        """
        Generate message id

        :param privacy_tensor: privacy tensor, eg:mpc tensor or he tensor...
        return: message id string
        """
        if (not self.regen):
            # if it has already generated, then return the message id string
            if privacy_tensor in self.rtt_graph_mapto_msgid.keys():
                return self.rtt_graph_mapto_msgid[privacy_tensor]

        # generate the message id
        idx = 0
        current_msg_id = ""
        for rtt_op in tf.get_default_graph().get_operations():
            if (self._is_privacy_op_name(rtt_op.name)):
                current_msg_id += "{0}\t{1}\n".format(rtt_op.name, idx)
                idx += 1

        # save the message id info
        if current_msg_id:
            self.rtt_graph_mapto_msgid[privacy_tensor] = current_msg_id

        # return the message id info
        return current_msg_id


    def gen_msgid_and_notified(self, loss):
        """
        generate the rosetta message id, and notified message id to player
        """
        msg_id = self._generate(loss)
        rtt_get_logger().debug(msg_id)
        py_msgid_handler = _rtt.msgid_handle.MsgIdHandle()
        py_msgid_handler.update_message_id_info(msg_id)

