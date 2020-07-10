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
from tensorflow.python.ops import nn
from latticex.rosetta.secure.decorator import SecureSigmoidCrossEntropy


def secure_sigmoid_cross_entropy_with_logits( # pylint: disable=invalid-name
                                    _sentinel=None,
                                    labels=None,
                                    logits=None,
                                    name=None):
    #tf.nn_ops._ensure_xent_args("sigmoid_cross_entropy_with_logits", _sentinel,
    #                       labels, logits)
    if _sentinel is not None:
        raise ValueError("Only call `%s` with "
                     "named arguments (labels=..., logits=..., ...)" % name)
    if labels is None or logits is None:
        raise ValueError("Both labels and logits must be provided.")
    
    # pylint: enable=protected-access
    #with tf.ops.name_scope(name, "logistic_loss", [logits, labels]) as name:
    with tf.name_scope(name, "logistic_loss", [logits, labels]) as name:
        logits = tf.convert_to_tensor(logits, name="logits")
        labels = tf.convert_to_tensor(labels, name="labels")
        try:
            labels.get_shape().merge_with(logits.get_shape())
        except ValueError:
            raise ValueError("logits and labels must have the same shape (%s vs %s)" %
                    (logits.get_shape(), labels.get_shape()))
        result = SecureSigmoidCrossEntropy(logits=logits, labels=labels, name=name)
        return result

nn.sigmoid_cross_entropy_with_logits = secure_sigmoid_cross_entropy_with_logits 
## Note: this is static direct REPLACEMENT after import this package!
tf.nn.sigmoid_cross_entropy_with_logits = secure_sigmoid_cross_entropy_with_logits 

def secure_sigmoid_cross_entropy_with_logits_v2(  # pylint: disable=invalid-name
                                    labels=None,
                                    logits=None,
                                    name=None):
    return secure_sigmoid_cross_entropy_with_logits(logits=logits,
                                            labels=labels, name=name)

nn.sigmoid_cross_entropy_with_logits_v2= secure_sigmoid_cross_entropy_with_logits_v2
